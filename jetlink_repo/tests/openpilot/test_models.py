"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

Which large model runs and where its bytes come from: the model manager's
catalog and pick as the fork's adapter hands them over, the pointer at the
commit, the file on disk, and the newer catalogs folded in.
"""
import dataclasses
import unittest
import urllib.request
from unittest import mock

from jetlink.openpilot import models
from tests.openpilot.fakes import OpenpilotTest


def bundle(ref: str, name: str, index: int = 0, version=19) -> dict:
  """A bundle as the catalog JSON carries it."""
  return {'ref': ref, 'display_name': name, 'index': index, 'minimum_selector_version': str(version)}


REF_A, REF_B, REF_C = 'a' * 40, 'b' * 40, 'c' * 40
POINTERS = {REF_A: {'oid': '1' * 64, 'size': 766_000_000},
            REF_B: {'oid': '2' * 64, 'size': 1_757_000_000}}


class ModelsTest(OpenpilotTest):
  def setUp(self):
    super().setUp()
    self.models = self.parts.models

  def catalog(self, *bundles):
    self.op.store['ModelManager_ModelsCache_Chestnut'] = {'bundles': list(bundles)}


class TestCatalog(ModelsTest):
  """The list is sunnypilot's big-model catalog, read as the model manager cached it."""

  def test_newest_first_with_names(self):
    self.catalog(bundle(REF_A, 'Alpha (September 04, 2026)', 3), bundle(REF_B, 'Beta', 9))
    self.assertEqual(self.models.catalog(), [{'name': 'Beta', 'ref': REF_B}, {'name': 'Alpha (September 04, 2026)', 'ref': REF_A}])

  def test_only_commits_of_this_selector_version(self):
    # a bundle without a comma commit has no ONNX to find; one for another
    # selector version is one the model manager itself would not list
    self.catalog(bundle('not-a-commit', 'Odd', 5), bundle(REF_A, 'Alpha', 1), {'display_name': 'Blank'},
                 bundle(REF_C, 'Gamma', 7, version=18))
    self.assertEqual([b['ref'] for b in self.models.catalog()], [REF_A])

  def test_the_selector_is_the_adapters(self):
    self.op.catalog_selector = 18
    self.catalog(bundle(REF_A, 'Alpha', 1), bundle(REF_C, 'Gamma', 7, version=18))
    self.assertEqual([b['ref'] for b in self.models.catalog()], [REF_C])

  def test_a_nameless_bundle_is_named_by_its_ref(self):
    self.catalog({'ref': REF_A, 'index': 1, 'minimum_selector_version': 19})
    self.assertEqual(self.models.catalog(), [{'name': REF_A[:10], 'ref': REF_A}])

  def test_no_catalog_yet_is_empty(self):
    self.assertEqual(self.models.catalog(), [])
    self.catalog()
    self.assertEqual(self.models.catalog(), [])

  def test_an_unreadable_catalog_is_empty_not_an_error(self):
    # read from the UI's param thread, where an exception takes the panel down
    self.op.store['ModelManager_ModelsCache_Chestnut'] = {'bundles': [{'ref': REF_A, 'minimum_selector_version': 'x'}]}
    self.assertEqual(self.models.catalog(), [])
    self.assertTrue(self.op.log.has('could not read the big-model catalog', 'exception'))

  def test_without_a_model_manager_there_is_no_catalog(self):
    self.op.keys = dataclasses.replace(self.op.keys, catalog=None, big_model=None)
    self.catalog(bundle(REF_A, 'Alpha', 1))
    self.assertEqual(self.models.catalog(), [])


