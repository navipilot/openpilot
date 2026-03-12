#!/usr/bin/env python3
import json
import re
import urllib.request

from pathlib import Path

from openpilot.frogpilot.assets.download_functions import (
  GITLAB_URL,
  download_file,
  get_repository_url,
  handle_error,
  handle_request_error,
  verify_download,
)
from openpilot.frogpilot.common.frogpilot_utilities import delete_file
from openpilot.frogpilot.common.frogpilot_variables import MODELS_PATH

MANIFEST_CANDIDATES = ("v21",)
TINYGRAD_VERSIONS = {"v8", "v9", "v10", "v11", "v12"}
DEFAULT_MODEL_KEY = "sc"
MODEL_KEY_CANONICAL_MAP = {
  "sc2": "sc",
}

CANCEL_DOWNLOAD_PARAM = "CancelModelDownload"
DOWNLOAD_PROGRESS_PARAM = "ModelDownloadProgress"
MODEL_DOWNLOAD_PARAM = "ModelToDownload"
MODEL_DOWNLOAD_ALL_PARAM = "DownloadAllModels"
UPDATE_TINYGRAD_PARAM = "UpdateTinygrad"


def _clean_model_name(name: str) -> str:
  return re.sub(r"[🗺️👀📡]", "", str(name or "")).strip()


