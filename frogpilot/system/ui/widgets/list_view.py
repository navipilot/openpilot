from collections.abc import Callable

import pyray as rl

from openpilot.system.ui.lib.application import FontWeight, MousePos, gui_app
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.widgets import list_view as lv
from openpilot.system.ui.widgets.list_view import ButtonAction, ItemAction, ListItem, MultipleButtonAction, TextAction, ToggleAction, _resolve_value

BUTTON_HEIGHT = lv.BUTTON_HEIGHT
BUTTON_WIDTH = lv.BUTTON_WIDTH
ICON_SIZE = lv.ICON_SIZE
ITEM_PADDING = lv.ITEM_PADDING
ITEM_TEXT_COLOR = lv.ITEM_TEXT_COLOR
ITEM_TEXT_FONT_SIZE = lv.ITEM_TEXT_FONT_SIZE
ITEM_TEXT_VALUE_COLOR = lv.ITEM_TEXT_VALUE_COLOR
RIGHT_ITEM_PADDING = lv.RIGHT_ITEM_PADDING
TOGGLE_WIDTH = lv.TOGGLE_WIDTH

FROGPILOT_ITEM_BASE_HEIGHT = 145
FROGPILOT_ITEM_DESC_V_OFFSET = 115
MULTI_BUTTON_TEXT_PADDING = 24
NUMERIC_BUTTON_FONT_SIZE = 50
NUMERIC_BUTTON_WIDTH = 150
NUMERIC_MIN_GAP_WIDTH = 40
NUMERIC_VALUE_WIDTH = 120