class TestModelIndex(ModelsTest):
  """Every catalog model, with the ONNX behind it once that has been looked up."""

  def index_with(self, bundles, pointers=POINTERS):
    with mock.patch.object(self.models, 'catalog', return_value=bundles), \
         mock.patch.object(self.models, 'pointers', return_value=pointers):
      return self.models.model_index()

  def test_a_resolved_model_carries_its_identity(self):
    (entry,) = self.index_with([{'name': 'Alpha', 'ref': REF_A}])
    self.assertEqual(entry, {'name': 'Alpha', 'ref': REF_A, 'oid': '1' * 64, 'size': 766_000_000})

  def test_an_unresolved_model_is_still_listed(self):
    # the pointer is fetched when the model is first asked for
    (entry,) = self.index_with([{'name': 'Gamma', 'ref': REF_C}])
    self.assertEqual((entry['name'], entry['oid'], entry['size']), ('Gamma', None, None))

  def test_a_size_is_known_only_once_resolved(self):
    with mock.patch.object(self.models, 'catalog', return_value=[{'name': 'Alpha', 'ref': REF_A},
                                                                 {'name': 'Gamma', 'ref': REF_C}]), \
         mock.patch.object(self.models, 'pointers', return_value=POINTERS):
      self.assertEqual(self.models.size_for('1' * 64), 766_000_000)
      self.assertIsNone(self.models.size_for('9' * 64))

  def test_a_second_read_within_the_ttl_costs_nothing(self):
    # the UI names the active model every frame
    with mock.patch.object(self.models, 'catalog', return_value=[]) as read, \
         mock.patch.object(self.models, 'pointers', return_value={}):
      first = self.models.model_index()
      self.assertIs(self.models.model_index(), first)
    self.assertEqual(read.call_count, 1)

  def test_after_the_ttl_it_is_read_again(self):
    with mock.patch.object(self.models, 'catalog', return_value=[]) as read, \
         mock.patch.object(self.models, 'pointers', return_value={}):
      self.models.model_index()
      with mock.patch.object(models.time, 'monotonic', return_value=models.time.monotonic() + models.INDEX_TTL):
        self.models.model_index()
    self.assertEqual(read.call_count, 2)


class TestResolvePointer(ModelsTest):
  """The pointer at a commit is the oid and size the Jetson is asked for,
  fetched the first time a model is asked for and kept for good."""

  POINTER = f"version https://git-lfs.github.com/spec/v1\noid sha256:{'3' * 64}\nsize 766040736\n"

  def setUp(self):
    super().setUp()
    self.models._index_cache = (float('inf'), [], {})   # a stale index must be dropped on a hit

  def response(self, body: bytes):
    r = mock.MagicMock()
    r.__enter__.return_value = r
    r.read.return_value = body
    return r

  def test_fetches_once_and_records_it(self):
    from jetlink.registry.lfs import POINTER_URL
    self.op.store['JetlinkModelPointers'] = dict(POINTERS)
    with mock.patch.object(urllib.request, 'urlopen', return_value=self.response(self.POINTER.encode())) as urlopen:
      self.assertEqual(self.models.resolve_pointer(REF_C), ('3' * 64, 766040736))
    self.assertEqual(urlopen.call_args.args[0], POINTER_URL.format(ref=REF_C))
    written = self.op.store['JetlinkModelPointers']
    self.assertEqual(written[REF_C], {'oid': '3' * 64, 'size': 766040736})
    self.assertEqual(written[REF_A], POINTERS[REF_A])
    self.assertIsNone(self.models._index_cache)
    self.assertTrue(self.op.log.has(f"{REF_C[:10]} is {'3' * 16}, 730 MB"))

  def test_it_is_written_blocking(self):
    # the next lookup reads it back, and a put still in flight would be lost under it
    with mock.patch.object(self.op, 'put') as put, \
         mock.patch.object(self.models, 'fetch_pointer', return_value=('3' * 64, 1)):
      self.models.resolve_pointer(REF_C)
    self.assertIs(put.call_args.kwargs['block'], True)

  def test_a_known_pointer_needs_no_fetch(self):
    self.op.store['JetlinkModelPointers'] = dict(POINTERS)
    with mock.patch.object(urllib.request, 'urlopen') as urlopen:
      self.assertEqual(self.models.resolve_pointer(REF_A), ('1' * 64, 766_000_000))
    urlopen.assert_not_called()

  def test_a_miss_raises_and_records_nothing(self):
    from jetlink.registry.catalog import RegistryError
    for failure in ({'side_effect': OSError('offline')}, {'return_value': self.response(b'<html>not found</html>')}):
      with self.subTest(failure), mock.patch.object(urllib.request, 'urlopen', **failure), \
           self.assertRaises(RegistryError):
        self.models.resolve_pointer(REF_C)
    self.assertNotIn('JetlinkModelPointers', self.op.store)

  def test_the_lookup_is_the_registry_s(self):
    """One resolver for both ends; the precompiled-pkl commits are tested there."""
    from jetlink.registry.lfs import Pointer
    with mock.patch('jetlink.registry.lfs.fetch_pointer', return_value=Pointer('4' * 64, 766354845)) as fetch:
      self.assertEqual(self.models.resolve_pointer(REF_C), ('4' * 64, 766354845))
    fetch.assert_called_once_with(REF_C, timeout=models.POINTER_TIMEOUT)


