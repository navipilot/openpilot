"""
Copyright © IQ.Lvbs, apart of Project Teal Lvbs, All Rights Reserved, licensed under https://konn3kt.com/tos
"""
import sys
from pathlib import Path

from openpilot.iqpilot import _proprietary_loader as loader


def _write(path: Path, content: str = "") -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  path.write_text(content)


def test_module_root_for_name_prefers_matching_bundle(tmp_path, monkeypatch):
  repo_root = tmp_path / "repo"
  model_root = repo_root / "artifacts" / "iqpilot_model_selector_private" / "python"
  hepha_root = repo_root / "artifacts" / "iqpilot_hephaestusd_private" / "python"

  _write(model_root / "iqpilot_private" / "__init__.py")
  _write(model_root / "iqpilot_private" / "models" / "__init__.py")
  _write(model_root / "iqpilot_private" / "models" / "manager.py", "VALUE = 'models'\n")

  _write(hepha_root / "iqpilot_private" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "hephaestus" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "hephaestus" / "hephaestusd.py", "VALUE = 'hepha'\n")

  monkeypatch.setattr(loader, "__file__", str(repo_root / "iqpilot" / "_proprietary_loader.py"))

  resolved = loader._module_root_for_name("iqpilot_private.konn3kt.hephaestus.hephaestusd")
  assert resolved == hepha_root


def test_load_private_module_uses_matching_bundle(tmp_path, monkeypatch):
  repo_root = tmp_path / "repo"
  model_root = repo_root / "artifacts" / "iqpilot_model_selector_private" / "python"
  hepha_root = repo_root / "artifacts" / "iqpilot_hephaestusd_private" / "python"

  _write(model_root / "iqpilot_private" / "__init__.py")
  _write(model_root / "iqpilot_private" / "models" / "__init__.py")
  _write(model_root / "iqpilot_private" / "models" / "manager.py", "VALUE = 'models'\n")

  _write(hepha_root / "iqpilot_private" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "hephaestus" / "__init__.py")
  _write(hepha_root / "iqpilot_private" / "konn3kt" / "hephaestus" / "hephaestusd.py", "VALUE = 'hepha'\n")

  monkeypatch.setattr(loader, "__file__", str(repo_root / "iqpilot" / "_proprietary_loader.py"))
  monkeypatch.setenv("IQPILOT_PROPRIETARY_ROOT", "")

  old_sys_path = list(sys.path)
  for key in [k for k in sys.modules if k.startswith("iqpilot_private") or k == "test_public_module"]:
    sys.modules.pop(key, None)

  try:
    sys.modules["test_public_module"] = type(sys)("test_public_module")
    private_module = loader.load_private_module("test_public_module", "iqpilot_private.konn3kt.hephaestus.hephaestusd")
    assert private_module.VALUE == "hepha"
    assert sys.modules["test_public_module"].VALUE == "hepha"
  finally:
    sys.path[:] = old_sys_path
    for key in [k for k in sys.modules if k.startswith("iqpilot_private") or k == "test_public_module"]:
      sys.modules.pop(key, None)
