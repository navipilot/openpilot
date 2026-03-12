import os
import pyray as rl
from collections.abc import Callable
from abc import ABC
from openpilot.common.params import Params
from openpilot.system.ui.lib.application import gui_app, FontWeight, MousePos
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.widgets import Widget
from openpilot.system.ui.widgets.button import Button, ButtonStyle
from openpilot.system.ui.widgets.toggle import Toggle, WIDTH as TOGGLE_WIDTH, HEIGHT as TOGGLE_HEIGHT
from openpilot.system.ui.widgets.label import gui_label
from openpilot.system.ui.widgets.html_render import HtmlRenderer, ElementType
from openpilot.common.filter_simple import FirstOrderFilter

ITEM_BASE_WIDTH = 600
ITEM_BASE_HEIGHT = 170
ITEM_PADDING = 20
ITEM_TEXT_FONT_SIZE = 50
ITEM_TEXT_COLOR = rl.WHITE
ITEM_TEXT_VALUE_COLOR = rl.Color(170, 170, 170, 255)
ITEM_DESC_TEXT_COLOR = rl.Color(128, 128, 128, 255)
ITEM_DESC_FONT_SIZE = 40
ITEM_DESC_V_OFFSET = 140
RIGHT_ITEM_PADDING = 20
ICON_SIZE = 80
BUTTON_WIDTH = 250
BUTTON_HEIGHT = 100
BUTTON_BORDER_RADIUS = 50
BUTTON_FONT_SIZE = 35
BUTTON_FONT_WEIGHT = FontWeight.MEDIUM

TEXT_PADDING = 20


def _resolve_value(value, default=""):
  if callable(value):
    return value()
  return value if value is not None else default


# Abstract base class for right-side items
class ItemAction(Widget, ABC):
  def __init__(self, width: int = BUTTON_HEIGHT, enabled: bool | Callable[[], bool] = True):
    super().__init__()
    self.set_rect(rl.Rectangle(0, 0, width, 0))
    self._enabled_source = enabled

  def get_width_hint(self) -> float:
    # Return's action ideal width, 0 means use full width
    return self._rect.width

  def set_enabled(self, enabled: bool | Callable[[], bool]):
    self._enabled_source = enabled

  @property
  def enabled(self):
    return _resolve_value(self._enabled_source, False)


class ToggleAction(ItemAction):
  def __init__(
    self, initial_state: bool = False, width: int = TOGGLE_WIDTH, enabled: bool | Callable[[], bool] = True, callback: Callable[[bool], None] | None = None
  ):
    super().__init__(width, enabled)
    self.toggle = Toggle(initial_state=initial_state, callback=callback)

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self.toggle.set_touch_valid_callback(touch_callback)

  def _render(self, rect: rl.Rectangle) -> bool:
    self.toggle.set_enabled(self.enabled)
    clicked = self.toggle.render(rl.Rectangle(rect.x, rect.y + (rect.height - TOGGLE_HEIGHT) / 2, self._rect.width, TOGGLE_HEIGHT))
    return bool(clicked)

  def set_state(self, state: bool):
    self.toggle.set_state(state)

  def get_state(self) -> bool:
    return self.toggle.get_state()


class ButtonAction(ItemAction):
  def __init__(self, text: str | Callable[[], str], width: int = BUTTON_WIDTH, enabled: bool | Callable[[], bool] = True):
    super().__init__(width, enabled)
    self._text_source = text
    self._value_source: str | Callable[[], str] | None = None
    self._pressed = False
    self._font = gui_app.font(FontWeight.NORMAL)

    def pressed():
      self._pressed = True

    self._button = Button(
      self.text,
      font_size=BUTTON_FONT_SIZE,
      font_weight=BUTTON_FONT_WEIGHT,
      button_style=ButtonStyle.LIST_ACTION,
      border_radius=BUTTON_BORDER_RADIUS,
      click_callback=pressed,
      text_padding=0,
    )
    self.set_enabled(enabled)

  def get_width_hint(self) -> float:
    value_text = self.value
    if value_text:
      text_width = measure_text_cached(self._font, value_text, ITEM_TEXT_FONT_SIZE).x
      return text_width + BUTTON_WIDTH + TEXT_PADDING
    else:
      return BUTTON_WIDTH

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self._button.set_touch_valid_callback(touch_callback)

  def set_text(self, text: str | Callable[[], str]):
    self._text_source = text

  def set_value(self, value: str | Callable[[], str]):
    self._value_source = value

  @property
  def text(self):
    return _resolve_value(self._text_source, tr("Error"))

  @property
  def value(self):
    return _resolve_value(self._value_source, "")

  def _render(self, rect: rl.Rectangle) -> bool:
    self._button.set_text(self.text)
    self._button.set_enabled(_resolve_value(self.enabled))
    button_rect = rl.Rectangle(rect.x + rect.width - BUTTON_WIDTH, rect.y + (rect.height - BUTTON_HEIGHT) / 2, BUTTON_WIDTH, BUTTON_HEIGHT)
    self._button.render(button_rect)

    value_text = self.value
    if value_text:
      value_rect = rl.Rectangle(rect.x, rect.y, rect.width - BUTTON_WIDTH - TEXT_PADDING, rect.height)
      gui_label(
        value_rect,
        value_text,
        font_size=ITEM_TEXT_FONT_SIZE,
        color=ITEM_TEXT_VALUE_COLOR,
        font_weight=FontWeight.NORMAL,
        alignment=rl.GuiTextAlignment.TEXT_ALIGN_LEFT,
        alignment_vertical=rl.GuiTextAlignmentVertical.TEXT_ALIGN_MIDDLE,
      )

    # TODO: just use the generic Widget click callbacks everywhere, no returning from render
    pressed = self._pressed
    self._pressed = False
    return pressed


class TextAction(ItemAction):
  def __init__(self, text: str | Callable[[], str], color: rl.Color = ITEM_TEXT_COLOR, enabled: bool | Callable[[], bool] = True):
    self._text_source = text
    self.color = color

    self._font = gui_app.font(FontWeight.NORMAL)
    initial_text = _resolve_value(text, "")
    text_width = measure_text_cached(self._font, initial_text, ITEM_TEXT_FONT_SIZE).x
    super().__init__(int(text_width + TEXT_PADDING), enabled)

  @property
  def text(self):
    return _resolve_value(self._text_source, tr("Error"))

  def get_width_hint(self) -> float:
    text_width = measure_text_cached(self._font, self.text, ITEM_TEXT_FONT_SIZE).x
    return text_width + TEXT_PADDING

  def _render(self, rect: rl.Rectangle) -> bool:
    gui_label(
      self._rect,
      self.text,
      font_size=ITEM_TEXT_FONT_SIZE,
      color=self.color,
      font_weight=FontWeight.NORMAL,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_RIGHT,
      alignment_vertical=rl.GuiTextAlignmentVertical.TEXT_ALIGN_MIDDLE,
    )
    return False

  def set_text(self, text: str | Callable[[], str]):
    self._text_source = text