class TestSelectedModel(ModelsTest):
  """The pick is the model manager's big-model slot, the same one a chestnut runs
  from, listed in the catalog or not; the catalog only supplies the default."""

  INDEX = [
    {'name': 'Alpha', 'ref': REF_A, 'oid': 'a' * 64, 'size': 10},
    {'name': 'Beta', 'ref': REF_B, 'oid': 'b' * 64, 'size': 20},
  ]

  def select_with(self, slot_ref, default=REF_B, index=None, known=None):
    pick = {'name': 'Picked', 'ref': slot_ref} if slot_ref else None
    with mock.patch.object(self.models, '_index', return_value=(self.INDEX if index is None else index, known or {})), \
         mock.patch.object(models, 'DEFAULT_BIG_MODEL_REF', default), \
         mock.patch.object(self.models, 'selected_slot', return_value=pick):
      return self.models.selected_model()

  def test_an_empty_slot_takes_the_default_big_model(self):
    assert self.select_with(None)['name'] == 'Beta'

  def test_the_default_is_jetlinks_not_the_chestnuts(self):
    # a chestnut's default is the model in the tree; the accelerator's is jetlink's
    from jetlink.registry.catalog import DEFAULT_BIG_MODEL_REF
    assert models.DEFAULT_BIG_MODEL_REF == DEFAULT_BIG_MODEL_REF

  def test_a_default_not_in_the_catalog_falls_to_the_newest(self):
    assert self.select_with(None, default='f' * 40)['name'] == 'Alpha'

  def test_the_pick_is_the_slot_with_its_pointer(self):
    for ref in (REF_A, REF_C):   # listed or not
      known = {ref: {'oid': 'c' * 64, 'size': '30'}}
      assert self.select_with(ref, known=known) == {'name': 'Picked', 'ref': ref, 'oid': 'c' * 64, 'size': 30}

  def test_a_pick_not_yet_resolved_has_no_oid(self):
    # the worker resolves it from the ref, as for any catalog model
    assert self.select_with(REF_C) == {'name': 'Picked', 'ref': REF_C, 'oid': None, 'size': None}

  def test_a_pick_needs_no_catalog(self):
    assert self.select_with(REF_C, index=[])['ref'] == REF_C

  def test_no_pick_and_no_catalog_is_no_model(self):
    assert self.select_with(None, index=[]) is None

  def test_the_name_is_the_picks_or_none(self):
    with mock.patch.object(self.models, 'selected_model', return_value={'name': 'Picked'}):
      self.assertEqual(self.models.selected_model_name(), 'Picked')
    with mock.patch.object(self.models, 'selected_model', return_value=None):
      self.assertIsNone(self.models.selected_model_name())

  def test_end_to_end_off_the_params(self):
    self.catalog(bundle(REF_A, 'Alpha', 1), bundle(REF_B, 'Beta', 2))
    self.op.store['JetlinkModelPointers'] = dict(POINTERS)
    self.op.store['ModelManager_ActiveBundleChestnut'] = {'ref': REF_A, 'displayName': 'Alpha'}
    self.assertEqual(self.models.selected_model(), {'name': 'Alpha', 'ref': REF_A, 'oid': '1' * 64, 'size': 766_000_000})