class FrogPilotListItem(ListItem):
  def __init__(self, title: str | Callable[[], str] = "", icon: str | None = None, description: str | Callable[[], str] | None = None,
               description_visible: bool = False, callback: Callable | None = None,
               action_item: ItemAction | None = None):
    super().__init__(title=title, icon=icon, description=description, description_visible=description_visible, callback=callback, action_item=action_item)
    self.set_rect(rl.Rectangle(0, 0, self._rect.width, FROGPILOT_ITEM_BASE_HEIGHT))

  def show_event(self):
    super().show_event()
    if self.action_item and hasattr(self.action_item, 'show_event'):
      self.action_item.show_event()

  def hide_event(self):
    super().hide_event()
    if self.action_item and hasattr(self.action_item, 'hide_event'):
      self.action_item.hide_event()

  def _render(self, _):
    if not self.is_visible:
      return

    # Don't draw items that are not in parent's viewport
    if ((self._rect.y + self.rect.height) <= self._parent_rect.y or
      self._rect.y >= (self._parent_rect.y + self._parent_rect.height)):
      return

    content_x = self._rect.x + ITEM_PADDING
    text_x = content_x

    # Only draw title and icon for items that have them
    if self.title:
      # Draw icon if present
      if self.icon:
        rl.draw_texture(self._icon_texture, int(content_x), int(self._rect.y + (FROGPILOT_ITEM_BASE_HEIGHT - self._icon_texture.height) // 2), rl.WHITE)
        text_x += ICON_SIZE + ITEM_PADDING

      # Draw main text
      text_size = measure_text_cached(self._font, self.title, ITEM_TEXT_FONT_SIZE)
      item_y = self._rect.y + (FROGPILOT_ITEM_BASE_HEIGHT - text_size.y) // 2
      rl.draw_text_ex(self._font, self.title, rl.Vector2(text_x, item_y), ITEM_TEXT_FONT_SIZE, 0, ITEM_TEXT_COLOR)

    # Draw description if visible
    if self.description_visible:
      content_width = int(self._rect.width - ITEM_PADDING * 2)
      description_height = self._html_renderer.get_total_height(content_width)
      description_rect = rl.Rectangle(
        self._rect.x + ITEM_PADDING,
        self._rect.y + FROGPILOT_ITEM_DESC_V_OFFSET,
        content_width,
        description_height
      )
      self._html_renderer.render(description_rect)

    # Draw right item if present
    if self.action_item:
      right_rect = self.get_right_item_rect(self._rect)
      right_rect.y = self._rect.y
      if self.action_item.render(right_rect) and self.action_item.enabled:
        # Right item was clicked/activated
        if self.callback:
          self.callback()

  def get_item_height(self, font: rl.Font, max_width: int) -> float:
    if not self.is_visible:
      return 0

    height = float(FROGPILOT_ITEM_BASE_HEIGHT)
    if self.description_visible:
      description_height = self._html_renderer.get_total_height(max_width)
      height += description_height - (FROGPILOT_ITEM_BASE_HEIGHT - FROGPILOT_ITEM_DESC_V_OFFSET) + ITEM_PADDING
    return height

  def get_right_item_rect(self, item_rect: rl.Rectangle) -> rl.Rectangle:
    if not self.action_item:
      return rl.Rectangle(0, 0, 0, 0)

    right_width = self.action_item.get_width_hint()
    if right_width == 0:  # Full width action (like DualButtonAction)
      return rl.Rectangle(item_rect.x + ITEM_PADDING, item_rect.y,
                          item_rect.width - (ITEM_PADDING * 2), FROGPILOT_ITEM_BASE_HEIGHT)

    # Clip width to available space, never overlapping this Item's title
    content_width = item_rect.width - (ITEM_PADDING * 2)
    title_width = measure_text_cached(self._font, self.title, ITEM_TEXT_FONT_SIZE).x
    right_width = min(content_width - title_width, right_width)

    right_x = item_rect.x + item_rect.width - right_width
    right_y = item_rect.y
    return rl.Rectangle(right_x, right_y, right_width, FROGPILOT_ITEM_BASE_HEIGHT)


class FrogPilotManageControl(ItemAction):
  def __init__(self, initial_state: bool = False, button_text: str | Callable[[], str] = "MANAGE",
               enabled: bool | Callable[[], bool] = True, toggle_callback: Callable[[bool], None] | None = None,
               button_callback: Callable | None = None):
    super().__init__(width=TOGGLE_WIDTH + RIGHT_ITEM_PADDING + BUTTON_WIDTH, enabled=enabled)
    self._toggle_action = ToggleAction(initial_state=initial_state, enabled=enabled, callback=toggle_callback)
    self._button_action = ButtonAction(text=button_text, width=BUTTON_WIDTH, enabled=enabled)
    self._button_callback = button_callback

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self._toggle_action.set_touch_valid_callback(touch_callback)
    self._button_action.set_touch_valid_callback(touch_callback)

  def _render(self, rect: rl.Rectangle):
    manage_enabled = self.enabled and self._toggle_action.get_state()
    self._toggle_action.set_enabled(self.enabled)
    self._button_action.set_enabled(manage_enabled)

    button_rect = rl.Rectangle(rect.x, rect.y, BUTTON_WIDTH, rect.height)
    toggle_rect = rl.Rectangle(rect.x + BUTTON_WIDTH + RIGHT_ITEM_PADDING, rect.y, TOGGLE_WIDTH, rect.height)

    button_pressed = self._button_action.render(button_rect)
    self._toggle_action.render(toggle_rect)
    if button_pressed and self.enabled and self._button_callback is not None:
      self._button_callback()


class FrogPilotMultipleButtonAction(MultipleButtonAction):
  def _get_button_width(self, text: str | Callable[[], str]) -> int:
    text = _resolve_value(text, "")
    text_width = measure_text_cached(self._font, text, 40).x
    return max(self.button_width, int(text_width + (MULTI_BUTTON_TEXT_PADDING * 2)))

  def get_width_hint(self) -> float:
    button_widths = [self._get_button_width(text) for text in self.buttons]
    return sum(button_widths) + max(0, len(button_widths) - 1) * RIGHT_ITEM_PADDING

  def _render(self, rect: rl.Rectangle):
    spacing = RIGHT_ITEM_PADDING
    button_y = rect.y + (rect.height - BUTTON_HEIGHT) / 2
    button_x = rect.x

    for i, _text in enumerate(self.buttons):
      button_width = self._get_button_width(_text)
      button_rect = rl.Rectangle(button_x, button_y, button_width, BUTTON_HEIGHT)

      # Check button state
      mouse_pos = rl.get_mouse_position()
      is_pressed = rl.check_collision_point_rec(mouse_pos, button_rect) and self.enabled and self.is_pressed
      is_selected = i == self.selected_button

      # Button colors
      if is_selected:
        bg_color = rl.Color(51, 171, 76, 255)  # Green
      elif is_pressed:
        bg_color = rl.Color(74, 74, 74, 255)  # Dark gray
      else:
        bg_color = rl.Color(57, 57, 57, 255)  # Gray

      if not self.enabled:
        bg_color = rl.Color(bg_color.r, bg_color.g, bg_color.b, 150)  # Dim

      # Draw button
      rl.draw_rectangle_rounded(button_rect, 1.0, 20, bg_color)

      # Draw text
      text = _resolve_value(_text, "")
      text_size = measure_text_cached(self._font, text, 40)
      text_x = button_x + (button_width - text_size.x) / 2
      text_y = button_y + (BUTTON_HEIGHT - text_size.y) / 2
      text_color = rl.Color(228, 228, 228, 255) if self.enabled else rl.Color(150, 150, 150, 255)
      rl.draw_text_ex(self._font, text, rl.Vector2(text_x, text_y), 40, 0, text_color)
      button_x += button_width + spacing

  def _handle_mouse_release(self, mouse_pos: MousePos):
    spacing = RIGHT_ITEM_PADDING
    button_y = self._rect.y + (self._rect.height - BUTTON_HEIGHT) / 2
    button_x = self._rect.x
    for i, _ in enumerate(self.buttons):
      button_width = self._get_button_width(self.buttons[i])
      button_rect = rl.Rectangle(button_x, button_y, button_width, BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, button_rect):
        if self.selected_button >= 0:
          self.selected_button = i
        if self.callback:
          self.callback(i)
      button_x += button_width + spacing


class FrogPilotButtonToggleAction(ItemAction):
  def __init__(self, buttons: list[str | Callable[[], str]], state_getters: list[Callable[[], bool]],
               state_setters: list[Callable[[bool], None]], button_width: int = BUTTON_WIDTH,
               enabled: bool | Callable[[], bool] = True):
    total_width = button_width * len(buttons) + RIGHT_ITEM_PADDING * max(0, len(buttons) - 1)
    super().__init__(total_width, enabled)
    self.buttons = buttons
    self._state_getters = state_getters
    self._state_setters = state_setters
    self.button_width = button_width
    self._font = gui_app.font(FontWeight.MEDIUM)

  def _get_button_width(self, text: str | Callable[[], str]) -> int:
    text = _resolve_value(text, "")
    text_width = measure_text_cached(self._font, text, 40).x
    return max(self.button_width, int(text_width + (MULTI_BUTTON_TEXT_PADDING * 2)))

  def get_width_hint(self) -> float:
    button_widths = [self._get_button_width(text) for text in self.buttons]
    return sum(button_widths) + max(0, len(button_widths) - 1) * RIGHT_ITEM_PADDING

  def _render(self, rect: rl.Rectangle):
    spacing = RIGHT_ITEM_PADDING
    button_y = rect.y + (rect.height - BUTTON_HEIGHT) / 2
    button_x = rect.x

    for i, _text in enumerate(self.buttons):
      button_width = self._get_button_width(_text)
      button_rect = rl.Rectangle(button_x, button_y, button_width, BUTTON_HEIGHT)
      is_active = self._state_getters[i]()

      if is_active:
        bg_color = rl.Color(51, 171, 76, 255)
      else:
        bg_color = rl.Color(57, 57, 57, 255)

      if not self.enabled:
        bg_color = rl.Color(bg_color.r, bg_color.g, bg_color.b, 150)

      rl.draw_rectangle_rounded(button_rect, 1.0, 20, bg_color)

      text = _resolve_value(_text, "")
      text_size = measure_text_cached(self._font, text, 40)
      text_x = button_x + (button_width - text_size.x) / 2
      text_y = button_y + (BUTTON_HEIGHT - text_size.y) / 2
      text_color = rl.Color(228, 228, 228, 255) if self.enabled else rl.Color(150, 150, 150, 255)
      rl.draw_text_ex(self._font, text, rl.Vector2(text_x, text_y), 40, 0, text_color)
      button_x += button_width + spacing

  def _handle_mouse_release(self, mouse_pos: MousePos):
    spacing = RIGHT_ITEM_PADDING
    button_y = self._rect.y + (self._rect.height - BUTTON_HEIGHT) / 2
    button_x = self._rect.x
    for i in range(len(self.buttons)):
      button_width = self._get_button_width(self.buttons[i])
      button_rect = rl.Rectangle(button_x, button_y, button_width, BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, button_rect) and self.enabled:
        self._state_setters[i](not self._state_getters[i]())
      button_x += button_width + spacing


HOLD_INITIAL_DELAY = 0.3
HOLD_SLOW_INTERVAL = 0.2
HOLD_FAST_INTERVAL = 0.1
HOLD_FAST_THRESHOLD = 5
HOLD_FAST_SNAP = 5


class FrogPilotNumericControl(ItemAction):
  def __init__(self, value_getter: Callable[[], int], value_setter: Callable[[int], None],
               value_formatter: Callable[[int], str] | None = None, min_value: int = 0, max_value: int = 100,
               step: int = 1, enabled: bool | Callable[[], bool] = True):
    super().__init__(width=(NUMERIC_BUTTON_WIDTH * 2) + NUMERIC_VALUE_WIDTH + (RIGHT_ITEM_PADDING * 2), enabled=enabled)
    self._value_getter = value_getter
    self._value_setter = value_setter
    self._value_formatter = value_formatter if value_formatter is not None else lambda value: str(value)
    self._min_value = min_value
    self._max_value = max_value
    self._step = max(1, step)

    self._decrease_button = ButtonAction(text="-", width=NUMERIC_BUTTON_WIDTH, enabled=enabled)
    self._increase_button = ButtonAction(text="+", width=NUMERIC_BUTTON_WIDTH, enabled=enabled)
    self._decrease_button._button._label.set_font_size(NUMERIC_BUTTON_FONT_SIZE)
    self._increase_button._button._label.set_font_size(NUMERIC_BUTTON_FONT_SIZE)
    self._font = gui_app.font(FontWeight.NORMAL)

    self._local_value: int = self._normalize_value(value_getter())

    self._hold_active: ButtonAction | None = None
    self._hold_start_time: float = 0.0
    self._hold_last_repeat_time: float = 0.0
    self._hold_tap_count: int = 0

  def show_event(self):
    self._local_value = self._normalize_value(self._value_getter())

  def hide_event(self):
    self._value_setter(self._local_value)

  def _normalize_value(self, value: int) -> int:
    try:
      return max(self._min_value, min(self._max_value, int(value)))
    except (TypeError, ValueError):
      return self._min_value

  def _render_button(self, button_action: ButtonAction, rect: rl.Rectangle) -> bool:
    button_action._button.set_text(button_action.text)
    button_action._button.set_enabled(button_action.enabled)
    button_action._button.render(rl.Rectangle(rect.x, rect.y + (rect.height - BUTTON_HEIGHT) / 2, rect.width, BUTTON_HEIGHT))

    pressed = button_action._pressed
    button_action._pressed = False
    return pressed

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self._decrease_button.set_touch_valid_callback(touch_callback)
    self._increase_button.set_touch_valid_callback(touch_callback)

  def _check_hold_repeat(self, button: ButtonAction) -> int:
    now = rl.get_time()
    if button._button.is_pressed and button.enabled:
      if self._hold_active is not button:
        self._hold_active = button
        self._hold_start_time = now
        self._hold_last_repeat_time = now
        self._hold_tap_count = 0
        return 0
      if now - self._hold_start_time < HOLD_INITIAL_DELAY:
        return 0
      fast = self._hold_tap_count >= HOLD_FAST_THRESHOLD and self._local_value % HOLD_FAST_SNAP == 0
      interval = HOLD_FAST_INTERVAL if fast else HOLD_SLOW_INTERVAL
      if now - self._hold_last_repeat_time >= interval:
        self._hold_last_repeat_time = now
        return HOLD_FAST_SNAP if fast else self._step
    elif self._hold_active is button:
      self._hold_active = None
    return 0

  def _render(self, rect: rl.Rectangle):
    can_decrease = self.enabled and self._local_value > self._min_value
    can_increase = self.enabled and self._local_value < self._max_value

    self._decrease_button.set_enabled(can_decrease)
    self._increase_button.set_enabled(can_increase)

    available_width = max(0, rect.width)
    max_button_width = max(0, int((available_width - (RIGHT_ITEM_PADDING * 2) - NUMERIC_MIN_GAP_WIDTH) / 2))
    button_width = min(NUMERIC_BUTTON_WIDTH, max_button_width)

    decrease_rect = rl.Rectangle(rect.x, rect.y, button_width, rect.height)
    increase_rect = rl.Rectangle(rect.x + rect.width - button_width, rect.y, button_width, rect.height)
    value_x = decrease_rect.x + decrease_rect.width + RIGHT_ITEM_PADDING
    value_right = increase_rect.x - RIGHT_ITEM_PADDING
    value_width = max(0, value_right - value_x)
    value_rect = rl.Rectangle(value_x, rect.y, value_width, rect.height)

    clicked_decrease = self._render_button(self._decrease_button, decrease_rect)
    clicked_increase = self._render_button(self._increase_button, increase_rect)

    hold_step = self._check_hold_repeat(self._decrease_button)
    if (clicked_decrease or hold_step) and can_decrease:
      step = hold_step if hold_step else self._step
      self._local_value = self._normalize_value(self._local_value - step)
      self._hold_tap_count += 1

    hold_step = self._check_hold_repeat(self._increase_button)
    if (clicked_increase or hold_step) and can_increase:
      step = hold_step if hold_step else self._step
      self._local_value = self._normalize_value(self._local_value + step)
      self._hold_tap_count += 1

    value_text = self._value_formatter(self._local_value)
    if value_rect.width > 0:
      text_size = measure_text_cached(self._font, value_text, ITEM_TEXT_FONT_SIZE)
      text_pos = rl.Vector2(
        value_rect.x + (value_rect.width - text_size.x) / 2,
        value_rect.y + (value_rect.height - text_size.y) / 2,
      )
      value_color = ITEM_TEXT_VALUE_COLOR if self.enabled else rl.Color(150, 150, 150, 255)
      rl.draw_text_ex(self._font, value_text, text_pos, ITEM_TEXT_FONT_SIZE, 0, value_color)


class FrogPilotNumericWithButtonControl(ItemAction):
  def __init__(self, value_getter: Callable[[], int], value_setter: Callable[[int], None],
               value_formatter: Callable[[int], str] | None = None, min_value: int = 0, max_value: int = 100,
               step: int = 1, button_text: str | Callable[[], str] = "Test",
               button_callback: Callable | None = None, enabled: bool | Callable[[], bool] = True):
    super().__init__(
      width=BUTTON_WIDTH + RIGHT_ITEM_PADDING + (NUMERIC_BUTTON_WIDTH * 2) + NUMERIC_VALUE_WIDTH + (RIGHT_ITEM_PADDING * 2),
      enabled=enabled,
    )
    self._numeric = FrogPilotNumericControl(
      value_getter=value_getter, value_setter=value_setter, value_formatter=value_formatter,
      min_value=min_value, max_value=max_value, step=step, enabled=enabled,
    )
    self._button = ButtonAction(text=button_text, width=BUTTON_WIDTH, enabled=enabled)
    self._button_callback = button_callback

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self._numeric.set_touch_valid_callback(touch_callback)
    self._button.set_touch_valid_callback(touch_callback)

  def _render(self, rect: rl.Rectangle):
    self._numeric.set_enabled(self.enabled)
    self._button.set_enabled(self.enabled)

    button_rect = rl.Rectangle(rect.x, rect.y, BUTTON_WIDTH, rect.height)
    numeric_rect = rl.Rectangle(
      rect.x + BUTTON_WIDTH + RIGHT_ITEM_PADDING, rect.y,
      rect.width - BUTTON_WIDTH - RIGHT_ITEM_PADDING, rect.height,
    )
    if self._button.render(button_rect) and self.enabled and self._button_callback:
      self._button_callback()
    self._numeric.render(numeric_rect)


def frogpilot_manage_control_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None, initial_state: bool = False,
                                  toggle_callback: Callable[[bool], None] | None = None, button_text: str | Callable[[], str] = "MANAGE",
                                  button_callback: Callable | None = None, icon: str = "", enabled: bool | Callable[[], bool] = True) -> ListItem:
  action = FrogPilotManageControl(
    initial_state=initial_state,
    button_text=button_text,
    enabled=enabled,
    toggle_callback=toggle_callback,
    button_callback=button_callback,
  )
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)


