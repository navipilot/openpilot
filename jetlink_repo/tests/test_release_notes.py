"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

Release notes from CHANGELOG.md (scripts/changelog.py), for the GitHub
release and the Mac app's update window.
"""
from pathlib import Path

from tests import load_script

ROOT = Path(__file__).resolve().parents[1]
changelog = load_script(ROOT / 'scripts' / 'changelog.py')

SAMPLE = """Jetlink v0.9.0
==============
**Driving**
* One

* Two

Jetlink v0.8.1
==============
* Older


Jetlink v0.8.0
==============
* Oldest
"""


def test_sections_newest_first_without_the_underline():
  assert changelog.sections(SAMPLE) == [
    ('v0.9.0', '**Driving**\n* One\n\n* Two'),
    ('v0.8.1', '* Older'),
    ('v0.8.0', '* Oldest'),
  ]


def test_notes_are_one_section_and_empty_for_an_unknown_tag():
  assert changelog.notes(SAMPLE, 'v0.8.1') == '* Older'
  assert changelog.notes(SAMPLE, 'v9.9.9') == ''


def test_history_puts_each_release_under_its_heading():
  assert changelog.history(SAMPLE, 'v0.9.0', 2) == '## Jetlink v0.9.0\n\n**Driving**\n* One\n\n* Two\n\n## Jetlink v0.8.1\n\n* Older'
  assert changelog.history(SAMPLE, 'v0.8.1', 10) == '## Jetlink v0.8.1\n\n* Older\n\n## Jetlink v0.8.0\n\n* Oldest'
  assert changelog.history(SAMPLE, 'v9.9.9', 10) == ''


def test_the_real_changelog_parses():
  text = (ROOT / 'CHANGELOG.md').read_text(encoding='utf-8')
  releases = changelog.sections(text)
  assert len(releases) > 5
  assert all(tag.startswith('v') and body for tag, body in releases)
  assert len({tag for tag, _ in releases}) == len(releases)