class TestWithoutAModelManager(ModelsTest):
  """A fork without sunnypilot's model manager: no pick, no catalog. The
  large model is jetlink's default, found by its ref like any pick."""

  def setUp(self):
    super().setUp()
    self.op.keys = dataclasses.replace(self.op.keys, catalog=None, big_model=None)

  def test_jetlinks_default_runs(self):
    from jetlink.registry.catalog import DEFAULT_BIG_MODEL_NAME, DEFAULT_BIG_MODEL_REF
    self.assertEqual(self.models.selected_model(),
                     {'name': DEFAULT_BIG_MODEL_NAME, 'ref': DEFAULT_BIG_MODEL_REF, 'oid': None, 'size': None})
    self.assertEqual(self.models.default_model_name(), 'Cinque Terre V3 Model')
    self.assertEqual(self.jl.status().model, 'Cinque Terre V3 Model')

  def test_once_resolved_it_carries_its_identity(self):
    from jetlink.registry.catalog import DEFAULT_BIG_MODEL_REF
    self.op.store['JetlinkModelPointers'] = {DEFAULT_BIG_MODEL_REF: {'oid': '9' * 64, 'size': 766_000_000}}
    self.assertEqual(self.models.selected_model()['oid'], '9' * 64)
    self.assertEqual(self.models.selected_model()['size'], 766_000_000)

  def test_it_is_provisioned_like_any_pick(self):
    from jetlink.openpilot import link, provision
    from jetlink.registry.catalog import DEFAULT_BIG_MODEL_REF
    client = mock.Mock()
    client.hello.return_value = {}
    client.ensure_engine.return_value = mock.Mock(sha256='9' * 64, to_dict=lambda: {'sha256': '9' * 64})
    run = provision.ProvisioningRun(self.parts)
    run.client = client
    with mock.patch.object(self.models, 'fetch_pointer', return_value=('9' * 64, 766_000_000)) as fetch:
      self.assertTrue(run.provision())
    fetch.assert_called_once_with(DEFAULT_BIG_MODEL_REF)
    self.assertEqual(client.ensure_engine.call_args.args[:2], ('9' * 64, 766_000_000))
    self.assertTrue(self.parts.spec.engine_ready_for('9' * 64))
    self.assertEqual(client.ensure_engine.call_args.kwargs['build_timeout'], link.BUILD_TIMEOUT)

  def test_a_fork_whose_catalog_is_not_fetched_yet_still_waits_for_it(self):
    # sunnypilot's model manager lists it shortly; its pick may not be the default
    self.op.keys = dataclasses.replace(self.op.keys, catalog='ModelManager_ModelsCache_Chestnut')
    self.assertIsNone(self.models.selected_model())


class TestDefaultModelName(ModelsTest):
  """The accelerator's default, named as the chestnut's DEFAULT_BIG_MODEL is: the
  catalog's display name without its build date."""

  def name_with(self, names, default=REF_B):
    index = [{'name': name, 'ref': ref, 'oid': None, 'size': None} for name, ref in zip(names, (REF_A, REF_B), strict=False)]
    with mock.patch.object(self.models, '_index', return_value=(index, {})), \
         mock.patch.object(models, 'DEFAULT_BIG_MODEL_REF', default):
      return self.models.default_model_name()

  def test_the_build_date_is_dropped(self):
    assert self.name_with(['Alpha', 'Cinque Terre V3 Model (September 17, 2026)']) == 'Cinque Terre V3 Model'

  def test_a_parenthesis_that_is_not_a_date_stays(self):
    assert self.name_with(['Alpha', 'Beta (big)']) == 'Beta (big)'

  def test_a_default_not_listed_names_the_newest(self):
    assert self.name_with(['Alpha (September 01, 2026)', 'Beta'], default='f' * 40) == 'Alpha'

  def test_no_catalog_is_no_name(self):
    assert self.name_with([]) is None


class TestSelectedSlot(ModelsTest):
  def read_with(self, slot):
    self.models._slot_cache = None
    self.op.store['ModelManager_ActiveBundleChestnut'] = slot
    return self.models.selected_slot()

  def test_a_second_read_within_the_ttl_costs_nothing(self):
    # the UI names the active model every frame
    self.op.store['ModelManager_ActiveBundleChestnut'] = {'ref': REF_A}
    with mock.patch.object(self.op, 'get', wraps=self.op.get) as read:
      assert self.models.selected_slot()['ref'] == REF_A
      assert self.models.selected_slot()['ref'] == REF_A
    assert read.call_count == 1

  def test_reads_the_slots_ref_and_name(self):
    assert self.read_with({'ref': REF_A, 'displayName': 'Alpha'}) == {'name': 'Alpha', 'ref': REF_A}
    assert self.read_with({'ref': REF_A}) == {'name': REF_A[:10], 'ref': REF_A}

  def test_anything_else_is_no_pick(self):
    for slot in (None, {}, {'ref': ''}, {'ref': 7}, 'junk'):
      assert self.read_with(slot) is None, slot

  def test_the_slot_is_the_key_the_adapter_names(self):
    self.op.keys = dataclasses.replace(self.op.keys, big_model='SomeOtherForksSlot')
    self.op.store['SomeOtherForksSlot'] = {'ref': REF_B}
    assert self.read_with({'ref': REF_A}) == {'name': REF_B[:10], 'ref': REF_B}


