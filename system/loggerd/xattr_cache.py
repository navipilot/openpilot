import errno

import os

try:
  import xattr  # type: ignore[import-not-found]
except ImportError:
  xattr = None

_cached_attributes: dict[tuple, bytes | None] = {}
_backend_disabled = False


def _backend_getxattr(path: str, attr_name: str) -> bytes:
  if xattr is not None:
    return xattr.getxattr(path, attr_name)
  if hasattr(os, "getxattr"):
    return os.getxattr(path, attr_name)
  raise OSError(errno.ENOTSUP, "xattr backend unavailable")


def _backend_setxattr(path: str, attr_name: str, attr_value: bytes) -> None:
  if xattr is not None:
    xattr.setxattr(path, attr_name, attr_value)
    return
  if hasattr(os, "setxattr"):
    os.setxattr(path, attr_name, attr_value)
    return
  raise OSError(errno.ENOTSUP, "xattr backend unavailable")


def _is_missing_attr_error(e: OSError) -> bool:
  return e.errno == errno.ENODATA or (hasattr(errno, "ENOATTR") and e.errno == errno.ENOATTR)


def _is_unsupported_error(e: OSError) -> bool:
  unsupported_errnos = {errno.ENOTSUP, errno.EOPNOTSUPP, errno.ENOSYS}
  return e.errno in unsupported_errnos

def getxattr(path: str, attr_name: str) -> bytes | None:
  global _backend_disabled

  key = (path, attr_name)
  if key not in _cached_attributes:
    response: bytes | None = None
    try:
      if not _backend_disabled:
        response = _backend_getxattr(path, attr_name)
    except OSError as e:
      # ENODATA (Linux) or ENOATTR (macOS) means attribute hasn't been set
      if _is_missing_attr_error(e):
        response = None
      elif _is_unsupported_error(e):
        _backend_disabled = True
        response = None
      else:
        raise
    _cached_attributes[key] = response
  return _cached_attributes[key]

def setxattr(path: str, attr_name: str, attr_value: bytes) -> None:
  global _backend_disabled

  key = (path, attr_name)
  if _backend_disabled:
    # In environments without xattr support, keep behavior stable for this process.
    _cached_attributes[key] = attr_value
    return

  try:
    _backend_setxattr(path, attr_name, attr_value)
  except OSError as e:
    if _is_unsupported_error(e):
      _backend_disabled = True
      _cached_attributes[key] = attr_value
      return
    _cached_attributes.pop(key, None)
    raise

  _cached_attributes[key] = attr_value
