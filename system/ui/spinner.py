#!/usr/bin/env python3
from dataclasses import dataclass
import pyray as rl
import select
import sys

from openpilot.system.ui.lib.application import FontWeight, gui_app
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.text import wrap_text
from openpilot.system.ui.widgets import Widget


@dataclass(frozen=True)
class SpinnerTheme:
  progress_bar_width: int
  progress_bar_height: int
  texture_size: int
  wrapped_spacing: int
  centered_spacing: int
  text_font_size: int
  line_height: int
  horizontal_margin: int
  progress_label_font_size: int | None = None


BIG_THEME = SpinnerTheme(
  progress_bar_width=1000,
  progress_bar_height=20,
  texture_size=360,
  wrapped_spacing=50,
  centered_spacing=150,
  text_font_size=96,
  line_height=104,
  horizontal_margin=100,
)

SMALL_THEME = SpinnerTheme(
  progress_bar_width=320,
  progress_bar_height=12,
  texture_size=92,
  wrapped_spacing=14,
  centered_spacing=26,
  text_font_size=30,
  line_height=36,
  horizontal_margin=24,
  progress_label_font_size=24,
)


DEGREES_PER_SECOND = 360.0  # one full rotation per second
SMALL_MAX_WRAPPED_LINES = 4
PROGRESS_LABEL_SPACING = 12
DARKGRAY = (55, 55, 55, 255)


def clamp(value, min_value, max_value):
  return max(min(value, max_value), min_value)


class Spinner(Widget):
  def __init__(self):
    super().__init__()
    self._theme = BIG_THEME if gui_app.big_ui() else SMALL_THEME
    self._comma_texture = gui_app.texture("images/spinner_comma.png", self._theme.texture_size, self._theme.texture_size)
    self._spinner_texture = gui_app.texture("images/spinner_track.png", self._theme.texture_size, self._theme.texture_size, alpha_premultiply=True)
    self._rotation = 0.0
    self._progress: int | None = None
    self._wrapped_lines: list[str] = []
    self._text_font_size = self._theme.text_font_size
    self._line_height = self._theme.line_height

  def set_text(self, text: str) -> None:
    if text.isdigit():
      self._progress = clamp(int(text), 0, 100)
      self._wrapped_lines = []
    else:
      self._progress = None
      self._wrapped_lines, self._text_font_size = self._wrap_text(text)
      scale = self._text_font_size / self._theme.text_font_size
      self._line_height = round(self._theme.line_height * scale)

  def _wrap_text(self, text: str) -> tuple[list[str], int]:
    max_width = gui_app.width - self._theme.horizontal_margin * 2
    font_sizes = [self._theme.text_font_size]
    if not gui_app.big_ui():
      font_sizes.extend([28, 26, 24])

    for font_size in font_sizes:
      wrapped_lines = wrap_text(text, font_size, max_width)
      if gui_app.big_ui() or len(wrapped_lines) <= SMALL_MAX_WRAPPED_LINES:
        return wrapped_lines, font_size

    return wrapped_lines, font_sizes[-1]

  def _content_height(self) -> float:
    if self._progress is not None:
      height = self._theme.texture_size + self._theme.centered_spacing + self._theme.progress_bar_height
      if self._theme.progress_label_font_size is not None:
        height += PROGRESS_LABEL_SPACING + self._theme.progress_label_font_size
      return height

    if self._wrapped_lines:
      return self._theme.texture_size + self._theme.wrapped_spacing + len(self._wrapped_lines) * self._line_height

    return self._theme.texture_size

  def _draw_spinner(self, center_x: float, center_y: float) -> None:
    spinner_origin = rl.Vector2(self._theme.texture_size / 2.0, self._theme.texture_size / 2.0)
    comma_position = rl.Vector2(center_x - self._theme.texture_size / 2.0, center_y - self._theme.texture_size / 2.0)

    rl.draw_texture_pro(
      self._spinner_texture,
      rl.Rectangle(0, 0, self._theme.texture_size, self._theme.texture_size),
      rl.Rectangle(center_x, center_y, self._theme.texture_size, self._theme.texture_size),
      spinner_origin,
      self._rotation,
      rl.WHITE,
    )
    rl.draw_texture_v(self._comma_texture, comma_position, rl.WHITE)

  def _render(self, rect: rl.Rectangle):
    total_height = self._content_height()
    top_y = rect.y + (rect.height - total_height) / 2.0
    center_x = rect.x + rect.width / 2.0
    center_y = top_y + self._theme.texture_size / 2.0

    y_pos = top_y + self._theme.texture_size
    y_pos += self._theme.wrapped_spacing if self._wrapped_lines else self._theme.centered_spacing

    delta_time = rl.get_frame_time()
    self._rotation = (self._rotation + DEGREES_PER_SECOND * delta_time) % 360.0

    self._draw_spinner(center_x, center_y)

    if self._progress is not None:
      bar = rl.Rectangle(
        center_x - self._theme.progress_bar_width / 2.0,
        y_pos,
        self._theme.progress_bar_width,
        self._theme.progress_bar_height,
      )
      rl.draw_rectangle_rounded(bar, 1, 10, DARKGRAY)

      bar.width *= self._progress / 100.0
      rl.draw_rectangle_rounded(bar, 1, 10, rl.WHITE)
      if self._theme.progress_label_font_size is not None:
        progress_text = f"{self._progress}%"
        font = gui_app.font(FontWeight.BOLD)
        text_size = measure_text_cached(font, progress_text, self._theme.progress_label_font_size)
        text_pos = rl.Vector2(center_x - text_size.x / 2.0, y_pos + bar.height + PROGRESS_LABEL_SPACING)
        rl.draw_text_ex(font, progress_text, text_pos, self._theme.progress_label_font_size, 0.0, rl.Color(255, 255, 255, 180))
    elif self._wrapped_lines:
      font = gui_app.font(FontWeight.BOLD if not gui_app.big_ui() else FontWeight.NORMAL)
      for i, line in enumerate(self._wrapped_lines):
        text_size = measure_text_cached(font, line, self._text_font_size)
        line_y = y_pos + i * self._line_height
        rl.draw_text_ex(font, line, rl.Vector2(center_x - text_size.x / 2.0, line_y), self._text_font_size, 0.0, rl.WHITE)


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


if __name__ == "__main__":
  main()
