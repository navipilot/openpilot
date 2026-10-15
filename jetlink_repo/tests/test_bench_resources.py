"""The bench sampler reads real Linux formats without depending on openpilot."""
from pathlib import Path

from tests import load_script

resources = load_script(Path(__file__).resolve().parents[1] / 'scripts/comma/bench_resources.py')


def test_proc_stat_handles_names_with_spaces_and_parentheses(tmp_path):
  # Field 3 starts after comm, not after an arbitrary whitespace split.
  fields = ['S'] + ['0'] * 21
  for index, value in [(7, 2), (9, 3), (11, 17), (12, 8), (19, 1000), (21, 40)]:
    fields[index] = str(value)
  (tmp_path / 'stat').write_text('123 (worker (usb)) ' + ' '.join(fields))
  (tmp_path / 'status').write_text('voluntary_ctxt_switches:\t7\nnonvoluntary_ctxt_switches:\t9\n')
  (tmp_path / 'schedstat').write_text('100 20 5\n')
  got = resources.task(tmp_path)
  assert got == {'user_ticks': 17, 'system_ticks': 8, 'minor_faults': 2, 'major_faults': 3,
                 'start_ticks': 1000, 'rss_pages': 40, 'voluntary_switches': 7,
                 'involuntary_switches': 9, 'schedstat': '100 20 5'}


def test_missing_process_is_not_an_empty_measurement(tmp_path):
  assert resources.task(tmp_path) is None
  (tmp_path / 'meminfo').write_text('MemFree: 4096 kB\nMemAvailable: 900000 kB\n')
  (tmp_path / 'vmstat').write_text('allocstall 6\ncompact_stall 2\nnr_dirty 10\npgfault 500\n')
  got = resources.sample([123], tmp_path, pss=True)
  assert got['processes'] == {}
  assert got['meminfo_kb'] == {'MemFree': 4096, 'MemAvailable': 900000}
  assert got['vmstat'] == {'allocstall': 6, 'compact_stall': 2, 'nr_dirty': 10}


def test_pss_sums_mappings_and_missing_pss_is_unknown(tmp_path):
  path = tmp_path / '123'
  path.mkdir()
  (path / 'stat').write_text('123 (modeld) S ' + '0 ' * 21)
  (path / 'smaps').write_text('Pss: 10 kB\nRss: 90 kB\nPss: 20 kB\n')
  assert resources.sample([123], tmp_path, pss=True)['processes']['123']['pss_kb'] == 30
  (path / 'smaps').unlink()
  assert resources.sample([123], tmp_path, pss=True)['processes']['123']['pss_kb'] is None


def test_the_recorded_sysctls_are_the_ones_the_link_tunes():
  assert 'vm.extra_free_kbytes' in resources.tuned_sysctls()
