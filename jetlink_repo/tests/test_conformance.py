"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

The Python half of the conformance suite (docs/conformance.md).

The Swift server is held to what the comma's Python does: Pinned.swift and
the wire, staging and registry fixtures come from the Python in this checkout.
This file holds the Python to the same files: every generator runs again into
a temporary directory and must write the committed bytes. So a Python change
that moves an output fails here, and regenerating to make it pass fails the
Swift until the Swift moves too.

The generators run in a fresh interpreter each, as they do from the command
line, so a graph built from another test module's helpers never sees what
earlier tests did to that module's state.

The staging fixtures come from a graph onnx shape-infers, so they are compared
only under the onnx release that made them
(JetlinkKit/Scripts/fixture-pins.txt).
"""
from __future__ import annotations

import importlib.metadata
import os
import subprocess
import sys
from pathlib import Path

import pytest

from tests import load_script

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / 'JetlinkKit' / 'Scripts'

# where each generator writes by default, relative to the checkout; their top
# level is light, numpy at most
CONFORMANCE_SCRIPT = load_script(SCRIPTS / 'make_conformance_fixtures.py')
CONFORMANCE = CONFORMANCE_SCRIPT.SERVER
REGISTRY = CONFORMANCE_SCRIPT.REGISTRY
PINS_SCRIPT = load_script(SCRIPTS / 'make_pins.py')
PINNED = PINS_SCRIPT.OUT.relative_to(ROOT)
# the releases the committed fixtures were made with
PINS = PINS_SCRIPT.fixture_pins()
FIXTURE_ONNX = PINS['onnx']


def _run(script: str, *args) -> None:
  env = {**os.environ, 'PYTHONPATH': str(ROOT)}
  result = subprocess.run([sys.executable, str(SCRIPTS / f'{script}.py'), *map(str, args)], cwd=ROOT, env=env,
                          capture_output=True, text=True, timeout=600)
  assert result.returncode == 0, f'{script} failed:\n{result.stdout}\n{result.stderr}'


def _same_tree(made: Path, committed: Path, skip=lambda name: False) -> list[str]:
  """What differs between two directories, file by file, as sentences."""
  problems = []
  made_files = {p.relative_to(made) for p in made.rglob('*') if p.is_file()}
  committed_files = {p.relative_to(committed) for p in committed.rglob('*') if p.is_file()}
  for rel in sorted(made_files | committed_files):
    if skip(rel.name):
      continue
    if rel not in committed_files:
      problems.append(f'{rel} is made but not committed')
    elif rel not in made_files:
      problems.append(f'{rel} is committed but no longer made')
    elif (made / rel).read_bytes() != (committed / rel).read_bytes():
      problems.append(f'{rel} differs')
  return problems


def _onnx_matches() -> bool:
  try:
    return importlib.metadata.version('onnx') == FIXTURE_ONNX
  except importlib.metadata.PackageNotFoundError:
    return False


def test_pinned_swift_is_current(tmp_path):
  made = tmp_path / 'Pinned.swift'
  _run('make_pins', '--out', made)
  assert made.read_text() == (ROOT / PINNED).read_text(), \
    'JetlinkKit/Sources/JetlinkKit/Pinned.swift is stale: run JetlinkKit/Scripts/make_pins.py and commit it'


def test_the_swift_package_links_the_pinned_onnxruntime():
  package = (ROOT / 'JetlinkKit' / 'Package.swift').read_text()
  assert f"pod-archive-onnxruntime-c-{PINS['onnxruntime']}.zip" in package


@pytest.mark.parametrize('part', list(CONFORMANCE_SCRIPT.PARTS))
def test_conformance_fixtures_are_what_the_python_makes(tmp_path, part):
  if part == 'staging' and not _onnx_matches():
    pytest.skip(f'the staging spec comes from onnx shape inference; fixtures made with onnx {FIXTURE_ONNX}')
  _run('make_conformance_fixtures', '--root', tmp_path, part)
  where = {'registry': REGISTRY}.get(part, CONFORMANCE)
  if where.suffix:
    made, committed = tmp_path / where, ROOT / where
    assert made.read_bytes() == committed.read_bytes(), f'{where} differs; see docs/conformance.md'
    return
  # wire and staging share one directory with the Swift's own goldens:
  # compare what this part writes
  problems = _same_tree(tmp_path / where, ROOT / where, skip=lambda name: not name.startswith(part))
  assert not problems, problems

