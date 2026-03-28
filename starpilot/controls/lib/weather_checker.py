#!/usr/bin/env python3
import json
import time
import urllib.error
import urllib.request
import urllib.parse

from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone

from openpilot.starpilot.common.starpilot_utilities import calculate_distance_to_point

CACHE_DISTANCE = 25
CHECK_INTERVAL = 15 * 60
MAX_RETRIES = 3
OPEN_METEO_URL = "https://api.open-meteo.com/v1/forecast"

# WMO Weather interpretation codes -> OpenWeatherMap condition IDs
# Reference: https://open-meteo.com/en/docs#weather_variable_documentation
WMO_TO_OWM = {
  0: 800,   # Clear sky
  1: 801,   # Mainly clear
  2: 802,   # Partly cloudy
  3: 803,   # Overcast
  45: 741,  # Fog
  48: 741,  # Depositing rime fog
  51: 300,  # Drizzle: Light
  53: 301,  # Drizzle: Moderate
  55: 302,  # Drizzle: Dense
  56: 311,  # Freezing Drizzle: Light
  57: 312,  # Freezing Drizzle: Dense
  61: 500,  # Rain: Slight
  63: 501,  # Rain: Moderate
  65: 502,  # Rain: Heavy
  66: 511,  # Freezing Rain: Light
  67: 511,  # Freezing Rain: Heavy
  71: 600,  # Snow fall: Slight
  73: 601,  # Snow fall: Moderate
  75: 602,  # Snow fall: Heavy
  77: 600,  # Snow grains
  80: 520,  # Rain showers: Slight
  81: 521,  # Rain showers: Moderate
  82: 522,  # Rain showers: Violent
  85: 620,  # Snow showers: Slight
  86: 621,  # Snow showers: Heavy
  95: 200,  # Thunderstorm: Slight/Moderate
  96: 201,  # Thunderstorm with slight hail
  99: 202,  # Thunderstorm with heavy hail
}

# Reference: https://openweathermap.org/weather-conditions
WEATHER_CATEGORIES = {
  "RAIN": {
    "ranges": [(300, 321), (500, 504)],
    "suffix": "rain",
  },
  "RAIN_STORM": {
    "ranges": [(200, 232), (511, 511), (520, 531)],
    "suffix": "rain_storm",
  },
  "SNOW": {
    "ranges": [(600, 622)],
    "suffix": "snow",
  },
  "LOW_VISIBILITY": {
    "ranges": [(701, 762)],
    "suffix": "low_visibility",
  },
  "CLEAR": {
    "ranges": [(800, 800)],
    "suffix": "clear",
  },
}


def _iso_to_unix(iso_str):
  """Convert an ISO8601 string (without timezone) to a UTC Unix timestamp."""
  try:
    return int(datetime.fromisoformat(iso_str).replace(tzinfo=timezone.utc).timestamp())
  except Exception:
    return 0


def _normalize_response(data):
  """Convert Open-Meteo's columnar response into the row-based format expected by StarPilot."""
  daily = data.get("daily", {})
  current = data.get("current", {})
  hourly = data.get("hourly", {})

  current_wmo = current.get("weather_code", 0)

  normalized = {
    "current": {
      "sunrise": _iso_to_unix(daily.get("sunrise", [None])[0] or ""),
      "sunset": _iso_to_unix(daily.get("sunset", [None])[0] or ""),
      "weather": [{"id": WMO_TO_OWM.get(current_wmo, 800)}],
    },
    "hourly": [
      {
        "dt": _iso_to_unix(t),
        "weather": [{"id": WMO_TO_OWM.get(wc, 800)}],
      }
      for t, wc in zip(hourly.get("time", []), hourly.get("weather_code", []))
    ],
  }

  return normalized


