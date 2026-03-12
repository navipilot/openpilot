from enum import IntEnum
from collections.abc import Callable
import pyray as rl

from openpilot.system.ui.lib.application import gui_app, FontWeight
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.widgets import Widget, DialogResult
from openpilot.system.ui.widgets.button import Button, ButtonStyle
from openpilot.system.ui.widgets.label import Label
from openpilot.system.ui.widgets.scroller_tici import Scroller

SELECTION_COLOR = rl.Color(70, 91, 234, 255) # #465BEA
HEADER_BG = rl.Color(51, 51, 51, 255)   # #333333
BACKGROUND_COLOR = rl.Color(27, 27, 27, 255) # #1B1B1B
BORDER_COLOR = rl.Color(80, 80, 80, 255)
MARGIN = 40
OUTER_MARGIN_X = 100
OUTER_MARGIN_Y = 80
BUTTON_HEIGHT = 90

class SortMode(IntEnum):
  ALPHABETICAL = 0
  DATE_NEWEST = 1
  DATE_OLDEST = 2
  FAVORITES = 3

class SelectionHeader(Widget):
  def __init__(self, text: str, is_expanded: bool, callback: Callable[[str], None]):
    super().__init__()
    self._text = text
    self._is_expanded = is_expanded
    self._callback = callback
    self._font = gui_app.font(FontWeight.BOLD)
    self._font_size = 40
    self._pressed = False
    self.set_rect(rl.Rectangle(0, 0, 0, 70))

  def set_parent_rect(self, parent_rect: rl.Rectangle) -> None:
    super().set_parent_rect(parent_rect)
    self._rect.width = parent_rect.width

  def _render(self, rect: rl.Rectangle):
    # Header background - Match Qt .series-header {#333333}
    bg_color = rl.Color(64, 64, 64, 255) if self._pressed else HEADER_BG
    rl.draw_rectangle_rounded(rect, 0.1, 10, bg_color)
    
    # Arrow - Match Qt text-based arrows
    arrow = "▼" if self._is_expanded else "▶"
    arrow_pos = rl.Vector2(rect.x + 30, rect.y + (rect.height - self._font_size) / 2)
    rl.draw_text_ex(self._font, arrow, arrow_pos, self._font_size, 0, rl.WHITE)
    
    # Text - Match Qt padding-left: 80px
    text_pos = rl.Vector2(rect.x + 80, rect.y + (rect.height - self._font_size) / 2)
    rl.draw_text_ex(self._font, self._text, text_pos, self._font_size, 0, rl.WHITE)

  def _handle_mouse_press(self, mouse_pos):
    if rl.check_collision_point_rec(mouse_pos, self._hit_rect):
        self._pressed = True

  def _handle_mouse_release(self, mouse_pos):
    if self._pressed and rl.check_collision_point_rec(mouse_pos, self._hit_rect):
      if self._callback:
        self._callback(self._text)
    self._pressed = False

