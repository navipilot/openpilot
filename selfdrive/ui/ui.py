#!/usr/bin/env python3
import os
import pyray as rl

from openpilot.system.hardware import TICI
from openpilot.common.realtime import config_realtime_process, set_core_affinity
from openpilot.common.watchdog import kick_watchdog
from openpilot.system.ui.lib.application import gui_app
from openpilot.selfdrive.ui.stall_monitor import UIStallMonitor
from openpilot.selfdrive.ui.layouts.main import MainLayout
from openpilot.selfdrive.ui.mici.layouts.main import MiciMainLayout
from openpilot.selfdrive.ui.ui_state import ui_state


def main():
  cores = {5, }
  config_realtime_process(0, 51)

  gui_app.init_window("UI")
  if gui_app.big_ui():
    main_layout = MainLayout()
  else:
    main_layout = MiciMainLayout()
  main_layout.set_rect(rl.Rectangle(0, 0, gui_app.width, gui_app.height))
  stall_monitor = UIStallMonitor("raylib_ui")
  gui_app.set_progress_hook(stall_monitor.progress)
  stall_monitor.progress("ui.loop_ready")
  stall_monitor.start()

  def render_layout() -> None:
    stall_monitor.progress("ui.before_layout_render")
    main_layout.render()
    stall_monitor.progress("ui.after_layout_render")

  try:
    for should_render in gui_app.render(render_callback=render_layout):
      stall_monitor.progress("ui.loop_iteration")
      kick_watchdog()
      stall_monitor.progress("ui.after_watchdog")
      ui_state.update()
      stall_monitor.progress("ui.after_state_update")
      if should_render:
        # reaffine after power save offlines our core
        if TICI and os.sched_getaffinity(0) != cores:
          try:
            set_core_affinity(list(cores))
          except OSError:
            pass
      stall_monitor.progress("ui.loop_idle")
  finally:
    gui_app.set_progress_hook(None)
    stall_monitor.stop()


if __name__ == "__main__":
  main()
