import sysconfig

from _build_path_shim import DEFAULT_INCLUDE_DIRS, DEFAULT_LIB_DIRS, resolve_paths

INCLUDE_DIR, LIB_DIR = resolve_paths(
  __name__,
  include_candidates=(
    sysconfig.get_path("include"),
    sysconfig.get_path("platinclude"),
    "/usr/include/python3.12",
    "/usr/local/include/python3.12",
    *DEFAULT_INCLUDE_DIRS,
  ),
  lib_candidates=(sysconfig.get_config_var("LIBDIR"), *DEFAULT_LIB_DIRS),
)
