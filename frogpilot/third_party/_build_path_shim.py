import importlib.machinery
import importlib.util
import os
from pathlib import Path
import sys

THIS_DIR = Path(__file__).resolve().parent

DEFAULT_INCLUDE_DIRS = (
  "/usr/local/include",
  "/usr/include",
  "/usr/include/aarch64-linux-gnu",
)

DEFAULT_LIB_DIRS = (
  "/usr/local/lib",
  "/usr/lib/aarch64-linux-gnu",
  "/system/vendor/lib64",
  "/usr/lib",
)


def _search_paths():
  paths = []
  for entry in sys.path:
    if not entry:
      continue
    try:
      if Path(entry).resolve() == THIS_DIR:
        continue
    except OSError:
      pass
    paths.append(entry)
  return paths


def _load_installed_module(name):
  spec = importlib.machinery.PathFinder.find_spec(name, _search_paths())
  if spec is None or spec.loader is None:
    return None

  current_module = sys.modules.get(name)
  module = importlib.util.module_from_spec(spec)
  try:
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module
  finally:
    if current_module is None:
      sys.modules.pop(name, None)
    else:
      sys.modules[name] = current_module


def _first_existing(candidates):
  fallback = ""
  seen = set()
  for candidate in candidates:
    if not candidate or candidate in seen:
      continue
    seen.add(candidate)
    if not fallback:
      fallback = candidate
    if os.path.isdir(candidate):
      return candidate
  return fallback


def resolve_paths(name, *, include_candidates=DEFAULT_INCLUDE_DIRS, lib_candidates=DEFAULT_LIB_DIRS):
  module = _load_installed_module(name)
  if module is not None:
    include_dir = getattr(module, "INCLUDE_DIR", None)
    lib_dir = getattr(module, "LIB_DIR", None)
    if include_dir and lib_dir:
      return include_dir, lib_dir

  return _first_existing(include_candidates), _first_existing(lib_candidates)
