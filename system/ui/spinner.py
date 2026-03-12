#!/usr/bin/env python3
import os
import pyray as rl
import select
import subprocess
import sys
import time
from collections import deque
from pathlib import Path

from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.text import wrap_text
from openpilot.system.ui.widgets import Widget

# Constants
if gui_app.big_ui():
  PROGRESS_BAR_WIDTH = 1000
  PROGRESS_BAR_HEIGHT = 20
  TEXTURE_SIZE = 360
  WRAPPED_SPACING = 50
  CENTERED_SPACING = 150
else:
  PROGRESS_BAR_WIDTH = 268
  PROGRESS_BAR_HEIGHT = 10
  TEXTURE_SIZE = 140
  WRAPPED_SPACING = 10
  CENTERED_SPACING = 20
DEGREES_PER_SECOND = 360.0  # one full rotation per second
MARGIN_H = 100
FONT_SIZE = 96
LINE_HEIGHT = 104
DARKGRAY = (55, 55, 55, 255)
RESET_TAP_COUNT = 8
RESET_TAP_WINDOW_S = 4.0

# FrogPilot variables
GREEN = (23, 134, 68, 242)


def clamp(value, min_value, max_value):
  return max(min(value, max_value), min_value)


def get_device_type() -> str:
  model_path = Path("/sys/firmware/devicetree/base/model")
  if model_path.is_file():
    try:
      model = model_path.read_text().strip("\x00")
      return model.split("comma ")[-1].strip().lower()
    except Exception:
      pass
  return ""


class Spinner(Widget):
  def __init__(self):
    super().__init__()
    self._comma_texture = gui_app.texture("images/spinner_comma.png", TEXTURE_SIZE, TEXTURE_SIZE)
    self._spinner_texture = gui_app.texture("images/spinner_track.png", TEXTURE_SIZE, TEXTURE_SIZE, alpha_premultiply=True)
    self._rotation = 0.0
    self._progress: int | None = None
    self._wrapped_lines: list[str] = []
    self._logo_rect = rl.Rectangle(0, 0, 0, 0)
    self._tap_times = deque(maxlen=RESET_TAP_COUNT)
    self._launch_reset = False
    self._allow_reset_gesture = os.path.isfile("/TICI") and get_device_type() not in ("tici", "tizi")

  def set_text(self, text: str) -> None:
    if text.isdigit():
      self._progress = clamp(int(text), 0, 100)
      self._wrapped_lines = []
    else:
      self._progress = None
      self._wrapped_lines = wrap_text(text, FONT_SIZE, gui_app.width - MARGIN_H)

  def _render(self, rect: rl.Rectangle):
    if self._wrapped_lines:
      # Calculate total height required for spinner and text
      spacing = WRAPPED_SPACING
      total_height = TEXTURE_SIZE + spacing + len(self._wrapped_lines) * LINE_HEIGHT
      center_y = (rect.height - total_height) / 2.0 + TEXTURE_SIZE / 2.0
    else:
      # Center spinner vertically
      spacing = CENTERED_SPACING
      center_y = rect.height / 2.0
    y_pos = center_y + TEXTURE_SIZE / 2.0 + spacing

    center = rl.Vector2(rect.width / 2.0, center_y)
    spinner_origin = rl.Vector2(TEXTURE_SIZE / 2.0, TEXTURE_SIZE / 2.0)
    comma_position = rl.Vector2(center.x - TEXTURE_SIZE / 2.0, center.y - TEXTURE_SIZE / 2.0)
    self._logo_rect = rl.Rectangle(comma_position.x, comma_position.y, TEXTURE_SIZE, TEXTURE_SIZE)

    delta_time = rl.get_frame_time()
    self._rotation = (self._rotation + DEGREES_PER_SECOND * delta_time) % 360.0

    # Draw rotating spinner and static comma logo
    rl.draw_texture_pro(self._spinner_texture, rl.Rectangle(0, 0, TEXTURE_SIZE, TEXTURE_SIZE),
                        rl.Rectangle(center.x, center.y, TEXTURE_SIZE, TEXTURE_SIZE),
                        spinner_origin, self._rotation, rl.WHITE)
    rl.draw_texture_v(self._comma_texture, comma_position, rl.WHITE)

    # Display the progress bar or text based on user input
    if self._progress is not None:
      bar = rl.Rectangle(center.x - PROGRESS_BAR_WIDTH / 2.0, y_pos, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT)
      rl.draw_rectangle_rounded(bar, 1, 10, DARKGRAY)

      bar.width *= self._progress / 100.0
      rl.draw_rectangle_rounded(bar, 1, 10, GREEN)
    elif self._wrapped_lines:
      for i, line in enumerate(self._wrapped_lines):
        text_size = measure_text_cached(gui_app.font(), line, FONT_SIZE)
        rl.draw_text_ex(gui_app.font(), line, rl.Vector2(center.x - text_size.x / 2, y_pos + i * LINE_HEIGHT),
                        FONT_SIZE, 0.0, rl.WHITE)

  def _handle_mouse_release(self, mouse_pos):
    if not self._allow_reset_gesture:
      return

    if not rl.check_collision_point_rec(mouse_pos, self._logo_rect):
      return

    now = time.monotonic()
    self._tap_times.append(now)
    if len(self._tap_times) == RESET_TAP_COUNT and (now - self._tap_times[0]) <= RESET_TAP_WINDOW_S:
      self._tap_times.clear()
      self._launch_reset = True

  @property
  def should_launch_reset(self) -> bool:
    return self._launch_reset


def _read_stdin():
  """Non-blocking read of available lines from stdin."""
  lines = []
  while True:
    rlist, _, _ = select.select([sys.stdin], [], [], 0.0)
    if not rlist:
      break
    line = sys.stdin.readline().strip()
    if line == "":
      break
    lines.append(line)
  return lines


def main():
  gui_app.init_window("Spinner")
  spinner = Spinner()
  for _ in gui_app.render():
    text_list = _read_stdin()
    if text_list:
      spinner.set_text(text_list[-1])

    spinner.render(rl.Rectangle(0, 0, gui_app.width, gui_app.height))
    if spinner.should_launch_reset:
      reset_script = Path(__file__).with_name("reset.py")
      try:
        proc = subprocess.Popen(
          [sys.executable, str(reset_script)],
          cwd=str(reset_script.parent),
          close_fds=True,
          stdout=subprocess.DEVNULL,
          stderr=subprocess.DEVNULL,
        )
      except OSError:
        spinner.set_text("Failed to launch reset UI")
        continue

      # Keep spinner alive if reset process exits immediately (prevents blank screen).
      time.sleep(0.2)
      if proc.poll() is not None:
        spinner.set_text("Reset UI failed to start")
        continue

      gui_app.request_close()
      break


if __name__ == "__main__":
  main()