class DualButtonAction(ItemAction):
  def __init__(
    self,
    left_text: str | Callable[[], str],
    right_text: str | Callable[[], str],
    left_callback: Callable = None,
    right_callback: Callable = None,
    enabled: bool | Callable[[], bool] = True,
  ):
    super().__init__(width=0, enabled=enabled)  # Width 0 means use full width
    self.left_button = Button(left_text, click_callback=left_callback, button_style=ButtonStyle.NORMAL, text_padding=0)
    self.right_button = Button(right_text, click_callback=right_callback, button_style=ButtonStyle.DANGER, text_padding=0)

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    self.left_button.set_touch_valid_callback(touch_callback)
    self.right_button.set_touch_valid_callback(touch_callback)

  def _render(self, rect: rl.Rectangle):
    button_spacing = 30
    button_height = 120
    button_width = (rect.width - button_spacing) / 2
    button_y = rect.y + (rect.height - button_height) / 2

    left_rect = rl.Rectangle(rect.x, button_y, button_width, button_height)
    right_rect = rl.Rectangle(rect.x + button_width + button_spacing, button_y, button_width, button_height)

    # expand one to full width if other is not visible
    if not self.left_button.is_visible:
      right_rect.x = rect.x
      right_rect.width = rect.width
    elif not self.right_button.is_visible:
      left_rect.width = rect.width

    # Render buttons
    self.left_button.render(left_rect)
    self.right_button.render(right_rect)


class MultipleButtonAction(ItemAction):
  def __init__(self, buttons: list[str | Callable[[], str]], button_width: int, selected_index: int = 0, callback: Callable = None):
    super().__init__(width=len(buttons) * button_width + (len(buttons) - 1) * RIGHT_ITEM_PADDING, enabled=True)
    self.buttons = buttons
    self.button_width = button_width
    self.selected_button = selected_index
    self.callback = callback
    self._font = gui_app.font(FontWeight.MEDIUM)

  def set_selected_button(self, index: int):
    if 0 <= index < len(self.buttons):
      self.selected_button = index

  def get_selected_button(self) -> int:
    return self.selected_button

  def _render(self, rect: rl.Rectangle):
    spacing = RIGHT_ITEM_PADDING
    button_y = rect.y + (rect.height - BUTTON_HEIGHT) / 2

    for i, _text in enumerate(self.buttons):
      button_x = rect.x + i * (self.button_width + spacing)
      button_rect = rl.Rectangle(button_x, button_y, self.button_width, BUTTON_HEIGHT)

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
      text_x = button_x + (self.button_width - text_size.x) / 2
      text_y = button_y + (BUTTON_HEIGHT - text_size.y) / 2
      text_color = rl.Color(228, 228, 228, 255) if self.enabled else rl.Color(150, 150, 150, 255)
      rl.draw_text_ex(self._font, text, rl.Vector2(text_x, text_y), 40, 0, text_color)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    spacing = RIGHT_ITEM_PADDING
    button_y = self._rect.y + (self._rect.height - BUTTON_HEIGHT) / 2
    for i, _ in enumerate(self.buttons):
      button_x = self._rect.x + i * (self.button_width + spacing)
      button_rect = rl.Rectangle(button_x, button_y, self.button_width, BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, button_rect):
        self.selected_button = i
        if self.callback:
          self.callback(i)


class CategoryButtonsAction(ItemAction):
  def __init__(self, buttons: list[tuple[str | Callable[[], str], Callable]], button_width: int = 150, enabled: bool | Callable[[], bool] = True):
    # Calculate width_hint based on actual text content (matches _render logic)
    padding = 20  # 10px per side
    total_text_width = 0

    for label, _ in buttons:
      text = _resolve_value(label, "")
      text_size = measure_text_cached(gui_app.font(FontWeight.MEDIUM), text, BUTTON_FONT_SIZE)
      total_text_width += text_size.x + padding

    total_width = total_text_width + (len(buttons) - 1) * RIGHT_ITEM_PADDING

    super().__init__(width=total_width, enabled=enabled)
    self.buttons = buttons
    self._font = gui_app.font(FontWeight.MEDIUM)

  def _render(self, rect: rl.Rectangle):
    spacing = RIGHT_ITEM_PADDING
    button_y = rect.y + (rect.height - BUTTON_HEIGHT) / 2

    # Calculate per-button width based on text + padding, scaled to fit
    padding = 20  # 10px per side
    button_widths = []
    ideal_total = 0

    for label, _ in self.buttons:
      text = _resolve_value(label, "")
      text_size = measure_text_cached(self._font, text, BUTTON_FONT_SIZE)
      ideal_width = text_size.x + padding
      button_widths.append(ideal_width)
      ideal_total += ideal_width

    ideal_total += (len(self.buttons) - 1) * spacing

    # Scale proportionally if total exceeds available space
    if ideal_total > rect.width:
      scale = rect.width / ideal_total
      button_widths = [max(80, w * scale) for w in button_widths]
    else:
      # Ensure minimum width
      button_widths = [max(80, w) for w in button_widths]

    # Start from left edge of rect - use cumulative sum for correct positioning
    current_button_x = rect.x

    for i, (label, _) in enumerate(self.buttons):
      btn_w = button_widths[i]
      button_rect = rl.Rectangle(current_button_x, button_y, btn_w, BUTTON_HEIGHT)

      # Check button state
      mouse_pos = rl.get_mouse_position()
      is_pressed = rl.check_collision_point_rec(mouse_pos, button_rect) and self.enabled and self.is_pressed

      bg_color = rl.Color(57, 57, 57, 255)  # Gray
      if is_pressed:
        bg_color = rl.Color(74, 74, 74, 255)  # Dark gray
      if not self.enabled:
        bg_color = rl.Color(bg_color.r, bg_color.g, bg_color.b, 150)  # Dim

      rl.draw_rectangle_rounded(button_rect, 1.0, 20, bg_color)

      # Draw text with proper centering
      text = _resolve_value(label, "")
      text_size = measure_text_cached(self._font, text, BUTTON_FONT_SIZE)
      text_x = current_button_x + (btn_w - text_size.x) / 2
      text_y = button_y + (BUTTON_HEIGHT - text_size.y) / 2
      text_color = rl.Color(228, 228, 228, 255) if self.enabled else rl.Color(150, 150, 150, 255)
      rl.draw_text_ex(self._font, text, rl.Vector2(text_x, text_y), BUTTON_FONT_SIZE, 0, text_color)

      # Advance position for next button
      current_button_x += btn_w + spacing

  def _handle_mouse_release(self, mouse_pos: MousePos):
    spacing = RIGHT_ITEM_PADDING
    button_y = self._rect.y + (self._rect.height - BUTTON_HEIGHT) / 2

    # Calculate per-button width based on text + padding, scaled to fit (same as _render)
    padding = 20  # 10px per side
    button_widths = []
    ideal_total = 0

    for label, _ in self.buttons:
      text = _resolve_value(label, "")
      text_size = measure_text_cached(self._font, text, BUTTON_FONT_SIZE)
      ideal_width = text_size.x + padding
      button_widths.append(ideal_width)
      ideal_total += ideal_width

    ideal_total += (len(self.buttons) - 1) * spacing

    # Scale proportionally if total exceeds available space
    if ideal_total > self._rect.width:
      scale = self._rect.width / ideal_total
      button_widths = [max(80, w * scale) for w in button_widths]
    else:
      # Ensure minimum width
      button_widths = [max(80, w) for w in button_widths]

    # Start from left edge of rect - use cumulative sum for correct positioning
    current_button_x = self._rect.x

    for i, (_, callback) in enumerate(self.buttons):
      btn_w = button_widths[i]
      button_rect = rl.Rectangle(current_button_x, button_y, btn_w, BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, button_rect):
        if callback:
          callback()
      # Advance position for next button
      current_button_x += btn_w + spacing


