from openpilot.common.params_pyx import Params as _Params, ParamKeyFlag, ParamKeyType, UnknownKeyName
assert _Params
assert ParamKeyFlag
assert ParamKeyType
assert UnknownKeyName


class Params(_Params):
  def get(self, key, block=False, return_default=False, encoding=None, default=None):
    try:
      value = super().get(key, block=block, return_default=return_default)
    except UnknownKeyName:
      return default
    if value is None:
      return default
    if encoding is not None and isinstance(value, bytes):
      try:
        return value.decode(encoding)
      except Exception:
        return value.decode("utf-8", errors="replace")
    return value

  def get_bool(self, key, block=False, default=False):
    try:
      result = super().get(key, block=block, return_default=True)
      if result is None:
        return bool(default)
      return bool(result)
    except UnknownKeyName:
      return bool(default)

  def get_int(self, key, block=False, return_default=False, default=0):
    val = self.get(key, block=block, return_default=return_default, encoding="utf-8")
    if val is None or val == "":
      return default
    try:
      return int(float(val))
    except ValueError:
      return default

  def get_float(self, key, block=False, return_default=False, default=0.0):
    val = self.get(key, block=block, return_default=return_default, encoding="utf-8")
    if val is None or val == "":
      return default
    try:
      return float(val)
    except ValueError:
      return default

  def put_int(self, key, val):
    t = self.get_type(key)
    if t == ParamKeyType.FLOAT:
      self.put(key, float(val))
    elif t == ParamKeyType.INT:
      self.put(key, int(val))
    elif t == ParamKeyType.BOOL:
      self.put(key, bool(val))
    else:
      self.put(key, str(int(val)))

  def put_float(self, key, val):
    t = self.get_type(key)
    if t == ParamKeyType.FLOAT:
      self.put(key, float(val))
    elif t == ParamKeyType.INT:
      self.put(key, int(val))
    elif t == ParamKeyType.BOOL:
      self.put(key, bool(val))
    else:
      self.put(key, str(float(val)))

if __name__ == "__main__":
  import sys

  params = Params()
  key = sys.argv[1]
  assert params.check_key(key), f"unknown param: {key}"

  if len(sys.argv) == 3:
    val = sys.argv[2]
    print(f"SET: {key} = {val}")
    params.put(key, val)
  elif len(sys.argv) == 2:
    print(f"GET: {key} = {params.get(key)}")