class TestShippedModelPath(ModelsTest):
  """A file counts only when it is the model we mean, at the size we expect.
  Models live one file per oid so switching back does not re-download."""

  MODEL = {'name': 'Alpha', 'ref': REF_A, 'oid': 'a' * 64, 'size': 4096}

  def setUp(self):
    super().setUp()
    self.patch(self.models, 'selected_model', return_value=self.MODEL)

  def fetched(self, size: int):
    path = self.models.model_dir() / self.models.model_file_name(self.MODEL)
    path.parent.mkdir(parents=True)
    path.write_bytes(b'\0' * size)
    return path

  def test_the_downloads_have_a_directory_of_their_own(self):
    # the model manager's cache clear deletes every file in its root that it
    # does not recognise, and leaves directories alone
    self.assertEqual(self.models.model_dir(), self.op.model_root() / 'jetlink')

  def test_a_download_lands_in_it_never_in_the_root(self):
    from jetlink.registry.lfs import Pointer
    with mock.patch('jetlink.registry.lfs.lfs_resolve', return_value='https://x/y'), \
         mock.patch('jetlink.registry.lfs.lfs_download', side_effect=lambda href, pointer, dest, **kw: dest) as download:
      dest = self.models.fetch_shipped_model()
    self.assertEqual(dest.parent, self.op.model_root() / 'jetlink')
    self.assertEqual(download.call_args.args[1], Pointer('a' * 64, 4096))

  def test_a_model_not_resolved_yet_is_no_path(self):
    with mock.patch.object(self.models, 'selected_model', return_value={**self.MODEL, 'oid': None, 'size': None}):
      assert self.models.shipped_model_path() is None

  def test_nothing_fetched_yet(self):
    assert self.models.shipped_model_path() is None

  def test_the_chosen_model_is_accepted(self):
    fetched = self.fetched(4096)
    assert self.models.shipped_model_path() == fetched

  def test_a_truncated_download_is_rejected(self):
    # Half a model is exactly what must not reach TensorRT.
    self.fetched(2048)
    assert self.models.shipped_model_path() is None

  def test_no_model_chosen_is_no_path(self):
    with mock.patch.object(self.models, 'selected_model', return_value=None):
      assert self.models.shipped_model_path() is None