def category_buttons_item(
  title: str | Callable[[], str],
  buttons: list[tuple[str | Callable[[], str], Callable]],
  description: str | Callable[[], str] | None = None,
  icon: str = "",
  button_width: int = BUTTON_WIDTH,
  enabled: bool | Callable[[], bool] = True,
  starpilot_icon: bool = False,
) -> "ListItem":
  action = CategoryButtonsAction(buttons, button_width, enabled=enabled)
  icon_to_use = "" if starpilot_icon else icon
  item = ListItem(title=title, description=description, icon=icon_to_use, action_item=action)
  if icon and starpilot_icon:
    item.set_icon(icon, starpilot=True)
  return item


class ListItem(Widget):
  def __init__(
    self,
    title: str | Callable[[], str] = "",
    icon: str | None = None,
    description: str | Callable[[], str] | None = None,
    description_visible: bool = False,
    callback: Callable | None = None,
    action_item: ItemAction | None = None,
  ):
    super().__init__()
    self._title = title
    self.set_icon(icon)
    self._description = description
    self.description_visible = description_visible
    self.set_click_callback(callback)
    self.description_opened_callback: Callable | None = None
    self.action_item = action_item

    self.set_rect(rl.Rectangle(0, 0, ITEM_BASE_WIDTH, ITEM_BASE_HEIGHT))
    self._font = gui_app.font(FontWeight.NORMAL)

    self._html_renderer = HtmlRenderer(text="", text_size={ElementType.P: ITEM_DESC_FONT_SIZE}, text_color=ITEM_DESC_TEXT_COLOR)
    self._parse_description(self.description)

    # Cached properties for performance
    self._prev_description: str | None = self.description

  def show_event(self):
    self._set_description_visible(False)

  def set_description_opened_callback(self, callback: Callable) -> None:
    self.description_opened_callback = callback

  def set_touch_valid_callback(self, touch_callback: Callable[[], bool]) -> None:
    super().set_touch_valid_callback(touch_callback)
    if self.action_item:
      self.action_item.set_touch_valid_callback(touch_callback)

  def set_parent_rect(self, parent_rect: rl.Rectangle):
    super().set_parent_rect(parent_rect)
    self._rect.width = parent_rect.width

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if not self.is_visible:
      return

    # Check not in action rect
    if self.action_item:
      action_rect = self.get_right_item_rect(self._rect)
      if rl.check_collision_point_rec(mouse_pos, action_rect):
        # Click was on right item, don't toggle description
        return

    self._set_description_visible(not self.description_visible)
    super()._handle_mouse_release(mouse_pos)

  def _set_description_visible(self, visible: bool):
    if self.description and self.description_visible != visible:
      self.description_visible = visible
      # do callback first in case receiver changes description
      if self.description_visible and self.description_opened_callback is not None:
        self.description_opened_callback()
        # Call _update_state to catch any description changes
        self._update_state()

      content_width = int(self._rect.width - ITEM_PADDING * 2)
      self._rect.height = self.get_item_height(self._font, content_width)

  def _update_state(self):
    # Detect changes if description is callback
    new_description = self.description
    if new_description != self._prev_description:
      self._parse_description(new_description)

  def _render(self, _):
    if not self.is_visible:
      return

    # Don't draw items that are not in parent's viewport
    if (self._rect.y + self.rect.height) <= self._parent_rect.y or self._rect.y >= (self._parent_rect.y + self._parent_rect.height):
      return

    content_x = self._rect.x + ITEM_PADDING
    text_x = content_x

    # Only draw title and icon for items that have them
    if self.title:
      # Draw icon if present
      if self.icon:
        rl.draw_texture(self._icon_texture, int(content_x), int(self._rect.y + (ITEM_BASE_HEIGHT - self._icon_texture.height) // 2), rl.WHITE)
        text_x += ICON_SIZE + ITEM_PADDING

      # Draw main text
      text_size = measure_text_cached(self._font, self.title, ITEM_TEXT_FONT_SIZE)
      item_y = self._rect.y + (ITEM_BASE_HEIGHT - text_size.y) // 2
      rl.draw_text_ex(self._font, self.title, rl.Vector2(text_x, item_y), ITEM_TEXT_FONT_SIZE, 0, ITEM_TEXT_COLOR)

    # Draw description if visible
    if self.description_visible:
      content_width = int(self._rect.width - ITEM_PADDING * 2)
      description_height = self._html_renderer.get_total_height(content_width)
      description_rect = rl.Rectangle(self._rect.x + ITEM_PADDING, self._rect.y + ITEM_DESC_V_OFFSET, content_width, description_height)
      self._html_renderer.render(description_rect)

    # Draw right item if present
    if self.action_item:
      right_rect = self.get_right_item_rect(self._rect)
      right_rect.y = self._rect.y
      if self.action_item.render(right_rect) and self.action_item.enabled:
        # Right item was clicked/activated
        if self._click_callback:
          self._click_callback()

  def set_icon(self, icon: str | None, starpilot: bool = False):
    self.icon = icon
    if not icon:
      self._icon_texture = None
    elif starpilot:
      self._icon_texture = gui_app.starpilot_texture(icon, ICON_SIZE, ICON_SIZE)
    elif self.icon:
      self._icon_texture = gui_app.texture(os.path.join("icons", self.icon), ICON_SIZE, ICON_SIZE)
    else:
      self._icon_texture = None

  def set_description(self, description: str | Callable[[], str] | None):
    self._description = description

  def _parse_description(self, new_desc):
    self._html_renderer.parse_html_content(new_desc)
    self._prev_description = new_desc

  @property
  def title(self):
    return _resolve_value(self._title, "")

  @property
  def description(self):
    return _resolve_value(self._description, "")

  def get_item_height(self, font: rl.Font, max_width: int) -> float:
    if not self.is_visible:
      return 0

    height = float(ITEM_BASE_HEIGHT)
    if self.description_visible:
      description_height = self._html_renderer.get_total_height(max_width)
      height += description_height - (ITEM_BASE_HEIGHT - ITEM_DESC_V_OFFSET) + ITEM_PADDING
    return height

  def get_right_item_rect(self, item_rect: rl.Rectangle) -> rl.Rectangle:
    if not self.action_item:
      return rl.Rectangle(0, 0, 0, 0)

    right_width = self.action_item.get_width_hint()
    if right_width == 0:  # Full width action (like DualButtonAction)
      return rl.Rectangle(item_rect.x + ITEM_PADDING, item_rect.y, item_rect.width - (ITEM_PADDING * 2), ITEM_BASE_HEIGHT)

    # Return rect at right edge of item, with action's full width
    # The action itself will handle positioning within this rect (right-aligned)
    right_x = item_rect.x + item_rect.width - right_width - ITEM_PADDING
    right_y = item_rect.y
    return rl.Rectangle(right_x, right_y, right_width, ITEM_BASE_HEIGHT)


# Factory functions
def simple_item(title: str | Callable[[], str], callback: Callable | None = None) -> ListItem:
  return ListItem(title=title, callback=callback)


def toggle_item(
  title: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  initial_state: bool = False,
  callback: Callable | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  starpilot_icon: bool = False,
) -> ListItem:
  action = ToggleAction(initial_state=initial_state, enabled=enabled, callback=callback)
  icon_to_use = "" if starpilot_icon else icon
  item = ListItem(title=title, description=description, action_item=action, icon=icon_to_use)
  if icon and starpilot_icon:
    item.set_icon(icon, starpilot=True)
  return item


def button_item(
  title: str | Callable[[], str],
  button_text: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  callback: Callable | None = None,
  enabled: bool | Callable[[], bool] = True,
  icon: str = "",
  starpilot_icon: bool = False,
) -> ListItem:
  action = ButtonAction(text=button_text, enabled=enabled)
  item = ListItem(title=title, description=description, action_item=action, callback=callback, icon=icon if not starpilot_icon else "")
  if icon and starpilot_icon:
    item.set_icon(icon, starpilot=True)
  return item


def text_item(
  title: str | Callable[[], str],
  value: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  callback: Callable | None = None,
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  action = TextAction(text=value, color=ITEM_TEXT_VALUE_COLOR, enabled=enabled)
  return ListItem(title=title, description=description, action_item=action, callback=callback)


def dual_button_item(
  left_text: str | Callable[[], str],
  right_text: str | Callable[[], str],
  left_callback: Callable = None,
  right_callback: Callable = None,
  description: str | Callable[[], str] | None = None,
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  action = DualButtonAction(left_text, right_text, left_callback, right_callback, enabled)
  return ListItem(title="", description=description, action_item=action)


def multiple_button_item(
  title: str | Callable[[], str],
  description: str | Callable[[], str],
  buttons: list[str | Callable[[], str]],
  selected_index: int,
  button_width: int = BUTTON_WIDTH,
  callback: Callable = None,
  icon: str = "",
  starpilot_icon: bool = False,
):
  action = MultipleButtonAction(buttons, button_width, selected_index, callback=callback)
  item = ListItem(title=title, description=description, icon=icon if not starpilot_icon else "", action_item=action)
  if icon and starpilot_icon:
    item.set_icon(icon, starpilot=True)
  return item


VALUE_BUTTON_WIDTH = 150
VALUE_BUTTON_HEIGHT = 80
VALUE_SLIDER_HEIGHT = 100
VALUE_DISPLAY_WIDTH = 250
VALUE_FONT_SIZE = 45
VALUE_TEXT_COLOR = rl.Color(224, 232, 121, 255)


class ValueSliderAction(ItemAction):
  def __init__(
    self,
    value: float | Callable[[], float],
    min_val: float = 0,
    max_val: float = 100,
    step: float = 1,
    unit: str = "",
    labels: dict[float, str] | None = None,
    callback: Callable[[float], None] | None = None,
    enabled: bool | Callable[[], bool] = True,
    negative: bool = False,
    default_value: float | None = None,
    fast_increase: bool = False,
    is_metric: bool = False,
  ):
    self._params = Params()
    self._value_source = value
    self._min_val = min_val
    self._max_val = max_val
    self._step = step
    self._unit = unit
    self._labels = labels or {}
    self._callback = callback
    self._negative = negative
    self._default_value = default_value
    self._fast_increase = fast_increase
    self._is_metric = is_metric

    self._font = gui_app.font(FontWeight.MEDIUM)
    self._value_font = gui_app.font(FontWeight.DISPLAY)

    self._decrement_pressed = False
    self._increment_pressed = False
    self._repeat_timer = 0.0
    self._repeat_delay = 0.5
    self._repeat_interval = 0.1

    self._params = Params()

    self._metric_multiplier = 1.0
    if self._is_metric:
      if self._unit == "mph":
        self._metric_multiplier = 1.60934
      elif self._unit == "feet":
        self._metric_multiplier = 0.3048
      elif self._unit == "inches":
        self._metric_multiplier = 2.54

    total_width = VALUE_DISPLAY_WIDTH + VALUE_BUTTON_WIDTH * 2 + 20
    super().__init__(width=total_width, enabled=enabled)

  def _get_value(self) -> float:
    value = self._value_source
    if callable(value):
      return float(value())
    if isinstance(value, (int, float)):
      return float(value)
    if isinstance(value, str) and value:
      param_key = value
      if self._step == 1:
        return float(self._params.get_int(param_key, return_default=True, default=0))
      else:
        return float(self._params.get_float(param_key, return_default=True, default=0.0))
    return 0.0

  def _get_display_text(self, value: float) -> str:
    display_val = value
    display_unit = self._unit

    if self._is_metric and ui_state.is_metric and self._metric_multiplier != 1.0:
      display_val = value * self._metric_multiplier
      if self._unit == "mph":
        display_unit = "km/h"
      elif self._unit == "feet":
        display_unit = "m"
      elif self._unit == "inches":
        display_unit = "cm"

    rounded_value = round(display_val / self._step) * self._step
    if self._labels and rounded_value in self._labels:
      return self._labels[rounded_value]
    if display_unit:
      return f"{rounded_value:g}{display_unit}"
    return str(rounded_value)

  def _update_value(self, delta: float):
    current = self._get_value()

    if self._is_metric and ui_state.is_metric and self._metric_multiplier != 1.0:
      delta = delta / self._metric_multiplier

    min_val = _resolve_value(self._min_val, 0)
    max_val = _resolve_value(self._max_val, 100)
    new_value = max(min_val, min(max_val, current + delta))
    new_value = round(new_value / self._step) * self._step

    if self._callback:
      self._callback(new_value)
    elif isinstance(self._value_source, str):
      param_key = self._value_source
      if param_key:
        if self._step == 1:
          self._params.put_int(param_key, int(new_value))
        else:
          self._params.put_float(param_key, new_value)

  def _handle_decrement(self, dt: float):
    if self._decrement_pressed:
      self._repeat_timer += dt
      if self._repeat_timer >= self._repeat_delay:
        repeat_count = int((self._repeat_timer - self._repeat_delay) / self._repeat_interval) + 1
        delta = -self._step
        if self._fast_increase:
          delta *= 5
        for _ in range(repeat_count):
          self._update_value(delta)
        self._repeat_timer = self._repeat_timer % self._repeat_interval
      return True
    return False

  def _handle_increment(self, dt: float):
    if self._increment_pressed:
      self._repeat_timer += dt
      if self._repeat_timer >= self._repeat_delay:
        repeat_count = int((self._repeat_timer - self._repeat_delay) / self._repeat_interval) + 1
        delta = self._step
        if self._fast_increase:
          delta *= 5
        for _ in range(repeat_count):
          self._update_value(delta)
        self._repeat_timer = self._repeat_timer % self._repeat_interval
      return True
    return False

  def get_width_hint(self) -> float:
    return self._rect.width

  def _render(self, rect: rl.Rectangle) -> bool:
    value = self._get_value()
    display_text = self._get_display_text(value)

    button_y = rect.y + (rect.height - VALUE_BUTTON_HEIGHT) / 2

    dec_btn_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH * 2 - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)

    min_val = _resolve_value(self._min_val, 0)
    max_val = _resolve_value(self._max_val, 100)
    dec_color = rl.Color(57, 57, 57, 255) if value > min_val else rl.Color(40, 40, 40, 255)
    inc_color = rl.Color(57, 57, 57, 255) if value < max_val else rl.Color(40, 40, 40, 255)

    if self._decrement_pressed:
      dec_color = rl.Color(74, 74, 74, 255)
    if self._increment_pressed:
      inc_color = rl.Color(74, 74, 74, 255)

    if not self.enabled:
      dec_color = rl.Color(dec_color.r, dec_color.g, dec_color.b, 128)
      inc_color = rl.Color(inc_color.r, inc_color.g, inc_color.b, 128)

    rl.draw_rectangle_rounded(dec_btn_rect, 0.3, 10, dec_color)
    rl.draw_rectangle_rounded(inc_btn_rect, 0.3, 10, inc_color)

    dec_text = "-"
    inc_text = "+"
    dec_size = measure_text_cached(self._value_font, dec_text, 50)
    inc_size = measure_text_cached(self._value_font, inc_text, 50)
    rl.draw_text_ex(
      self._value_font,
      dec_text,
      rl.Vector2(dec_btn_rect.x + (dec_btn_rect.width - dec_size.x) / 2, dec_btn_rect.y + (dec_btn_rect.height - dec_size.y) / 2),
      50,
      0,
      rl.WHITE,
    )
    rl.draw_text_ex(
      self._value_font,
      inc_text,
      rl.Vector2(inc_btn_rect.x + (inc_btn_rect.width - inc_size.x) / 2, inc_btn_rect.y + (inc_btn_rect.height - inc_size.y) / 2),
      50,
      0,
      rl.WHITE,
    )

    value_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH * 2 - VALUE_DISPLAY_WIDTH - 30, rect.y, VALUE_DISPLAY_WIDTH, rect.height)
    gui_label(
      value_rect,
      display_text,
      font_size=VALUE_FONT_SIZE,
      color=VALUE_TEXT_COLOR if self.enabled else rl.Color(100, 100, 100, 255),
      font_weight=FontWeight.DISPLAY,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_RIGHT,
      alignment_vertical=rl.GuiTextAlignmentVertical.TEXT_ALIGN_MIDDLE,
    )

    return False

  def _handle_mouse_release(self, mouse_pos: MousePos):
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2

    dec_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH * 2 - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)

    value = self._get_value()
    min_val = _resolve_value(self._min_val, 0)
    max_val = _resolve_value(self._max_val, 100)

    # Only one button can be triggered - check all but use elif
    if rl.check_collision_point_rec(mouse_pos, inc_btn_rect) and self.enabled and value < max_val:
      self._update_value(self._step)
    elif rl.check_collision_point_rec(mouse_pos, dec_btn_rect) and self.enabled and value > min_val:
      self._update_value(-self._step)

    # Reset all pressed states
    self._decrement_pressed = False
    self._increment_pressed = False
    self._repeat_timer = 0.0

  def _handle_mouse_event(self, mouse_event):
    # Don't call super - we handle everything here to avoid double-processing
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2

    dec_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH * 2 - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)

    # Visual feedback only - set pressed state
    if mouse_event.left_pressed:
      if rl.check_collision_point_rec(mouse_event.pos, inc_btn_rect) and self.enabled:
        self._increment_pressed = True
      elif rl.check_collision_point_rec(mouse_event.pos, dec_btn_rect) and self.enabled:
        self._decrement_pressed = True

    # Reset on release for visual feedback
    if mouse_event.left_released:
      self._decrement_pressed = False
      self._increment_pressed = False


