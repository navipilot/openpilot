import pyray as rl
from collections.abc import Callable

from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.widgets import Widget, DialogResult
from openpilot.system.ui.widgets.button import Button, ButtonStyle
from openpilot.system.ui.widgets.label import Label, FontWeight
from openpilot.system.ui.widgets.keyboard import Keyboard, KeyboardLayout

MARGIN = 50
BUTTON_HEIGHT = 160
OUTER_MARGIN_X = 200
OUTER_MARGIN_Y = 150
BACKGROUND_COLOR = rl.Color(27, 27, 27, 255)

class InputDialog(Widget):
  def __init__(self, title: str, default_text: str = "", hint_text: str = "", on_close: Callable[[DialogResult, str], None] | None = None):
    super().__init__()
    self._title = title
    self._text = default_text
    self._hint = hint_text
    self._on_close = on_close
    
    self._dialog_result = DialogResult.NO_ACTION
    
    self._title_label = Label(title, 70, FontWeight.BOLD, text_color=rl.Color(201, 201, 201, 255))
    self._cancel_button = Button("Cancel", self._cancel_button_callback)
    self._confirm_button = Button("Confirm", self._confirm_button_callback, button_style=ButtonStyle.PRIMARY)
    
    self._keyboard = Keyboard(self._on_key_pressed, self._on_keyboard_done, layout=KeyboardLayout.QWERTY)
    
    self._font = gui_app.font(FontWeight.MEDIUM)

  def _on_key_pressed(self, key: str):
    if key == "\b":
      self._text = self._text[:-1]
    else:
      self._text += key

  def _on_keyboard_done(self):
    self._confirm_button_callback()

  def _cancel_button_callback(self):
    self._dialog_result = DialogResult.CANCEL
    if self._on_close:
        self._on_close(self._dialog_result, self._text)

  def _confirm_button_callback(self):
    self._dialog_result = DialogResult.CONFIRM
    if self._on_close:
        self._on_close(self._dialog_result, self._text)

  @property
  def result(self) -> DialogResult:
    return self._dialog_result
    
  @property
  def text(self) -> str:
    return self._text

  def show_event(self):
    super().show_event()
    self._dialog_result = DialogResult.NO_ACTION

  def _render(self, rect: rl.Rectangle):
    # Dim background
    rl.draw_rectangle(0, 0, int(rect.width), int(rect.height), rl.Color(0, 0, 0, 200))

    # Dialog Box
    dialog_rect = rl.Rectangle(
      rect.x + OUTER_MARGIN_X,
      rect.y + OUTER_MARGIN_Y,
      rect.width - 2 * OUTER_MARGIN_X,
      rect.height - 2 * OUTER_MARGIN_Y,
    )
    rl.draw_rectangle_rounded(dialog_rect, 0.05, 10, BACKGROUND_COLOR)

    # Title
    title_rect = rl.Rectangle(dialog_rect.x + MARGIN, dialog_rect.y + MARGIN, dialog_rect.width - 2 * MARGIN, 100)
    self._title_label.render(title_rect)

    # Text Input Field
    input_rect = rl.Rectangle(dialog_rect.x + MARGIN, title_rect.y + title_rect.height + 40, dialog_rect.width - 2 * MARGIN, 120)
    rl.draw_rectangle_rounded(input_rect, 0.1, 10, rl.Color(40, 40, 40, 255))
    
    display_text = self._text
    text_color = rl.WHITE
    if not display_text:
        display_text = self._hint
        text_color = rl.Color(128, 128, 128, 255)
        
    text_size = rl.measure_text_ex(self._font, display_text, 50, 0)
    text_pos = rl.Vector2(input_rect.x + 40, input_rect.y + (input_rect.height - text_size.y) / 2)
    rl.draw_text_ex(self._font, display_text, text_pos, 50, 0, text_color)

    # Blinking cursor
    if (rl.get_time() % 1.0) < 0.5:
        cursor_x = text_pos.x + (text_size.x if self._text else 0) + 5
        rl.draw_rectangle(int(cursor_x), int(text_pos.y), 4, 50, rl.WHITE)

    # Keyboard
    keyboard_rect = rl.Rectangle(
      dialog_rect.x + MARGIN,
      input_rect.y + input_rect.height + 40,
      dialog_rect.width - 2 * MARGIN,
      400
    )
    self._keyboard.render(keyboard_rect)

    # Buttons
    btn_y = dialog_rect.y + dialog_rect.height - BUTTON_HEIGHT - MARGIN
    btn_width = (dialog_rect.width - 3 * MARGIN) / 2
    
    cancel_rect = rl.Rectangle(dialog_rect.x + MARGIN, btn_y, btn_width, BUTTON_HEIGHT)
    confirm_rect = rl.Rectangle(dialog_rect.x + 2 * MARGIN + btn_width, btn_y, btn_width, BUTTON_HEIGHT)

    self._cancel_button.render(cancel_rect)
    self._confirm_button.render(confirm_rect)