class SelectionItem(Widget):
  def __init__(self, text: str, is_selected: bool, is_favorite: bool, callback: Callable[[str], None], fav_callback: Callable[[str], None] = None):
    super().__init__()
    self._text = text
    self._is_selected = is_selected
    self._is_favorite = is_favorite
    self._callback = callback
    self._fav_callback = fav_callback
    self._font = gui_app.font(FontWeight.MEDIUM)
    self._font_size = 48
    self._pressed = False
    self._fav_pressed = False
    self.set_rect(rl.Rectangle(0, 0, 0, 110))

  def set_parent_rect(self, parent_rect: rl.Rectangle) -> None:
    super().set_parent_rect(parent_rect)
    self._rect.width = parent_rect.width

  def _render(self, rect: rl.Rectangle):
    # Background for item - Match Qt .model-option:checked {#465BEA}
    if self._is_selected:
        bg_color = rl.Color(70, 91, 234, 255) # #465BEA
    else:
        bg_color = rl.Color(90, 90, 90, 255) if self._pressed else rl.Color(79, 79, 79, 255) # #4F4F4F
    
    rl.draw_rectangle_rounded(rect, 0.1, 10, bg_color)
    
    # Selection Border - Match Qt {3px WHITE}
    if self._is_selected:
      rl.draw_rectangle_rounded_lines_ex(rect, 0.1, 10, 3, rl.WHITE)
    
    # Favorite Star - Left side
    star = "♥" if self._is_favorite else "♡"
    star_pos = rl.Vector2(rect.x + 25, rect.y + (rect.height - self._font_size) / 2)
    rl.draw_text_ex(self._font, star, star_pos, self._font_size + 10, 0, rl.WHITE)
    
    # Text
    text_size = rl.measure_text_ex(self._font, self._text, self._font_size, 0)
    text_pos = rl.Vector2(rect.x + 90, rect.y + (rect.height - text_size.y) / 2)
    rl.draw_text_ex(self._font, self._text, text_pos, self._font_size, 0, rl.WHITE)

    # Indicator (Dot for selection instead of radio)
    if self._is_selected:
        circle_center = rl.Vector2(rect.x + rect.width - 50, rect.y + rect.height / 2)
        rl.draw_circle_v(circle_center, 12, rl.WHITE)

  @property
  def _fav_rect(self) -> rl.Rectangle:
    return rl.Rectangle(self._rect.x, self._rect.y, 80, self._rect.height)

  def _handle_mouse_press(self, mouse_pos):
    if rl.check_collision_point_rec(mouse_pos, self._fav_rect):
        self._fav_pressed = True
    elif rl.check_collision_point_rec(mouse_pos, self._hit_rect):
        self._pressed = True

  def _handle_mouse_release(self, mouse_pos):
    if self._fav_pressed and rl.check_collision_point_rec(mouse_pos, self._fav_rect):
        if self._fav_callback:
            self._fav_callback(self._text)
    elif self._pressed and rl.check_collision_point_rec(mouse_pos, self._hit_rect):
      if self._callback:
        self._callback(self._text)
    self._pressed = False
    self._fav_pressed = False

  def _handle_mouse_press(self, mouse_pos):
    if rl.check_collision_point_rec(mouse_pos, self._hit_rect):
      self._pressed = True

  def _handle_mouse_release(self, mouse_pos):
    if self._pressed and rl.check_collision_point_rec(mouse_pos, self._hit_rect):
      if self._callback:
        self._callback(self._text)
    self._pressed = False