class ValueButtonSliderAction(ValueSliderAction):
  def __init__(
    self,
    value: float | Callable[[], float],
    min_val: float = 0,
    max_val: float = 100,
    step: float = 1,
    unit: str = "",
    button_text: str = "Reset",
    button_callback: Callable | None = None,
    labels: dict[float, str] | None = None,
    callback: Callable[[float], None] | None = None,
    enabled: bool | Callable[[], bool] = True,
    negative: bool = False,
    default_value: float | None = None,
    fast_increase: bool = False,
    sub_toggles: list[tuple[str, bool]] | None = None,
    is_metric: bool = False,
  ):
    super().__init__(value, min_val, max_val, step, unit, labels, callback, enabled, negative, default_value, fast_increase, is_metric)
    self._button_text = button_text
    self._button_callback = button_callback
    self._sub_toggles = sub_toggles or []
    self._button_pressed = False

    button_width = 180
    total_width = VALUE_DISPLAY_WIDTH + VALUE_BUTTON_WIDTH * 2 + button_width + 40
    self._rect.width = total_width

  def _render(self, rect: rl.Rectangle) -> bool:
    value = self._get_value()
    display_text = self._get_display_text(value)

    button_y = rect.y + (rect.height - VALUE_BUTTON_HEIGHT) / 2
    button_width = 180

    dec_btn_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH * 2 - button_width - 40, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH - button_width - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    action_btn_rect = rl.Rectangle(
      rect.x + rect.width - button_width, button_y + (VALUE_BUTTON_HEIGHT - VALUE_BUTTON_HEIGHT) / 2, button_width, VALUE_BUTTON_HEIGHT
    )

    min_val = _resolve_value(self._min_val, 0)
    max_val = _resolve_value(self._max_val, 100)
    dec_color = rl.Color(57, 57, 57, 255) if value > min_val else rl.Color(40, 40, 40, 255)
    inc_color = rl.Color(57, 57, 57, 255) if value < max_val else rl.Color(40, 40, 40, 255)

    if self._decrement_pressed:
      dec_color = rl.Color(74, 74, 74, 255)
    if self._increment_pressed:
      inc_color = rl.Color(74, 74, 74, 255)

    if not self.enabled:
      dec_color = rl.Color(dec_color.r, dec_color.g, dec_color.b, 128)
      inc_color = rl.Color(inc_color.r, inc_color.g, inc_color.b, 128)

    rl.draw_rectangle_rounded(dec_btn_rect, 0.3, 10, dec_color)
    rl.draw_rectangle_rounded(inc_btn_rect, 0.3, 10, inc_color)

    dec_text = "-"
    inc_text = "+"
    dec_size = measure_text_cached(self._value_font, dec_text, 50)
    inc_size = measure_text_cached(self._value_font, inc_text, 50)
    rl.draw_text_ex(
      self._value_font,
      dec_text,
      rl.Vector2(dec_btn_rect.x + (dec_btn_rect.width - dec_size.x) / 2, dec_btn_rect.y + (dec_btn_rect.height - dec_size.y) / 2),
      50,
      0,
      rl.WHITE,
    )
    rl.draw_text_ex(
      self._value_font,
      inc_text,
      rl.Vector2(inc_btn_rect.x + (inc_btn_rect.width - inc_size.x) / 2, inc_btn_rect.y + (inc_btn_rect.height - inc_size.y) / 2),
      50,
      0,
      rl.WHITE,
    )

    value_rect = rl.Rectangle(rect.x + rect.width - VALUE_BUTTON_WIDTH * 2 - button_width - VALUE_DISPLAY_WIDTH - 50, rect.y, VALUE_DISPLAY_WIDTH, rect.height)
    gui_label(
      value_rect,
      display_text,
      font_size=VALUE_FONT_SIZE,
      color=VALUE_TEXT_COLOR if self.enabled else rl.Color(100, 100, 100, 255),
      font_weight=FontWeight.DISPLAY,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_RIGHT,
      alignment_vertical=rl.GuiTextAlignmentVertical.TEXT_ALIGN_MIDDLE,
    )

    btn_color = rl.Color(57, 57, 57, 255)
    if self._button_pressed:
      btn_color = rl.Color(74, 74, 74, 255)
    if not self.enabled:
      btn_color = rl.Color(btn_color.r, btn_color.g, btn_color.b, 128)

    rl.draw_rectangle_rounded(action_btn_rect, 0.3, 10, btn_color)

    btn_text = _resolve_value(self._button_text, "Reset")
    btn_font = gui_app.font(FontWeight.MEDIUM)
    btn_size = measure_text_cached(btn_font, btn_text, 35)
    rl.draw_text_ex(
      btn_font,
      btn_text,
      rl.Vector2(action_btn_rect.x + (action_btn_rect.width - btn_size.x) / 2, action_btn_rect.y + (action_btn_rect.height - btn_size.y) / 2),
      35,
      0,
      rl.WHITE if self.enabled else rl.Color(150, 150, 150, 255),
    )

    return False

  def _handle_mouse_release(self, mouse_pos: MousePos):
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2
    button_width = 180

    dec_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH * 2 - button_width - 40, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH - button_width - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    action_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - button_width, button_y, button_width, VALUE_BUTTON_HEIGHT)

    value = self._get_value()
    min_val = _resolve_value(self._min_val, 0)
    max_val = _resolve_value(self._max_val, 100)

    # Only one button can be triggered - check all but use elif
    if rl.check_collision_point_rec(mouse_pos, action_btn_rect) and self.enabled:
      if self._button_callback:
        self._button_callback()
    elif rl.check_collision_point_rec(mouse_pos, inc_btn_rect) and self.enabled and value < max_val:
      self._update_value(self._step)
    elif rl.check_collision_point_rec(mouse_pos, dec_btn_rect) and self.enabled and value > min_val:
      self._update_value(-self._step)

    # Reset all pressed states
    self._decrement_pressed = False
    self._increment_pressed = False
    self._button_pressed = False
    self._repeat_timer = 0.0

  def _handle_mouse_event(self, mouse_event):
    # Don't call super - we handle everything here to avoid double-processing
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2
    button_width = 180

    dec_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH * 2 - button_width - 40, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    inc_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - VALUE_BUTTON_WIDTH - button_width - 20, button_y, VALUE_BUTTON_WIDTH, VALUE_BUTTON_HEIGHT)
    action_btn_rect = rl.Rectangle(self._rect.x + self._rect.width - button_width, button_y, button_width, VALUE_BUTTON_HEIGHT)

    # Visual feedback only - set pressed state
    if mouse_event.left_pressed:
      if rl.check_collision_point_rec(mouse_event.pos, action_btn_rect) and self.enabled:
        self._button_pressed = True
      elif rl.check_collision_point_rec(mouse_event.pos, inc_btn_rect) and self.enabled:
        self._increment_pressed = True
      elif rl.check_collision_point_rec(mouse_event.pos, dec_btn_rect) and self.enabled:
        self._decrement_pressed = True

    # Reset on release for visual feedback
    if mouse_event.left_released:
      self._decrement_pressed = False
      self._increment_pressed = False
      self._button_pressed = False


