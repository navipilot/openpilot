from __future__ import annotations
import json
import os
import threading
from pathlib import Path

from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.frogpilot.assets.model_manager import ModelManager
from openpilot.system.hardware import HARDWARE, PC
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.system.ui.widgets.list_view import toggle_item, button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotDrivingModelLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._toggle_items = {}

    if PC:
      self._model_dir = Path(os.path.expanduser("~/.comma/frogpilot/data/models"))
    else:
      self._model_dir = Path("/data/frogpilot/models")
    self._model_dir.mkdir(parents=True, exist_ok=True)

    self._available_models = []
    self._available_model_names = []
    self._available_model_series = []
    self._available_model_versions = []
    self._model_released_dates = {}
    self._model_file_to_name = {}
    self._model_series_map = {}
    self._model_version_map = {}
    self._current_model_name = ""

    self._toggle_items["ModelRandomizer"] = toggle_item(
      tr_noop("Model Randomizer"),
      tr_noop("<b>Driving models are chosen at random each drive</b> and feedback prompts are used to find the model that best suits your needs."),
      self._params.get_bool("ModelRandomizer"),
      callback=self._on_model_randomizer_toggled,
    )

    self._toggle_items["AutomaticallyDownloadModels"] = toggle_item(
      tr_noop("Automatically Download New Models"),
      tr_noop("<b>Automatically download new driving models</b> as they become available."),
      self._params.get_bool("AutomaticallyDownloadModels"),
      callback=self._on_auto_download_toggled,
    )

    self._select_model_btn = button_item(
        tr_noop("Select Driving Model"),
        lambda: tr("SELECT"),
        tr_noop("<b>Select the active driving model.</b>"),
        callback=self._on_select_model_clicked,
      )
    self._download_model_btn = button_item(
        tr_noop("Download Driving Models"),
        lambda: tr("DOWNLOAD"),
        tr_noop("<b>Download driving models to the device.</b>"),
        callback=self._on_download_clicked,
      )
    self._delete_model_btn = button_item(
        tr_noop("Delete Driving Models"),
        lambda: tr("DELETE"),
        tr_noop("<b>Delete driving models from the device.</b>"),
        callback=self._on_delete_clicked,
      )

    items = [
      self._select_model_btn,
      self._download_model_btn,
      self._delete_model_btn,
      self._toggle_items["ModelRandomizer"],
      self._toggle_items["AutomaticallyDownloadModels"],
      button_item(
        tr_noop("Manage Model Blacklist"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Add or remove models</b> from the Model Randomizer's blacklist."),
        callback=self._on_blacklist_clicked,
      ),
      button_item(
        tr_noop("Manage Model Ratings"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Reset or view the saved ratings</b> for the driving models."),
        callback=self._on_scores_clicked,
      ),
    ]
    
    self._model_manager = ModelManager(self._params, self._params_memory)
    self._download_thread = None

    self._scroller = Scroller(items, line_separator=True, spacing=0)
    self._update_model_metadata()

  def _render(self, rect: rl.Rectangle):
    self._update_state()
    super()._render(rect)

  def show_event(self):
    super().show_event()
    self._update_model_metadata()

  def refresh_visibility(self):
    current_level = int(self._params.get("TuningLevel", return_default=True, default="1") or "1")
    for key, item in self._toggle_items.items():
      min_level = self._tuning_levels.get(key, 0)
      item.set_visible(current_level >= min_level)

  def _on_auto_download_toggled(self, state: bool):
    self._params.put_bool("AutomaticallyDownloadModels", state)

  def _is_model_installed(self, key: str) -> bool:
    if not key:
      return False

    has_thneed = False
    has_policy_meta = False
    has_policy_tg = False
    has_vision_meta = False
    has_vision_tg = False
    found_any = False

    for file in self._model_dir.iterdir():
      if not (file.name.startswith(key) or file.name.startswith(key + "_")):
        continue

      found_any = True
      ext = file.suffix.lower()
      base = file.stem

      if ext == ".thneed":
        has_thneed = True
      elif ext == ".pkl":
        if "_driving_policy_metadata" in base:
          has_policy_meta = True
        elif "_driving_policy_tinygrad" in base:
          has_policy_tg = True
        elif "_driving_vision_metadata" in base:
          has_vision_meta = True
        elif "_driving_vision_tinygrad" in base:
          has_vision_tg = True

    if has_thneed:
      return True

    return has_policy_meta and has_policy_tg and has_vision_meta and has_vision_tg

  def _update_model_metadata(self):
    available_models_raw = self._params.get("AvailableModels", encoding='utf-8')
    if not available_models_raw:
      manager = ModelManager(self._params, self._params_memory)
      manager.update_models()
      available_models_raw = self._params.get("AvailableModels", encoding='utf-8')

    self._available_models = [m.strip() for m in (available_models_raw or "").split(",") if m.strip()]
    self._available_model_names = [m.strip() for m in (self._params.get("AvailableModelNames", encoding='utf-8') or "").split(",") if m.strip()]
    self._available_model_series = [m.strip() for m in (self._params.get("AvailableModelSeries", encoding='utf-8') or "").split(",") if m.strip()]
    self._available_model_versions = [m.strip() for m in (self._params.get("ModelVersions", encoding='utf-8') or "").split(",")]
    released_dates = (self._params.get("ModelReleasedDates", encoding='utf-8') or "").split(",")

    self._model_file_to_name.clear()
    self._model_series_map.clear()
    self._model_version_map.clear()
    self._model_released_dates.clear()

    size = min(len(self._available_models), len(self._available_model_names))
    for i in range(size):
      key = self._available_models[i].strip()
      name = self._available_model_names[i].strip()
      if not key or not name:
        continue

      series = self._available_model_series[i].strip() if i < len(self._available_model_series) else tr("Custom Series")
      self._model_file_to_name[key] = name
      self._model_series_map[key] = series
      if i < len(self._available_model_versions):
        version = self._available_model_versions[i].strip()
        if version:
          self._model_version_map[key] = version
      if i < len(released_dates):
        date = released_dates[i].strip()
        if date:
          self._model_released_dates[key] = date

    model_key = self._params.get("Model") or self._params.get("DrivingModel")
    if model_key:
      if isinstance(model_key, bytes):
        model_key = model_key.decode()
    if not model_key or not self._is_model_installed(model_key):
      model_key = self._params.get_default_value("Model") or self._params.get_default_value("DrivingModel") or ""
      if isinstance(model_key, bytes):
        model_key = model_key.decode()

    self._current_model_name = self._model_file_to_name.get(model_key, "Default")
    self._select_model_btn.action_item._text_source = self._current_model_name

  def _show_selection_dialog(self, title: str, options: dict[str, str] | list[str], current_val: str, on_confirm: Callable):
    if not options:
      gui_app.set_modal_overlay(alert_dialog(tr("No options available.")))
      return

    if isinstance(options, list):
      def _on_close_list(res, val):
        if res == DialogResult.CONFIRM:
          on_confirm(val)
      dialog = SelectionDialog(title, options, current_val, on_close=_on_close_list)
      gui_app.set_modal_overlay(dialog)
      return

    grouped = {}
    name_to_key = {}
    for key, name in options.items():
      series = self._model_series_map.get(key, tr("Custom Series"))
      if series not in grouped:
        grouped[series] = []
      grouped[series].append(name)
      name_to_key[name] = key
    
    for series in grouped:
      grouped[series].sort()
    
    sorted_series = sorted(grouped.keys())
    if "StarPilot" in sorted_series:
      sorted_series.remove("StarPilot")
      sorted_series.insert(0, "StarPilot")
    
    final_grouped = {s: grouped[s] for s in sorted_series}

    def _on_close_grouped(res, val):
      if res == DialogResult.CONFIRM:
        key = name_to_key.get(val, val)
        on_confirm(key)

    def _on_favorite_toggled(key):
      favs = [f.strip() for f in (self._params.get("UserFavorites", encoding='utf-8') or "").split(",") if f.strip()]
      if key in favs:
        favs.remove(key)
      else:
        favs.append(key)
      self._params.put("UserFavorites", ",".join(favs))

    user_favs = [f.strip() for f in (self._params.get("UserFavorites", encoding='utf-8') or "").split(",") if f.strip()]
    comm_favs = [f.strip() for f in (self._params.get("CommunityFavorites", encoding='utf-8') or "").split(",") if f.strip()]

    dialog = SelectionDialog(
        title, 
        final_grouped, 
        current_val, 
        on_close=_on_close_grouped,
        model_released_dates=self._model_released_dates,
        model_file_to_name=self._model_file_to_name,
        user_favorites=user_favs,
        community_favorites=comm_favs,
        on_favorite_toggled=_on_favorite_toggled
    )
    gui_app.set_modal_overlay(dialog)

  def _on_select_model_clicked(self):
    installed_models = {k: v for k, v in self._model_file_to_name.items() if self._is_model_installed(k)}
    if not installed_models:
      return

    def _on_confirm(model_key):
      self._params.put("Model", model_key)
      self._params.put("DrivingModel", model_key)
      self._params.put("DrivingModelName", installed_models[model_key])
      model_version = self._model_version_map.get(model_key, "")
      if model_version:
        self._params.put("ModelVersion", model_version)
        self._params.put("DrivingModelVersion", model_version)
      self._update_model_metadata()

      if ui_state.started:
        reboot_dialog = ConfirmDialog(
          tr("Reboot required to take effect. Reboot now?"),
          tr("Reboot"),
          tr("Cancel"),
          on_close=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None
        )
        gui_app.set_modal_overlay(reboot_dialog)

    self._show_selection_dialog(tr("Select Driving Model"), installed_models, self._current_model_name, _on_confirm)

  def _on_download_clicked(self):
    is_downloading = self._params_memory.get("ModelToDownload") or self._params_memory.get_bool("DownloadAllModels")
    if is_downloading:
      self._params_memory.remove("ModelToDownload")
      self._params_memory.put_bool("DownloadAllModels", False)
      self._params_memory.remove("ModelDownloadProgress")
      return

    not_installed = {k: v for k, v in self._model_file_to_name.items() if not self._is_model_installed(k)}
    def _on_confirm(model_key):
      self._params_memory.put("ModelToDownload", model_key)
      self._params_memory.put("ModelDownloadProgress", "Downloading...")

    self._show_selection_dialog(tr("Select Model to Download"), not_installed, "", _on_confirm)

  def _on_delete_clicked(self):
    installed = {k: v for k, v in self._model_file_to_name.items() if self._is_model_installed(k)}
    default_key = self._params.get_default_value("Model") or ""
    if isinstance(default_key, bytes):
      default_key = default_key.decode()
    current_key = self._params.get("Model", encoding='utf-8') or ""
    deletable = {k: v for k, v in installed.items() if k != default_key and k != current_key}
    
    def _on_confirm(model_key):
      def _execute_delete(confirm_res):
          if confirm_res == DialogResult.CONFIRM:
              for file in self._model_dir.iterdir():
                  if file.name.startswith(model_key):
                      file.unlink()
              self._update_model_metadata()
      
      confirm = ConfirmDialog(
          tr(f"Are you sure you want to delete the '{deletable[model_key]}' model?"),
          tr("Delete"),
          on_close=_execute_delete
      )
      gui_app.set_modal_overlay(confirm)

    self._show_selection_dialog(tr("Select Model to Delete"), deletable, "", _on_confirm)

  def _on_blacklist_clicked(self):
    blacklisted = [m.strip() for m in (self._params.get("BlacklistedModels", encoding='utf-8') or "").split(",") if m.strip()]
    blacklisted = [b for b in blacklisted if b]

    def _on_action_selected(res, val):
        if res == DialogResult.CONFIRM:
            if val == tr("ADD"):
                blacklistable = {k: v for k, v in self._model_file_to_name.items() if k not in blacklisted}
                self._show_selection_dialog(tr("Add to Blacklist"), blacklistable, "", lambda k: self._params.put("BlacklistedModels", ",".join(blacklisted + [k])))
            elif val == tr("REMOVE"):
                options = {k: self._model_file_to_name.get(k, k) for k in blacklisted}
                def _remove(k):
                   blacklisted.remove(k)
                   self._params.put("BlacklistedModels", ",".join(blacklisted))
                self._show_selection_dialog(tr("Remove from Blacklist"), options, "", _remove)
            elif val == tr("RESET ALL"):
                self._params.remove("BlacklistedModels")

    dialog = SelectionDialog(
        tr("Manage Blacklist"),
        [tr("ADD"), tr("REMOVE"), tr("RESET ALL")],
        on_close=_on_action_selected
    )
    gui_app.set_modal_overlay(dialog)

  def _on_scores_clicked(self):
    scores_raw = self._params.get("ModelDrivesAndScores", encoding='utf-8') or ""
    if not scores_raw:
      gui_app.set_modal_overlay(alert_dialog(tr("No model ratings found.")))
      return
    
    try:
        scores = json.loads(scores_raw)
        lines = [f"{k}: {v.get('Score', 0)}% ({v.get('Drives', 0)} drives)" for k, v in scores.items()]
        confirm = ConfirmDialog("\n".join(lines), tr("Close"), rich=True)
        gui_app.set_modal_overlay(confirm)
    except:
        pass

  def _on_model_randomizer_toggled(self, state: bool):
    self._params.put_bool("ModelRandomizer", state)
    if state:
        not_installed = [k for k in self._model_file_to_name if not self._is_model_installed(k)]
        if not_installed:
            def _on_download_confirm(res):
                if res == DialogResult.CONFIRM:
                    self._params_memory.put_bool("DownloadAllModels", True)
                    self._params_memory.put("ModelDownloadProgress", "Downloading...")
            
            confirm = ConfirmDialog(
                tr("Model Randomizer works best with all models. Download all now?"),
                tr("Download All"),
                on_close=_on_download_confirm
            )
            gui_app.set_modal_overlay(confirm)

  def _update_state(self):
    if not self.is_visible:
        return

    model_to_download = self._params_memory.get("ModelToDownload", encoding='utf-8') or ""
    download_all = self._params_memory.get_bool("DownloadAllModels")
    progress = self._params_memory.get("ModelDownloadProgress", encoding='utf-8') or ""
    is_downloading = bool(model_to_download or download_all)

    if is_downloading and (self._download_thread is None or not self._download_thread.is_alive()):
        def _download_task():
            try:
                if download_all:
                    print("Starting [All Models] download thread...")
                    self._model_manager.download_all_models()
                else:
                    print(f"Starting [{model_to_download}] download thread...")
                    self._model_manager.download_model(model_to_download)
                print("Download thread finished successfully.")
            except Exception as e:
                print(f"Download thread CRASHED: {e}")
                import traceback
                traceback.print_exc()
            finally:
                self._download_thread = None
        
        self._download_thread = threading.Thread(target=_download_task, daemon=True)
        self._download_thread.start()

    if is_downloading:
        self._download_model_btn.action_item._text_source = tr("CANCEL")
        self._download_model_btn.action_item._value_source = progress if progress else tr("Downloading...")
    else:
        self._download_model_btn.action_item._text_source = tr("DOWNLOAD")
        parked = not ui_state.started
        online = ui_state.sm["deviceState"].networkType != 0 if ui_state.sm.valid.get("deviceState", False) else True
        if not online:
            self._download_model_btn.action_item._value_source = tr("Offline...")
        elif not parked:
            self._download_model_btn.action_item._value_source = tr("Not parked")
        else:
            self._download_model_btn.action_item._value_source = ""
        
        all_installed = all(self._is_model_installed(k) for k in self._model_file_to_name)
        if all_installed:
             self._download_model_btn.action_item._value_source = tr("All Downloaded!")