class TestFetchShippedModel(ModelsTest):
  """The bytes come through jetlink's registry downloader; all the checkout adds
  is the LFS server its own .lfsconfig names, asked first."""

  MODEL = {'name': 'Alpha', 'ref': REF_A, 'oid': 'a' * 64, 'size': 4096}

  def setUp(self):
    super().setUp()
    self.patch(self.models, 'selected_model', return_value=self.MODEL)

  def lfsconfig(self, text: str) -> None:
    (self.op.basedir / '.lfsconfig').write_text(text)

  def test_the_checkouts_server_comes_first(self):
    from jetlink.registry.lfs import LFS_ENDPOINTS
    self.lfsconfig('[lfs]\n\turl = https://example.com/info/lfs/\n')
    self.assertEqual(self.models.lfs_endpoints(), ['https://example.com/info/lfs', *LFS_ENDPOINTS])

  def test_without_one_it_is_jetlinks_list(self):
    # a release ships no .lfsconfig
    from jetlink.registry.lfs import LFS_ENDPOINTS
    self.assertEqual(self.models.lfs_endpoints(), list(LFS_ENDPOINTS))

  def test_no_duplicate_when_it_is_already_one_of_jetlinks(self):
    from jetlink.registry.lfs import LFS_ENDPOINTS
    self.lfsconfig(f'[lfs]\n\turl = {LFS_ENDPOINTS[0]}\n')
    self.assertEqual(self.models.lfs_endpoints(), list(LFS_ENDPOINTS))

  def test_already_fetched_is_returned_as_is(self):
    dest = self.models.model_dir() / self.models.model_file_name(self.MODEL)
    dest.parent.mkdir(parents=True)
    dest.write_bytes(b'\0' * 4096)
    with mock.patch('jetlink.registry.lfs.lfs_resolve') as resolve:
      self.assertEqual(self.models.fetch_shipped_model(), dest)
    resolve.assert_not_called()

  def test_falls_through_to_the_next_server(self):
    from jetlink.registry.lfs import LFS_ENDPOINTS, Pointer
    self.lfsconfig('[lfs]\n\turl = https://dead.example/info/lfs\n')
    stop = object()
    with mock.patch('jetlink.registry.lfs.lfs_resolve', side_effect=[None, 'https://x/y']) as resolve, \
         mock.patch('jetlink.registry.lfs.lfs_download', side_effect=lambda href, pointer, dest, **kw: dest) as download:
      dest = self.models.fetch_shipped_model(should_stop=stop)
    self.assertEqual([c.args[0] for c in resolve.call_args_list], ['https://dead.example/info/lfs', LFS_ENDPOINTS[0]])
    self.assertEqual(download.call_args.args[:3], ('https://x/y', Pointer('a' * 64, 4096), dest))
    self.assertIs(download.call_args.kwargs['should_stop'], stop)

  def no_waits(self):
    from jetlink.openpilot import models
    return mock.patch.object(models, 'DOWNLOAD_RETRY_DELAYS', (0.0,))

  def test_nowhere_to_get_it_raises_once_the_attempts_are_spent(self):
    from jetlink.registry.catalog import NetworkError
    with mock.patch('jetlink.registry.lfs.lfs_resolve', return_value=None) as resolve, self.no_waits(), \
         self.assertRaises(NetworkError):
      self.models.fetch_shipped_model()
    # the one wait (no_waits) between two attempts
    self.assertEqual(resolve.call_count, 2 * len(self.models.lfs_endpoints()))

  def test_a_failed_transfer_is_tried_again_and_told_of(self):
    # a car's connection stalls; each attempt asks the LFS server for a fresh
    # address, which expires, and carries on from the .part (lfs_download)
    from jetlink.registry.catalog import NetworkError
    waits = []
    transfers = iter([NetworkError('read timed out'), None])

    def download(href, pointer, dest, **kw):
      if (e := next(transfers)) is not None:
        raise e
      return dest

    with mock.patch('jetlink.registry.lfs.lfs_resolve', return_value='https://x/y') as resolve, \
         mock.patch('jetlink.registry.lfs.lfs_download', side_effect=download), self.no_waits():
      dest = self.models.fetch_shipped_model(retrying=lambda e, delay: waits.append((str(e), delay)))
    self.assertEqual(dest.name, self.models.model_file_name(self.MODEL))
    self.assertEqual(resolve.call_count, 2)
    self.assertEqual(waits, [('read timed out', 0.0)])

  def test_a_stopped_run_stops_waiting_to_try_again(self):
    from jetlink.openpilot import models
    from jetlink.registry.catalog import NetworkError
    with mock.patch('jetlink.registry.lfs.lfs_resolve', return_value=None), \
         mock.patch.object(models, 'DOWNLOAD_RETRY_DELAYS', (60.0,)), self.assertRaises(NetworkError):
      self.models.fetch_shipped_model(should_stop=lambda: True)

  def test_a_model_with_its_file_here_has_it(self):
    self.assertFalse(self.models.has_file(self.MODEL))
    dest = self.models.model_dir() / self.models.model_file_name(self.MODEL)
    dest.parent.mkdir(parents=True)
    dest.write_bytes(b'\0' * 100)
    self.assertFalse(self.models.has_file(self.MODEL), 'a short file is not the model')
    dest.write_bytes(b'\0' * 4096)
    self.assertTrue(self.models.has_file(self.MODEL))
    self.assertFalse(self.models.has_file({**self.MODEL, 'oid': None}))

  def test_nothing_chosen_is_nothing_fetched(self):
    with mock.patch.object(self.models, 'selected_model', return_value={**self.MODEL, 'oid': None}):
      self.assertIsNone(self.models.fetch_shipped_model())


OLD, NEW = 'a' * 40, 'e' * 40


def full_bundle(ref: str, index: int, selector: str, name: str) -> dict:
  return {'ref': ref, 'index': index, 'minimum_selector_version': selector, 'is_big': True, 'is_20hz': True,
          'display_name': name, 'short_name': name[:4], 'generation': '12', 'environment': 'development',
          'runner': 'tinygrad', 'build_time': '2026-09-25T00:00:00Z', 'overrides': {'folder': 'Master Models'},
          'models': [{'type': 'chunked', 'artifact': {'file_name': f'{name}.pkl', 'download_uri': {'url': 'x', 'sha256': 'y'}}}]}