class DualValueSliderAction(ItemAction):
  def __init__(
    self,
    value1: float | Callable[[], float],
    value2: float | Callable[[], float],
    min_val: float = 0,
    max_val: float = 100,
    step: float = 1,
    unit: str = "",
    label1: str = "",
    label2: str = "",
    callback1: Callable[[float], None] | None = None,
    callback2: Callable[[float], None] | None = None,
    enabled: bool | Callable[[], bool] = True,
    labels: dict[float, str] | None = None,
    is_metric: bool = False,
  ):
    self._value1_source = value1
    self._value2_source = value2
    self._min_val = min_val
    self._max_val = max_val
    self._step = step
    self._unit = unit
    self._label1 = label1
    self._label2 = label2
    self._callback1 = callback1
    self._callback2 = callback2
    self._labels = labels or []
    self._params = Params()

    self._metric_multiplier = 1.0
    if is_metric:
      if self._unit == "mph":
        self._metric_multiplier = 1.60934
      elif self._unit == "feet":
        self._metric_multiplier = 0.3048
      elif self._unit == "inches":
        self._metric_multiplier = 2.54

    self._slider1 = ValueSliderAction(value1, min_val, max_val, step, unit, labels, callback1, enabled, is_metric=is_metric)
    self._slider2 = ValueSliderAction(value2, min_val, max_val, step, unit, labels, callback2, enabled, is_metric=is_metric)

    total_width = (VALUE_DISPLAY_WIDTH + VALUE_BUTTON_WIDTH * 2 + 20) * 2 + 40
    super().__init__(width=total_width, enabled=enabled)

  def _render(self, rect: rl.Rectangle) -> bool:
    half_width = (rect.width - 40) / 2

    slider1_rect = rl.Rectangle(rect.x, rect.y, half_width, rect.height)
    slider2_rect = rl.Rectangle(rect.x + half_width + 40, rect.y, half_width, rect.height)

    if self._label1:
      label_rect = rl.Rectangle(slider1_rect.x, slider1_rect.y - 30, half_width, 25)
      gui_label(label_rect, self._label1, font_size=30, color=rl.Color(170, 170, 170, 255), font_weight=FontWeight.NORMAL)

    if self._label2:
      label_rect = rl.Rectangle(slider2_rect.x, slider2_rect.y - 30, half_width, 25)
      gui_label(label_rect, self._label2, font_size=30, color=rl.Color(170, 170, 170, 255), font_weight=FontWeight.NORMAL)

    return False

  def _handle_mouse_event(self, mouse_event):
    half_width = (self._rect.width - 40) / 2
    slider1_rect = rl.Rectangle(self._rect.x, self._rect.y, half_width, self._rect.height)
    slider2_rect = rl.Rectangle(self._rect.x + half_width + 40, self._rect.y, half_width, self._rect.height)

    adjusted_pos1 = MousePos(
      rl.Vector2(mouse_event.pos.x - slider1_rect.x + self._rect.x, mouse_event.pos.y),
      mouse_event.left_pressed,
      mouse_event.left_released,
      mouse_event.right_pressed,
      mouse_event.right_released,
    )
    adjusted_pos2 = MousePos(
      rl.Vector2(mouse_event.pos.x - slider2_rect.x + self._rect.x + half_width + 40, mouse_event.pos.y),
      mouse_event.left_pressed,
      mouse_event.left_released,
      mouse_event.right_pressed,
      mouse_event.right_released,
    )

    if 0 <= mouse_event.pos.x - slider1_rect.x < half_width:
      self._slider1._handle_mouse_event(adjusted_pos1)
    if 0 <= mouse_event.pos.x - slider2_rect.x - half_width - 40 < half_width:
      self._slider2._handle_mouse_event(adjusted_pos2)