class WeatherChecker:
  def __init__(self, StarPilotPlanner):
    self.starpilot_planner = StarPilotPlanner

    self.is_daytime = False

    self.increase_following_distance = 0
    self.increase_stopped_distance = 0
    self.reduce_acceleration = 0
    self.reduce_lateral_acceleration = 0
    self.sunrise = 0
    self.sunset = 0
    self.weather_id = 0

    self.hourly_forecast = None
    self.last_gps_position = None
    self.last_updated = None
    self.requesting = False

    self.check_interval = CHECK_INTERVAL

    self.executor = ThreadPoolExecutor(max_workers=1)

  def update_offsets(self, starpilot_toggles):
    suffix = WEATHER_CATEGORIES["CLEAR"]["suffix"]
    for category in WEATHER_CATEGORIES.values():
      if any(start <= self.weather_id <= end for start, end in category["ranges"]):
        suffix = category["suffix"]
        break

    if suffix != WEATHER_CATEGORIES["CLEAR"]["suffix"]:
      self.increase_following_distance = getattr(starpilot_toggles, f"increase_following_distance_{suffix}")
      self.increase_stopped_distance = getattr(starpilot_toggles, f"increase_stopped_distance_{suffix}")
      self.reduce_acceleration = getattr(starpilot_toggles, f"reduce_acceleration_{suffix}")
      self.reduce_lateral_acceleration = getattr(starpilot_toggles, f"reduce_lateral_acceleration_{suffix}")
    else:
      self.increase_following_distance = 0
      self.increase_stopped_distance = 0
      self.reduce_acceleration = 0
      self.reduce_lateral_acceleration = 0

  def update_weather(self, now, starpilot_toggles):
    if self.last_gps_position and self.last_updated:
      distance = calculate_distance_to_point(
        self.last_gps_position["latitude"],
        self.last_gps_position["longitude"],
        self.starpilot_planner.gps_position.get("latitude"),
        self.starpilot_planner.gps_position.get("longitude")
      )
      if distance / 1000 > CACHE_DISTANCE:
        self.hourly_forecast = None
        self.last_updated = None

    if self.sunrise and self.sunset:
      self.is_daytime = self.sunrise <= int(now.timestamp()) < self.sunset

    if self.last_updated and (now - self.last_updated).total_seconds() < self.check_interval:
      if self.hourly_forecast:
        current_forecast = min(self.hourly_forecast, key=lambda f: abs(f["dt"] - now.timestamp()))
        self.weather_id = current_forecast.get("weather", [{}])[0].get("id", 0)
        self.update_offsets(starpilot_toggles)
      return

    if self.requesting:
      return

    self.requesting = True

    def complete_request(future):
      self.requesting = False
      data = future.result()
      if data:
        self.last_updated = now
        self.hourly_forecast = data.get("hourly")
        self.last_gps_position = self.starpilot_planner.gps_position

        if "current" in data:
          source_data = data.get("current", {})
          current_data = source_data
        else:
          source_data = data
          current_data = source_data.get("sys", source_data)

        self.sunrise = current_data.get("sunrise", 0)
        self.sunset = current_data.get("sunset", 0)
        self.weather_id = source_data.get("weather", [{}])[0].get("id", 0)

      self.update_offsets(starpilot_toggles)

    def make_request():
      params = {
        "latitude": self.starpilot_planner.gps_position["latitude"],
        "longitude": self.starpilot_planner.gps_position["longitude"],
        "current": "weather_code",
        "hourly": "weather_code",
        "daily": "sunrise,sunset",
        "timezone": "UTC",
      }
      url = f"{OPEN_METEO_URL}?{urllib.parse.urlencode(params)}"

      for attempt in range(1, MAX_RETRIES + 1):
        try:
          req = urllib.request.Request(url, headers={"User-Agent": "starpilot/1.0"})
          with urllib.request.urlopen(req, timeout=10) as response:
            return _normalize_response(json.loads(response.read().decode()))
        except Exception:
          if attempt < MAX_RETRIES:
            time.sleep(5)
          continue
      return None

    future = self.executor.submit(make_request)
    future.add_done_callback(complete_request)
