"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

What the comma asks of the registry: the catalog, a model's lfs pointer, and
the download.

Every test is offline. Network calls go through an injected `opener` that maps
a URL to a fixture.
"""
from __future__ import annotations

import hashlib
import io
import json
import urllib.error
from pathlib import Path

import pytest

from jetlink.registry.catalog import EXTRA_CATALOG_URL, NetworkError, RegistryError, VerifyError, parse_catalog
from jetlink.registry.lfs import (COMMIT_PATCH_URL, DRIVING_MODELS_TREE_URL, LFS_ENDPOINTS, POINTER_URL, PULL_PATCH_URL, Pointer,
                                  diff_pointer, fetch_pointer, lfs_download, lfs_resolve, parse_pointer_text)

FIXTURES = Path(__file__).parent / 'fixtures'
REF = 'f877d7a0ccc3cce943c76e285214c020cd65c899'
OID = 'a086d5249fc308bb73993d1e64630c669d4c7df5bde85f42ad61902543648525'
SIZE = 765953504
NEWEST = '37bfa1413edcdc2e8844984b83727c33f81d8f46'
BLOB = b'onnx' * 1024
BLOB_SHA = hashlib.sha256(BLOB).hexdigest()
HREF = 'https://blob.example/object'


def fixture(name: str) -> bytes:
  return (FIXTURES / name).read_bytes()


class FakeResponse:
  def __init__(self, data: bytes, status: int = 200):
    self._buf = io.BytesIO(data)
    self.status = status

  def read(self, n: int = -1) -> bytes:
    return self._buf.read(n if n is not None and n >= 0 else -1)

  def __enter__(self):
    return self

  def __exit__(self, *_):
    return False


class FakeOpener:
  """A urlopen that serves fixtures and refuses everything else. A catalog
  version it has no fixture for is a 404, as it is on GitHub: the newest one
  sunnypilot has published is the last to come back. So is the extra list."""

  def __init__(self, routes: dict):
    self.routes = routes
    self.calls: list[str] = []

  def __call__(self, url, timeout=None, **_):
    url = url.full_url if hasattr(url, 'full_url') else url
    self.calls.append(url)
    body = self.routes.get(url)
    if body is None:
      from jetlink.registry.catalog import CATALOG_URL_TEMPLATE
      if url.startswith(CATALOG_URL_TEMPLATE.split('{version}')[0]) or url == EXTRA_CATALOG_URL:
        raise not_found(url)
      raise urllib.error.URLError(f"no route for {url}")
    if isinstance(body, Exception):
      raise body
    if callable(body):
      body = body()
    return FakeResponse(body if isinstance(body, bytes) else json.dumps(body).encode())


# --- catalog -----------------------------------------------------------------

def test_parse_catalog_keeps_the_big_models_newest_first():
  models = parse_catalog(json.loads(fixture('catalog_chestnut_v25.json')))
  assert len(models) == 13
  assert models[0].ref == NEWEST
  assert models[0].short_name == 'CTMV2'
  assert [m.index for m in models] == sorted((m.index for m in models), reverse=True)


@pytest.mark.parametrize('mutate', [
  {'minimum_selector_version': '18'},
  {'ref': 'not-a-commit'},
  {'is_big': False},
])
def test_parse_catalog_drops_a_bundle_it_cannot_use(mutate):
  data = json.loads(fixture('catalog_chestnut_v25.json'))
  data['bundles'][0].update(mutate)
  models = parse_catalog(data)
  assert len(models) == 12
  assert all(m.ref != 'fa0c6876d3cf070e91e25e5353ceadc68a5b3285' for m in models)


def test_parse_catalog_survives_rubbish():
  assert parse_catalog({}) == []
  assert parse_catalog({'bundles': 'nope'}) == []
  assert parse_catalog({'bundles': [None, 5, {'ref': REF, 'minimum_selector_version': 'x', 'is_big': True}]}) == []


def test_a_duplicate_ref_is_listed_once():
  data = json.loads(fixture('catalog_chestnut_v25.json'))
  data['bundles'].append(dict(data['bundles'][0], display_name='A copy', index=99))
  models = parse_catalog(data)
  assert len(models) == 13
  assert all(m.name != 'A copy' for m in models)


# --- pointers ----------------------------------------------------------------

def test_parse_pointer_text_reads_the_fixture():
  pointer = parse_pointer_text(fixture('pointer_f877d7a0.txt').decode())
  assert pointer == Pointer(OID, SIZE)


@pytest.mark.parametrize('text', [
  'not a pointer at all',
  f"oid sha256:{OID}\n",                       # no size
  'oid sha256:nothex\nsize 12\n',
  f"oid sha256:{OID}\nsize twelve\n",
  'x' * 5000,                                  # an onnx served where a pointer was expected
])
def test_parse_pointer_text_refuses_anything_else(text):
  assert parse_pointer_text(text) is None


def test_fetch_pointer_reads_the_commits_pointer():
  opener = FakeOpener({POINTER_URL.format(ref=REF): fixture('pointer_f877d7a0.txt')})
  assert fetch_pointer(REF, opener=opener) == Pointer(OID, SIZE)
  assert opener.calls == [POINTER_URL.format(ref=REF)]


def _bundle(ref: str, index: int, selector: str = '19', name: str = '', **extra) -> dict:
  return {'ref': ref, 'index': index, 'minimum_selector_version': selector, 'is_big': True,
          'display_name': name or ref[:6], 'short_name': name[:4], 'generation': '12', 'environment': 'development',
          'runner': 'tinygrad', 'build_time': '2026-09-25T00:00:00Z', 'overrides': {'folder': 'Master Models'},
          'models': [{'type': 'chunked', 'artifact': {'file_name': f'{ref[:6]}.pkl'}}], **extra}


class TestNewerCatalogs:
  """A model sunnypilot publishes after this release is still listed."""

  def url(self, v):
    from jetlink.registry.catalog import CATALOG_URL_TEMPLATE
    return CATALOG_URL_TEMPLATE.format(version=v)

  def test_versions_are_probed_up_to_the_first_missing_one(self):
    from jetlink.registry.catalog import CATALOG_VERSION, fetch_catalogs
    v = CATALOG_VERSION
    fresh = 'e' * 40
    opener = FakeOpener({self.url(v): {'bundles': []}, self.url(v + 1): {'bundles': []},
                         self.url(v + 2): {'bundles': [_bundle(fresh, 99, '20')]},
                         self.url(v + 3): not_found(self.url(v + 3))})
    assert [b['ref'] for b in fetch_catalogs(opener=opener)['bundles']] == [fresh]
    assert opener.calls == [self.url(v), self.url(v + 1), self.url(v + 2), self.url(v + 3), EXTRA_CATALOG_URL]

  def test_an_outage_past_the_pin_is_a_failure_not_a_short_list(self):
    from jetlink.registry.catalog import CATALOG_VERSION, fetch_catalogs
    fresh = 'e' * 40
    opener = FakeOpener({self.url(CATALOG_VERSION): {'bundles': []},
                         self.url(CATALOG_VERSION + 1): {'bundles': [_bundle(fresh, 99, '20')]},
                         self.url(CATALOG_VERSION + 2): urllib.error.URLError('down')})
    with pytest.raises(NetworkError):
      fetch_catalogs(opener=opener)

  def test_the_merge_keeps_builds_at_our_version_and_adds_the_rest_for_an_accelerator(self):
    from jetlink.registry.catalog import merge_catalogs
    a, b, c = 'a' * 40, 'b' * 40, 'c' * 40
    pinned = {'tinygrad_ref': 'pinned', 'bundles': [_bundle(a, 1), _bundle(b, 2)]}
    next_runtime = {'tinygrad_ref': 'next', 'bundles': [_bundle(a, 1, '20'), _bundle(b, 2, '20'), _bundle(c, 3, '20', 'Old name')]}
    newest = {'tinygrad_ref': 'newer', 'bundles': [_bundle(c, 3, '20', 'Cinque Terre V4')]}
    merged = merge_catalogs([pinned, next_runtime, newest])
    assert merged['tinygrad_ref'] == 'pinned'
    by_ref = {x['ref']: x for x in merged['bundles']}
    assert by_ref[a] == pinned['bundles'][0] and by_ref[b] == pinned['bundles'][1]
    assert by_ref[c]['display_name'] == 'Cinque Terre V4'
    assert by_ref[c]['minimum_selector_version'] == '19' and by_ref[c]['models'] == []
    assert by_ref[c]['overrides'] == {'folder': 'Master Models'}
    assert [m.ref for m in parse_catalog(merged)] == [c, b, a]


class TestExtraModels:
  """zoompilot's own list, after sunnypilot's: catalog/extra_big_models.json."""

  def routes(self, extra) -> dict:
    from jetlink.registry.catalog import CATALOG_URL
    return {CATALOG_URL: {'tinygrad_ref': 'pinned', 'bundles': [_bundle('a' * 40, 13)]}, EXTRA_CATALOG_URL: extra}

  def test_an_extra_model_is_listed_after_sunnypilots(self):
    from jetlink.registry.catalog import fetch_catalogs
    preview = _bundle('b' * 40, 14, name='A preview', models=[])
    merged = fetch_catalogs(opener=FakeOpener(self.routes({'bundles': [preview]})))
    assert merged['tinygrad_ref'] == 'pinned'
    assert [b['ref'] for b in merged['bundles']] == ['a' * 40, 'b' * 40]
    assert merged['bundles'][1] == preview

  def test_sunnypilots_entry_for_the_same_commit_wins(self):
    from jetlink.registry.catalog import fetch_catalogs
    ours = _bundle('a' * 40, 14, name='Our name', models=[])
    merged = fetch_catalogs(opener=FakeOpener(self.routes({'bundles': [ours]})))
    assert len(merged['bundles']) == 1 and merged['bundles'][0]['models']

  def test_a_missing_list_adds_nothing_and_an_outage_fails(self):
    from jetlink.registry.catalog import fetch_catalogs
    assert len(fetch_catalogs(opener=FakeOpener(self.routes(not_found(EXTRA_CATALOG_URL))))['bundles']) == 1
    with pytest.raises(NetworkError):
      fetch_catalogs(opener=FakeOpener(self.routes(urllib.error.URLError('down'))))

  def test_the_list_in_this_checkout_is_one_the_comma_can_parse(self):
    """The fields the fork's ModelParser._parse_bundle indexes: one missing and
    the comma drops every catalog it merged this into."""
    data = json.loads((Path(__file__).parents[1] / 'catalog' / 'extra_big_models.json').read_text())
    for bundle in data['bundles']:
      int(bundle['index']), int(bundle['generation']), int(bundle['minimum_selector_version'])
      assert bundle['short_name'] and bundle['display_name'] and bundle['environment'] and bundle['runner']
      assert bundle['is_big'] is True and bundle['models'] == []
      assert isinstance(bundle['overrides'], dict) and all(isinstance(v, str) for v in bundle['overrides'].values())
    assert [m.ref for m in parse_catalog(data)] == [b['ref'] for b in sorted(data['bundles'], key=lambda b: -int(b['index']))]


# --- lfs ---------------------------------------------------------------------

def test_lfs_resolve_returns_the_href():
  opener = FakeOpener({f"{LFS_ENDPOINTS[0]}/objects/batch": fixture('lfs_batch_response.json')})
  href = lfs_resolve(LFS_ENDPOINTS[0], Pointer(OID, SIZE), opener=opener)
  assert href.startswith('https://gitlab.com/commaai/openpilot-lfs.git/gitlab-lfs/objects/')


def test_lfs_resolve_is_none_when_the_server_lacks_it_or_is_down():
  missing = FakeOpener({f"{LFS_ENDPOINTS[0]}/objects/batch": fixture('lfs_batch_missing.json')})
  assert lfs_resolve(LFS_ENDPOINTS[0], Pointer(OID, SIZE), opener=missing) is None
  assert lfs_resolve(LFS_ENDPOINTS[0], Pointer(OID, SIZE), opener=FakeOpener({})) is None


# --- download ----------------------------------------------------------------

def download(tmp_path, blob: bytes = BLOB, oid: str = BLOB_SHA, size: int = len(BLOB), opener=None, **kw) -> Path:
  return lfs_download(HREF, Pointer(oid, size), tmp_path / 'models' / 'model.onnx',
                      opener=opener or FakeOpener({HREF: blob}), **kw)


def test_a_download_takes_its_name_only_once_it_verifies(tmp_path):
  seen = []
  path = download(tmp_path, progress=seen.append)
  assert path == tmp_path / 'models' / 'model.onnx'
  assert path.read_bytes() == BLOB
  assert [p.name for p in path.parent.iterdir()] == ['model.onnx']
  assert seen and seen[-1] == 1.0


def test_a_wrong_hash_leaves_nothing_behind(tmp_path):
  with pytest.raises(VerifyError, match='hash'):
    download(tmp_path, oid='b' * 64)
  assert not list((tmp_path / 'models').iterdir())


def test_a_short_download_leaves_nothing_behind(tmp_path):
  with pytest.raises(VerifyError, match='bytes'):
    download(tmp_path, size=len(BLOB) + 99)
  assert not list((tmp_path / 'models').iterdir())


class RangeOpener:
  """Serves BLOB from the Range a request asks for (206), or whole (200) when
  `honour` is off; `cut` bytes in, the transfer stalls (a read timeout)."""

  def __init__(self, blob: bytes = BLOB, honour: bool = True, cut: int | None = None):
    self.blob, self.honour, self.cut = blob, honour, cut
    self.ranges: list[str | None] = []

  def __call__(self, request, timeout=None, **_):
    rng = dict(request.header_items()).get('Range') if hasattr(request, 'header_items') else None
    self.ranges.append(rng)
    start = int(rng.split('=')[1].rstrip('-')) if rng and self.honour else 0
    body, cut = self.blob[start:], self.cut
    response = FakeResponse(body, 206 if rng and self.honour else 200)
    if cut is not None:
      read = response.read

      def stalling(n=-1):
        nonlocal cut
        if cut <= 0:
          raise TimeoutError('The read operation timed out')
        chunk = read(min(n, cut) if n >= 0 else cut)
        cut -= len(chunk)
        return chunk
      response.read = stalling
    return response


@pytest.fixture
def small_chunks(monkeypatch):
  from jetlink.registry import lfs
  monkeypatch.setattr(lfs, 'CHUNK', 1024)


def part_of(tmp_path) -> Path:
  return tmp_path / 'models' / 'model.onnx.part'


def test_a_stopped_download_keeps_its_part_for_the_next_one(tmp_path, small_chunks):
  # the owner stops a run at ignition; the bytes it got are not thrown away
  stops = iter([False, True])
  with pytest.raises(RegistryError, match='stopped'):
    download(tmp_path, opener=RangeOpener(), should_stop=lambda: next(stops))
  assert part_of(tmp_path).read_bytes() == BLOB[:1024]
  assert not (tmp_path / 'models' / 'model.onnx').exists()


def test_a_stalled_download_carries_on_from_its_part(tmp_path, small_chunks):
  # 2026-10-06: a 30 s stall 85 s into 755 MB threw the whole download away
  with pytest.raises(NetworkError, match='timed out'):
    download(tmp_path, opener=RangeOpener(cut=2048))
  assert part_of(tmp_path).stat().st_size == 2048
  opener = RangeOpener()
  path = download(tmp_path, opener=opener)
  assert opener.ranges == ['bytes=2048-']
  assert path.read_bytes() == BLOB and not part_of(tmp_path).exists()


def test_a_server_that_ignores_the_range_starts_over(tmp_path, small_chunks):
  with pytest.raises(NetworkError):
    download(tmp_path, opener=RangeOpener(cut=1024))
  path = download(tmp_path, opener=RangeOpener(honour=False))
  assert path.read_bytes() == BLOB


def test_a_part_that_was_never_this_model_fails_and_goes(tmp_path):
  part_of(tmp_path).parent.mkdir(parents=True)
  part_of(tmp_path).write_bytes(b'x' * 100)
  with pytest.raises(VerifyError, match='hash'):
    download(tmp_path, opener=RangeOpener())
  assert not part_of(tmp_path).exists()


def test_a_part_longer_than_the_model_is_dropped_and_started_over(tmp_path):
  part_of(tmp_path).parent.mkdir(parents=True)
  part_of(tmp_path).write_bytes(BLOB + b'extra')
  opener = RangeOpener()
  assert download(tmp_path, opener=opener).read_bytes() == BLOB
  assert opener.ranges == [None]


def test_a_whole_part_is_verified_without_asking_again(tmp_path):
  # a run stopped between the last byte and taking the name
  part_of(tmp_path).parent.mkdir(parents=True)
  part_of(tmp_path).write_bytes(BLOB)
  opener = RangeOpener()
  assert download(tmp_path, opener=opener).read_bytes() == BLOB
  assert opener.ranges == []


def test_a_failed_transfer_is_a_network_error(tmp_path):
  with pytest.raises(NetworkError, match='could not download'):
    lfs_download(HREF, Pointer(BLOB_SHA, len(BLOB)), tmp_path / 'model.onnx', opener=FakeOpener({}))
  assert not list(tmp_path.iterdir())


# --- a commit that ships a precompiled pkl -----------------------------------
# Cinque Terre V3, as github and huggingface served it on 2026-09-25.

V3_REF = 'bf3e3631b3f91d92a1020a5e0dd4298b93ff4244'
V3_OID = '404a18cfd86d29637d20c697dfde245bb47c666ae016730ab674c65f4d1e1aa4'
V3_SIZE = 766354845
V3_FOLDER = 'f78ed37d-afad-4dbc-8050-40ea885eedde'


def not_found(url):
  return urllib.error.HTTPError(url, 404, 'Not Found', {}, None)


def patch_head(subject: str) -> bytes:
  return (f"From {V3_REF} Mon Sep 17 00:00:00 2001\nFrom: Bruce Wayne <x@example.com>\n"
          f"Date: Tue, 15 Sep 2026 23:30:35 -0700\nSubject: [PATCH] {subject}\n\n---\n"
          " openpilot/selfdrive/modeld/models/big_driving_tinygrad.pkl | 2 +-\n").encode()


def onnx_entry(path: str, oid: str = V3_OID, size: int = V3_SIZE) -> dict:
  return {'type': 'file', 'path': f"{path}/big_driving_supercombo.onnx", 'size': size,
          'lfs': {'oid': oid, 'size': size, 'pointerSize': 134}}


def export_routes(subject='Use f78ed37d for the precompiled eGPU driving model', folder_files=None) -> dict:
  url = POINTER_URL.format(ref=V3_REF)
  return {
    url: not_found(url),
    COMMIT_PATCH_URL.format(ref=V3_REF): patch_head(subject),
    DRIVING_MODELS_TREE_URL: [{'type': 'directory', 'path': '1a421175-db71-4e3d-9d62-e2166421b02b'},
                              {'type': 'directory', 'path': V3_FOLDER},
                              {'type': 'file', 'path': 'README.md', 'size': 21}],
    f"{DRIVING_MODELS_TREE_URL}/{V3_FOLDER}?recursive=true": folder_files or [
      {'type': 'directory', 'path': f"{V3_FOLDER}/12864"}, onnx_entry(f"{V3_FOLDER}/12864")],
  }


def test_a_commit_without_the_onnx_resolves_to_the_export_its_subject_names():
  assert fetch_pointer(V3_REF, opener=FakeOpener(export_routes())) == Pointer(V3_OID, V3_SIZE)


def test_the_export_repo_is_the_last_lfs_server_asked():
  assert LFS_ENDPOINTS[-1] == 'https://huggingface.co/commaai/openpilot_driving_models.git/info/lfs'


def test_a_subject_that_names_the_checkpoint_picks_among_several():
  files = [onnx_entry(f"{V3_FOLDER}/12000", oid='1' * 64), onnx_entry(f"{V3_FOLDER}/12864")]
  opener = FakeOpener(export_routes(subject=f"{V3_FOLDER}/12864", folder_files=files))
  assert fetch_pointer(V3_REF, opener=opener) == Pointer(V3_OID, V3_SIZE)
  opener = FakeOpener(export_routes(folder_files=files))
  with pytest.raises(RegistryError, match='2 copies'):
    fetch_pointer(V3_REF, opener=opener)


def test_a_folded_subject_is_read_whole_and_the_diffstat_is_not():
  from jetlink.registry.lfs import commit_subject
  head = patch_head('Use f78ed37d for the precompiled eGPU\n driving model')
  assert commit_subject(V3_REF, opener=FakeOpener({COMMIT_PATCH_URL.format(ref=V3_REF): head})) == \
    'Use f78ed37d for the precompiled eGPU driving model'


@pytest.mark.parametrize(('subject', 'match'), [
  ('Update tinygrad and use retargetable model artifacts (#38933)', 'names no export'),
  ('Use 0badc0de for the precompiled eGPU driving model', 'no folder'),
])
def test_a_subject_that_leads_nowhere_says_so(subject, match):
  routes = export_routes(subject=subject)
  # the pull request a squash merge names carries no model either
  routes[PULL_PATCH_URL.format(number=38933)] = patch_head('Update tinygrad and use retargetable model artifacts')
  with pytest.raises(RegistryError, match=match):
    fetch_pointer(V3_REF, opener=FakeOpener(routes))


# ResAction (#39037), as github served it on 2026-10-05: 2fb4ac4aa0 adds the
# ONNX, 219f4e7ba3 ("compiled") swaps it for the pkl.
RES_COMPILED = '219f4e7ba38cda8788cc31f2d0a5ca49d9566231'
RES_POINTER = Pointer('1563b85f6bd00d9e2edf50bdb69646f0d95d5ac0f218d5abe3fe54a13427d71b', 792485215)
SQUASH = 'f' * 40


def compiled_routes(ref: str, patch: bytes) -> dict:
  url = POINTER_URL.format(ref=ref)
  return {url: not_found(url), COMMIT_PATCH_URL.format(ref=ref): patch}


def test_a_commit_that_compiles_the_onnx_away_resolves_to_the_one_it_deleted():
  opener = FakeOpener(compiled_routes(RES_COMPILED, fixture('patch_219f4e7b.patch')))
  assert fetch_pointer(RES_COMPILED, opener=opener) == RES_POINTER
  assert DRIVING_MODELS_TREE_URL not in opener.calls, "'compiled' names no export to look up"


def test_a_squash_merge_resolves_to_the_last_onnx_its_pull_request_carried():
  routes = compiled_routes(SQUASH, patch_head('ResAction (#39037)'))
  routes[PULL_PATCH_URL.format(number=39037)] = fixture('pull_39037.patch')
  assert fetch_pointer(SQUASH, opener=FakeOpener(routes)) == RES_POINTER


def test_a_squash_merge_of_a_precompiled_pull_request_follows_the_export_it_named():
  routes = {**export_routes(), **compiled_routes(SQUASH, patch_head('Cinque v3 (#38932)'))}
  routes[PULL_PATCH_URL.format(number=38932)] = patch_head('Use f78ed37d for the precompiled eGPU driving model')
  assert fetch_pointer(SQUASH, opener=FakeOpener(routes)) == Pointer(V3_OID, V3_SIZE)


def test_an_outage_reading_the_pull_request_is_a_failure():
  routes = compiled_routes(SQUASH, patch_head('ResAction (#39037)'))
  routes[PULL_PATCH_URL.format(number=39037)] = urllib.error.HTTPError('x', 503, 'Unavailable', {}, None)
  with pytest.raises(NetworkError):
    fetch_pointer(SQUASH, opener=FakeOpener(routes))


def test_diff_pointer_takes_the_models_pointer_and_nothing_else():
  def diff(path: str, old: Pointer | None, new: Pointer | None) -> str:
    lines = [f"diff --git a/{path} b/{path}", '--- a/' + path, '+++ b/' + path, '@@ -1,3 +1,3 @@']
    lines += ['-version https://git-lfs.github.com/spec/v1', f"-oid sha256:{old.oid}", f"-size {old.size}"] if old else []
    lines += ['+version https://git-lfs.github.com/spec/v1', f"+oid sha256:{new.oid}", f"+size {new.size}"] if new else []
    return '\n'.join(lines) + '\n'
  model = 'openpilot/selfdrive/modeld/models/big_driving_supercombo.onnx'
  a, b = Pointer('a' * 64, 1), Pointer('b' * 64, 2)
  assert diff_pointer(diff(model, a, b)) == b, "a changed model is the new one"
  assert diff_pointer(diff(model, a, None)) == a, "a deleted model is the one it was"
  assert diff_pointer(diff(model, None, a) + diff(model, a, b)) == b, "the last diff wins"
  assert diff_pointer(diff('openpilot/selfdrive/modeld/models/big_driving_tinygrad.pkl', a, b)) is None
  assert diff_pointer(diff(model + '.bak', a, b)) is None
  assert diff_pointer(diff(model, a, None) + diff('README.md', None, None)) == a
  assert diff_pointer('') is None and diff_pointer('not a patch') is None


def test_only_a_missing_file_falls_back_and_an_outage_does_not():
  url = POINTER_URL.format(ref=V3_REF)
  routes = export_routes()
  routes[url] = urllib.error.HTTPError(url, 503, 'Unavailable', {}, None)
  opener = FakeOpener(routes)
  with pytest.raises(NetworkError):
    fetch_pointer(V3_REF, opener=opener)
  assert opener.calls == [url]