class ButtonToggleAction(ItemAction):
  def __init__(
    self,
    state: bool | Callable[[], bool],
    sub_toggles: list[str] | None = None,
    sub_toggle_names: list[str] | None = None,
    callback: Callable[[bool], None] | None = None,
    sub_callbacks: list[Callable] | None = None,
    enabled: bool | Callable[[], bool] = True,
    exclusive: bool = False,
  ):
    self._state_source = state
    self._sub_toggles = sub_toggles or []
    self._sub_toggle_names = sub_toggle_names or []
    self._callback = callback
    self._sub_callbacks = sub_callbacks or []
    self._exclusive = exclusive
    self._params = Params()

    self._toggle = Toggle(initial_state=_resolve_value(state, False), callback=callback)

    button_width = 180
    total_width = TOGGLE_WIDTH + len(self._sub_toggles) * (button_width + 10)
    super().__init__(width=total_width, enabled=enabled)

  def get_width_hint(self) -> float:
    return self._rect.width

  def _render(self, rect: rl.Rectangle) -> bool:
    toggle_rect = rl.Rectangle(rect.x, rect.y + (rect.height - TOGGLE_HEIGHT) / 2, TOGGLE_WIDTH, TOGGLE_HEIGHT)
    self._toggle.set_enabled(self.enabled)
    clicked = self._toggle.render(toggle_rect)

    button_width = 180
    button_spacing = 10
    button_y = rect.y + (rect.height - VALUE_BUTTON_HEIGHT) / 2

    x_offset = rect.x + TOGGLE_WIDTH + 20

    for i, (sub_key, sub_name) in enumerate(zip(self._sub_toggles, self._sub_toggle_names)):
      btn_rect = rl.Rectangle(x_offset + i * (button_width + button_spacing), button_y, button_width, VALUE_BUTTON_HEIGHT)

      sub_value = False
      if isinstance(sub_key, str) and sub_key:
        sub_value = self._params.get_bool(sub_key)

      btn_color = rl.Color(51, 171, 76, 255) if sub_value else rl.Color(57, 57, 57, 255)
      if not self.enabled:
        btn_color = rl.Color(btn_color.r, btn_color.g, btn_color.b, 128)

      rl.draw_rectangle_rounded(btn_rect, 0.3, 10, btn_color)

      text_size = measure_text_cached(gui_app.font(FontWeight.MEDIUM), sub_name, 30)
      rl.draw_text_ex(
        gui_app.font(FontWeight.MEDIUM),
        sub_name,
        rl.Vector2(btn_rect.x + (btn_rect.width - text_size.x) / 2, btn_rect.y + (btn_rect.height - text_size.y) / 2),
        30,
        0,
        rl.WHITE if self.enabled else rl.Color(150, 150, 150, 255),
      )

    return clicked

  def _handle_mouse_release(self, mouse_pos: MousePos):
    button_width = 180
    button_spacing = 10
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2

    x_offset = self._rect.x + TOGGLE_WIDTH + 20

    for i, (sub_key, sub_callback) in enumerate(zip(self._sub_toggles, self._sub_callbacks)):
      btn_rect = rl.Rectangle(x_offset + i * (button_width + button_spacing), button_y, button_width, VALUE_BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, btn_rect) and self.enabled:
        if isinstance(sub_key, str) and sub_key:
          current = self._params.get_bool(sub_key)
          self._params.put_bool(sub_key, not current)
        if sub_callback:
          sub_callback()


