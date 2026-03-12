# StarPilot UI Architecture: Qt to Raylib Porting Cheat Sheet

This document serves as the definitive reference for porting the Qt-based Big UI to the Raylib-based Big UI in StarPilot. It provides direct parallels, code patterns, and implementation guidance.

> **Last updated: March 17, 2026 (Late Session)** — **Fully implemented Driving Model panel** with High-Fidelity Qt SelectionDialog (Favorites/Stars, Premium styling, Sort cycling), Model Auto-Fetch, and robust background download management. Completed Sounds panel with Alert Volume Controller and Custom Alerts. Added StarPilotState singleton, InputDialog, Lateral full implementation (5 sub-panels), 11 value factory functions, Params helpers, and sound testing architecture.

---

## Table of Contents

1. [High-Level Architecture Comparison](#1-high-level-architecture-comparison)
2. [Key Parallels: Qt to Raylib](#2-key-parallels-qt-to-raylib)
3. [Code Structure Patterns](#3-code-structure-patterns)
4. [Onroad UI Comparison](#4-onroad-ui-comparison)
5. [Sidebar Comparison](#5-sidebar-comparison)
6. [Settings Layout Comparison](#6-settings-layout-comparison)
7. [Implementation Differences](#7-implementation-differences)
8. [Files Requiring Work for Full Port](#8-files-requiring-work-for-full-port)
   - [Core Infrastructure](#81-priority-1-core-infrastructure)
   - [Implementation Guide](#implementation-guide-adding-new-starpilot-panels)
   - [Key Learnings](#key-learnings-from-starpilot-implementation)
   - [Common Pitfalls](#common-pitfalls-and-solutions)
   - [Onroad Overlays](#82-priority-2-onroad-overlays)
   - [Additional Features](#83-priority-3-additional-features)
9. [StarPilot Complete Implementation Roadmap](#9-starpilot-complete-implementation-roadmap)
   - [Architecture Overview](#91-architecture-overview)
   - [Current Status](#92-current-status)
   - [Required: Value Control Factory Functions](#93-required-value-control-factory-functions)
   - [Required: Three-Level Navigation System](#94-required-three-level-navigation-system)
   - [Complete Panel Implementation Details](#95-complete-panel-implementation-details)
10. [Technical Reference](#10-technical-reference)
    - [Core Application Class](#101-core-application-class)
    - [Widget Base Class](#102-widget-base-class)
    - [UI State Management](#103-ui-state-management)
    - [Environment Variables](#104-environment-variables)
    - [Device Detection](#105-device-detection)
    - [Widget Factory Functions](#106-widget-factory-functions-reference)
    - [ListItem Class](#107-listitem-class-reference)
11. [RayGUI Analysis (Aborted)](#11-raygui-analysis-aborted)

---

## 1. High-Level Architecture Comparison

### 1.1 Qt UI Structure (Current Legacy)

```
MainWindow (QStackedLayout)
├── HomeWindow (QHBoxLayout)
│   ├── Sidebar (fixed 300px width)
│   └── QStackedLayout
│       ├── OffroadHome (home.cc)
│       ├── OnroadWindow (onroad_home.cc)
│       ├── BodyWindow (body.cc)
│       └── DriverViewWindow (driverview.cc)
├── SettingsWindow (QStackedWidget)
│   └── Multiple panels (Device, Network, Toggles, Software, Firehose, Developer)
└── OnboardingWindow
```

**Entry Point:** `selfdrive/ui/main.cc` → compiled binary `selfdrive/ui/ui`

**Key Files:**
- `selfdrive/ui/qt/window.cc` (Main window management)
- `selfdrive/ui/qt/home.cc` (Home window container)
- `selfdrive/ui/qt/sidebar.cc` (Sidebar implementation)
- **StarPilot Qt Panels**: `starpilot/ui/qt/offroad/starpilot_settings.cc/h` (plus `lateral_settings.cc`, `longitudinal_settings.cc`, etc.)
- **Raylib Port**: `selfdrive/ui/layouts/settings/starpilot` (Modularized!)
- **UI State Singleton**: `selfdrive/ui/lib/starpilot_state.py` (Centralized `StarPilotState`)
- `selfdrive/ui/qt/offroad/settings.cc` (Settings panels)

### 1.2 Raylib Big UI Structure (Current)

```
MainLayout (Widget)
├── Sidebar (300px width - selfdrive/ui/layouts/sidebar.py)
└── State Machine (MainState enum)
    ├── HOME → HomeLayout (selfdrive/ui/layouts/home.py)
    ├── SETTINGS → SettingsLayout (selfdrive/ui/layouts/settings/settings.py)
    └── ONROAD → AugmentedRoadView (selfdrive/ui/onroad/augmented_road_view.py)
```
The StarPilot Raylib Big UI is built on a modular "Panel" architecture. 
The main entry point is `StarPilotLayout` (in `main_panel.py`), which manages a stack of `StarPilotPanel` instances.

**Entry Point:** `selfdrive/ui/ui.py` (lines 13-36)

**Key Files:**
- `selfdrive/ui/ui.py` (Entry point)
- `selfdrive/ui/layouts/main.py` (Main layout container)
- `selfdrive/ui/layouts/sidebar.py` (Sidebar implementation)
- `system/ui/lib/application.py` (GuiApplication class)

### 1.3 Device Resolution Reference

| Device | UI Framework | Resolution |
|--------|-------------|------------|
| TICI/TIZI | Qt (C++) | 2160x1080 |
| MICI/PC | Raylib (Python) | Big: 2160x1080 / Small: 536x240 |

**Note:** The Raylib UI is used on ALL devices (TICI, TIZI, MICI, PC). Big UI (2160x1080) is shown on TICI/TIZI or when BIG=1 environment variable is set.

---

## 2. Key Parallels: Qt to Raylib

### 2.1 Navigation & Layout

| Qt Component | Raylib Equivalent | File Location |
|-------------|-------------------|---------------|
| `QStackedLayout` | `MainState` enum + dictionary | `layouts/main.py:15-32` |
| `QHBoxLayout` | Manual rect calculation | `layouts/main.py:61-65` |
| `QWidget.show()/hide()` | `show_event()`/`hide_event()` | `widgets/__init__.py:180-184` |
| `setFixedWidth(300)` | `SIDEBAR_WIDTH = 300` | `layouts/sidebar.py:12` |
| `setVisible(bool)` | `set_visible(bool)` | `widgets/__init__.py:66-67` |
| `QStackedWidget` | Dictionary + `_set_current_layout()` | `layouts/main.py:83-87` |

### 2.2 Widget System

| Qt Widget | Raylib Widget | File Location |
|-----------|---------------|--------------|
| `QFrame` | `Widget` (base class) | `widgets/__init__.py:22-184` |
| `QPushButton` | `Button` | `widgets/button.py:12-24` |
| `QToggle` | `Toggle` | `widgets/toggle.py:17-79` |
| `QLabel` | `Label`, `MiciLabel`, `UnifiedLabel` | `widgets/label.py:30+` |
| Custom paint | `_render()` method | All widgets |
| `mousePressEvent` | `_handle_mouse_event()` | `widgets/__init__.py:114-153` |
| `QLayout` | Manual rect calculation | Per layout file |

### 2.3 Settings Panels

| Qt Panel | Raylib Panel | Status | Location |
|----------|--------------|--------|----------|
| `DevicePanel` | `DeviceLayout` | ✅ Implemented | `layouts/settings/device.py` |
| `NetworkPanel` | `NetworkUI` | ✅ Implemented | `system/ui/widgets/network.py` |
| `TogglesPanel` | `TogglesLayout` | ⚠️ Basic only | `layouts/settings/toggles.py` |
| `SoftwarePanel` | `SoftwareLayout` | ✅ Implemented | `layouts/settings/software.py` |
| `FirehosePanel` | `FirehoseLayout` | ✅ Implemented | `layouts/settings/firehose.py` |
| `DeveloperPanel` | `DeveloperLayout` | ✅ Implemented | `layouts/settings/developer.py` |

### 2.4 StarPilot Qt Panels

| Qt Panel | Purpose | Qt File Location | Raylib Status |
|----------|---------|-----------------|---------------|
| `StarPilotDataPanel` | Data settings | `starpilot/ui/qt/offroad/data_settings.cc` | 🟡 Implemented (Stub) |
| `StarPilotDevicePanel` | Device controls | `starpilot/ui/qt/offroad/device_settings.cc` | 🟡 Implemented (Stub) |
| `StarPilotLateralPanel` | Steering controls | `starpilot/ui/qt/offroad/lateral_settings.cc` | ✅ Implemented (5 sub-panels with real controls) |
| `StarPilotLongitudinalPanel` | Gas/Brake controls | `starpilot/ui/qt/offroad/longitudinal_settings.cc` | 🟡 Implemented (Weather sub-panels with real value controls) |
| `StarPilotMapsPanel` | Map data | `starpilot/ui/qt/offroad/maps_settings.cc` | 🟡 Implemented (Stub) |
| `StarPilotModelPanel` | Driving model | `starpilot/ui/qt/offroad/model_settings.cc` | ✅ Implemented (Auto-fetch, Selection, Download, Favorites, Sort) |
| `StarPilotNavigationPanel` | Navigation | `starpilot/ui/qt/offroad/navigation_settings.cc` | 🟡 Implemented (Stub) |
| `StarPilotSoundsPanel` | Alerts and sounds | `starpilot/ui/qt/offroad/sounds_settings.cc` | ✅ Implemented (Alert Volume Controller + Custom Alerts sub-panels) |
| `StarPilotThemesPanel` | Theme settings | `starpilot/ui/qt/offroad/theme_settings.cc` | 🟡 Implemented (MANAGE buttons) |
| `StarPilotUtilitiesPanel` | Utilities | `starpilot/ui/qt/offroad/utilities.cc` | 🟡 Implemented (Stub) |
| `StarPilotVehiclesPanel` | Vehicle settings | `starpilot/ui/qt/offroad/vehicle_settings.cc` | 🟡 Implemented (Stub) |
| `StarPilotVisualsPanel` | Appearance/Visuals | `starpilot/ui/qt/offroad/visual_settings.cc` | 🟡 Implemented (MANAGE buttons) |
| `StarPilotWheelPanel` | Wheel controls | `starpilot/ui/qt/offroad/wheel_settings.cc` | 🟡 Implemented (Stub) |

**Main Entry:** `starpilot/ui/qt/offroad/starpilot_settings.cc` - Contains category navigation
**Raylib Implementation:** `selfdrive/ui/layouts/settings/starpilot.py` - Full sub-panel hierarchy implemented

**Qt Back Navigation:**
- Uses Qt signals: `closeSubPanel()`, `closeSubSubPanel()` signals
- Parent `SettingsWindow` emits these signals, child panels listen and respond
- Each sub-panel has a way to emit signal to return to previous view

### 2.5 StarPilot Raylib Folders
- `selfdrive/ui/layouts/settings/starpilot/` (Directory containing 14+ setting modules)

### 2.6 StarPilot Key Parallels
- **Qt `StackedLayout`** ≈ **Raylib `_panels` dictionary in `main_panel.py`**
- **Qt `StarPilotPanelType`** ≈ **Raylib `StarPilotPanelType` in `panel.py`** (Integer-based routing)
- **Qt `Panel::showEvent()`** ≈ **Raylib `Widget.show_event()`** (Used for dynamic range calculation)

---

## 3. Code Structure Patterns

### 3.1 Signal/Slot vs Callback Pattern

#### Qt Signal/Slot
```cpp
// From selfdrive/ui/qt/home.cc
QObject::connect(sidebar, &Sidebar::openSettings, this, &HomeWindow::openSettings);
QObject::connect(home, &OffroadHome::openSettings, this, &HomeWindow::openSettings);
QObject::connect(settingsWindow, &SettingsWindow::closeSettings, this, &MainWindow::closeSettings);
```

#### Raylib Callback
```python
# From selfdrive/ui/layouts/main.py
def _setup_callbacks(self):
    self._sidebar.set_callbacks(on_settings=self._on_settings_clicked,
                                on_flag=self._on_bookmark_clicked,
                                open_settings=lambda: self.open_settings(PanelType.TOGGLES))
    self._layouts[MainState.HOME]._setup_widget.set_open_settings_callback(...)
    self._layouts[MainState.SETTINGS].set_callbacks(on_close=self._set_mode_for_state)
    self._layouts[MainState.ONROAD].set_click_callback(self._on_onroad_clicked)
    device.add_interactive_timeout_callback(self._set_mode_for_state)
```

### 3.2 Widget Callback Registration

#### Qt
```cpp
// From selfdrive/ui/qt/widgets/toggle.cc
QObject::connect(toggle, &Toggle::stateChanged, this, &TogglesPanel::toggleToggled);
```

#### Raylib
```python
# From system/ui/widgets/toggle.py
class Toggle(Widget):
    def __init__(self, initial_state: bool = False, callback: Callable[[bool], None] | None = None):
        self._callback = callback
    
    def _handle_mouse_release(self, mouse_pos: MousePos):
        self._state = not self._state
        if self._callback:
            self._callback(self._state)
```

### 3.3 Paint Event vs Render Method

#### Qt Custom Paint
```cpp
// From selfdrive/ui/qt/sidebar.cc
void Sidebar::drawMetric(QPainter &p, const QPair<QString, QString> &label, QColor c, int y) {
    const QRect rect = {30, y, 240, 126};
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(c));
    p.drawRoundedRect(QRect(rect.x() + 4, rect.y() + 4, 100, 118), 18, 18);
}
```

#### Raylib Render
```python
# From selfdrive/ui/layouts/sidebar.py
def _draw_metric(self, rect: rl.Rectangle, label: str, value: str, color: rl.Color):
    # Draw colored bar
    bar_rect = rl.Rectangle(rect.x + 4, rect.y + 4, 100, 118)
    rl.draw_rectangle_rounded(bar_rect, 1.0, 18, color)
```

### 3.4 Toggle Implementation Comparison

#### Qt Toggle (`widgets/toggle.cc`)
- Uses `QPropertyAnimation` for knob animation
- Custom painting with `QPainter`
- 80px height, variable width
- Green (#178644) when ON, Gray (#393939) when OFF

#### Raylib Toggle (`widgets/toggle.py`)
```python
WIDTH, HEIGHT = 160, 80
BG_HEIGHT = 60
ANIMATION_SPEED = 8.0

ON_COLOR = rl.Color(51, 171, 76, 255)   # Green
OFF_COLOR = rl.Color(0x39, 0x39, 0x39, 255)  # Dark gray

class Toggle(Widget):
    def _render(self, rect: rl.Rectangle):
        # Background
        bg_rect = rl.Rectangle(self._rect.x + 5, self._rect.y + 10, WIDTH - 10, BG_HEIGHT)
        rl.draw_rectangle_rounded(bg_rect, 1.0, 10, bg_color)
        
        # Knob
        knob_x = self._rect.x + HEIGHT / 2 + (WIDTH - HEIGHT) * self._progress
        rl.draw_circle(int(knob_x), int(knob_y), HEIGHT / 2, knob_color)
```

### 3.5 State Management Pattern

#### Qt State
```cpp
// From selfdrive/ui/qt/home.cc
void HomeWindow::updateState(const UIState &s, const StarPilotUIState &fs) {
    if (s.scene.started) {
        if (starpilot_scene.driver_camera_timer >= UI_FREQ / 2) {
            showDriverView(true, true);
        } else {
            slayout->setCurrentWidget(onroad);
        }
    }
}
```

#### Raylib State
```python
# From selfdrive/ui/layouts/main.py
class MainState(IntEnum):
    HOME = 0
    SETTINGS = 1
    ONROAD = 2

class MainLayout(Widget):
    def _handle_onroad_transition(self):
        if ui_state.started != self._prev_onroad:
            self._prev_onroad = ui_state.started
            self._set_mode_for_state()
    
    def _set_mode_for_state(self):
        if ui_state.started:
            self._set_current_layout(MainState.ONROAD)
        else:
            self._set_current_layout(MainState.HOME)
```

---

## 4. Onroad UI Comparison

### 4.1 Qt Onroad Components

```
OnroadWindow (onroad_home.cc)
├── AnnotatedCameraWidget (camera + lane lines + path)
│   ├── ModelRenderer (lane lines, path predictions)
│   └── HudRenderer (speed, cruise control)
├── OnroadAlerts (alert messages)
├── StarPilotAnnotatedCameraWidget (StarPilot overlays)
└── StarPilotOnroadWindow (additional overlays)
    ├── Blind spot visualization
    ├── FPS counter
    ├── Steering torque metrics
    └── Turn signal indicators
```

**Key Files:**
- `selfdrive/ui/qt/onroad/onroad_home.cc`
- `selfdrive/ui/qt/onroad/annotated_camera.cc`
- `selfdrive/ui/qt/onroad/hud.cc`
- `selfdrive/ui/qt/onroad/alerts.cc`
- `selfdrive/ui/qt/onroad/buttons.cc`
- `starpilot/ui/qt/onroad/starpilot_onroad.cc`

### 4.2 Raylib Onroad Components

```
AugmentedRoadView (augmented_road_view.py - extends CameraView)
├── ModelRenderer (lane lines, path predictions) ✅
├── HudRenderer (speed, cruise control, status) ✅
├── AlertRenderer (driving alerts) ✅
└── DriverStateRenderer (driver monitoring) ✅
```

**Key Files:**
- `selfdrive/ui/onroad/augmented_road_view.py` (234 lines)
- `selfdrive/ui/onroad/model_renderer.py`
- `selfdrive/ui/onroad/hud_renderer.py` (180 lines)
- `selfdrive/ui/onroad/alert_renderer.py`
- `selfdrive/ui/onroad/driver_state.py`
- `selfdrive/ui/onroad/cameraview.py`

### 4.3 Onroad Features: Qt vs Raylib

| Feature | Qt | Raylib | Notes |
|---------|-----|--------|-------|
| Camera feed | ✅ | ✅ | |
| Lane lines | ✅ | ✅ | |
| Path predictions | ✅ | ✅ | |
| Speed display | ✅ | ✅ | |
| Cruise control | ✅ | ✅ | |
| Alert messages | ✅ | ✅ | |
| Driver monitoring | ✅ | ✅ | |
| Blind spot metrics | ✅ | ❌ | Needs port |
| FPS counter | ✅ | ❌ | Needs port |
| Steering torque | ✅ | ❌ | Needs port |
| Turn signals | ✅ | ❌ | Needs port |
| Theme support | ✅ | ⚠️ | Partial |

---

## 5. Sidebar Comparison

### 5.1 Qt Sidebar (`qt/sidebar.cc` - 302 lines)

```cpp
// Fixed 300px width
Sidebar::Sidebar(QWidget *parent) : QFrame(parent) {
    setFixedWidth(300);
    
    // Metrics displayed:
    // - Temperature (with color coding)
    // - CPU usage (with tap-to-cycle)
    // - Memory usage (with tap-to-cycle)
    // - Panda status
    // - Network status
    
    // Buttons:
    // - Home button (180x180)
    // - Flag button
    // - Settings button (200x117)
}
```

### 5.2 Raylib Sidebar (`layouts/sidebar.py` - 229 lines)

```python
SIDEBAR_WIDTH = 300
METRIC_HEIGHT = 126
METRIC_WIDTH = 240
METRIC_MARGIN = 30
FONT_SIZE = 35

class Sidebar(Widget):
    def __init__(self):
        # Same metrics: Temperature, CPU, Memory, Panda, Network
        # Same buttons with texture-based rendering
        
        self._home_img = gui_app.texture("images/button_home.png", HOME_BTN.width, HOME_BTN.height)
        self._flag_img = gui_app.texture("images/button_flag.png", HOME_BTN.width, HOME_BTN.height)
        self._settings_img = gui_app.texture("images/button_settings.png", SETTINGS_BTN.width, SETTINGS_BTN.height)
```

### 5.3 Differences

| Aspect | Qt | Raylib |
|--------|-----|--------|
| Width | 300px | 300px |
| Temperature | ✅ with color | ✅ with color |
| CPU | ✅ with tap cycle | ✅ basic |
| Memory | ✅ with tap cycle | ✅ basic |
| Panda | ✅ | ✅ |
| Network | ✅ | ✅ |
| Developer toggle | ✅ | ❌ |

---

## 6. Settings Layout Comparison

### 6.1 Qt Settings Structure

```cpp
// From selfdrive/ui/qt/offroad/settings.cc
SettingsWindow::SettingsWindow(QWidget *parent) {
    // Left sidebar navigation (icon + text)
    // Panel selector: Device, Network, Toggles, Software, Firehose, Developer
    
    // Panel content area on right
    // Scrollable content within each panel
}
```

### 6.2 Raylib Settings Structure

```python
# From selfdrive/ui/layouts/settings/settings.py
SIDEBAR_WIDTH = 500  # Wider than main sidebar
NAV_BTN_HEIGHT = 110
PANEL_MARGIN = 50

class PanelType(IntEnum):
    DEVICE = 0
    NETWORK = 1
    TOGGLES = 2
    SOFTWARE = 3
    FIREHOSE = 4
    DEVELOPER = 5

class SettingsLayout(Widget):
    def _render(self, rect: rl.Rectangle):
        # Left sidebar (500px) with nav buttons
        # Right panel area
        self._draw_sidebar(sidebar_rect)
        self._draw_current_panel(panel_rect)
```

### 6.2.1 Settings Panel Width Comparison

| Panel | Qt Sidebar | Raylib Sidebar |
|-------|------------|----------------|
| Main Settings | 300px | 500px |

---

## 7. Implementation Differences

### 7.1 Language & Framework

| Aspect | Qt | Raylib |
|--------|-----|--------|
| Language | C++ | Python |
| Graphics API | Qt QPainter | pyray (OpenGL wrapper) |
| Event System | Qt event loop | raylib input + custom mouse thread |

### 7.2 Event Loop

#### Qt Event Loop
- Qt's built-in signal/slot mechanism
- `QApplication::exec()` runs the loop
- Events dispatched via `QObject::event()`

#### Raylib Event Loop
```python
# From selfdrive/ui/ui.py
def main():
    gui_app.init_window("UI")
    if gui_app.big_ui():
        main_layout = MainLayout()
    else:
        main_layout = MiciMainLayout()
    
    for should_render in gui_app.render():  # Generator
        ui_state.update()
        if should_render:
            main_layout.render()
```

### 7.3 Text Rendering

#### Qt
```cpp
p.setFont(InterFont(35, QFont::DemiBold));
p.drawText(rect, Qt::AlignCenter, text);
```

#### Raylib
```python
# From system/ui/widgets/label.py
font = gui_app.font(FontWeight.NORMAL)
rl.draw_text_ex(font, text, position, font_size, spacing, color)
```

### 7.4 Layout Calculations

#### Qt (Automatic)
```cpp
QHBoxLayout *main_layout = new QHBoxLayout(this);
main_layout->setMargin(0);
main_layout->setSpacing(0);
main_layout->addWidget(sidebar);
```

#### Raylib (Manual)
```python
# From selfdrive/ui/layouts/main.py
def _update_layout_rects(self):
    self._sidebar_rect = rl.Rectangle(self._rect.x, self._rect.y, SIDEBAR_WIDTH, self._rect.height)
    x_offset = SIDEBAR_WIDTH if self._sidebar.is_visible else 0
    self._content_rect = rl.Rectangle(self._rect.y + x_offset, self._rect.y, self._rect.width - x_offset, self._rect.height)
```

### 7.5 Animations

#### Qt (Property Animation)
```cpp
// From selfdrive/ui/qt/widgets/toggle.cc
_anim = new QPropertyAnimation(this, "offset_circle", this);
_anim->setStartValue(on ? left + immediateOffset : right - immediateOffset);
_anim->setEndValue(on ? right : left);
_anim->setDuration(animation_duration);
_anim->start();
```

#### Raylib (Manual Interpolation)
```python
# From system/ui/widgets/toggle.py
def update(self):
    if abs(self._progress - self._target) > 0.01:
        delta = rl.get_frame_time() * ANIMATION_SPEED
        self._progress += delta if self._progress < self._target else -delta
        self._progress = max(0.0, min(1.0, self._progress))
```

---

## 8. Files Requiring Work for Full Port

### 8.1 Priority 1: Core Infrastructure

| File | Description | Status |
|------|-------------|--------|
| `selfdrive/ui/layouts/settings/` | StarPilot panels | 🟡 Structure Done (stubs for most, Lateral fully implemented) |
| `system/ui/widgets/toggle.py` | Add description support | ✅ Done |
| `system/ui/widgets/input_dialog.py` | Text input dialog with keyboard | ✅ Done (NEW) |
| `system/ui/widgets/selection_dialog.py` | Radio-button selection dialog | ✅ Done (NEW) |
| `system/ui/widgets/confirm_dialog.py` | Confirm/alert dialog with rich text | ✅ Done (UPDATED) |
| `selfdrive/ui/lib/starpilot_state.py` | Car state + StarPilotState singleton | ✅ Done (NEW) |
| `common/params.py` | Params get_int/get_float/put_int/put_float | ✅ Done (UPDATED) |
| `selfdrive/ui/layouts/sidebar.py` | Developer metrics toggle | 🔴 Not Started |

### 8.1.1 StarPilot Panel Hierarchy (Implemented)

```
StarPilot Settings
├── Tuning Level ✅
├── Category: Alerts and Sounds → SOUNDS ✅ (Alert Volume Controller + Custom Alerts sub-panels with real controls)
│   ├── Alert Volume Controller ✅ (7 volume sliders with Test buttons, persistent subprocess for offroad sound testing)
│   └── Custom Alerts ✅ (5 toggles with conditional visibility: BSM, ShowSpeedLimits/SpeedLimitController)
├── Category: Driving Controls
│   ├── DRIVING_MODEL ✅ (Auto-fetch, Premium SelectionDialog, Managed background downloads, Blacklist/Ratings)
│   ├── LONGITUDINAL 🟡 (MANAGE buttons + Weather sub-panels with real value_item controls)
│   └── LATERAL ✅ (5 sub-panels with REAL controls: value_item, value_button_item, toggle_item)
│       ├── Advanced Lateral Tuning ✅ (8 controls: 5 value_button_item + 3 toggle_item)
│       ├── Always On Lateral ✅ (3 controls)
│       ├── Lane Changes ✅ (6 controls with conditional visibility)
│       ├── Lateral Tuning ✅ (3 toggles with reboot confirmation)
│       └── Quality of Life ✅ (1 value_button_item with sub-toggle)
├── Category: Navigation
│   ├── MAPS 🟡 (stub)
│   └── NAVIGATION 🟡 (stub)
├── Category: System Settings
│   ├── DATA 🟡 (stub)
│   ├── DEVICE 🟡 (stub)
│   └── UTILITIES 🟡 (stub)
├── Category: Theme and Appearance
│   ├── VISUALS 🟡 (5 MANAGE buttons)
│   └── THEMES 🟡 (stub)
└── Category: Vehicle Settings
    ├── VEHICLE 🟡 (stub)
    └── WHEEL 🟡 (stub)
```

Legend: ✅ = Full | 🟡 = Structure/stub | 🔴 = Not started

#### Completed (March 16, 2026)
- Added `StarPilotLayout` entry to `settings/settings.py`
- Created `layouts/settings/starpilot.py` with full sub-panel navigation system
- Added `StarPilotPanelType` enum with 14 panel types for proper sub-panel routing:
  - MAIN, SOUNDS, DRIVING_MODEL, LONGITUDINAL, LATERAL, MAPS, NAVIGATION,
  - DATA, DEVICE, UTILITIES, VISUALS, THEMES, VEHICLE, WHEEL
- Implemented `StarPilotSoundsLayout` with 6 toggle items for custom alerts
- Implemented `StarPilotDrivingModelLayout` with toggles + buttons
- Implemented `StarPilotLongitudinalLayout`, `StarPilotLateralLayout` with MANAGE buttons
- Implemented stub layouts for all remaining panels (Maps, Navigation, Data, Device, Utilities, Visuals, Themes, Vehicle, Wheel)
- Added tuning level visibility filtering to sub-panels (`set_tuning_levels()`, `refresh_visibility()`)
- Uses `button_item` with "MANAGE" button text matching Qt UI
- Added `starpilot_texture()` method to `GuiApplication` for loading StarPilot assets
- Added `starpilot_icon` parameter to `button_item()` and `toggle_item()` for StarPilot icon support
- StarPilot assets remain in `starpilot/assets/` (not copied)
- Descriptions use HTML bold tags (`<b>...</b>`) matching Qt format
- Toggle callbacks properly persist settings via Params
- **Hierarchical back navigation**: Settings sidebar back button handles depth > 0 (go back) vs depth 0 (close)
- **Category buttons**: Fixed with per-button width calculation + cumulative positioning + matching width_hint

#### Completed (March 17, 2026)
- **Created `StarPilotState` singleton** (`selfdrive/ui/lib/starpilot_state.py`) with:
  - `StarPilotCarState` dataclass with all car type, capability, and value fields
  - Reads `CarParamsPersistent`, `FrogPilotCarParamsPersistent`, `LiveTorqueParameters`, `FrogPilotToggles` from Params
  - Throttled updates (2.0s interval) to avoid slowing UI
  - PC/desktop fallback mode with configurable car make/model
  - Global import: `from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state`
- **Fully implemented Lateral panel** with 5 sub-panels and real controls:
  - `StarPilotAdvancedLateralLayout`: 5 `value_button_item` controls with car-specific dynamic ranges (`starpilot_state.car_state.steerKp * 0.5` to `* 1.5`) + Reset buttons + 3 toggle items with conditional visibility based on car state
  - `StarPilotAlwaysOnLateralLayout`: 2 toggles (with reboot confirmation) + 1 value slider
  - `StarPilotLaneChangesLayout`: 6 controls with conditional visibility (`LaneChanges AND NudgelessLaneChange`)
  - `StarPilotLateralTuneLayout`: 3 toggles (NNFF, NNFFLite, TurnDesires) with reboot confirmation
  - `StarPilotLateralQOLLayout`: 1 `value_button_item` with sub-toggle
- **Weather sub-panels now use real `value_item()` controls** with `put_int()` callbacks
- **Created `InputDialog` widget** (`system/ui/widgets/input_dialog.py`):
  - Full keyboard input with `Keyboard` widget integration
  - Text input field with hint text, blinking cursor
  - Confirm/Cancel buttons, dimmed background overlay
  - Callback: `on_close(DialogResult, str)` returns entered text
- **Created `SelectionDialog` widget** (`system/ui/widgets/selection_dialog.py`):
  - Radio-button selection from a list of options
  - Scrollable list using `Scroller` widget
  - Visual feedback: green dot for selected, circle outline for unselected
  - Callback: `on_close(DialogResult, int, str)` returns index and text
- **Extended `Params` wrapper** (`common/params.py`):
  - `get_int(key, block, return_default, default)` — Parses string param as int
  - `get_float(key, block, return_default, default)` — Parses string param as float
  - `put_int(key, val)` — Type-aware save (checks `get_type()` for FLOAT/INT/BOOL/string)
  - `put_float(key, val)` — Same type-aware save for float values
  - Fixes `TypeError` when saving FLOAT-typed params like `IncreaseFollowingLowVisibility`
- **Updated `ConfirmDialog`** (`system/ui/widgets/confirm_dialog.py`):
  - Added `rich` mode with `HtmlRenderer` + `Scroller` for rich text content
  - Scrollable text area for long messages
  - Keyboard shortcut support: Enter (confirm), Escape (cancel)
- **Settings back button icon changed** from `icons/close2.png` → `icons/backspace.png`
- **Zero-size image guard** in `application.py`: `_load_image_from_path` now returns early if `image.width == 0 or image.height == 0`, preventing potential crashes from empty textures

#### Completed (March 17, 2026 - Late Session)
- **Fully Implemented Driving Model Panel**: 
  - Integrated `ModelManager` for auto-fetching model lists if empty.
  - Implemented managed background download thread in `starpilot.py` for desktop/PC environments (Daemon-less).
  - Added blacklist and ratings management dialogs.
- **High-Fidelity SelectionDialog Enhancements**:
  - **Favorites (Star) System**: Every item can be starred (♥/♡). Persisted in `UserFavorites` and `CommunityFavorites`.
  - **Premium Styling**: Matched Qt header (`#333333` BG) and selection (`#465BEA` BG + 3px white border).
  - **Cyclic Sort Mode**: Sort button cycles through: Alphabetical -> Newest -> Oldest -> Favorites First.
  - **Disclosure Triangles**: Used text-based symbols (`▶`/`▼`) for collapsible categories to match Qt without complex mesh drawing.
  - **Width Inheritance**: Fixed hit-testing and alignment by ensuring list items span the full width of the scroller.

#### Completed (March 17, 2026) - Sounds Panel Full Implementation
- **Sounds panel restructured** to use sub-panel navigation:
  - Main panel has 2 MANAGE buttons: "Alert Volume Controller" and "StarPilot Alerts"
  - `StarPilotVolumeControlLayout`: 7 volume sliders (Disengage, Engage, Prompt, PromptDistracted, Refuse, WarningSoft, WarningImmediate)
  - `StarPilotCustomAlertsLayout`: 5 toggles (GoatScream, GreenLightAlert, LeadDepartingAlert, LoudBlindspotAlert, SpeedLimitChangedAlert)
- **Volume slider features**:
  - Range 0-101 (0=Muted, 101=Auto, 1-100=percentage)
  - WarningSoft/WarningImmediate have min=25 (can't go below 25%)
  - Each slider has "Test" button to preview sound
- **Sound testing architecture** (critical fix):
  - **Offroad**: Uses persistent Python subprocess (matching Qt's `initializeSoundPlayer()`)
  - Subprocess runs a while loop reading `path|volume\n` lines from stdin
  - Uses `sounddevice` to play WAV files with volume scaling
  - No threading - stdin automatically serializes writes (subprocess isolation prevents memory corruption)
  - Theme sounds take priority over stock sounds (`ACTIVE_THEME_PATH / "sounds"`)
  - **Onroad**: Uses `TestAlert` param in `params_memory` (handled by `soundd.py`)
- **Custom Alerts conditional visibility**:
  - `LoudBlindspotAlert`: Visible only if `starpilot_state.car_state.hasBSM`
  - `SpeedLimitChangedAlert`: Visible if `ShowSpeedLimits` OR (`hasOpenpilotLongitudinal` AND `SpeedLimitController`)
- **Sub-panel navigation**: `_setup_sounds_sub_panels()` in `StarPilotLayout` wires up navigation callbacks

### 8.1.1 Implementation Guide - Adding New StarPilot Sub-Panels (The Modular Way)

Since the StarPilot settings split (March 2026), adding or updating panels follows a modular file-based pattern:

1.  **Create a new module** in `selfdrive/ui/layouts/settings/starpilot/` (e.g., `new_feature.py`).
2.  **Inherit from `StarPilotPanel`** (from `panel.py`):
    ```python
    from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
    
    class StarPilotNewFeatureLayout(StarPilotPanel):
      def __init__(self):
        super().__init__()
        # Build Layout...
    ```
3.  **Restoring Base Class Methods (IMPORTANT)**: If you override `__init__`, always check if you need to restore core methods that the monolithic refactor moved to the base class:
    -   `self.set_tuning_levels()`
    -   `self._update_state()` (for Driving Model)
    -   `self.refresh_visibility()`
4.  **Register in `main_panel.py`**:
    -   Import the new class.
    -   Add a key to `StarPilotPanelType`.
    -   Add to the `_panels` dictionary in `StarPilotLayout`.
5.  **Update `refresh_visibility` in `main_panel.py`**: Ensure the new panel is included in the loop to react to tuning level changes.

#### Key Learning: Large File Prevention
Don't let any single panel layout exceed ~500 lines. If a panel has many sub-sub-panels (like Longitudinal), break those into their own classes or separate files (e.g. `longitudinal_tuning.py`).

#### Key Learnings from StarPilot Implementation

1.  **Native Pattern is Critical**: Always use `Scroller` + factory functions (`button_item`, `toggle_item`, etc.) - never draw custom rectangles. This ensures:
    -   Consistent styling with other panels
    -   Proper scrolling behavior
    -   Built-in description toggle on tap
    -   Proper touch/mouse event handling

2.  **Sub-Panel Navigation & Boilerplate Reduction**: StarPilot uses a DRY architecture via the `StarPilotPanel(Widget)` base class:
    -   Inheriting from `StarPilotPanel` automatically provides: `self._params`, `self._params_memory`, `self._tuning_levels`, and navigation lifecycle functions.
    -   It also automatically handles standard `_render()` and `show_event()` routing for `self._scroller` and `self._sub_panels` maps.
    -   Main categories route to sub-panels via a two-level navigation system using `StarPilotPanelType` enums.

3.  **Asset Loading for External Packages**:
    -   Use `gui_app.texture()` for openpilot assets (`selfdrive/assets/`)
    -   Use `gui_app.starpilot_texture()` for assets located in the core `starpilot` assets folder.
```python
item.set_icon("icon_steering.png", starpilot=True) # Uses starpilot_texture internally
```
    -   Never copy assets between folders - always load from original location
    -   Note: StarPilot toggle icons are in `starpilot/assets/toggle_icons/`

4.  **Icon Handling in List Items**:
    ```python
    # For StarPilot icons, use starpilot_icon=True parameter
    item = button_item(
      title_fn, button_fn, desc_fn,
      icon="toggle_icons/icon_sound.png",  # Full path from assets root
      starpilot_icon=True,  # This flag is critical!
    )
    ```

5.  **Description Formatting**: Use HTML bold tags to match Qt:
    ```python
    description=tr_noop("<b>Description text here</b>")
    ```

6.  **Button Text Patterns**:
    -   Navigation/management: "MANAGE", "VIEW", "CHANGE"
    -   NOT ">" or similar symbols

7.  **Python 3.12 Compatibility**:
    -   Use `from __future__ import annotations` at top of files
    -   Use `Optional[X]` instead of `X | None` for type hints
    -   Use `from typing import Callable` for callbacks

8.  **Known Issues**:
    -   Empty string handling in `set_icon()` can cause division by zero if texture loading fails
    -   Icons from StarPilot assets require `starpilot_icon=True` parameter

9.  **Back Navigation Pattern**:
    -   In Raylib, there's no built-in "back" button like in Qt's stacked layout
    -   Each sub-panel needs a "Back" button to return to the parent/main view
    -   Pattern: Create a callback in the parent layout (`_back_to_main`) and pass it to child layouts
    -   Use `button_item` with a "<" or "Back" button text for the back action
    -   See `starpilot.py` for the complete implementation

10. **Premium UI "TLC" (High-Fidelity Parity)**:
    -   **Colors**: Use specific Qt hex codes (e.g., `#333333` for headers, `#465BEA` for selection).
    -   **Hit Testing**: Ensure buttons/items span the full width of their container. Users expect to be able to click anywhere on the row, not just the text.
    -   **Transitions**: When a download starts, the UI should immediately reflect it (e.g., "CANCEL" button instead of "DOWNLOAD").
    -   **Symbols**: Raylib handles Unicode characters well—use fonts that support characters like `▶`, `▼`, `♥`, and `♡` for quick, clean UI elements.

11. **Background Process Management**:
    -   **Daemon-less Downloads**: On PC/Desktop where `starpilot_process.py` isn't running, the UI must manage its own background threads.
    -   **Thread Safety**: Always wrap long-running UI-initiated threads in `try-except` blocks. If they crash, they fail silently in Raylib.
    -   **Progress Tracking**: Poll `Params` (memory) for progress updates (`ModelDownloadProgress`) to keep the UI responsive.

12. **Tuning Level System & Developer Panel Visibility**

    The Qt UI has a sophisticated tuning level system that controls which StarPilot settings are visible:
    -   **Tuning Levels**: 0=Minimal, 1=Standard, 2=Advanced, 3=Developer
    -   Each StarPilot toggle has a minimum required tuning level (stored in `starpilotToggleLevels` map)
    -   Toggle visibility: `tuningLevel >= starpilotToggleLevels[key]`

    **IMPORTANT - Two Different "Developer" Concepts:**
    -   **"Developer" Tuning Level** (level 3 in StarPilot's Tuning Level selector): The highest level that unlocks ALL StarPilot settings
    -   **"Developer" Panel** (separate panel in Settings sidebar): ONLY visible when Tuning Level >= 3

    These are different features that share the same name! This matches Qt behavior:
    ```cpp
    // From settings.cc - Developer panel visibility
    void SettingsWindow::updateDeveloperToggle(int tuningLevel) {
      for (QAbstractButton *btn : nav_btns->buttons()) {
        if (btn->text() == tr("Developer")) {
          btn->setVisible(tuningLevel >= 3);
          break;
        }
      }
    }
    ```

    **Implementation in Raylib:**
    -   `StarPilotLayout` stores reference to `SettingsLayout` via `set_settings_layout()`
    -   When tuning level changes, calls `settings_layout.refresh_developer_visibility()`
    -   Developer panel checked dynamically on each render

13. **Category Buttons with Horizontal Layout**

    Qt displays multiple buttons horizontally next to each category title. Implemented in Raylib using a new custom action:

    **New Component: `CategoryButtonsAction`**
    ```python
    # In system/ui/widgets/list_view.py
    class CategoryButtonsAction(ItemAction):
      def __init__(self, buttons, button_width=180, enabled=True):
        # Auto-sizes buttons based on text content
        # Renders multiple buttons horizontally
    ```

    **Factory Function:**
    ```python
    category_buttons_item(
      title="Category Title",
      buttons=[("BUTTON1", callback1), ("BUTTON2", callback2)],
      description="Description text",
      icon="icon.png",
      starpilot_icon=True,
    )
    ```

    **CATEGORIES Structure** (in `starpilot.py`):
    ```python
    CATEGORIES = [
      {"title": "Alerts and Sounds", "icon": "icon_sound.png", "desc": "...",
       "buttons": [("MANAGE", "SOUNDS", 0)]},
      {"title": "Driving Controls", "icon": "icon_steering.png", "desc": "...",
       "buttons": [("DRIVING MODEL", "DRIVING_MODEL", 0), ("GAS / BRAKE", "LONGITUDINAL", 0), ("STEERING", "LATERAL", 0)]},
      # ... etc
    ]
    ```

    **IMPORTANT - Each Button Needs Unique Panel Type**:
    -   Qt uses stacked layouts where each category button switches to a different panel
    -   Originally we made the mistake of mapping all 3 Driving buttons to the same "DRIVING" panel
    -   Fix: Each button must map to its own unique `StarPilotPanelType` (DRIVING_MODEL, LONGITUDINAL, LATERAL)
    -   The panel dictionary must have entries for each unique panel type

    Each button tuple is: `(button_label, panel_key, min_level)`
    -   `button_label`: Text shown on button
    -   `panel_key`: Which panel to open
    -   `min_level`: Minimum tuning level required to see this button

    **Tuning Level Filtering:**
    -   Level 0 (Minimal): Shows buttons with min_level 0
    -   Level 1 (Standard): Shows buttons with min_level 0 or 1
    -   Level 2 (Advanced): Shows buttons with min_level 0, 1, or 2
    -   Level 3 (Developer): Shows all buttons

#### StarPilot Panel Categories (6 Main Categories)

| Category | Qt Panel | Raylib File | Sub-panels |
|----------|----------|-------------|------------|
| Alerts and Sounds | `StarPilotSoundsPanel` | `sounds.py` | 1 |
| Driving Controls | `DrivingModel`, `Lateral`, `Longitudinal` | `driving_model.py`, `lateral.py`, `longitudinal.py` | 5 |
| Navigation | `Maps`, `Navigation` | `maps.py`, `navigation.py` | 1 |
| System Settings | `Data`, `Device`, `Utilities` | `data.py`, `device.py`, `utilities.py` | 3 |
| Theme and Appearance | `Visuals`, `Themes` | `visuals.py`, `themes.py` | 2 |
| Vehicle Settings | `Vehicles`, `Wheel` | `vehicle.py`, `wheel.py` | 2 |

**Total: 14 modules across 6 categories**

#### Common Pitfalls and Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| Division by zero when loading texture | Empty string passed to texture loader, or file doesn't exist | Pass empty string to ListItem init, then call `set_icon(path, starpilot=True)` separately. Ensure StarPilot icons use `starpilot_icon=True` flag |
| Icons not loading | Wrong asset path or missing starpilot flag | Use full path (`toggle_icons/icon.png`) and `starpilot_icon=True` |
| UI doesn't match native panels | Custom drawing instead of Scroller | Always use Scroller + factory functions |
| Button text wrong | Using ">" instead of "MANAGE" | Use descriptive button text from Qt UI |
| Descriptions don't render | Missing HTML tags | Use `<b>...</b>` tags in descriptions |
| Sub-panel navigation | Need to switch between sub-panels | Use `StarPilotPanelType` enum + dictionary pattern like in `starpilot.py` |
| **No way to go back** | Sub-panels have no back button | Use hierarchical back button in settings sidebar - implements global back navigation (depth > 0 = go back, depth 0 = close settings) |
| **Confusing "Developer" naming** | Two different features with same name | Remember: "Developer" Tuning Level (level 3) controls StarPilot toggle visibility; "Developer" Panel (Settings sidebar) only appears at Tuning Level >= 3 |
| **Toggle visibility not filtering** | Sub-panels don't check tuning level | Implement `set_tuning_levels()` and `refresh_visibility()` methods in each sub-panel layout |
| **Category buttons not side-by-side** | Need horizontal button layout next to title | Use `category_buttons_item()` factory function with `CategoryButtonsAction` |
| **Each category button opens same panel** | Wrong - all 3 Driving buttons opened same panel | Each button needs unique panel_key in CATEGORIES + unique StarPilotPanelType enum value + panel dict entry |
| **Category buttons overlapping title** | CategoryButtonsAction positioning was wrong | Buttons should start at `rect.x` (left edge of action rect), NOT `rect.x + rect.width`. The action rect is already positioned at the right side of the item by `get_right_item_rect`. |
| **Category buttons text overflow** | Text like "DRIVING MODEL" (~220px) overflows fixed-width 150px buttons | **Solution**: Per-button width based on text + padding (20px), scaled proportionally to fit available space. Algorithm: (1) For each button, calculate `ideal_width = text_width + 20px`, (2) Calculate total ideal width + spacing, (3) If total > available space, scale all widths proportionally, else use ideal widths. BUTTON_FONT_SIZE = 35 (matches Qt). |
| **Category buttons overlapping/gaps** | Button positioning used wrong formula: `i * (btn_w + spacing)` instead of cumulative sum. This caused buttons to overlap or have inconsistent gaps. | **Fix**: Use cumulative positioning. Initialize `current_button_x = rect.x`, then after drawing each button, advance: `current_button_x += btn_w + spacing`. This correctly positions each button after the previous one. Example with 3 buttons (widths: 146, 128, 92, spacing: 20): Button1@rect.x, Button2@rect.x+166, Button3@rect.x+314. Total width: 406px with proper 20px gaps. |
| **Category buttons overflow (Vehicle Settings)** | width_hint in __init__ used fixed 150px per button, but _render used dynamic text-based widths. Mismatch caused incorrect space allocation, leading to overflow/overlap. | **Fix**: Calculate width_hint in __init__ the same way as _render: iterate through buttons, measure text, add padding (20px), sum with spacing. Example for Vehicle Settings: "VEHICLE SETTINGS"(140px) + "WHEEL CONTROLS"(130px) + padding(40px) + spacing(20px) = ~330px. This ensures the action rect has exactly the space needed. |
| **Navigation callbacks missing** | Refactor broke manual wiring of sub-panel back buttons | **Fix**: Re-wire `panel.set_back_callback(self._go_back)` in parent layouts (`sounds.py`, `lateral.py`) |
| **`StarPilotPanel` method loss** | Refactor moved methods to base but some subclasses didn't call them | **Fix**: Ensure subclasses call `self.set_tuning_levels()` and `self._update_state()` in `__init__` before return |
| **Model lists empty** | `DrivingModel` missing `_update_model_metadata()` in `__init__` | **Fix**: Explicitly call metadata update and `_update_state` to populate lists before first render |
| **Reboot confirmation pattern** | Many toggles (NNFF, ForceTorqueController, AlwaysOnLateral) require reboot when changed while driving. | **Pattern**: Helper method `_on_reboot_toggle(key, state)` that: (1) saves param, (2) checks `ui_state.started`, (3) shows `ConfirmDialog` via `gui_app.set_modal_overlay()`, (4) calls `HARDWARE.reboot()` on confirm. |
| **Lambda capture in loops** | Creating panel callbacks in a loop (`for cat in CATEGORIES`) where `panel_type` changes but all lambdas capture the same variable. | **Fix**: Use default argument capture: `lambda p=panel_type: self._set_current_panel(p)`. Without `p=panel_type`, all callbacks would reference the last value of `panel_type`. |
| **Sound testing only first button works** | Spawning a new thread per Test button click. `sounddevice` doesn't handle concurrent `sd.play()` calls well - subsequent calls get queued/dropped, and multiple threads cause memory corruption (double free). | **Fix**: Use persistent Python subprocess (matching Qt's `initializeSoundPlayer()`). Subprocess runs a while loop reading `path|volume\n` from stdin. Each click writes to subprocess stdin - no threading locks needed since stdin serializes automatically. This is critical because sounddevice is NOT thread-safe even with locks - must run in isolated subprocess. |

### 8.2 Priority 2: Onroad Overlays

| Feature | Qt File | Status |
|---------|---------|--------|
| Blind spot visualization | `starpilot/ui/qt/onroad/starpilot_onroad.cc` | 🔴 Not Started |
| FPS counter overlay | `selfdrive/ui/onroad/hud_renderer.py` | 🟡 Partial |
| Steering torque | `selfdrive/ui/onroad/hud_renderer.py` | 🔴 Not Started |
| Turn signals | `selfdrive/ui/onroad/hud_renderer.py` | 🔴 Not Started |

### 8.3 Priority 3: Additional Features

| Feature | Qt File | Status |
|---------|---------|--------|
| Drive stats widget | `starpilot/ui/qt/widgets/drive_stats.cc` | 🔴 Not Started |
| Drive summary widget | `starpilot/ui/qt/widgets/drive_summary.cc` | 🔴 Not Started |
| Developer sidebar | `starpilot/ui/qt/widgets/developer_sidebar.cc` | 🔴 Not Started |
| Theme system | Multiple files | 🟡 Partial |

---

## 9. StarPilot Complete Implementation Roadmap

> **Note**: This roadmap was created through detailed analysis of ALL Qt source files in `starpilot/ui/qt/offroad/`. The Qt implementation has **200+ controls** across multiple levels of sub-panels with complex conditional visibility rules.

### 9.1 Architecture Overview

The StarPilot UI has a **three-level hierarchy**:

```
Level 1: Main StarPilot Panel (CATEGORY buttons)
    │
    └── Level 2: Sub-Panel (MANAGE buttons → controls)
              │
              └── Level 3: Sub-Sub-Panel (additional controls)
                        │
                        └── Level 4: Sub-Sub-Sub-Panel (Weather → conditions)
```

**Examples**:
- Driving Controls (category) → GAS/BRAKE (LONGITUDINAL panel) → MANAGE button → Conditional Experimental Mode sub-panel
- Driving Controls → GAS/BRAKE → MANAGE button → Weather → Low Visibility/Rain/Rainstorm/Snow (4 conditions)

**Total Controls**: 200+ toggles, buttons, and value sliders across all panels

### 9.2 Current Status

| Component | Status | Notes |
|-----------|--------|-------|
| Top-level navigation | ✅ Done | 6 categories with horizontal buttons |
| **Panel Modularization** | ✅ Done | split into 14 distinct files (March 2026) |
| StarPilotPanelType (14 types) | ✅ Done | Unique panel types for routing in `panel.py` |
| Sounds panel (toggles) | ✅ Done | Extracted to `sounds.py` |
| **Value control factory functions** | ✅ Done | 8 types: `value_item`, `value_button_item`, `dual_value_item`, `button_toggle_item`, `buttons_item`, `selection_button_item`, `label_item`, `category_buttons_item` |
| **Three-level navigation** | ✅ Done | Panel stack, navigation callbacks, Weather 4-condition sub-panels |
| **StarPilotState singleton** | ✅ Done | Real car param parsing, desktop fallback, 2s throttle |
| **InputDialog widget** | ✅ Done | Keyboard input, hint text, blinking cursor |
| **SelectionDialog widget** | ✅ Done | Radio-button selection, scrollable, green dot indicator |
| **Params helpers** | ✅ Done | get_int, get_float, put_int, put_float |
| **ConfirmDialog rich text** | ✅ Done | HtmlRenderer + Scroller, keyboard shortcuts |
| **Lateral panel** | ✅ Done | 5 sub-panels, ~21 real controls using starpilot_state |
| **Longitudinal panel** | 🟡 Partial | Weather sub-panels (4 conditions with real value_item controls) done, rest not started |
| Sounds panel (volume sliders) | ✅ Done | 7 volume controls + Test buttons + persistent subprocess for offroad sound testing |
| Driving Model panel | 🔴 Not Started | 9 toggles + 7 buttons + dialogs |
| Visual panel | 🔴 Not Started | 5 sub-panels, ~35 controls |
| Themes panel | 🔴 Not Started | Download/select for 7 theme types |
| Navigation panel | 🔴 Not Started | Mapbox keys, setup instructions |
| Data panel | 🔴 Not Started | Backups, storage, stats |
| Device panel | 🔴 Not Started | 2 sub-panels, ~14 controls |
| Vehicle panel | 🔴 Not Started | 5 sub-panels, ~20 controls |
| Wheel panel | 🔴 Not Started | 4 button controls |
| Utilities panel | 🔴 Not Started | 6+ buttons |
| Maps panel | 🔴 Not Started | Download, countries, states |
| Conditional visibility | ✅ Done | Sounds panel Custom Alerts has real conditional visibility rules (BSM, ShowSpeedLimits, SpeedLimitController). Lateral panel has car-state conditional visibility. |
| Metric unit conversion | 🟡 Partial | `is_metric=True` param on value_item exists but conversion logic needs wiring |
| Onroad overlays | 🔴 Not Started | 4 features |
| Developer sidebar | 🔴 Not Started | 1 feature |

### 9.3 Required: Value Control Factory Functions

The Qt UI uses **6 different control types** for value inputs. All need Raylib equivalents:

#### Control Types in Qt:

1.  **`StarPilotParamValueControl`** - Simple value slider
    -   Horizontal slider with min/max/step
    -   Value display with unit label
    -   Optional custom labels (e.g., "Off", "Instant", "X seconds")
    -   Optional metric unit conversion

2.  **`StarPilotParamValueButtonControl`** - Value slider + button
    -   Same as above + "Reset" or "Test" button
    -   Used for: SteerDelay, SteerFriction, volumes, ClusterOffset, PauseLateralSpeed, CESignalSpeed
    -   Button can trigger: Reset to default, Test sound, etc.

3.  **`StarPilotDualParamValueControl`** - Two connected sliders
    -   Two value controls displayed together with shared unit
    -   Used for: CESpeed (Without Lead + With Lead)
    -   Both sliders convert together on metric toggle

4.  **`StarPilotButtonToggleControl`** - Toggle + sub-toggles
    -   Toggle switch with additional sub-option toggles
    -   Used for: CECurves, CELead, MapGears, ToyotaDoors, SLCConfirmation, PedalsOnUI
    -   Can be exclusive (selecting one disables others) or additive

5.  **`StarPilotButtonsControl`** - Multiple action buttons
    -   Row of buttons (DELETE, DOWNLOAD, SELECT, etc.)
    -   Used for: Theme downloads, backups, model management, Weather key
    -   Button count varies: 2-4 buttons per control
    -   Can show/hide individual buttons dynamically

6.  **`ButtonParamControl`** - Button selection (not value)
    -   Opens dialog to select from options
    -   Used for: AccelerationProfile, DecelerationProfile, CameraView, SLCFallback, SLCOverride

7.  **`LabelControl`** - Read-only display
    -   Shows static text or dynamic values
    -   Used for: Vehicle info, Stats, Calibration values, Download status
    -   No user interaction

8.  **`StarPilotManageControl`** - Opens sub-panel
    -   MANAGE/VIEW button that opens nested panel
    -   Used for: All main panel categories

#### Required Factory Functions:

```python
# 1. Simple value slider (StarPilotParamValueControl equivalent)
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
    is_metric: bool = False,  # Auto-convert units
    labels: dict[float, str] = None,  # Custom labels: {0: "Off", 1: "1 second", etc.}
    negative: bool = False,  # Allow negative values
) -> ListItem:
    """Slider for value adjustment with optional unit label"""

# 2. Value slider with button (StarPilotParamValueButtonControl equivalent)
def value_button_item(
    title: str | Callable[[], str],
    value: float | Callable[[], float],
    min_val: float = 0,
    max_val: float = 100,
    step: float = 1,
    unit: str = "",
    button_text: str = "Reset",  # "Reset" or "Test"
    button_callback: Callable | None = None,
    description: str | Callable[[], str] | None = None,
    callback: Callable[[float], None] | None = None,
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
    sub_toggles: list[str] = [],  # Optional sub-toggle param keys
    sub_toggle_names: list[str] = [],  # Display names
    has_sub_toggle: bool = False,  # Has sub-toggle alongside value
    labels: dict[float, str] = None,
    negative: bool = False,
) -> ListItem:
    """Slider with action button (Reset/Test) and optional sub-toggles"""

# 3. Dual value control (StarPilotDualParamValueControl equivalent)
def dual_value_item(
    title: str | Callable[[], str],
    value1: float | Callable[[], float],
    value2: float | Callable[[], float],
    min_val: float = 0,
    max_val: float = 100,
    step: float = 1,
    unit: str = "",
    label1: str = "",  # e.g., "Without Lead"
    label2: str = "",  # e.g., "With Lead"
    description: str | Callable[[], str] | None = None,
    callback1: Callable[[float], None] | None = None,
    callback2: Callable[[float], None] | None = None,
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
) -> ListItem:
    """Two connected value sliders with labels"""

# 4. Button toggle control (StarPilotButtonToggleControl equivalent)
def button_toggle_item(
    title: str | Callable[[], str],
    state: bool | Callable[[], bool],
    sub_toggles: list[str] = [],  # List of sub-toggle param keys
    sub_toggle_names: list[str] = [],  # Display names
    description: str | Callable[[], str] | None = None,
    callback: Callable[[bool], None] | None = None,
    sub_callbacks: list[Callable] = [],
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
    exclusive: bool = False,  # If true, selecting one disables others
) -> ListItem:
    """Toggle with optional sub-toggles (e.g., "With Lead", "Lower Limits")"""

# 5. Multi-button control (StarPilotButtonsControl equivalent)
def buttons_item(
    title: str | Callable[[], str],
    buttons: list[str],  # e.g., ["DELETE", "DOWNLOAD", "SELECT"]
    button_callbacks: list[Callable] = [],
    description: str | Callable[[], str] | None = None,
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
    initial_value: str = "",  # Display current selection
) -> ListItem:
    """Multiple action buttons in a row"""

# 6. Button selection control (ButtonParamControl equivalent)
def selection_button_item(
    title: str | Callable[[], str],
    options: list[str],  # e.g., ["Standard", "Eco", "Sport", "Sport+"]
    selected_index: int = 0,
    description: str | Callable[[], str] | None = None,
    callback: Callable[[int, str], None] = None,  # index, option_text
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
) -> ListItem:
    """Button that opens selection dialog"""

# 7. Label control (LabelControl equivalent) - read-only display
def label_item(
    title: str | Callable[[], str],
    value: str | Callable[[], str],
    description: str | Callable[[], str] | None = None,
    icon: str = "",
    enabled: bool | Callable[[], bool] = True,
) -> ListItem:
    """Read-only label display"""
```

**Features needed across all value controls**:
-   Horizontal slider with min/max/step
-   Current value display with unit
-   Optional "Reset" or "Test" button
-   Metric unit auto-conversion (imperial/metric toggle)
-   Labels update based on metric setting
-   Default value display in title (e.g., "Actuator Delay (Default: 0.5)")
-   Custom labels for special values (e.g., "Off", "Muted", "Auto")
-   Warning labels for certain controls

### 9.4 Required: Three-Level Navigation System

Each MANAGE button in Qt leads to a sub-panel with more controls. The Longitudinal panel has **THREE levels** (Weather):

#### 9.4.1 Raylib Implementation Pattern

The Raylib implementation uses a **panel stack + callback pattern** to achieve hierarchical navigation:

**1. StarPilotLayout (Main Container):**
```python
class StarPilotLayout(Widget):
  def __init__(self):
    # Panel stack tracks navigation history
    self._panel_stack: list[tuple[StarPilotPanelType, str]] = []
    self._sub_panel_callbacks: dict[str, Callable] = {}
    
    # Set up sub-panel navigation
    self._setup_longitudinal_sub_panels()
    
  def navigate_back(self):
    # Pop from stack, update sub-panel visibility
    if self._panel_stack:
      self._panel_stack.pop()
      self._update_sub_panel_visibility()
    else:
      self._set_current_panel(StarPilotPanelType.MAIN)
      
  def _push_sub_panel(self, sub_panel_name: str):
    self._panel_stack.append((self._current_panel, sub_panel_name))
    self._update_sub_panel_visibility()
```

**2. StarPilotLongitudinalLayout (Sub-Panel Container):**
```python
class StarPilotLongitudinalLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    # Navigation callbacks and routing are handled by StarPilotPanel.__init__()
    
    self._sub_panels: dict[str, Widget] = {
      "weather": StarPilotWeatherLayout(),
      "low_visibility": StarPilotLowVisibilityLayout(),
    }
    
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
```

**3. Weather Condition Layout (Leaf Panel):**
```python
class StarPilotWeatherLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    # Callbacks and state routing are inherited out of the box
    
    items = [
      button_item(
        tr_noop("Low Visibility"),
        lambda: tr("MANAGE"),
        description,
        callback=lambda: self._navigate_to("low_visibility"),
      ),
      # ... more conditions
    ]
    self._scroller = Scroller(items, line_separator=True, spacing=0)
```

**Key Patterns:**
1.  **Navigation callback chain**: Parent → Child → Grandchild flows through callbacks
2.  **State-based rendering**: Parent panel decides which sub-panel to render based on `_current_sub_panel`
3.  **Back navigation**: Settings sidebar's back button pops the panel stack
4.  **Separate layout classes**: Each sub-panel is its own Widget class with its own Scroller

**Navigation Flow:**
```
Driving Controls → GAS/BRAKE (Longitudinal)
  → Weather button calls _navigate_to("weather")
  → _navigate_callback pushes to panel stack
  → Longitudinal renders StarPilotWeatherLayout
  → Low Visibility button calls _navigate("low_visibility")
  → _navigate_callback pushes to panel stack  
  → Longitudinal renders StarPilotLowVisibilityLayout
  → Back button pops stack → returns to Weather
```

1.  **Sub-sub-panel enum**: Add `StarPilotSubPanelType` for controls within sub-panels
2.  **Back button**: Each sub-panel needs back navigation to parent
3.  **Navigation stack**: Track depth for proper back navigation
4.  **Three-level handling**: Weather → 4 weather conditions → offset controls

**Navigation Signals in Qt**:
```cpp
// Signals for multi-level navigation
emit openSubPanel();       // Level 1 → Level 2
emit openSubSubPanel();    // Level 2 → Level 3 (personalities, SLC QOL/Offsets/Visuals)
emit openSubSubSubPanel(); // Level 3 → Level 4 (Weather conditions)

// Close signals (from parent window)
emit closeSubPanel();      // Back from Level 2 → Level 1
emit closeSubSubPanel();   // Back from Level 3 → Level 2
emit closeSubSubSubPanel(); // Back from Level 4 → Level 3
```

**Longitudinal Panel Structure** (17 sub-panels, ~80 controls):
```
FROGPILOT LONGITUDINAL LAYOUT (top level - MANAGE buttons)
├── Advanced Longitudinal Tuning → StarPilotAdvancedLongLayout
│   └── 8 value controls
├── Conditional Experimental Mode → StarPilotConditionalExpLayout
│   └── 7 controls + dual slider CESpeed
├── Curve Speed Controller → StarPilotCurveSpeedLayout
│   └── 4 controls
├── Driving Personalities → StarPilotPersonalityLayout (sub-sub)
│   ├── Traffic Personality → StarPilotTrafficLayout (sub-sub-sub)
│   │   └── 7 value controls + reset
│   ├── Aggressive Personality → StarPilotAggressiveLayout
│   │   └── 7 value controls + reset
│   ├── Standard Personality → StarPilotStandardLayout
│   │   └── 7 value controls + reset
│   └── Relaxed Personality → StarPilotRelaxedLayout
│       └── 7 value controls + reset
├── Longitudinal Tuning → StarPilotLongTuneLayout
│   └── 8 controls
├── Quality of Life → StarPilotLongQOLLayout
│   └── 7 controls + Weather Presets → StarPilotWeatherLayout (LEVEL 3)
│       ├── Low Visibility → StarPilotLowVisibilityLayout (LEVEL 4)
│       │   └── 4 offset controls
│       ├── Rain → StarPilotRainLayout
│       │   └── 4 offset controls
│       ├── Rainstorm → StarPilotRainStormLayout
│       │   └── 4 offset controls
│       └── Snow → StarPilotSnowLayout
│           └── 4 offset controls
│       └── Set Weather Key (ADD/TEST buttons)
└── Speed Limit Controller → StarPilotSLCLayout
    ├── Main SLC Controls (~10 toggles/controls)
    ├── SLC Quality of Life → StarPilotSLCQOLLayout (sub-sub)
    │   └── SLCConfirmation (TOGGLE+TOGGLE): Lower Limits + Higher Limits
    ├── SLC Offsets → StarPilotSLCOffsetsLayout (sub-sub)
    │   └── 7 speed range offsets
    └── SLC Visuals → StarPilotSLCVisualsLayout (sub-sub)
        └── 2 toggles
```

### 9.5 Complete Panel Implementation Details

#### 9.5.1 Sounds Panel
**Qt File**: `starpilot/ui/qt/offroad/sounds_settings.cc` (222 lines)

**Raylib Implementation**: `selfdrive/ui/layouts/settings/starpilot/sounds.py`
-   `StarPilotSoundsLayout`: Main panel with 2 MANAGE buttons
-   `StarPilotVolumeControlLayout`: 7 volume sliders with Test buttons
-   `StarPilotCustomAlertsLayout`: 5 toggles with conditional visibility

**Main Panel** (2 items):
| Control | Type | Description |
|--------|------|-------------|
| Alert Volume Controller | MANAGE | Opens sub-panel with 7 volume sliders |
| StarPilot Alerts | MANAGE | Opens sub-panel with 5 toggles |

**Alert Volume Control Sub-Panel** (7 value controls):
| Control | Range | Default | Special |
|--------|-------|---------|---------|
| Disengage Volume | 0-101 | 100 | "Test" button; 0=Muted, 101=Auto |
| Engage Volume | 0-101 | 100 | "Test" button |
| Prompt Volume | 0-101 | 100 | "Test" button |
| Prompt Distracted Volume | 0-101 | 100 | "Test" button |
| Refuse Volume | 0-101 | 100 | "Test" button |
| Warning Soft Volume | 25-101 | 100 | "Test" button; min 25 |
| Warning Immediate Volume | 25-101 | 100 | "Test" button; min 25 |

**Values**: 0=Muted, 101=Auto, 1-100=percentage

**Sound Test Implementation** (Raylib):
-   **Offroad**: Persistent Python subprocess reads `path|volume\n` from stdin, plays WAV via sounddevice
-   **Onroad**: Sets `TestAlert` param in params_memory (handled by `soundd.py`)
-   Theme sounds checked first (`ACTIVE_THEME_PATH / "sounds"`), fallback to stock sounds

**Custom Alerts Sub-Panel** (5 toggles):
| Control | Param Key | Visibility Condition |
|--------|-----------|---------------------|
| Goat Scream | `GoatScream` | Always visible |
| Green Light Alert | `GreenLightAlert` | Always visible |
| Lead Departing Alert | `LeadDepartingAlert` | Always visible |
| Loud Blindspot Alert | `LoudBlindspotAlert` | Requires `hasBSM` |
| Speed Limit Changed Alert | `SpeedLimitChangedAlert` | Requires `ShowSpeedLimits` OR (`hasOpenpilotLongitudinal` AND `SpeedLimitController`) |

---

#### 9.5.2 Driving Model Panel
**Qt File**: `starpilot/ui/qt/offroad/model_settings.cc` (814 lines)

**Main Panel** (9 controls):
| Control | Type | Param Key | Special |
|---------|------|-----------|---------|
| AutomaticallyDownloadModels | Toggle | `AutomaticallyDownloadModels` | Auto-download new models |
| Download Driving Models | BUTTON | `DownloadModel` | DOWNLOAD / DOWNLOAD ALL / CANCEL |
| Delete Driving Models | BUTTON | `DeleteModel` | DELETE / DELETE ALL |
| Model Randomizer | Toggle | `ModelRandomizer` | Random model each drive |
| Recovery Power | VALUE+RESET | `RecoveryPower` | Range 0.5-2.0, step 0.1, Level 3 only |
| Stop Distance | VALUE+RESET | `StopDistance` | Range 4-10m, step 0.1, Level 3 only |
| Manage Blacklisted Models | BUTTON | `ManageBlacklistedModels` | ADD / REMOVE / REMOVE ALL |
| Manage Model Ratings | BUTTON | `ManageScores` | RESET / VIEW |
| Select Driving Model | BUTTON | `SelectModel` | Opens model selection dialog |

**Model Selection Dialog Features**:
-   Groups models by series (Custom Series, Driving Policy, etc.)
-   Shows icons: 🗺️ (Navigation), 📡 (Radar), 👀 (VOACC)
-   Supports user favorites and community favorites
-   Sort modes: alphabetical, release date, rating
-   Shows release dates when available
-   Requires reboot after selection if started

**Model Delete Dialog**:
-   Groups deletable models by series
-   Confirmation dialog before delete

**Blacklist Management**:
-   ADD: Select from available models to blacklist
-   REMOVE: Select from blacklisted models to remove
-   REMOVE ALL: Clear entire blacklist (with confirmation)

**Model Ratings Panel** (Sub-panel):
-   Shows each model's: Drives count, Score percentage
-   Read-only LabelControls

**Conditional Visibility**:
-   `ManageBlacklistedModels`, `ManageScores`: Only if `ModelRandomizer` is ON
-   `SelectModel`: Only if `ModelRandomizer` is OFF
-   `RecoveryPower`, `StopDistance`: Only if tuningLevel == 3 (Developer)

**Button States**:
-   Download/Delete buttons: Disabled when no models available or downloading
-   Online/parked requirement for downloads

---

#### 9.5.3 Lateral Panel - 5 Sub-Panels
**Qt File**: `starpilot/ui/qt/offroad/lateral_settings.cc` (428 lines)

**Advanced Lateral Tuning** (8 controls):
| Control | Type | Range | Step | Default | Special |
|---------|------|-------|------|---------|---------|
| SteerDelay | VALUE+RESET | 0.01-1.0 | 0.01 | car-specific | seconds, show default in title |
| SteerFriction | VALUE+RESET | 0-1 | 0.01 | car-specific | |
| SteerKP | VALUE+RESET | car×0.5 to car×1.5 | 0.01 | car-specific | Auto-scales to car |
| SteerLatAccel | VALUE+RESET | car×0.5 to car×1.5 | 0.01 | car-specific | Auto-scales to car |
| SteerRatio | VALUE+RESET | car×0.5 to car×1.5 | 0.01 | car-specific | Auto-scales to car |
| ForceAutoTune | Toggle | - | - | - | |
| ForceAutoTuneOff | Toggle | - | - | - | |
| ForceTorqueController | Toggle | - | - | - | |

**Car-Specific Ranges** (in showEvent):
-   SteerKP: `parent->steerKp * 0.5` to `parent->steerKp * 1.5`
-   SteerLatAccel: `parent->latAccelFactor * 0.5` to `parent->latAccelFactor * 1.5`
-   SteerRatio: `parent->steerRatio * 0.5` to `parent->steerRatio * 1.5`

**Always On Lateral** (2 controls):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| AlwaysOnLateralLKAS | Toggle | - | - | Enable with LKAS |
| PauseAOLOnBrake | VALUE | 0-99 | 1 | mph/kmh auto-convert |

**Lane Changes** (5 controls):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| LaneChanges | MANAGE | - | - | Parent toggle |
| NudgelessLaneChange | Toggle | - | - | |
| LaneChangeTime | VALUE | 0-5 | 0.1 | seconds (labels: "Instant", "0.5 seconds", etc.) |
| MinimumLaneChangeSpeed | VALUE | 0-99 | 1 | mph/kmh |
| LaneDetectionWidth | VALUE | 0-15 | 0.1 | feet/meters (auto-convert) |
| OneLaneChange | Toggle | - | - | |

**Lateral Tuning** (3 controls):
| Control | Type | Special |
|---------|------|---------|
| TurnDesires | Toggle | |
| NNFF | Toggle | Requires NNFF log, not angle car |
| NNFFLite | Toggle | |

**Quality of Life** (1 control):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| PauseLateralSpeed | VALUE+TOGGLE | 0-99 | 1 | Has "Turn Signal Only" sub-toggle |

**Reboot Requirements** (toggle changed while started):
-   AlwaysOnLateral, ForceTorqueController, NNFF, NNFFLite: All require reboot confirmation

**Metric Conversion** (updateMetric):
-   LaneDetectionWidth: FOOT_TO_METER (imperial: 0-15 feet, metric: 0-5 meters)
-   MinimumLaneChangeSpeed, PauseAOLOnBrake, PauseLateralSpeed: MILE_TO_KM (imperial: 0-99 mph, metric: 0-150 km/h)

**Conditional Visibility** (Advanced Lateral):
| Control | Visibility Condition |
|---------|---------------------|
| SteerDelay | Only if != 0 |
| SteerFriction | Only if !=0 AND (auto-tune off OR not forcing) AND (torque car OR forcing torque OR using NNFF) AND NOT using NNFF |
| SteerKP | Only if !=0 AND (torque car OR forcing) AND NOT angle car |
| SteerLatAccel | Only if !=0 AND (auto-tune off OR not forcing) AND (torque car OR forcing) AND NOT using NNFF |
| SteerRatio | Only if !=0 AND (auto-tune off OR not forcing) |
| ForceAutoTune | Only if NOT hasAutoTune AND NOT angle car AND (torque car OR forcingTorque OR usingNNFF) |
| ForceAutoTuneOff | Only if hasAutoTune |
| ForceTorqueController | Only if NOT angle car AND NOT torque car |

**Conditional Visibility** (Lane Changes):
-   `LaneChangeTime`, `LaneDetectionWidth`: Only if LaneChanges AND NudgelessLaneChange both ON

**Conditional Visibility** (Lateral Tuning):
-   `NNFF`: Only if hasNNFFLog AND NOT angle car
-   `NNFFLite`: Only if NOT usingNNFF AND NOT angle car

---

#### 9.5.4 Longitudinal Panel - 17 Sub-Panels (THREE LEVELS!)
**Qt File**: `starpilot/ui/qt/offroad/longitudinal_settings.cc` (1085 lines)

**Structure**:
```
LONGITUDINAL PANEL (Main)
├── Advanced Longitudinal Tuning (8 controls)
├── Conditional Experimental Mode (7 controls + dual slider)
├── Curve Speed Controller (4 controls)
├── Driving Personalities (MANAGE) → Sub-Sub-Panel
│   ├── Traffic Personality (MANAGE) → Sub-Sub-Sub-Panel (7 controls + reset)
│   ├── Aggressive Personality (MANAGE) → Sub-Sub-Sub-Panel (7 controls + reset)
│   ├── Standard Personality (MANAGE) → Sub-Sub-Sub-Panel (7 controls + reset)
│   └── Relaxed Personality (MANAGE) → Sub-Sub-Sub-Panel (7 controls + reset)
├── Longitudinal Tuning (8 controls)
├── Quality of Life (8 controls)
│   └── Weather Presets (MANAGE) → LEVEL 3
│       ├── Low Visibility (MANAGE) → LEVEL 4 (4 offsets)
│       │   └── 4 offset controls
│       ├── Rain (MANAGE) → LEVEL 4 (4 offsets)
│       │   └── 4 offset controls
│       ├── Rainstorm (MANAGE) → LEVEL 4 (4 offsets)
│       │   └── 4 offset controls
│       └── Snow (MANAGE) → LEVEL 4 (4 offsets)
│           └── 4 offset controls
│       └── Set Weather Key (ADD/TEST buttons)
└── Speed Limit Controller (MANAGE) → Sub-Sub-Panel
    ├── Main SLC Controls (~10 toggles/controls)
    ├── SLC Quality of Life (MANAGE) → Sub-Sub-Sub-Panel
    │   └── SLCConfirmation (TOGGLE+TOGGLE): Lower Limits + Higher Limits
    ├── SLC Offsets (MANAGE) → Sub-Sub-Sub-Panel (7 speed range offsets)
    └── SLC Visuals (MANAGE) → Sub-Sub-Sub-Panel (2 toggles)
```

**Advanced Longitudinal Tuning** (8 controls):
| Control | Type | Range | Step | Unit | Special |
|---------|------|-------|------|------|---------|
| LongitudinalActuatorDelay | VALUE | 0-1 | 0.01 | seconds | show default in title |
| MaxDesiredAcceleration | VALUE | 0.1-4.0 | 0.1 | m/s² | |
| StartAccel | VALUE | 0-4 | 0.01 | m/s² | show default in title |
| VEgoStarting | VALUE | 0.01-1 | 0.01 | m/s² | show default in title |
| StopAccel | VALUE | -4-0 | 0.01 | m/s² | negative, show default |
| StoppingDecelRate | VALUE | 0.001-1 | 0.001 | m/s² | show default |
| VEgoStopping | VALUE | 0.01-1 | 0.01 | m/s² | show default |

**Conditional Experimental Mode** (8 controls - includes dual slider):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| CESpeed | DUAL VALUE | 0-99 (0-150 metric) | 1 | TWO sliders: Without Lead + With Lead |
| CECurves | TOGGLE+TOGGLE | - | - | Main + "With Lead" sub-toggle |
| CEStopLights | Toggle | - | - | "Detected" stop lights/signs |
| CELead | TOGGLE+TOGGLE | - | - | Main + "Slower Lead" + "Stopped Lead" |
| CEModelStopTime | VALUE | 0-9 | 1 | seconds (labels: "Off", "1 second", etc.) |
| CESignalSpeed | VALUE+TOGGLE | 0-99 (0-150 metric) | 1 | mph/kmh + "Not For Detected Lanes" |
| ShowCEMStatus | Toggle | - | - | Status widget |

**Dual Slider Implementation**:
-   Uses `StarPilotDualParamValueControl` wrapping two `StarPilotParamValueControl`
-   CESpeed: First slider "Below" (no lead), Second slider "With Lead"
-   Both sliders share same unit and get converted together

**Curve Speed Controller** (4 controls):
| Control | Type | Special |
|---------|------|---------|
| CalibratedLateralAcceleration | LABEL | Read-only, shows "X.XX m/s²" |
| CalibrationProgress | LABEL | Read-only, shows "XX.XX%" |
| ResetCurveData | BUTTON | RESET |
| ShowCSCStatus | Toggle | Status widget |

**Driving Personalities** (4 sub-panels, each 7 controls + reset):

*Each personality has*: 
-   Follow: 0.5-3s (Traffic) or 1-3s (others), step 0.01
-   JerkAcceleration: 25-200%, step 1
-   JerkDeceleration: 25-200%, step 1
-   JerkDanger: 25-200%, step 1
-   JerkSpeedDecrease: 25-200%, step 1
-   JerkSpeed: 25-200%, step 1
-   Reset button

| Personality | Follow Range | Default Follow | Icon |
|------------|--------------|----------------|------|
| Traffic | 0.5-3.0 | N/A | traffic.png |
| Aggressive | 1-3 | 1.25s | aggressive.png |
| Standard | 1-3 | 1.45s | standard.png |
| Relaxed | 1-3 | 1.75s | relaxed.png |

**Longitudinal Tuning** (8 controls):
| Control | Type | Options | Special |
|---------|------|---------|---------|
| AccelerationProfile | BUTTON SELECT | Standard, Eco, Sport, Sport+ | |
| DecelerationProfile | BUTTON SELECT | Standard, Eco, Sport | |
| HumanAcceleration | Toggle | | |
| HumanFollowing | Toggle | | |
| HumanLaneChanges | Toggle | | Requires radar |
| LeadDetectionThreshold | VALUE | 25-50% | |
| TacoTune | Toggle | | "Taco Bell Run" turn speed |
| MapGears | TOGGLE+TOGGLE | | Acceleration + Deceleration mapped to gears |

**Quality of Life** (8 controls):
| Control | Type | Range | Step | Unit | Special |
|---------|------|-------|------|------|---------|
| CustomCruise | VALUE | 1-99 | 1 | mph/kmh | Cruise interval |
| CustomCruiseLong | VALUE | 1-99 | 1 | mph/kmh | Cruise interval (hold) |
| ForceStops | Toggle | - | - | | Force stop at detected lights |
| IncreasedStoppedDistance | VALUE | 0-10 | 1 | feet/meters | |
| MapGears | TOGGLE+TOGGLE | - | - | See LongitudinalTune |
| SetSpeedOffset | VALUE | 0-99 | 1 | mph/kmh | |
| ReverseCruise | Toggle | - | - | Reverse +/- behavior |
| WeatherPresets | MANAGE | - | - | Opens Weather panel (LEVEL 3) |

**Weather** (4 conditions × 4 controls = 16 controls) - LEVEL 3:
| Condition | Controls (each) | Range |
|----------|----------------|-------|
| Low Visibility | IncreaseFollowing, IncreasedStoppedDistance, ReduceAcceleration, ReduceLateralAccel | Following: 0-3s, Stopped: 0-10ft, Accel: 0-99%, Lateral: 0-99% |
| Rain | Same 4 offsets | Same |
| Rainstorm | Same 4 offsets | Same |
| Snow | Same 4 offsets | Same |

**Weather Key Management**:
-   ADD button: Opens InputDialog for 32-character key
-   TEST button: Makes network request to OpenWeatherMap API
-   Shows "Testing..." while request in progress
-   Validates both v2.5 and v3.0 API

**Speed Limit Controller** (Main + 3 sub-panels):

*Main SLC* (10+ controls):
| Control | Type | Options | Special |
|---------|------|---------|---------|
| SLCFallback | BUTTON SELECT | Set Speed, Experimental Mode, Previous Limit | |
| SLCOverride | BUTTON SELECT | None, Set With Gas Pedal, Max Set Speed | |
| SLCQOL | MANAGE | | Opens QOL sub-panel |
| SLCConfirmation | TOGGLE+TOGGLE | | Lower Limits + Higher Limits |
| SLCLookaheadHigher | VALUE | 0-30 | seconds |
| SLCLookaheadLower | VALUE | 0-30 | seconds |
| SetSpeedLimit | Toggle | | Match speed limit on engage |
| SLCMapboxFiller | Toggle | | Use Mapbox as fallback |
| SLCPriority | BUTTON | SELECT | Opens priority dialog |
| SLCOffsets | MANAGE | | Opens Offsets sub-panel |

*SLC Priority Dialog*:
-   Two-step selection: Primary then Secondary
-   Options: Dashboard (if hasDashSpeedLimits), Map Data, Highest, Lowest, None
-   Stores as SLCPriority1, SLCPriority2

*SLC Offsets Sub-Panel* (7 controls):
| Control | Imperial Range | Metric Range | Special |
|---------|----------------|--------------|---------|
| Offset1 | -99-99 mph (0-24 mph range) | -150-150 km/h (0-29 km/h range) | |
| Offset2 | -99-99 mph (25-34 mph range) | -150-150 km/h (30-49 km/h range) | |
| Offset3 | -99-99 mph (35-44 mph range) | -150-150 km/h (50-59 km/h range) | |
| Offset4 | -99-99 mph (45-54 mph range) | -150-150 km/h (60-79 km/h range) | |
| Offset5 | -99-99 mph (55-64 mph range) | -150-150 km/h (80-99 km/h range) | |
| Offset6 | -99-99 mph (65-74 mph range) | -150-150 km/h (100-119 km/h range) | |
| Offset7 | -99-99 mph (75-99 mph range) | -150-150 km/h (120-140 km/h range) | |

*SLC Visuals Sub-Panel* (2 controls):
| Control | Type |
|---------|------|
| ShowSLCOffset | Toggle |
| SpeedLimitSources | Toggle |

**Metric Conversion Constants**:
```cpp
FOOT_TO_METER = 0.3048
METER_TO_FOOT = 3.28084
MILE_TO_KM = 1.60934
KM_TO_MILE = 0.621371
```

**Conditional Visibility**:
-   `CEStopLights`: Only if tuningLevel < CEModelStopTime level
-   `CustomCruise`, `CustomCruiseLong`, `SetSpeedLimit`, `SetSpeedOffset`: Only if NOT hasPCMCruise
-   `HumanLaneChanges`: Only if hasRadar
-   `MapGears`: Only if isToyota AND NOT isTSK
-   `ReverseCruise`: Only if isToyota
-   `SLCMapboxFiller`: Only if MapboxSecretKey is set
-   `StartAccel`: Only if NOT (LongitudinalTune AND HumanAcceleration)
-   `StoppingDecelRate`, `VEgoStarting`, `VEgoStopping`: Only if NOT (isGM AND ExperimentalGMTune) AND NOT (isToyota AND FrogsGoMoosTweak)

---

#### 9.5.5 Visual Panel - 5 Sub-Panels
**Qt File**: `starpilot/ui/qt/offroad/visual_settings.cc` (328 lines)

**Advanced Custom UI** (6 toggles):
| Control | Param Key |
|---------|-----------|
| HideSpeed | `HideSpeed` |
| HideLeadMarker | `HideLeadMarker` |
| HideMaxSpeed | `HideMaxSpeed` |
| HideAlerts | `HideAlerts` |
| HideSpeedLimit | `HideSpeedLimit` |
| WheelSpeed | `WheelSpeed` |

**Driving Screen Widgets** (7 controls):
| Control | Type | Special |
|---------|------|---------|
| AccelerationPath | Toggle | Requires longitudinal |
| AdjacentPath | Toggle | |
| BlindSpotPath | Toggle | Requires BSM |
| Compass | Toggle | |
| OnroadDistanceButton | Toggle | Requires longitudinal |
| PedalsOnUI | TOGGLE+TOGGLE | Dynamic + Static sub-toggles (exclusive) |
| RotatingWheel | Toggle | |

**PedalsOnUI Sub-Toggles**:
-   Dynamic: When selected, disables Static
-   Static: When selected, disables Dynamic
-   Uses special exclusive toggle behavior

**Model UI** (5 value controls):
| Control | Range | Step | Unit | Metric Range | Special |
|---------|-------|------|------|--------------|---------|
| DynamicPathWidth | 0-100 | 1 | % | same | |
| LaneLinesWidth | 0-24 | 1 | inches | 0-60 cm | Auto-convert to cm |
| PathEdgeWidth | 0-100 | 1 | % | same | |
| PathWidth | 0-10 | 0.1 | feet | 0-3 m | Auto-convert to meters |
| RoadEdgesWidth | 0-24 | 1 | inches | 0-60 cm | Auto-convert to cm |

**PathEdgeWidth Labels**: "Off", "1%" through "100%"

**Navigation UI** (4 toggles):
| Control | Param Key |
|---------|-----------|
| RoadNameUI | `RoadNameUI` |
| ShowSpeedLimits | `ShowSpeedLimits` |
| SLCMapboxFiller | `SLCMapboxFiller` |
| UseVienna | `UseVienna` |

**Quality of Life** (4 controls):
| Control | Type | Options | Special |
|---------|------|---------|---------|
| CameraView | BUTTON SELECT | Auto, Driver, Standard, Wide | |
| DriverCamera | Toggle | | Show in reverse |
| StoppedTimer | Toggle | | |

**Metric Conversion**:
-   LaneLinesWidth, RoadEdgesWidth: INCH_TO_CM / CM_TO_INCH (imperial: 0-24 inches, metric: 0-60 cm)
-   PathWidth: FOOT_TO_METER (imperial: 0-10 feet, metric: 0-3 meters)

**Conditional Visibility**:
-   `AccelerationPath`, `OnroadDistanceButton`, `PedalsOnUI`, `HideLeadMarker`: Only if hasOpenpilotLongitudinal
-   `BlindSpotPath`: Only if hasBSM
-   `HideSpeedLimit`: Only if hasOpenpilotLongitudinal AND SpeedLimitController
-   `ShowSpeedLimits`: Only if NOT SpeedLimitController OR NOT hasOpenpilotLongitudinal
-   `SLCMapboxFiller`: Only if ShowSpeedLimits AND (NOT SpeedLimitController OR NOT hasOpenpilotLongitudinal) AND MapboxSecretKey set
-   `UseVienna`: Only if ShowSpeedLimits OR SpeedLimitController

---

#### 9.5.6 Themes Panel
**Qt File**: `starpilot/ui/qt/offroad/theme_settings.cc` (935 lines)

**Main Panel**:
| Control | Type | Buttons | Special |
|---------|------|---------|---------|
| Custom Themes | MANAGE | - | Opens 7 theme types |
| Holiday Themes | TOGGLE | - | |
| Rainbow Path | Toggle | | |
| Random Events | Toggle | | |
| Random Themes | TOGGLE+TOGGLE | - | Includes "Include Holiday Themes" |
| Startup Alert | BUTTON | STOCK, FROGPILOT, CUSTOM, CLEAR | |
| Download Status | LABEL | - | Shows download progress |

**Custom Themes Sub-Panel** (7 theme types, each with DELETE/DOWNLOAD/SELECT):
| Theme Type | Param Key | Buttons | Directory |
|------------|-----------|---------|-----------|
| Boot Logo | `BootLogo` | DELETE, DOWNLOAD, SELECT | boot_logos/ |
| Color Scheme | `ColorScheme` | DELETE, DOWNLOAD, SELECT | colors/ |
| Distance Icon | `DistanceIconPack` | DELETE, DOWNLOAD, SELECT | distance_icons/ |
| Icon Pack | `IconPack` | DELETE, DOWNLOAD, SELECT | (root level) |
| Signal Animation | `SignalAnimation` | DELETE, DOWNLOAD, SELECT | signal_animations/ |
| Sound Pack | `SoundPack` | DELETE, DOWNLOAD, SELECT | sounds/ |
| Wheel Icon | `WheelIcon` | DELETE, DOWNLOAD, SELECT | steering_wheels/ |

**Theme Button Behaviors**:
-   DELETE: Opens selection dialog, then confirmation dialog
-   DOWNLOAD: Opens selection dialog, starts download, shows progress
-   SELECT: Opens selection dialog (includes "Stock" + holidays for Color/Distance)

**Theme Download States**:
-   Shows "Downloading..." during download
-   Buttons disabled while downloading
-   CANCEL available during download

**Holiday Themes** (13 holidays):
-   New Year's, Valentine's Day, St. Patrick's Day, World Frog Day, April Fools, Easter, May the Fourth, Cinco de Mayo, Stitch Day, Fourth of July, Halloween, Thanksgiving, Christmas

**Random Themes Behavior**:
-   Pick random theme between each drive
-   Optionally include holiday themes

**Theme Name Parsing**:
-   Format: "name~creator" (stored lowercase)
-   User-created themes: "name-user_created"
-   Display with 🌟 for user-created, "- by: creator" for community

---

#### 9.5.7 Vehicle Panel - 5 Sub-Panels
**Qt File**: `starpilot/ui/qt/offroad/vehicle_settings.cc` (467 lines)

**Main Panel** (3 controls + 5 sub-panel buttons):
| Control | Type | Special |
|---------|------|---------|
| Car Make | BUTTON SELECT | 28 car makes |
| Car Model | BUTTON SELECT | Dynamic based on make |
| ForceFingerprint | Toggle | Disable auto fingerprint |
| DisableOpenpilotLongitudinal | Toggle | Use stock ACC, requires reboot |

**Car Makes** (28 total):
Acura, Audi, Buick, Cadillac, Chevrolet, Chrysler, CUPRA, Dodge, Ford, Genesis, GMC, Holden, Honda, Hyundai, Jeep, Kia, Lexus, Lincoln, MAN, Mazda, Nissan, Peugeot, Ram, Rivian, SEAT, Škoda, Subaru, Tesla, Toyota, Volkswagen

**Car Model Selection**:
-   Reads from opendbc/car/{folder}/values.py
-   Groups by platform (e.g., "Toyota", "Genesis", etc.)

**Sub-Panel Buttons** (MANAGE/VIEW):
-   GM Settings, HKG Settings, Subaru Settings, Toyota Settings, Vehicle Info

**GM Sub-Panel** (4 controls):
| Control | Type | Special |
|---------|------|---------|
| GMPedalLongitudinal | Toggle | Requires pedal |
| RemoteStartBootsComma | Toggle | Flash panda firmware |
| RemapCancelToDistance | Toggle | Requires BOLT + pedal |
| VoltSNG | Toggle | |

**Remote Start Flow**:
-   Toggle triggers confirmation
-   Shows prompt about Panda firmware update
-   Runs flash in background thread
-   Reboots after flash complete

**HKG Sub-Panel** (1 control):
| Control | Type |
|---------|------|
| TacoTuneHacks | Toggle |

**Subaru Sub-Panel** (1 control):
| Control | Type |
|---------|------|
| SubaruSNG | Toggle |

**Toyota Sub-Panel** (5 controls):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| ToyotaDoors | TOGGLE+TOGGLE | - | - | Lock + Unlock |
| ClusterOffset | VALUE+RESET | 1.000-1.050 | 0.001 | x multiplier |
| FrogsGoMoosTweak | Toggle | | | |
| LockDoorsTimer | VALUE | 0-300 | 5 | seconds, WARNING label |
| SNGHack | Toggle | | | |

**ClusterOffset**: 
-   Shows "x1.000" to "x1.050"
-   Reset to default 1.000
-   Displayed as multiplication factor

**LockDoorsTimer Labels**:
-   0 = "Never"
-   1-300 = "X seconds"

**Warning Label**:
-   LockDoorsTimer: "<b>Warning:</b> openpilot can't detect if keys are still inside the car..."

**Vehicle Info Sub-Panel** (7 read-only labels):
| Control | Value |
|---------|-------|
| Hardware Detected | "None" or "comma Pedal, SDSU, ZSS" |
| Blind Spot Support | "Yes" / "No" |
| Pedal Support | "Yes" / "No" |
| openpilot Longitudinal | "Yes" / "No" |
| Radar Support | "Yes" / "No" |
| SDSU Support | "Yes" / "No" |
| Stop-and-Go Support | "Yes" / "No" |

**Reboot Requirements**:
-   TacoTuneHacks: Requires reboot
-   RemapCancelToDistance: Requires reboot
-   DisableOpenpilotLongitudinal: Requires reboot if disabling while started

**Conditional Visibility**:
-   `GMPedalLongitudinal`: Only if hasPedal OR (PC and canUsePedal)
-   `RemapCancelToDistance`: Only if isBolt AND (hasPedal OR (PC and canUsePedal))
-   `SubaruSNG`: Only if hasSNG
-   `TacoTuneHacks`: Only if isHKGCanFd
-   `VoltSNG`: Only if isVolt AND NOT hasSNG
-   `SNGHack`: Only if NOT hasSNG

---

#### 9.5.8 Wheel Panel
**Qt File**: `starpilot/ui/qt/offroad/wheel_settings.cc` (84 lines)

**4 Controls** (each opens selection dialog):
| Control | Param Key | Options |
|---------|-----------|---------|
| Distance Button | `DistanceButtonControl` | No Action, Change Personality, Force Coast, Pause Accel/Brake, Toggle Experimental, Toggle Traffic Mode |
| Distance Button (Long Press) | `LongDistanceButtonControl` | Same + Pause Steering |
| Distance Button (Very Long Press) | `VeryLongDistanceButtonControl` | Same |
| LKAS Button | `LKASButtonControl` | No Action, Pause Steering (unless Subaru or LKAS allowed for AOL) |

**Conditional Visibility**:
-   `LKASButtonControl`: NOT Subaru AND NOT (LKAS allowed for AOL AND AlwaysOnLateral AND AlwaysOnLateralLKAS)

---

#### 9.5.9 Navigation Panel
**Qt File**: `starpilot/ui/qt/offroad/navigation_settings.cc` (330 lines)

**Main Panel**:
| Control | Type | Special |
|---------|------|---------|
| Manage Your Settings At | LABEL | Shows "IP:8082" or "Offline..." |
| Public Mapbox Key | BUTTON | ADD/TEST |
| Secret Mapbox Key | BUTTON | ADD/TEST |
| Mapbox Setup Instructions | VIEW | Opens instructions sub-panel |
| Speed Limit Filler | BUTTON TOGGLE | CANCEL / Manually Update Speed Limits |

**Key Management**:
-   ADD: Opens InputDialog for key entry
-   TEST: Validates key works
-   Keys stored in params (Public/Secret Mapbox)

**Instructions Sub-Panel**:
-   Displays setup images based on current step
-   Step-based instruction flow

---

#### 9.5.10 Data Panel
**Qt File**: `starpilot/ui/qt/offroad/data_settings.cc` (872 lines)

**Main Panel**:
| Control | Type | Special |
|---------|------|---------|
| Delete Driving Data | BUTTON | DELETE |
| Delete Error Logs | BUTTON | DELETE |
| Screen Recordings | BUTTONS | DELETE / DELETE ALL / RENAME |
| StarPilot Backups | BUTTONS | BACKUP / DELETE / DELETE ALL / RESTORE |
| Toggle Backups | BUTTONS | BACKUP / DELETE / DELETE ALL / RESTORE |
| StarPilot Stats | BUTTONS | RESET / VIEW |

**Stats Sub-Panel** (20+ read-only labels):
-   Drives, Distance, Time, Events, Time percentages, etc.

---

#### 9.5.11 Device Panel - 2 Sub-Panels
**Qt File**: `starpilot/ui/qt/offroad/device_settings.cc` (247 lines)

**Device Management Sub-Panel** (7 controls):
| Control | Type | Range | Step | Unit | Special |
|---------|------|-------|------|------|---------|
| DeviceShutdown | VALUE | 0-33 | 1 | - | Labels: 0=5min, 1-3=15-45min, 4+=hours |
| NoLogging | Toggle | - | - | WARNING toggle |
| NoUploads | TOGGLE+TOGGLE | - | - | Main + "Disable Onroad Only" |
| HigherBitrate | Toggle | - | - | |
| LowVoltageShutdown | VALUE | 11.8-12.5 | 0.1 | volts | |
| IncreaseThermalLimits | Toggle | - | - | WARNING toggle |
| UseKonikServer | Toggle | - | - | |

**DeviceShutdown Labels**:
-   0 = "5 minutes"
-   1-3 = "15/30/45 minutes"
-   4+ = "X hours" (4=4h, 5=5h, etc.)

**Screen Management Sub-Panel** (7 controls):
| Control | Type | Range | Step | Special |
|---------|------|-------|------|---------|
| ScreenBrightness | VALUE | 1-101 | 1 | 0=Off, 101=Auto |
| ScreenBrightnessOnroad | VALUE | 0-101 | 1 | 0=Off, 101=Auto |
| ScreenRecorder | BUTTON TOGGLE | - | - | START/STOP recording |
| ScreenTimeout | VALUE | 5-60 | 5 | seconds |
| ScreenTimeoutOnroad | VALUE | 5-60 | 5 | seconds |
| StandbyMode | Toggle | - | - | |

**Screen Brightness Labels**:
-   0 = "Off"
-   1-100 = "X%"
-   101 = "Auto"

---

#### 9.5.12 Utilities Panel
**Qt File**: `starpilot/ui/qt/offroad/utilities.cc` (369 lines)

**Main Panel** (6 controls):
| Control | Type | Special |
|---------|------|---------|
| Debug Mode | Toggle | |
| Flash Panda | BUTTON | FLASH |
| Force Drive State | BUTTONS | OFFROAD / ONROAD / OFF |
| Pair to "The Pond" | BUTTON | PAIR/UNPAIR |
| Report a Bug | BUTTON | REPORT |
| Reset Toggles to Default | BUTTON | RESET |
| Reset Toggles to Stock | BUTTON | RESET |

---

#### 9.5.13 Maps Panel
**Qt File**: `starpilot/ui/qt/offroad/maps_settings.cc` (278 lines)

**Main Panel**:
| Control | Type | Special |
|---------|------|---------|
| Automatically Update Maps | BUTTON SELECT | Manually/Weekly/Monthly |
| Download Maps | BUTTON | DOWNLOAD/CANCEL |
| Last Updated | LABEL | Shows date or "Never" |
| Map Sources | BUTTONS | COUNTRIES / STATES |
| Progress | LABEL | Shows during download |
| Time Elapsed | LABEL | |
| Time Remaining | LABEL | |
| Remove Maps | BUTTON | REMOVE |
| Storage Used | LABEL | Shows MB |

**Countries Sub-Panel**: 7 continents with checkbox lists
**States Sub-Panel**: 5 US regions with checkbox lists

### 9.6 Conditional Visibility Logic

Qt implements complex visibility rules based on multiple factors:

#### Metric Conversion Constants (from Qt):
```cpp
// Distance conversions
FOOT_TO_METER = 0.3048
METER_TO_FOOT = 3.28084
INCH_TO_CM = 2.54
CM_TO_INCH = 0.393701

// Speed conversions
MILE_TO_KM = 1.60934
KM_TO_MILE = 0.621371
```

#### Value Range Conversion by Unit Type:
| Unit Type | Imperial Range | Metric Range | Conversion |
|-----------|---------------|--------------|------------|
| Speed (mph/kmh) | 0-99 | 0-150 | MILE_TO_KM |
| Speed (mph/kmh) offset | -99 to 99 | -150 to 150 | MILE_TO_KM |
| Distance (feet) | 0-10 | 0-3 | FOOT_TO_METER |
| Distance (feet) offset | 0-10 | 0-3 | FOOT_TO_METER |
| Small distance (inches) | 0-24 | 0-60 | INCH_TO_CM |
| Lane width (feet) | 0-15 | 0-5 | FOOT_TO_METER |

#### Special Value Labels:
```cpp
// Volume (0-101)
0 = "Muted"
1-100 = "X%"
101 = "Auto"

// Device shutdown (0-33)
0 = "5 minutes"
1 = "15 minutes"
2 = "30 minutes"
3 = "45 minutes"
4+ = "X hours"

// Speed/time values
0 = "Off" (for many controls)
0 = "Instant" (for LaneChangeTime)
```

#### Visibility Factors in Qt:
1.  **Tuning Level** (0-3): Base visibility threshold
2.  **Car Type**: isGM, isHKG, isToyota, isSubaru, isAngleCar, isTorqueCar, isVolt, isBolt, isTSK, isHKGCanFd
3.  **Car Capabilities**: hasBSM, hasRadar, hasPedal, hasSNG, hasNNFFLog, hasAutoTune, hasDashSpeedLimits, hasZSS, canUsePedal, canUseSDSU
4.  **Feature Flags**: Other toggles states (LaneChanges, NudgelessLaneChange, ModelRandomizer, etc.)
5.  **Longitudinal Control**: hasOpenpilotLongitudinal, hasPCMCruise
6.  **Device State**: started (driving), parked, online, isFrogsGoMoo
7.  **Special Conditions**: hasOpenpilotLongitudinalControlDisabled, hasAlphaLongitudinal

#### Additional Car State Properties Needed:
```python
class StarPilotCarState:
    # Car detection
    isGM: bool = False
    isHKG: bool = False  # Hyundai/Kia/Genesis
    isToyota: bool = False
    isSubaru: bool = False
    isVolt: bool = False  # Specific GM model
    isBolt: bool = False  # Specific GM model
    isAngleCar: bool = False
    isTorqueCar: bool = False
    isTSK: bool = False  # Toyota Safety Connect
    isHKGCanFd: bool = False
    
    # Capabilities
    hasBSM: bool = False  # Blind spot monitoring
    hasRadar: bool = False
    hasPedal: bool = False
    hasSNG: bool = False  # Stop and Go
    hasNNFFLog: bool = False
    hasAutoTune: bool = False
    hasOpenpilotLongitudinal: bool = False
    hasDashSpeedLimits: bool = False
    hasZSS: bool = False  # ZSS steering sensor
    canUsePedal: bool = False
    canUseSDSU: bool = False
    
    # Device/car state
    isFrogsGoMoo: bool = False
    hasPCMCruise: bool = False
    lkasAllowedForAOL: bool = False
    openpilotLongitudinalControlDisabled: bool = False
    hasAlphaLongitudinal: bool = False
    
    # Car values for range calculations
    steerActuatorDelay: float = 0.0
    friction: float = 0.0
    steerKp: float = 0.0
    latAccelFactor: float = 0.0
    steerRatio: float = 0.0
    longitudinalActuatorDelay: float = 0.0
    startAccel: float = 0.0
    stopAccel: float = 0.0
    stoppingDecelRate: float = 0.0
    vEgoStarting: float = 0.0
    vEgoStopping: float = 0.0
```

#### Examples from Qt:

**From lateral_settings.cc**:
```python
def updateToggles(self):
    for key, toggle in toggles.items():
        min_level = parent.starpilotToggleLevels[key]
        visible = parent.tuningLevel >= min_level.toDouble()
        
        # AlwaysOnLateralLKAS
        if key == "AlwaysOnLateralLKAS":
            visible &= parent.lkasAllowedForAOL
            
        # ForceAutoTune
        elif key == "ForceAutoTune":
            visible &= not parent.hasAutoTune
            visible &= not parent.isAngleCar
            visible &= parent.isTorqueCar or forcingTorqueController or usingNNFF
            
        # NNFF
        elif key == "NNFF":
            visible &= parent.hasNNFFLog
            visible &= not parent.isAngleCar
            
        # LaneChangeTime
        elif key == "LaneChangeTime":
            visible &= params.getBool("LaneChanges")
            visible &= params.getBool("NudgelessLaneChange")
            
        # SteerKP
        elif key == "SteerKP":
            visible &= parent.steerKp != 0
            visible &= parent.isTorqueCar or forcingTorqueController or usingNNFF
            visible &= not parent.isAngleCar
            
        toggle.setVisible(visible)
```

**From longitudinal_settings.cc**:
```python
# Conditional Experimental Mode visibility
if key == "CEStopLights":
    visible &= parent.tuningLevel < parent.starpilotToggleLevels["CEModelStopTime"].toDouble()

# Cruise controls
elif key in ["CustomCruise", "CustomCruiseLong", "SetSpeedLimit", "SetSpeedOffset"]:
    visible &= not parent.hasPCMD

# MapGears
elif key == "MapGears":
    visible &= parent.isToyota
    visible &= not parent.isTSK

# HumanLaneChanges
elif key == "HumanLaneChanges":
    visible &= parent.hasRadar

# StartAccel
elif key == "StartAccel":
    visible &= not (params.getBool("LongitudinalTune") and params.getBool("HumanAcceleration"))
```

**From visual_settings.cc**:
```python
if key == "AccelerationPath":
    visible &= parent.hasOpenpilotLongitudinal
    
elif key == "BlindSpotPath":
    visible &= parent.hasBSM
    
elif key == "PedalsOnUI":
    visible &= parent.hasOpenpilotLongitudinal
    
elif key == "HideSpeedLimit":
    visible &= parent.hasOpenpilotLongitudinal AND SpeedLimitController
    
elif key == "ShowSpeedLimits":
    visible &= not params.getBool("SpeedLimitController") or not parent.hasOpenpilotLongitudinal
    
elif key == "SLCMapboxFiller":
    visible &= params.getBool("ShowSpeedLimits")
    visible &= not params.getBool("SpeedLimitController") or not parent.hasOpenpilotLongitudinal
    visible &= not params.get("MapboxSecretKey").empty()
```

**From vehicle_settings.cc**:
```python
if key in gmKeys:
    visible &= parent.isGM
elif key in hkgKeys:
    visible &= parent.isHKG
elif key in subaruKeys:
    visible &= parent.isSubaru
elif key in toyotaKeys:
    visible &= parent.isToyota

# Specific conditions
if key == "SNGHack":
    visible &= not parent.hasSNG
    
elif key == "GMPedalLongitudinal":
    visible &= parent.hasPedal or (Hardware.PC() and parent.canUsePedal)
    
elif key == "RemapCancelToDistance":
    visible &= parent.isBolt and (parent.hasPedal or (Hardware.PC() and parent.canUsePedal))
```

---

### 9.7 Onroad Overlays (Priority 4)

| Feature | Qt File | Implementation |
|---------|---------|----------------|
| Blind spot visualization | `starpilot_onroad.cc` | Read BSM from car state, draw indicator |
| FPS counter overlay | `hud_renderer.py` | Already partial, add toggle |
| Steering torque | `hud_renderer.py` | Read from car state, display value |
| Turn signals | `hud_renderer.py` | Read signal state, show indicator |

---

### 9.8 Developer Sidebar (Priority 5)

**File**: `selfdrive/ui/layouts/sidebar.py`

Add toggle in sidebar for developer metrics. Show additional metrics when enabled (CPU, memory, etc. with more detail).

---

### 9.9 Implementation Order

1.  **Phase 1**: ✅ Add value factory functions to list_view.py (7 types done!)
2.  **Phase 2**: ✅ Implement three-level navigation system (sub-sub-sub-panels for Weather)
3.  **Phase 3**: ✅ Create StarPilotState singleton (car state parsing + desktop fallback)
4.  **Phase 4**: ✅ Create InputDialog & SelectionDialog widgets
5.  **Phase 5**: ✅ Extend Params with get_int/get_float/put_int/put_float
6.  **Phase 6**: ✅ Implement Lateral 5 sub-panels (~21 controls, car-specific ranges, conditional visibility)
7.  **Phase 7**: ✅ Implement Weather sub-panels with real value_item controls + put_int callbacks
8.  **Phase 8**: ✅ Complete Sounds panel (7 volume sliders + Test buttons + sub-panel + custom alerts with conditional visibility)
9.  **Phase 9**: Complete Driving Model (model selection dialog, blacklist, ratings)
10. **Phase 10**: Implement Longitudinal remaining sub-panels (~60+ controls)
11. **Phase 11**: Complete Visual 5 sub-panels (~35 controls)
12. **Phase 12**: Complete Themes (7 theme types, download/delete/select buttons)
13. **Phase 13**: Complete Vehicle 5 sub-panels (~20 controls, car make/model selection)
14. **Phase 14**: Complete remaining panels (Device, Navigation, Data, Maps, Utilities, Wheel)
15. **Phase 15**: Add conditional visibility logic to ALL panels (Lateral done, rest pending)
16. **Phase 16**: Implement real button actions (download, delete, reset, dialogs)
17. **Phase 17**: Add metric unit conversion support (constants and label updates)
18. **Phase 18**: Onroad overlays (4 features)
19. **Phase 19**: Developer sidebar toggle

---

### 9.10 Qt Reference Files Summary

| Panel | Qt File | Lines | Sub-Panels | Controls | Special Features |
|-------|---------|-------|------------|---------|------------------|
| Sounds | `sounds_settings.cc` | 222 | 2 | 14+ | Test buttons, sub-panel navigation |
| Model | `model_settings.cc` | 814 | 2 | 9+ | Download/delete dialogs, model selection |
| Lateral | `lateral_settings.cc` | 428 | 5 | ~25 | Car-specific ranges, metric conversion |
| Longitudinal | `longitudinal_settings.cc` | 1085 | 17 | ~100+ | THREE-LEVEL NAV, dual sliders, weather |
| Visual | `visual_settings.cc` | 328 | 5 | ~35 | Auto unit conversion |
| Themes | `theme_settings.cc` | 935 | 1 | ~50+ | Download/select for 7 types |
| Navigation | `navigation_settings.cc` | 330 | 1 | 5+ | Key management, setup instructions |
| Data | `data_settings.cc` | 872 | 1 | 6+ | Backup/restore, stats viewing |
| Device | `device_settings.cc` | 247 | 2 | ~14 | Screen recording |
| Vehicle | `vehicle_settings.cc` | 467 | 5 | ~20 | Car-specific visibility |
| Wheel | `wheel_settings.cc` | 84 | 0 | 4 | Dynamic options based on car |
| Utilities | `utilities.cc` | 369 | 0 | 7 | Network pairing, reporting |
| Maps | `maps_settings.cc` | 278 | 2 | 9+ | Country/state selection |
| **TOTAL** | | **~5559** | **43** | **~350+** | |

---

### 9.11 Key Qt Classes to Port

| Qt Class | Purpose | Where Used |
|----------|---------|------------|
| `StarPilotParamValueControl` | Simple value slider | Lateral, Longitudinal, Visual, Device |
| `StarPilotParamValueButtonControl` | Value + Reset/Test button | Sounds, Model, Lateral, Vehicle |
| `StarPilotDualParamValueControl` | Two connected sliders | Longitudinal (CESpeed) |
| `StarPilotButtonToggleControl` | Toggle + sub-toggles | Longitudinal, Visual, Vehicle |
| `StarPilotButtonsControl` | Multiple action buttons | Themes, Data, Device, Model |
| `ButtonParamControl` | Option selection button | Longitudinal (profiles), Visual (camera) |
| `StarPilotManageControl` | Opens sub-panel | All main panels |
| `LabelControl` | Read-only display | Device info, Stats, Calibration |
| `MapSelectionControl` | Checkbox list | Maps panel |

---

### 9.12 Special UI Behaviors

#### Reboot Requirements
Some toggles require a system reboot when changed while driving:
```cpp
// Toggles that require reboot
QSet<QString> rebootKeys = {
    "AlwaysOnLateral",        // Lateral
    "ForceTorqueController", // Lateral
    "NNFF",                  // Lateral
    "NNFFLite",              // Lateral
    "DisableOpenpilotLongitudinal", // Vehicle
    "TacoTuneHacks",         // Vehicle (HKG)
    "RemapCancelToDistance", // Vehicle (GM)
    "RemoteStartBootsComma", // Vehicle (GM - triggers flash)
};

// Confirmation pattern
if (StarPilotConfirmationDialog::toggleReboot(this)) {
    Hardware::reboot();
}
```

#### Warning Toggles
Some toggles have warning labels:
-   `NoLogging`: "Warning: ..."
-   `IncreaseThermalLimits`: "Warning: ..."
-   `LockDoorsTimer`: "<b>Warning:</b> openpilot can't detect if keys are still inside the car..."

#### Network Operations
Weather key testing:
```cpp
// Tests OpenWeatherMap API v2.5 and v3.0
QString url30 = "https://api.openweathermap.org/data/3.0/onecall?lat=...&appid=" + key;
QString url25 = "https://api.openweathermap.org/data/2.5/weather?lat=...&appid=" + key;
```

#### Sound Testing
Sound alert testing:
```cpp
// Offroad: Play via Python sounddevice
QString stockPath = "selfdrive/assets/sounds/" + snakeCaseAlert + ".wav";
QString themePath = "starpilot/assets/active_theme/sounds/" + snakeCaseAlert + ".wav";
float volume = params.getFloat(key) / 100.0f;

// Onroad: Send to params_memory for onroad testing
params_memory.put("TestAlert", camelCaseAlert);
```

#### Theme Name Parsing
```cpp
// Format: "name~creator" (stored lowercase)
// User-created: "name-user_created"
// Display: "Name 🌟" or "Name - by: creator"
QString baseName = value.split("~")[0];
bool userCreated = value.endsWith("-user_created");
```

#### Personality Reset Flow
Each personality has a reset button:
```cpp
// Example: Aggressive personality reset
params.putFloat("AggressiveFollow", defaultValue);
params.putFloat("AggressiveJerkAcceleration", defaultValue);
// ... other values
aggressiveFollowToggle->refresh();
aggressiveAccelerationToggle->refresh();
// ... refresh all toggles
```

#### Button State Management
Buttons can be dynamically shown/hidden/enabled:
```cpp
downloadButton->setEnabledButtons(0, !allDownloaded && online && parked);
downloadButton->setVisibleButton(1, !downloading);
downloadButton->setText(0, downloading ? "CANCEL" : "DOWNLOAD");
downloadButton->setValue("Downloading...");
```

#### Description Expansion
Qt panels track which descriptions are expanded:
```cpp
// Toggle description visibility
toggle->showDescription();  // Expand
toggle->hideDescription();  // Collapse

// Update layout after description change
connect(toggle, &AbstractControl::showDescriptionEvent, this, &Panel::update);
```

#### Model Selection Features
-   Groups by series (Custom Series, Driving Policy, etc.)
-   Icons: 🗺️ (Navigation), 📡 (Radar), 👀 (VOACC)
-   Sort modes: alphabetical, release date, rating
-   User favorites (stored in params)
-   Community favorites (stored in params)
-   Release dates (displayed in dialog)

---

### 9.12 Required Car State Properties

For conditional visibility, need to track:

```python
class StarPilotCarState:
    # ========== Car Type Detection ==========
    isGM: bool = False           # General Motors
    isHKG: bool = False          # Hyundai/Kia/Genesis
    isToyota: bool = False       # Toyota/Lexus
    isSubaru: bool = False       # Subaru
    isVolt: bool = False         # 2017 Chevy Volt (specific GM)
    isBolt: bool = False         # Chevy Bolt (specific GM)
    isAngleCar: bool = False     # Uses angle-based steering
    isTorqueCar: bool = False    # Uses torque-based steering
    isTSK: bool = False          # Toyota Safety Connect
    isHKGCanFd: bool = False     # Hyundai/Kia with CanFD
    
    # ========== Car Capabilities ==========
    hasBSM: bool = False         # Blind spot monitoring
    hasRadar: bool = False       # Radar sensor
    hasPedal: bool = False      # Pedal interceptor
    hasSNG: bool = False         # Stop and Go
    hasNNFFLog: bool = False    # NNFF log file exists
    hasAutoTune: bool = False    # Has auto-tune capability
    hasOpenpilotLongitudinal: bool = False  # Can control accel/brake
    hasDashSpeedLimits: bool = False  # Dashboard speed limits
    hasZSS: bool = False         # ZSS steering sensor
    canUsePedal: bool = False    # Can use pedal for longitudinal
    canUseSDSU: bool = False     # Can use SDSU
    
    # ========== Device/Car State ==========
    isFrogsGoMoo: bool = False   # FrogsGoMoo device
    hasPCMCruise: bool = False   # Has PCM cruise control
    lkasAllowedForAOL: bool = False  # LKAS allowed for Always On Lateral
    openpilotLongitudinalControlDisabled: bool = False
    hasAlphaLongitudinal: bool = False
    
    # ========== Car Values for Range Calculation ==========
    # These are read from car params and used to calculate slider ranges
    steerActuatorDelay: float = 0.0   # Default: car-specific
    friction: float = 0.0             # Default: car-specific
    steerKp: float = 0.0              # Default: car-specific
    latAccelFactor: float = 0.0       # Default: car-specific
    steerRatio: float = 0.0           # Default: car-specific
    
    # Longitudinal values
    longitudinalActuatorDelay: float = 0.0
    startAccel: float = 0.0
    stopAccel: float = 0.0
    stoppingDecelRate: float = 0.0
    vEgoStarting: float = 0.0
    vEgoStopping: float = 0.0
```

**How Car Values Are Used**:
```python
# In showEvent(), ranges are calculated based on car values
steerKpMin = parent.steerKp * 0.5
steerKpMax = parent.steerKp * 1.5

# Title shows default value
title = f"Actuator Delay (Default: {parent.steerActuatorDelay})"
```

**How to Get Car Values**:
```cpp
// From Qt - reads from params
steerActuatorDelay = params.getFloat("SteerActuatorDelay");
if (steerActuatorDelay == 0) {
    steerActuatorDelay = params.getFloat("SteerActuatorDelayDefault");
}
```

---

## 10. Technical Reference

### 10.1 Core Application Class

**File:** `system/ui/lib/application.py`

**Key Properties:**
-   `width` - Screen width (2160 or 536)
-   `height` - Screen height (1080 or 240)
-   `target_fps` - Target frame rate (default: 60, tizi: 20)
-   `frame` - Current frame number

**Key Methods:**
-   `init_window(title, fps)` - Initialize raylib window
-   `render()` - Main render loop as generator
-   `texture(asset_path, width, height)` - Load and cache textures from openpilot assets (`selfdrive/assets/`)
-   `starpilot_texture(asset_path, width, height)` - Load and cache textures from StarPilot assets (`starpilot/assets/`)
-   `font(font_weight)` - Get font by weight
-   `set_modal_overlay(overlay, callback)` - Show modal dialogs

**Recent Changes:**
-   `_load_image_from_path` now returns early if `image.width == 0 or image.height == 0` (zero-size guard)
-   `starpilot_texture()` uses `importlib.resources` to resolve `starpilot/assets/` path

### 10.2 Widget Base Class

**File:** `system/ui/widgets/__init__.py`

**Key Methods:**
-   `render(rect)` - Main render entry point
-   `_render(rect)` - Abstract method for subclasses
-   `_process_mouse_events()` - Handle touch/click input
-   `show_event()` / `hide_event()` - Lifecycle hooks
-   `set_visible(visible)` - Show/hide widget
-   `set_enabled(enabled)` - Enable/disable widget

### 10.3 UI State Management

**File:** `selfdrive/ui/ui_state.py`

**UIState Singleton:**
-   Subscribes to message bus via `cereal.messaging.SubMaster`
-   Properties: `started`, `engaged`, `status`, `is_metric`, `has_longitudinal_control`
-   Methods: `update()`, `update_params()`

**Device Class:**
-   Manages screen brightness, wakefulness, interactive timeouts

### 10.4 Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `BIG=1` | Force big UI on any device | 0 (false) |
| `FPS` | Target FPS | 60 (20 on tizi) |
| `SCALE` | UI scaling factor | 1.0 |
| `SHOW_FPS=1` | Display FPS counter | 0 (false) |
| `SHOW_TOUCHES=1` | Show touch points | 0 (false) |
| `ENABLE_VSYNC=1` | Enable vertical sync | 0 (false) |

### 10.5 Device Detection

**File:** `system/ui/lib/application.py`

```python
@staticmethod
def big_ui() -> bool:
    return HARDWARE.get_device_type() in ('tici', 'tizi') or BIG_UI
```

**Device Types:**
-   `tici` - Big UI
-   `tizi` - Big UI
-   `mici` - Small UI
-   `pc` - Small UI (unless BIG=1)

### 10.6 New Dialog Widgets Reference

**InputDialog** (`system/ui/widgets/input_dialog.py`):
```python
InputDialog(
  title: str,              # Dialog title
  default_text: str = "",  # Pre-filled text
  hint_text: str = "",     # Placeholder text (gray)
  on_close: Callable[[DialogResult, str], None] | None = None  # (result, entered_text)
)
# Usage: text entry for Mapbox keys, weather API keys, naming themes, etc.
# Features: Keyboard widget, blinking cursor, dimmed background
```

**SelectionDialog** (`system/ui/widgets/selection_dialog.py`):
```python
SelectionDialog(
  title: str,
  options: list[str],                # Selectable options
  current_selection: int = 0,        # Initially selected index
  on_close: Callable[[DialogResult, int, str], None] | None = None  # (result, index, text)
)
# Usage: model selection, car make/model, profile selection, theme selection
# Features: Radio-button UI (green dot), scrollable list, dimmed background
```

**ConfirmDialog** (`system/ui/widgets/confirm_dialog.py`):
```python
ConfirmDialog(
  text: str,
  confirm_text: str,
  cancel_text: str | None = None,    # None defaults to "Cancel"
  rich: bool = False,                # NEW: Enable HtmlRenderer for rich text
  on_close: Callable[[DialogResult], None] | None = None
)
# alert_dialog(message, button_text) - convenience for single-button alerts
# Features: Rich text with HtmlRenderer + Scroller, keyboard shortcuts (Enter/Escape)
```

> [!TIP]
> **Pro Tip: Dynamic Ranges with Lambda Capture**
> For settings that depend on car-specific values (like PID/### Use the StarPilotState Singleton
Don't parse `Params` in individual widgets. Use the `starpilot_state` instance.
```python
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state

# Access car state or tuning levels
if starpilot_state.car_state.isGM:
  ...
```
ensures that if the car state changes (e.g. from a CarParams update), the UI reflects the new limits immediately without needing a layout reset.
>
> ```python
> min_val=lambda: starpilot_state.car_state.steerKp * 0.5,
> max_val=lambda: starpilot_state.car_state.steerKp * 1.5,
> ```

### 10.7 StarPilotState Singleton Reference

**File:** `selfdrive/ui/lib/starpilot_state.py`

**Global import:** `from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state`

**Key properties:**
-   `starpilot_state.car_state` — `StarPilotCarState` dataclass with all car type/capability/value fields
-   `starpilot_state.tuning_level` — Current tuning level (0-3)
-   `starpilot_state.toggle_levels` — Dict of toggle key → minimum tuning level

**Data sources parsed:**
1.  `CarParamsPersistent` — Car fingerprint, lateral tuning, capabilities
2.  `FrogPilotCarParamsPersistent` — canUsePedal, canUseSDSU, openpilotLongitudinalControlDisabled
3.  `LiveTorqueParameters` — hasAutoTune (useParams)
4.  `FrogPilotToggles` — JSON blob with supplementary toggles

**Update throttling:** 2.0s minimum interval between heavy param parsing

**Desktop fallback:** When `PC=True` and car is "mock", applies configurable fallback defaults (GM Bolt by default)

**Usage pattern in sub-panels:**
```python
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state

# Dynamic range from car state
value_button_item(
  lambda: tr("Kp Factor") + f" (Default: {starpilot_state.car_state.steerKp:.2f})",
  "SteerKP",
  min_val=lambda: starpilot_state.car_state.steerKp * 0.5,
  max_val=lambda: starpilot_state.car_state.steerKp * 1.5,
  ...
)

# Conditional visibility from car state
toggle_item(
  ...,
  enabled=lambda: not starpilot_state.car_state.isAngleCar and not starpilot_state.car_state.isTorqueCar
)
```

### 10.8 Params Helpers Reference

**File:** `common/params.py`

```python
# Reading typed params (with safe fallbacks)
params.get_int("TuningLevel", return_default=True, default=1)   # → int
params.get_float("SteerDelay", return_default=True, default=0.0) # → float

# Writing typed params (auto-detects ParamKeyType)
params.put_int("IncreaseFollowingLowVisibility", 2)   # Checks get_type(), casts appropriately
params.put_float("SteerDelay", 0.5)                    # Same type-aware behavior

# Type detection order: FLOAT → float(), INT → int(), BOOL → bool(), else → str()
```

### 10.9 Widget Factory Functions Reference

**File:** `system/ui/widgets/list_view.py`

These factory functions create standardized list items for settings panels:

```python
# Toggle item (on/off switch)
def toggle_item(
  title: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  initial_state: bool = False,
  callback: Callable | None = None,
  icon: str = "",
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  # Creates a toggle switch with icon and description
  action = ToggleAction(initial_state=initial_state, enabled=enabled, callback=callback)
  return ListItem(title=title, description=description, action_item=action, icon=icon)

# Button item (navigation/ action)
def button_item(
  title: str | Callable[[], str],
  button_text: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  callback: Callable | None = None,
  enabled: bool | Callable[[], bool] = True,
  icon: str = "",
  starpilot_icon: bool = False,  # For StarPilot assets
) -> ListItem:
  # Creates a button with title, description, and button text
  action = ButtonAction(text=button_text, enabled=enabled)
  item = ListItem(title=title, description=description, action_item=action, callback=callback, icon=icon if not starpilot_icon else "")
  if icon and starpilot_icon:
    item.set_icon(icon, starpilot=True)
  return item

# Text item (read-only display)
def text_item(
  title: str | Callable[[], str],
  value: str | Callable[[], str],
  description: str | Callable[[], str] | None = None,
  callback: Callable | None = None,
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  # Displays title and value side by side
  action = TextAction(text=value, color=ITEM_TEXT_VALUE_COLOR, enabled=enabled)
  return ListItem(title=title, description=description, action_item=action, callback=callback)

# Dual button item (two buttons - e.g., Reboot | Power Off)
def dual_button_item(
  left_text: str | Callable[[], str],
  right_text: str | Callable[[], str],
  left_callback: Callable = None,
  right_callback: Callable = None,
  description: str | Callable[[], str] | None = None,
  enabled: bool | Callable[[], bool] = True,
) -> ListItem:
  # Creates two buttons in one row
  action = DualButtonAction(left_text, right_text, left_callback, right_callback, enabled)
  return ListItem(title="", description=description, action_item=action)

# Multiple button item (selection buttons - e.g., Aggressive | Standard | Relaxed)
def multiple_button_item(
  title: str | Callable[[], str],
  description: str | Callable[[], str],
  buttons: list[str | Callable[[], str]],
  selected_index: int,
  button_width: int = BUTTON_WIDTH,
  callback: Callable = None,
  icon: str = "",
):
  # Creates multiple selectable buttons in a row
  action = MultipleButtonAction(buttons, button_width, selected_index, callback=callback)
  return ListItem(title=title, description=description, icon=icon, action_item=action)

# Simple item (just title, no action)
def simple_item(title: str | Callable[[], str], callback: Callable | None = None) -> ListItem:
  return ListItem(title=title, callback=callback)
```

**Value Control Factory Functions (StarPilot-specific):**
```python
# Value slider (param_key-based, reads/writes automatically)
def value_item(title, param_key, min_val, max_val, step, unit, description, callback, icon, enabled, is_metric, labels, negative)

# Value slider + action button (Reset/Test)
def value_button_item(title, param_key, min_val, max_val, step, button_text, button_callback, description, callback, icon, enabled, sub_toggles, labels, is_metric, negative)

# Two connected sliders (e.g., CESpeed "Without Lead" + "With Lead")
def dual_value_item(title, value1, value2, min_val, max_val, step, unit, label1, label2, description, callback1, callback2, icon, enabled)

# Toggle + sub-toggles (e.g., CECurves with "With Lead" sub-toggle)
def button_toggle_item(title, state, sub_toggles, sub_toggle_names, description, callback, sub_callbacks, icon, enabled, exclusive)

# Multiple action buttons (DELETE/DOWNLOAD/SELECT)
def buttons_item(title, buttons, button_callbacks, description, icon, enabled, initial_value)

# Button that opens selection dialog
def selection_button_item(title, options, selected_index, description, callback, icon, enabled)

# Read-only label display
def label_item(title, value, description, icon, enabled)

# Category buttons (horizontal row of buttons next to title)
def category_buttons_item(title, buttons, description, icon, button_width, enabled, starpilot_icon)

# Multiple selectable buttons with starpilot_icon support
def multiple_button_item(title, description, buttons, selected_index, button_width, callback, icon, starpilot_icon)
```

#### New Factor Functions Details (Added March 17, 2026)

**`dual_value_item`**:
-   Parameters: `title`, `value1`, `value2`, `min_val`, `max_val`, `step`, `unit`, `label1`, `label2`, `description`, `callback1`, `callback2`, `icon`, `enabled`, `labels`, `is_metric`.
-   Usage: Two connected sliders in one row (e.g. CESpeed "Without Lead" and "With Lead").

**`button_toggle_item`**:
-   Parameters: `title`, `state`, `sub_toggles`, `sub_toggle_names`, `description`, `callback`, `sub_callbacks`, `icon`, `enabled`, `exclusive`.
-   Usage: Main toggle with a "sub-menu" of associated buttons/toggles (e.g. CECurves with "With Lead").

**`buttons_item`**:
-   Parameters: `title`, `buttons`, `button_callbacks`, `description`, `icon`, `enabled`, `initial_value`.
-   Usage: Row of action buttons (e.g. themes Download/Delete/Select).

**`selection_button_item`**:
-   Parameters: `title`, `options`, `selected_index`, `description`, `callback`, `icon`, `enabled`.
-   Usage: Button that triggers a `SelectionDialog`.

**`label_item`**:
-   Parameters: `title`, `value`, `description`, `icon`, `enabled`.
-   Usage: Passive display of information (read-only).

**Key Constants:**
```python
ITEM_BASE_WIDTH = 1840  # Default item width
ITEM_BASE_HEIGHT = 126   # Default item height
ICON_SIZE = 80          # Icon dimensions
BUTTON_WIDTH = 255      # Default button width
ITEM_PADDING = 50       # Padding inside items
RIGHT_ITEM_PADDING = 20 # Spacing between items
BUTTON_FONT_SIZE = 35   # Category button text size
```

### 10.10 ListItem Class Reference

**File:** `system/ui/widgets/list_view.py`

The `ListItem` class is the base widget for all settings list items:

```python
class ListItem(Widget):
  def __init__(self, title: str | Callable[[], str] = "", 
               icon: str | None = None, 
               description: str | Callable[[], str] | None = None,
               description_visible: bool = False, 
               callback: Callable | None = None,
               action_item: ItemAction | None = None):
    # title: Display title (string or callable for dynamic text)
    # icon: Icon filename (auto-loaded from icons/ or StarPilot assets)
    # description: Expandable description text (supports HTML bold tags)
    # callback: Called when action_item is activated (via set_click_callback)
    # action_item: The right-side widget (Toggle, Button, Text, CategoryButtons, etc.)
```

**Key Methods:**
-   `set_icon(icon, starpilot=False)` - Set icon, optionally from StarPilot assets
-   `set_description(description)` - Update description text
-   `set_visible(visible)` - Show/hide item
-   `set_enabled(enabled)` - Enable/disable item
-   `set_click_callback(callback)` - Set callback (renamed from `.callback` attribute)
-   `get_item_height(font, max_width)` - Calculate item height (including expanded description)
-   **Sound Test Process**: Uses a persistent Python subprocess to play sounds without blocking the UI thread or or causing `sounddevice` crashes. Spawned in `sounds.py`.
-   **Panel Modularization**: The massive `starpilot.py` was successfully split into 14 distinct modules in `selfdrive/ui/layouts/settings/starpilot/` for better maintainability (March 2026).

---

> [!IMPORTANT]
> **The Golden Rule of UI Parity**
> When porting from Qt (C++), **functional parity is not enough**. You must achieve **visual parity**. 
> -   If the Qt UI has a 3px border, the Raylib UI must have a 3px border.
> -   If the Qt UI uses a specific hex color (e.g. `#333333`), DO NOT use a generic Gray. Use the exact hex.
> -   Premium interfaces feel "alive" through responsive feedback (instant label changes) and high-fidelity styling.
> -   Avoid "patchwork" designs; if a widget doesn't support the required look, extend the widget rather than hacking the layout.

---

## Appendix A: File Location Reference

### Qt Files (for reference)
-   `selfdrive/ui/qt/window.cc/h` - Main window
-   `selfdrive/ui/qt/home.cc/h` - Home window container
-   `selfdrive/ui/qt/sidebar.cc/h` - Sidebar
-   `selfdrive/ui/qt/onroad/onroad_home.cc/h` - Onroad UI
-   `selfdrive/ui/qt/offroad/settings.cc/h` - Settings window

### Raylib Files (current implementation)
- `common/params.py` - Extended Params wrapper (get_int, get_float, put_int, put_float)
3. **Inherit from `StarPilotPanel`**:
   Sub-panels should inherit from `StarPilotPanel` and override `_render` (usually just `self._scroller.render(rect)`).
   ```python
   from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
   ```

---

## Appendix B: StarPilot-Specific Qt Files Reference

### Onroad
- `starpilot/ui/qt/onroad/starpilot_onroad.cc/h` - Main StarPilot onroad
- `starpilot/ui/qt/onroad/starpilot_buttons.cc/h` - Custom buttons
- `starpilot/ui/qt/onroad/starpilot_annotated_camera.cc/h` - Camera with overlays

### Offroad Settings
- `starpilot/ui/qt/offroad/starpilot_settings.cc/h` - Main StarPilot settings
- `starpilot/ui/qt/offroad/lateral_settings.cc/h` - Steering settings
- `starpilot/ui/qt/offroad/longitudinal_settings.cc/h` - Gas/brake settings
- `starpilot/ui/qt/offroad/theme_settings.cc/h` - Theme settings
- `starpilot/ui/qt/offroad/visual_settings.cc/h` - Visual settings

### Widgets
- `starpilot/ui/qt/widgets/drive_stats.cc/h` - Drive statistics
- `starpilot/ui/qt/widgets/drive_summary.cc/h` - Drive summary
- `starpilot/ui/qt/widgets/developer_sidebar.cc/h` - Developer sidebar

---

## Appendix C: Reboot Confirmation Pattern

Used by toggles that require reboot when changed while driving (ALwaysOnLateral, NNFF, ForceTorqueController, etc.):

```python
def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state
    if ui_state.started:
        from openpilot.system.ui.lib.application import gui_app
        from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
        from openpilot.system.ui.widgets import DialogResult
        def _confirm_reboot(res):
            gui_app.set_modal_overlay(None)
            if res == DialogResult.CONFIRM:
                from openpilot.system.hardware import HARDWARE
                HARDWARE.reboot()
        dialog = ConfirmDialog("Reboot required to take effect. Reboot now?", "Reboot", "Cancel", on_close=_confirm_reboot)
        gui_app.set_modal_overlay(dialog)
```

---

## Appendix D: Sub-Panel Navigation Pattern (Lateral Example)

Pattern for implementing sub-panels within a StarPilot panel:

```python
class StarPilotLateralLayout(StarPilotPanel):
    def __init__(self):
        super().__init__()
        
        self._sub_panels = {
            "advanced_lateral": StarPilotAdvancedLateralLayout(),
            "always_on_lateral": StarPilotAlwaysOnLateralLayout(),
            # ... more sub-panels
        }
        
        # Wire up navigation for sub-panels that have their own sub-panels
        for name, panel in self._sub_panels.items():
            if hasattr(panel, 'set_navigate_callback'):
                panel.set_navigate_callback(self._navigate_to)
        
        # Main panel items are button_items with callback=lambda: self._navigate_to("key")
        items = [
            button_item("Advanced Lateral Tuning", lambda: tr("MANAGE"), ...,
                        callback=lambda: self._navigate_to("advanced_lateral")),
        ]
        
        # All underlying routing and render/show_event forwarding is handled by StarPilotPanel automatically!
```

---

*Document generated for StarPilot UI porting efforts*
*Last updated: March 17, 2026*

---

## 11. RayGUI Analysis (Aborted)

### 11.1 Investigation Hypothesis
RayGUI is an immediate-mode GUI auxiliary module natively shipped with Raylib. A hypothesis was formed that swapping the custom python-based `widgets` (`Button`, `Toggle`, `Slider`) for pyray.Gui* calls (`gui_button`, `gui_toggle`, `gui_check_box`) would simplify the code while providing parity.

### 11.2 Line-By-Line Line Findings

A systematic, line-by-line analysis of `system/ui/widgets/` was conducted. **Conclusion: RayGUI does not possess the native capability to replace the existing custom UI.** 

1. **`toggle.py` (`Toggle`)**
   - **Custom Implementation:** Draws a perfect pill shape natively using `draw_rectangle_rounded(..., 1.0, ...)`, a distinct white knob via `draw_circle()`, and calculates smooth interpolation between On/Off states across frame ticks. Uses Qt legacy color palettes.
   - **RayGUI Limitation:** Native `gui_toggle` is a flat button. Native `gui_toggle_slider` does not natively interpolate its knob tracking smoothly; it snaps instantly. It also inherently demands text be drawn *inside* the slider pill.

2. **`button.py` (`Button`, `SmallButton`, `WideRoundedButton`)**
   - **Custom Implementation:** Uses an 11-style `ButtonStyle` enum mixing disparate border thicknesses, hover alpha filters, text, and base colors dynamically. Many buttons (`SmallButton`, `WidishRoundedButton`, `IconButton`) natively map to explicit *texture assets* for standard, pressed, and disabled states (e.g. `"icons_mici/setup/small_red_pill_pressed.png"`).
   - **RayGUI Limitation:** `gui_button` relies on a global style memory object. Swapping the entire structural geometry and color arrays via `GuiSetStyle()` before every button draw is messy and slower. Natively mapping 3D-shaded `.png` button textures natively to RayGUI forms is not officially supported without deep customization that defeats the simplicity of immediate-mode wrapping.

3. **`slider.py` (`SmallSlider`, `LargerSlider`)**
   - **Custom Implementation:** These are highly specialized "Swipe-to-Confirm" controls incorporating `FirstOrderFilter` physics modeling for bounce-back, drag thresholds, and custom red circle draggable knobs.
   - **RayGUI Limitation:** RayGUI does not offer a native swipe-to-confirm touch primitive or drag physics.

4. **`input_dialog.py` & On-Screen Interactions**
   - **Custom Implementation:** Custom Touch injected keyboard.
   - **RayGUI Limitation:** `gui_text_box` binds to GLFW and physical hardware events natively, breaking compatibility with our pure touch-first context.

### 11.3 Conclusion
The custom widgets authored in Python are mechanically superior, highly tailored to the automotive Qt styling aesthetics, and strictly capable of things RayGUI primitives are not (smooth physics, pure texture-mapping, swipe-to-confirm). The Raylib port will continue relying natively on the existing python `system/ui/widgets/` suite.
