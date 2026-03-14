from _build_path_shim import DEFAULT_INCLUDE_DIRS, DEFAULT_LIB_DIRS, resolve_paths

INCLUDE_DIR, LIB_DIR = resolve_paths(__name__, include_candidates=DEFAULT_INCLUDE_DIRS, lib_candidates=DEFAULT_LIB_DIRS)