class MultiButtonsAction(ItemAction):
  def __init__(
    self,
    buttons: list[str | Callable[[], str]],
    button_callbacks: list[Callable] | None = None,
    enabled: bool | Callable[[], bool] = True,
    initial_value: str = "",
  ):
    self._buttons_source = buttons
    self._button_callbacks = button_callbacks or []
    self._initial_value = initial_value

    button_width = 180
    total_width = len(buttons) * button_width + (len(buttons) - 1) * RIGHT_ITEM_PADDING
    super().__init__(width=total_width, enabled=enabled)

    self._font = gui_app.font(FontWeight.MEDIUM)

  def get_width_hint(self) -> float:
    return self._rect.width

  def _render(self, rect: rl.Rectangle) -> bool:
    buttons = _resolve_value(self._buttons_source, [])
    button_width = 180
    spacing = RIGHT_ITEM_PADDING
    button_y = rect.y + (rect.height - VALUE_BUTTON_HEIGHT) / 2

    pressed_any = False

    for i, btn_text in enumerate(buttons):
      btn_x = rect.x + i * (button_width + spacing)
      btn_rect = rl.Rectangle(btn_x, button_y, button_width, VALUE_BUTTON_HEIGHT)

      btn_color = rl.Color(57, 57, 57, 255)
      if not self.enabled:
        btn_color = rl.Color(btn_color.r, btn_color.g, btn_color.b, 128)

      rl.draw_rectangle_rounded(btn_rect, 0.3, 10, btn_color)

      text = _resolve_value(btn_text, "")
      text_size = measure_text_cached(self._font, text, 30)
      text_x = btn_rect.x + (btn_rect.width - text_size.x) / 2
      text_y = btn_rect.y + (btn_rect.height - text_size.y) / 2
      rl.draw_text_ex(self._font, text, rl.Vector2(text_x, text_y), 30, 0, rl.WHITE if self.enabled else rl.Color(150, 150, 150, 255))

    return False

  def _handle_mouse_release(self, mouse_pos: MousePos):
    buttons = _resolve_value(self._buttons_source, [])
    button_width = 180
    spacing = RIGHT_ITEM_PADDING
    button_y = self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2

    for i, callback in enumerate(self._button_callbacks):
      btn_rect = rl.Rectangle(self._rect.x + i * (button_width + spacing), button_y, button_width, VALUE_BUTTON_HEIGHT)
      if rl.check_collision_point_rec(mouse_pos, btn_rect) and self.enabled:
        if callback:
          callback()