PINNED = {'tinygrad_ref': 'pinned', 'bundles': [full_bundle(OLD, 12, '19', 'Cinque Terre V3')]}
NEWER = {'tinygrad_ref': 'next', 'bundles': [full_bundle(OLD, 12, '20', 'Cinque Terre V3'),
                                             full_bundle(NEW, 13, '20', 'Cinque Terre V4')]}


class TestBigCatalog(ModelsTest):
  """A big model sunnypilot publishes after this build is still pickable with a
  Jetson: the model manager's catalog has the newer catalogs' models folded
  in, as entries a chestnut never downloads. That the model manager accepts
  them is the fork's to test."""

  def merged(self, newer=NEWER):
    probe_result = {'side_effect': newer} if isinstance(newer, Exception) else {'return_value': newer}
    with mock.patch('jetlink.registry.catalog.fetch_catalogs', **probe_result) as probe:
      out = self.jl.extend_catalog(PINNED)
    return out, probe

  def test_a_model_only_a_newer_catalog_lists_is_folded_in(self):
    out, probe = self.merged()
    probe.assert_called_once_with()
    self.assertEqual([b['ref'] for b in out['bundles']], [OLD, NEW])
    # nothing for a chestnut to fetch
    self.assertEqual(out['bundles'][1]['models'], [])
    self.assertTrue(self.op.log.has('1 model(s) only newer catalogs list'))

  def test_a_model_the_pinned_catalog_has_keeps_its_build(self):
    out, _ = self.merged()
    self.assertIs(out['bundles'][0], PINNED['bundles'][0])
    self.assertEqual(out['tinygrad_ref'], 'pinned')

  def test_nothing_newer_or_a_failed_probe_leaves_it_alone(self):
    self.assertIs(self.merged(newer=PINNED)[0], PINNED)
    self.assertIs(self.merged(newer=OSError('offline'))[0], PINNED)
    self.assertTrue(self.op.log.has('could not check for newer catalogs', 'exception'))

  def test_a_failed_probe_keeps_what_the_last_one_found(self):
    # the model manager's cached copy has the newer model as merge_catalogs made
    # it; a probe that fails now must not drop a pick that is only listed there
    last, _ = self.merged()
    self.op.put('ModelManager_ModelsCache_Chestnut', {**last, 'extended': True}, block=True)
    out, _ = self.merged(newer=OSError('offline'))
    self.assertEqual([b['ref'] for b in out['bundles']], [OLD, NEW])
    self.assertEqual(out['bundles'][1], last['bundles'][1])
    # sunnypilot's own entries come from the fetch, never the cache
    self.assertIs(out['bundles'][0], PINNED['bundles'][0])
    self.assertTrue(self.op.log.has('keeping 1 model(s) the last probe found'))

  def test_a_failed_probe_keeps_a_model_a_newer_catalog_built_at_our_selector(self):
    # the fork fetches v25 and v26 adds Cinque Terre V3 at selector 19, builds
    # and all: one failed probe dropped it from the picker and reset the pick
    newer = {'tinygrad_ref': 'pinned', 'bundles': [*PINNED['bundles'], full_bundle(NEW, 13, '19', 'Cinque Terre V4')]}
    last, _ = self.merged(newer=newer)
    self.assertEqual([b['ref'] for b in last['bundles']], [OLD, NEW])
    self.op.put('ModelManager_ModelsCache_Chestnut', {**last, 'extended': True}, block=True)
    out, _ = self.merged(newer=OSError('offline'))
    self.assertEqual([b['ref'] for b in out['bundles']], [OLD, NEW])
    self.assertEqual(out['bundles'][1], newer['bundles'][1])
    self.assertTrue(self.op.log.has('keeping 1 model(s) the last probe found'))

  def test_the_selector_is_the_adapters(self):
    with mock.patch('jetlink.registry.catalog.fetch_catalogs', return_value=NEWER), \
         mock.patch('jetlink.registry.catalog.merge_catalogs', return_value=PINNED) as merge:
      self.jl.extend_catalog(PINNED)
    self.assertEqual(merge.call_args.kwargs['selector'], 19)


if __name__ == '__main__':
  unittest.main()