class ModelManager:
  def __init__(self, params, params_memory, boot_run=False):
    self.params = params
    self.params_memory = params_memory
    self.downloading_model = False

    self.available_models = [entry for entry in (self.params.get("AvailableModels") or "").split(",") if entry]
    self.model_versions = [entry for entry in (self.params.get("ModelVersions") or "").split(",") if entry]
    self.model_series = [entry for entry in (self.params.get("AvailableModelSeries") or "").split(",") if entry]
    self.available_model_names = [entry for entry in (self.params.get("AvailableModelNames") or "").split(",") if entry]

    self._ensure_model_params()
    if boot_run:
      self._sync_selected_model_version()

  @staticmethod
  def _canonical_model_key(model_key: str) -> str:
    key = (model_key or "").strip()
    return MODEL_KEY_CANONICAL_MAP.get(key, key)

  def _param_text(self, key: str) -> str:
    raw = self.params.get(key)
    if raw is None:
      return ""
    if isinstance(raw, bytes):
      return raw.decode("utf-8", errors="ignore").strip()
    return str(raw).strip()

  def _default_param_text(self, key: str) -> str:
    try:
      default_value = self.params.get_default_value(key)
    except Exception:
      return ""
    if default_value is None:
      return ""
    if isinstance(default_value, bytes):
      return default_value.decode("utf-8", errors="ignore").strip()
    return str(default_value).strip()

  def _set_model_param_keys(self, model_key: str | None = None, model_name: str | None = None, model_version: str | None = None):
    if model_key is not None and model_key != "":
      canonical_key = self._canonical_model_key(model_key)
      self.params.put("Model", canonical_key)
      self.params.put("DrivingModel", canonical_key)
    if model_name is not None and model_name != "":
      self.params.put("DrivingModelName", model_name)
    if model_version is not None and model_version != "":
      self.params.put("ModelVersion", model_version)
      self.params.put("DrivingModelVersion", model_version)

  def _ensure_model_params(self):
    selected_model = self._selected_model()
    current_version = self._param_text("ModelVersion") or self._param_text("DrivingModelVersion")
    if not current_version:
      current_version = self._default_param_text("ModelVersion") or self._default_param_text("DrivingModelVersion") or "v11"

    selected_name = self._param_text("DrivingModelName")
    if not selected_name and selected_model in self.available_models:
      selected_index = self.available_models.index(selected_model)
      if selected_index < len(self.available_model_names):
        selected_name = self.available_model_names[selected_index]

    self._set_model_param_keys(selected_model, selected_name, current_version)

  def _model_key_aliases(self, model_key: str) -> list[str]:
    canonical_key = self._canonical_model_key(model_key)
    aliases = [canonical_key]
    # Preserve legacy alias lookups (e.g. sc2) even when canonicalized to sc.
    for alias, canonical in MODEL_KEY_CANONICAL_MAP.items():
      if canonical == canonical_key:
        aliases.append(alias)
    if model_key.endswith("_default"):
      aliases.append(model_key[:-8])
    # v21 manifest uses legacy IDs with a trailing "2" (e.g. sc -> sc2).
    if model_key and not model_key.endswith("2"):
      aliases.append(f"{model_key}2")
    return [alias for alias in dict.fromkeys(aliases) if alias]

  def _model_version_map(self) -> dict[str, str]:
    return {
      model_key: self.model_versions[index]
      for index, model_key in enumerate(self.available_models)
      if index < len(self.model_versions) and model_key
    }

  def _selected_model(self) -> str:
    selected = self._param_text("Model") or self._param_text("DrivingModel")
    if selected:
      return self._canonical_model_key(selected)
    default_value = self._default_param_text("Model") or self._default_param_text("DrivingModel")
    if default_value:
      return self._canonical_model_key(default_value)
    return DEFAULT_MODEL_KEY

  def _required_files(self, model_key: str, model_version: str) -> list[str]:
    if model_version not in TINYGRAD_VERSIONS:
      return []

    filenames = [
      f"{model_key}_driving_policy_tinygrad.pkl",
      f"{model_key}_driving_vision_tinygrad.pkl",
      f"{model_key}_driving_policy_metadata.pkl",
      f"{model_key}_driving_vision_metadata.pkl",
    ]

    if model_version == "v12":
      filenames += [
        f"{model_key}_driving_off_policy_tinygrad.pkl",
        f"{model_key}_driving_off_policy_metadata.pkl",
      ]

    return filenames

  def _is_model_downloaded(self, model_key: str, model_version: str) -> bool:
    required_files = self._required_files(model_key, model_version)
    if not required_files:
      return False
    return all((MODELS_PATH / filename).is_file() for filename in required_files)

  def _sync_selected_model_version(self):
    version_map = self._model_version_map()
    name_map = {model_key: model_name for model_key, model_name in zip(self.available_models, self.available_model_names)}
    selected = self._selected_model()
    version = version_map.get(selected)
    if version:
      self._set_model_param_keys(selected, name_map.get(selected), version)
      return

    for alias in self._model_key_aliases(selected):
      version = version_map.get(alias)
      if version:
        selected_name = name_map.get(selected) or name_map.get(alias) or self._param_text("DrivingModelName")
        self._set_model_param_keys(selected, selected_name, version)
        return

    fallback_version = self._param_text("ModelVersion") or self._param_text("DrivingModelVersion")
    if not fallback_version:
      fallback_version = self._default_param_text("ModelVersion") or self._default_param_text("DrivingModelVersion") or "v11"
    self._set_model_param_keys(selected, name_map.get(selected, ""), fallback_version)

  @staticmethod
  def _fetch_manifest(url: str) -> list[dict]:
    try:
      with urllib.request.urlopen(url, timeout=10) as response:
        payload = json.loads(response.read().decode("utf-8"))
      return payload.get("models", []) if isinstance(payload, dict) else []
    except Exception as error:
      handle_request_error(error, None, None, None, None)
      return []

  def _get_manifest(self, repo_url: str) -> tuple[str | None, list[dict]]:
    for manifest_version in MANIFEST_CANDIDATES:
      model_info = self._fetch_manifest(f"{repo_url}/Versions/model_names_{manifest_version}.json")
      if not model_info:
        continue

      # Desktop/dev build is tinygrad-only.
      filtered = [model for model in model_info if model.get("version") in TINYGRAD_VERSIONS]
      if not filtered:
        continue

      return manifest_version, filtered

    return None, []

  def _remove_stale_model_files(self):
    valid_keys = set(self.available_models)
    for model_file in MODELS_PATH.glob("*_driving_*"):
      model_key = model_file.name.split("_driving_", 1)[0]
      if model_key not in valid_keys:
        delete_file(model_file, print_error=False)

    for temp_file in MODELS_PATH.glob("tmp*"):
      delete_file(temp_file, print_error=False)

  def _enforce_selected_model(self):
    if not self.available_models:
      return

    selected = self._selected_model()
    aliases = self._model_key_aliases(selected)
    if any(alias in self.available_models for alias in aliases):
      self._sync_selected_model_version()
      return

    try:
      default_model = self._default_param_text("Model") or self._default_param_text("DrivingModel")
    except Exception:
      default_model = DEFAULT_MODEL_KEY

    candidates = self._model_key_aliases(default_model) + self._model_key_aliases(DEFAULT_MODEL_KEY) + self.available_models
    replacement = next((entry for entry in candidates if entry in self.available_models), self.available_models[0])

    replacement_index = self.available_models.index(replacement)
    replacement_name = self.available_model_names[replacement_index] if replacement_index < len(self.available_model_names) else replacement
    self._set_model_param_keys(replacement, replacement_name, None)
    self._sync_selected_model_version()

  def update_model_params(self, model_info: list[dict], manifest_version: str):
    del manifest_version
    self.available_models = [str(model.get("id") or "").strip() for model in model_info]
    self.available_model_names = [_clean_model_name(model.get("name")) for model in model_info]
    self.model_versions = [str(model.get("version") or "").strip() for model in model_info]
    self.model_series = [str(model.get("series") or "Custom Series").strip() for model in model_info]

    released_dates = [str(model.get("released") or "2023-01-01").strip() for model in model_info]
    community_favorites = [model_key for model_key, model in zip(self.available_models, model_info) if model.get("community_favorite", False)]

    self.params.put("AvailableModels", ",".join(self.available_models))
    self.params.put("AvailableModelNames", ",".join(self.available_model_names))
    self.params.put("AvailableModelSeries", ",".join(self.model_series))
    self.params.put("ModelReleasedDates", ",".join(released_dates))
    self.params.put("ModelVersions", ",".join(self.model_versions))
    self.params.put("CommunityFavorites", ",".join(community_favorites))

    self._sync_selected_model_version()

    try:
      version_map = {model_key: version for model_key, version in zip(self.available_models, self.model_versions)}
      versions_file = MODELS_PATH / ".model_versions.json"
      versions_file.parent.mkdir(parents=True, exist_ok=True)
      versions_file.write_text(json.dumps(version_map))
    except Exception as error:
      print(f"Failed to write model versions cache: {error}")

  def check_models(self, boot_run: bool):
    del boot_run  # Not currently needed, retained for call-site parity.
    self._remove_stale_model_files()
    self._enforce_selected_model()

  def update_models(self, boot_run=False):
    if self.downloading_model:
      return

    repo_url = get_repository_url()
    if repo_url is None:
      print("GitHub and GitLab are offline...")
      return

    manifest_version, model_info = self._get_manifest(repo_url)
    if not model_info:
      print("No compatible tinygrad manifest found.")
      return

    self.update_model_params(model_info, manifest_version or "unknown")
    self.check_models(boot_run)

  def download_model(self, model_to_download: str):
    self.downloading_model = True

    repo_url = get_repository_url()
    if not repo_url:
      handle_error(None, "GitHub and GitLab are offline...", "Repository unavailable", MODEL_DOWNLOAD_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
      self.downloading_model = False
      return

    version_map = self._model_version_map()
    model_version = version_map.get(model_to_download)
    required_files = self._required_files(model_to_download, model_version or "")
    if not required_files:
      handle_error(None, f"Unsupported model format for {model_to_download}", "Model download failed", MODEL_DOWNLOAD_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
      self.downloading_model = False
      return

    for filename in required_files:
      file_path = MODELS_PATH / filename
      file_url = f"{repo_url}/Models/{filename}"

      download_file(CANCEL_DOWNLOAD_PARAM, file_path, DOWNLOAD_PROGRESS_PARAM, file_url, MODEL_DOWNLOAD_PARAM, self.params_memory)
      if self.params_memory.get_bool(CANCEL_DOWNLOAD_PARAM):
        handle_error(None, "Download cancelled...", "Download cancelled...", MODEL_DOWNLOAD_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
        self.downloading_model = False
        return

      if verify_download(file_path, file_url):
        continue

      fallback_url = f"{GITLAB_URL}/Models/{filename}"
      download_file(CANCEL_DOWNLOAD_PARAM, file_path, DOWNLOAD_PROGRESS_PARAM, fallback_url, MODEL_DOWNLOAD_PARAM, self.params_memory)
      if self.params_memory.get_bool(CANCEL_DOWNLOAD_PARAM):
        handle_error(None, "Download cancelled...", "Download cancelled...", MODEL_DOWNLOAD_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
        self.downloading_model = False
        return

      if not verify_download(file_path, fallback_url):
        handle_error(file_path, "Verification failed...", f"Verification failed for {filename}", MODEL_DOWNLOAD_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
        self.downloading_model = False
        return

    self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, "Downloaded!")
    self.params_memory.remove(MODEL_DOWNLOAD_PARAM)
    self.downloading_model = False

  def download_all_models(self):
    repo_url = get_repository_url()
    if not repo_url:
      handle_error(None, "GitHub and GitLab are offline...", "Repository unavailable", MODEL_DOWNLOAD_ALL_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
      return

    manifest_version, model_info = self._get_manifest(repo_url)
    if not model_info:
      handle_error(None, "Unable to fetch models...", "Model list unavailable", MODEL_DOWNLOAD_ALL_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
      return

    self.update_model_params(model_info, manifest_version or "unknown")

    for model_key, model_name in zip(self.available_models, self.available_model_names):
      if self.params_memory.get_bool(CANCEL_DOWNLOAD_PARAM):
        handle_error(None, "Download cancelled...", "Download cancelled...", MODEL_DOWNLOAD_ALL_PARAM, DOWNLOAD_PROGRESS_PARAM, self.params_memory)
        return

      model_version = self._model_version_map().get(model_key, "")
      if self._is_model_downloaded(model_key, model_version):
        continue

      self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, f"Downloading \"{model_name}\"...")
      self.download_model(model_key)
      if self.params_memory.get_bool(CANCEL_DOWNLOAD_PARAM):
        return

    self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, "All models downloaded!")
    self.params_memory.remove(MODEL_DOWNLOAD_ALL_PARAM)

  def update_tinygrad(self):
    # This branch ships tinygrad runtime in-tree. "Update" here refreshes local model files.
    self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, "Updating...")

    for model_file in MODELS_PATH.glob("*_driving_*"):
      if model_file.is_file():
        delete_file(model_file, print_error=False)

    model_versions_file = MODELS_PATH / ".model_versions.json"
    if model_versions_file.is_file():
      delete_file(model_versions_file, print_error=False)

    self.params.put_bool("TinygradUpdateAvailable", False)
    self.params_memory.remove(UPDATE_TINYGRAD_PARAM)
    self.params_memory.remove(CANCEL_DOWNLOAD_PARAM)

    if self.params.get_bool("AutomaticallyDownloadModels"):
      self.params_memory.put_bool(MODEL_DOWNLOAD_ALL_PARAM, True)
      self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, "Downloading...")
    else:
      self.params_memory.put(DOWNLOAD_PROGRESS_PARAM, "Updated!")
