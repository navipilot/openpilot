from typing import TYPE_CHECKING

__all__ = [
  "CONFIGS",
  "get_process_config",
  "get_custom_params_from_lr",
  "replay_process",
  "replay_process_with_name",
]

if TYPE_CHECKING:
  from openpilot.selfdrive.test.process_replay.process_replay import CONFIGS, get_custom_params_from_lr, get_process_config, replay_process, replay_process_with_name


def __getattr__(name: str):
  if name in __all__:
    from openpilot.selfdrive.test.process_replay.process_replay import CONFIGS, get_custom_params_from_lr, get_process_config, replay_process, replay_process_with_name

    exports = {
      "CONFIGS": CONFIGS,
      "get_process_config": get_process_config,
      "get_custom_params_from_lr": get_custom_params_from_lr,
      "replay_process": replay_process,
      "replay_process_with_name": replay_process_with_name,
    }
    return exports[name]

  raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