class SelectionButtonAction(ItemAction):
  def __init__(
    self,
    options: list[str],
    selected_index: int = 0,
    callback: Callable[[int, str], None] | None = None,
    enabled: bool | Callable[[], bool] = True,
  ):
    self._options = options
    self._selected_index = selected_index
    self._callback = callback
    self._params = Params()
    self._button_pressed = False

    button_width = 300
    super().__init__(width=button_width, enabled=enabled)

    self._font = gui_app.font(FontWeight.MEDIUM)

  def get_width_hint(self) -> float:
    return self._rect.width

  def _render(self, rect: rl.Rectangle) -> bool:
    btn_rect = rl.Rectangle(rect.x, rect.y + (rect.height - VALUE_BUTTON_HEIGHT) / 2, self._rect.width, VALUE_BUTTON_HEIGHT)

    btn_color = rl.Color(57, 57, 57, 255)
    if self._button_pressed:
      btn_color = rl.Color(74, 74, 74, 255)
    if not self.enabled:
      btn_color = rl.Color(btn_color.r, btn_color.g, btn_color.b, 128)

    rl.draw_rectangle_rounded(btn_rect, 0.3, 10, btn_color)

    current_option = self._options[self._selected_index] if 0 <= self._selected_index < len(self._options) else ""
    text_size = measure_text_cached(self._font, current_option, 35)
    rl.draw_text_ex(
      self._font,
      current_option,
      rl.Vector2(btn_rect.x + 20, btn_rect.y + (btn_rect.height - text_size.y) / 2),
      35,
      0,
      rl.WHITE if self.enabled else rl.Color(150, 150, 150, 255),
    )

    arrow_text = "▼"
    arrow_size = measure_text_cached(self._font, arrow_text, 25)
    rl.draw_text_ex(
      self._font,
      arrow_text,
      rl.Vector2(btn_rect.x + btn_rect.width - arrow_size.x - 20, btn_rect.y + (btn_rect.height - arrow_size.y) / 2),
      25,
      0,
      rl.WHITE if self.enabled else rl.Color(150, 150, 150, 255),
    )

    return self._button_pressed

  def _handle_mouse_release(self, mouse_pos: MousePos):
    btn_rect = rl.Rectangle(self._rect.x, self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2, self._rect.width, VALUE_BUTTON_HEIGHT)
    if rl.check_collision_point_rec(mouse_pos, btn_rect) and self.enabled and self._button_pressed:
      if self._callback:
        self._callback(self._selected_index, self._options[self._selected_index] if 0 <= self._selected_index < len(self._options) else "")
    self._button_pressed = False

  def _handle_mouse_event(self, mouse_event):
    super()._handle_mouse_event(mouse_event)
    btn_rect = rl.Rectangle(self._rect.x, self._rect.y + (self._rect.height - VALUE_BUTTON_HEIGHT) / 2, self._rect.width, VALUE_BUTTON_HEIGHT)
    if mouse_event.left_pressed:
      if rl.check_collision_point_rec(mouse_event.pos, btn_rect) and self.enabled:
        self._button_pressed = True
    if mouse_event.left_released:
      if not rl.check_collision_point_rec(mouse_event.pos, btn_rect):
        self._button_pressed = False


class LabelAction(ItemAction):
  def __init__(
    self,
    value: str | Callable[[], str],
    enabled: bool | Callable[[], bool] = True,
  ):
    self._value_source = value
    self._font = gui_app.font(FontWeight.NORMAL)
    initial_text = _resolve_value(value, "")
    text_width = measure_text_cached(self._font, initial_text, ITEM_TEXT_FONT_SIZE).x
    super().__init__(width=int(text_width + TEXT_PADDING), enabled=enabled)

  def get_width_hint(self) -> float:
    text = _resolve_value(self._value_source, "")
    text_width = measure_text_cached(self._font, text, ITEM_TEXT_FONT_SIZE).x
    return text_width + TEXT_PADDING

  def _render(self, rect: rl.Rectangle) -> bool:
    value = _resolve_value(self._value_source, "")
    gui_label(
      rect,
      value,
      font_size=ITEM_TEXT_FONT_SIZE,
      color=ITEM_TEXT_VALUE_COLOR if self.enabled else rl.Color(100, 100, 100, 255),
      font_weight=FontWeight.NORMAL,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_RIGHT,
      alignment_vertical=rl.GuiTextAlignmentVertical.TEXT_ALIGN_MIDDLE,
    )
    return False


def value_item(
  title: str | Callable[[], str],
  value: float | Callable[[], float],
  min_val: float = 0,
  max_val: float = 100,
  step: float = 1,
  unit: str = "",
  description: str | Callable[[], str] | None = None,
  callback: Callable[[float], None] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  labels: dict[float, str] | None = None,
  negative: bool = False,
  default_value: float | None = None,
  fast_increase: bool = False,
  is_metric: bool = False,
) -> ListItem:
  action = ValueSliderAction(value, min_val, max_val, step, unit, labels, callback, enabled, negative, default_value, fast_increase, is_metric)
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def value_button_item(
  title: str | Callable[[], str],
  value: float | Callable[[], float],
  min_val: float = 0,
  max_val: float = 100,
  step: float = 1,
  unit: str = "",
  button_text: str = "Reset",
  button_callback: Callable | None = None,
  description: str | Callable[[], str] | None = None,
  callback: Callable[[float], None] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  sub_toggles: list[tuple[str, bool]] | None = None,
  labels: dict[float, str] | None = None,
  negative: bool = False,
  default_value: float | None = None,
  fast_increase: bool = False,
  is_metric: bool = False,
) -> ListItem:
  action = ValueButtonSliderAction(
    value, min_val, max_val, step, unit, button_text, button_callback, labels, callback, enabled, negative, default_value, fast_increase, sub_toggles, is_metric
  )
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def dual_value_item(
  title: str | Callable[[], str],
  value1: float | Callable[[], float],
  value2: float | Callable[[], float],
  min_val: float = 0,
  max_val: float = 100,
  step: float = 1,
  unit: str = "",
  label1: str = "",
  label2: str = "",
  description: str | Callable[[], str] | None = None,
  callback1: Callable[[float], None] | None = None,
  callback2: Callable[[float], None] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  labels: dict[float, str] | None = None,
  is_metric: bool = False,
) -> ListItem:
  action = DualValueSliderAction(value1, value2, min_val, max_val, step, unit, label1, label2, callback1, callback2, enabled, labels, is_metric)
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def button_toggle_item(
  title: str | Callable[[], str],
  state: bool | Callable[[], bool],
  sub_toggles: list[str] | None = None,
  sub_toggle_names: list[str] | None = None,
  description: str | Callable[[], str] | None = None,
  callback: Callable[[bool], None] | None = None,
  sub_callbacks: list[Callable] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  exclusive: bool = False,
) -> ListItem:
  action = ButtonToggleAction(state, sub_toggles, sub_toggle_names, callback, sub_callbacks, enabled, exclusive)
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def buttons_item(
  title: str | Callable[[], str],
  buttons: list[str],
  button_callbacks: list[Callable] | None = None,
  description: str | Callable[[], str] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
  initial_value: str = "",
) -> ListItem:
  action = MultiButtonsAction(buttons, button_callbacks, enabled, initial_value)
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def selection_button_item(
  title: str | Callable[[], str],
  options: list[str],
  selected_index: int = 0,
  description: str | Callable[[], str] | None = None,
  callback: Callable[[int, str], None] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  action = SelectionButtonAction(options, selected_index, callback, enabled)
  return ListItem(title=title, description=description, icon=icon, action_item=action)


def label_item(
  title: str | Callable[[], str],
  value: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  action = LabelAction(value, enabled)
  return ListItem(title=title, description=description, icon=icon, action_item=action)
