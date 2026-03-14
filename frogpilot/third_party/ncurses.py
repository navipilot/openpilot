from _build_path_shim import DEFAULT_LIB_DIRS, resolve_paths

INCLUDE_DIR, LIB_DIR = resolve_paths(
  __name__,
  include_candidates=("/usr/include/ncursesw", "/usr/include/ncurses", "/usr/local/include", "/usr/include"),
  lib_candidates=DEFAULT_LIB_DIRS,
)