def frogpilot_multiple_button_item(title: str | Callable[[], str], description: str | Callable[[], str], buttons: list[str | Callable[[], str]], selected_index: int,
                                   button_width: int = BUTTON_WIDTH, callback: Callable | None = None, icon: str = ""):
  action = FrogPilotMultipleButtonAction(buttons, button_width, selected_index, callback=callback)
  return FrogPilotListItem(title=title, description=description, icon=icon, action_item=action)


def frogpilot_numeric_control_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                                   value_getter: Callable[[], int] | None = None, value_setter: Callable[[int], None] | None = None,
                                   value_formatter: Callable[[int], str] | None = None, min_value: int = 0, max_value: int = 100,
                                   step: int = 1, icon: str = "", enabled: bool | Callable[[], bool] = True) -> ListItem:
  if value_getter is None or value_setter is None:
    raise ValueError("frogpilot_numeric_control_item requires both value_getter and value_setter")

  action = FrogPilotNumericControl(
    value_getter=value_getter,
    value_setter=value_setter,
    value_formatter=value_formatter,
    min_value=min_value,
    max_value=max_value,
    step=step,
    enabled=enabled,
  )
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)


def frogpilot_toggle_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None, initial_state: bool = False,
                          callback: Callable | None = None, icon: str = "",
                          enabled: bool | Callable[[], bool] = True) -> FrogPilotListItem:
  action = ToggleAction(initial_state=initial_state, enabled=enabled, callback=callback)
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)