class SelectionDialog(Widget):
  def __init__(self, title: str, options, current_selection: str = "", 
               on_close: Callable[[DialogResult, str], None] | None = None,
               model_released_dates: dict[str, str] | None = None,
               model_file_to_name: dict[str, str] | None = None,
               user_favorites: list[str] | None = None,
               community_favorites: list[str] | None = None,
               on_favorite_toggled: Callable[[str], None] | None = None):
    super().__init__()
    self._title = title
    self._options_raw = options 
    self._selected_value = current_selection
    self._on_close = on_close
    self._model_released_dates = model_released_dates or {}
    self._name_to_file = {v: k for k, v in (model_file_to_name or {}).items()}
    self._user_favorites = user_favorites or []
    self._community_favorites = community_favorites or []
    self._on_favorite_toggled = on_favorite_toggled
    
    self._sort_mode = SortMode.ALPHABETICAL
    self._expanded_series = {s: True for s in (options.keys() if isinstance(options, dict) else [])}
    
    self._title_label = Label(title, 60, FontWeight.BOLD, text_color=rl.WHITE)
    self._sort_button = Button("Alphabetical", self._toggle_sort, button_style=ButtonStyle.NORMAL)
    self._cancel_button = Button("Cancel", self._cancel_button_callback)
    self._confirm_button = Button("Select", self._confirm_button_callback, button_style=ButtonStyle.PRIMARY)
    
    self._scroller = None
    self._build_scroller()

  def _toggle_sort(self):
    self._sort_mode = SortMode((int(self._sort_mode) + 1) % 4)
    modes = ["Alphabetical", "Date (Newest)", "Date (Oldest)", "Favorites First"]
    self._sort_button.set_text(modes[int(self._sort_mode)])
    self._build_scroller()

  def _toggle_series(self, series: str):
    self._expanded_series[series] = not self._expanded_series.get(series, True)
    self._build_scroller()

  def _build_scroller(self):
    items = []
    
    if isinstance(self._options_raw, dict):
      series_keys = list(self._options_raw.keys())
      priority_series = ["FrogPilot", "Comma", "Experimental"]
      sorted_series_keys = []
      for p in priority_series:
        if p in series_keys:
          sorted_series_keys.append(p)
          series_keys.remove(p)
      sorted_series_keys.extend(sorted(series_keys))
      
      for series in sorted_series_keys:
        models = self._options_raw[series]
        if not models:
            continue
            
        items.append(SelectionHeader(series, self._expanded_series.get(series, True), self._toggle_series))
        
        if self._expanded_series.get(series, True):
          sorted_models = list(models)
          if self._sort_mode == SortMode.ALPHABETICAL:
              sorted_models.sort()
          elif self._sort_mode == SortMode.DATE_NEWEST:
              def get_date(m):
                  key = self._name_to_file.get(m, m)
                  return self._model_released_dates.get(key, "0000-00-00")
              sorted_models.sort(key=get_date, reverse=True)
          elif self._sort_mode == SortMode.DATE_OLDEST:
              def get_date(m):
                  key = self._name_to_file.get(m, m)
                  return self._model_released_dates.get(key, "9999-99-99")
              sorted_models.sort(key=get_date)
          elif self._sort_mode == SortMode.FAVORITES:
              def is_fav(m):
                  key = self._name_to_file.get(m, m)
                  return key in self._user_favorites or key in self._community_favorites
              sorted_models.sort(key=is_fav, reverse=True)
          
          for model in sorted_models:
            key = self._name_to_file.get(model, model)
            is_selected = (model == self._selected_value or key == self._selected_value)
            is_fav = key in self._user_favorites or key in self._community_favorites
            items.append(SelectionItem(
              text=model,
              is_selected=is_selected,
              is_favorite=is_fav,
              callback=self._on_item_selected,
              fav_callback=self._toggle_favorite
            ))
    else:
      for option in self._options_raw:
        items.append(SelectionItem(
          text=option,
          is_selected=(option == self._selected_value),
          is_favorite=False,
          callback=self._on_item_selected
        ))
    
    self._scroller = Scroller(items, line_separator=False, spacing=10)
    self._scroller.show_event()

  def _toggle_favorite(self, model_name: str):
    key = self._name_to_file.get(model_name, model_name)
    if self._on_favorite_toggled:
        self._on_favorite_toggled(key)
    # Update local state for instant feedback
    if key in self._user_favorites:
        self._user_favorites.remove(key)
    else:
        self._user_favorites.append(key)
    self._build_scroller()

  def _on_item_selected(self, val):
    self._selected_value = val
    # Instant visual update
    if self._scroller:
      for item in self._scroller._items:
          if isinstance(item, SelectionItem):
              item._is_selected = (item._text == val)

  def _cancel_button_callback(self):
    if self._on_close:
      self._on_close(DialogResult.CANCEL, "")
    gui_app.set_modal_overlay(None)

  def _confirm_button_callback(self):
    if self._on_close:
      self._on_close(DialogResult.CONFIRM, self._selected_value)
    gui_app.set_modal_overlay(None)

  def show_event(self):
    super().show_event()
    if self._scroller:
        self._scroller.show_event()

  def _render(self, rect: rl.Rectangle):
    # Dim background
    rl.draw_rectangle(0, 0, int(rl.get_screen_width()), int(rl.get_screen_height()), rl.Color(0, 0, 0, 180))

    # Dialog Box
    dialog_rect = rl.Rectangle(
      rect.x + OUTER_MARGIN_X,
      rect.y + OUTER_MARGIN_Y,
      rect.width - 2 * OUTER_MARGIN_X,
      rect.height - 2 * OUTER_MARGIN_Y,
    )
    rl.draw_rectangle_rounded(dialog_rect, 0.04, 12, BACKGROUND_COLOR)
    rl.draw_rectangle_rounded_lines_ex(dialog_rect, 0.04, 12, 2, BORDER_COLOR)

    # Title
    title_width = dialog_rect.width - 2 * MARGIN - 260
    self._title_label.render(rl.Rectangle(dialog_rect.x + MARGIN, dialog_rect.y + MARGIN, title_width, 80))
    
    # Sort Button
    self._sort_button.render(rl.Rectangle(dialog_rect.x + dialog_rect.width - MARGIN - 240, dialog_rect.y + MARGIN, 240, 80))

    # Bottom Buttons
    btn_y = dialog_rect.y + dialog_rect.height - BUTTON_HEIGHT - MARGIN
    btn_width = (dialog_rect.width - 3 * MARGIN) / 2
    
    self._cancel_button.render(rl.Rectangle(dialog_rect.x + MARGIN, btn_y, btn_width, BUTTON_HEIGHT))
    self._confirm_button.render(rl.Rectangle(dialog_rect.x + 2 * MARGIN + btn_width, btn_y, btn_width, BUTTON_HEIGHT))

    # Scrollable Options List
    scroller_y = dialog_rect.y + MARGIN + 80 + 20
    scroller_rect = rl.Rectangle(
      dialog_rect.x + MARGIN,
      scroller_y,
      dialog_rect.width - 2 * MARGIN,
      btn_y - scroller_y - 20
    )
    self._scroller.render(scroller_rect)

    return DialogResult.NO_ACTION