def frogpilot_button_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                           button_text: str | Callable[[], str] = "SELECT", callback: Callable | None = None,
                           icon: str = "", enabled: bool | Callable[[], bool] = True) -> FrogPilotListItem:
  action = ButtonAction(text=button_text, width=BUTTON_WIDTH, enabled=enabled)
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon, callback=callback)


def frogpilot_button_param_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                                 button_text: str | Callable[[], str] = "SELECT",
                                 value_getter: str | Callable[[], str] | None = None, callback: Callable | None = None,
                                 icon: str = "", enabled: bool | Callable[[], bool] = True) -> FrogPilotListItem:
  action = ButtonAction(text=button_text, width=BUTTON_WIDTH, enabled=enabled)
  if value_getter is not None:
    action.set_value(value_getter)
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon, callback=callback)


def frogpilot_button_toggle_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                                  buttons: list[str | Callable[[], str]] | None = None,
                                  state_getters: list[Callable[[], bool]] | None = None,
                                  state_setters: list[Callable[[bool], None]] | None = None,
                                  button_width: int = BUTTON_WIDTH, icon: str = "",
                                  enabled: bool | Callable[[], bool] = True) -> FrogPilotListItem:
  action = FrogPilotButtonToggleAction(
    buttons=buttons or [], state_getters=state_getters or [],
    state_setters=state_setters or [], button_width=button_width, enabled=enabled,
  )
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)


def frogpilot_label_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                          value_getter: str | Callable[[], str] = "", icon: str = "") -> FrogPilotListItem:
  action = TextAction(text=value_getter, color=ITEM_TEXT_VALUE_COLOR)
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)


def frogpilot_numeric_with_button_item(title: str | Callable[[], str], description: str | Callable[[], str] | None = None,
                                        value_getter: Callable[[], int] | None = None, value_setter: Callable[[int], None] | None = None,
                                        value_formatter: Callable[[int], str] | None = None, min_value: int = 0, max_value: int = 100,
                                        step: int = 1, button_text: str | Callable[[], str] = "Test",
                                        button_callback: Callable | None = None, icon: str = "",
                                        enabled: bool | Callable[[], bool] = True) -> FrogPilotListItem:
  if value_getter is None or value_setter is None:
    raise ValueError("frogpilot_numeric_with_button_item requires both value_getter and value_setter")

  action = FrogPilotNumericWithButtonControl(
    value_getter=value_getter, value_setter=value_setter, value_formatter=value_formatter,
    min_value=min_value, max_value=max_value, step=step, button_text=button_text,
    button_callback=button_callback, enabled=enabled,
  )
  return FrogPilotListItem(title=title, description=description, action_item=action, icon=icon)
