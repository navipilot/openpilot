#!/usr/bin/env python3
import json
import math
import requests
import time

from collections import OrderedDict, deque
from datetime import datetime, timedelta, timezone

from cereal import log, messaging

from openpilot.common.constants import CV
from openpilot.common.gps import get_gps_location_service
from openpilot.common.params import Params

from openpilot.starpilot.common.starpilot_utilities import calculate_distance_to_point, calculate_lane_width, is_url_pingable

NetworkType = log.DeviceState.NetworkType

BOUNDING_BOX_RADIUS_DEGREE = 0.1
MAX_ENTRIES = 1_000_000
MAX_PENDING_ADDITIONS = 2_000
MAX_OVERPASS_DATA_BYTES = 1_073_741_824
MAX_OVERPASS_REQUESTS = 10_000
METERS_PER_DEG_LAT = 111_320
PENDING_FLUSH_BATCH_SIZE = 1_000
PENDING_FLUSH_INTERVAL_S = 60.0
VETTING_INTERVAL_DAYS = 7
ACTIVE_SEGMENT_BUFFER_METERS = 25
MAX_BEARING_DELTA = 45
MIN_LATERAL_MATCH_BUFFER = 12

OVERPASS_API_URL = "https://overpass-api.de/api/interpreter"
OVERPASS_STATUS_URL = "https://overpass-api.de/api/status"

class MapSpeedLogger:
  def __init__(self):
    self.params = Params(return_defaults=True)
    self.params_memory = Params(memory=True)

    self.cached_box = None
    self.previous_coordinates = None

    self.cached_segments = {}

    self.dataset_additions = deque(maxlen=MAX_PENDING_ADDITIONS)
    self.filtered_dataset = []
    self.last_dataset_flush = time.monotonic()

    self.overpass_requests = self.params.get("OverpassRequests")
    self.overpass_requests.setdefault("day", datetime.now(timezone.utc).day)
    self.overpass_requests.setdefault("total_bytes", 0)
    self.overpass_requests.setdefault("total_requests", 0)

    self.session = requests.Session()
    self.session.headers.update({"Accept-Language": "en"})
    self.session.headers.update({"User-Agent": "starpilot-map-speed-logger/1.0 (https://github.com/FrogAi/StarPilot)"})

    self.gps_location_service = get_gps_location_service(self.params)

    self.sm = messaging.SubMaster(["deviceState", "starpilotCarState", "starpilotPlan", self.gps_location_service, "mapdOut", "modelV2"])
    self.refresh_filtered_dataset()

  @property
  def can_make_overpass_request(self):
    return self.overpass_requests["total_bytes"] < MAX_OVERPASS_DATA_BYTES and self.overpass_requests["total_requests"] < MAX_OVERPASS_REQUESTS

  @property
  def should_stop_processing(self):
    return self.sm["deviceState"].started or not self.params_memory.get_bool("UpdateSpeedLimits")

  @staticmethod
  def cleanup_dataset(dataset):
    cleaned_data = OrderedDict()

    for item in dataset:
      if "last_vetted" in item:
        required = {"incorrect_limit", "last_vetted", "segment_id", "source", "speed_limit", "start_coordinates"}
      else:
        required = {"bearing", "end_coordinates", "incorrect_limit", "road_name", "road_width", "source", "speed_limit", "start_coordinates"}

      if not required.issubset(item.keys()):
        continue

      entry_copy = item.copy()
      entry_copy.pop("last_vetted", None)
      if "last_vetted" in item:
        entry_copy = {key: entry_copy[key] for key in required if key != "last_vetted"}

      key = json.dumps(entry_copy, sort_keys=True)
      cleaned_data[key] = item

    return deque(cleaned_data.values(), maxlen=MAX_ENTRIES)

  @staticmethod
  def meters_to_deg_lat(meters):
    return meters / METERS_PER_DEG_LAT

  @staticmethod
  def meters_to_deg_lon(meters, latitude):
    return meters / (METERS_PER_DEG_LAT * math.cos(latitude * CV.DEG_TO_RAD))

  @staticmethod
  def bearing_delta(bearing1, bearing2):
    return abs((bearing1 - bearing2 + 180) % 360 - 180)

  @staticmethod
  def has_live_match_fields(entry):
    required = {"bearing", "end_coordinates", "road_name", "road_width", "speed_limit", "start_coordinates"}
    return required.issubset(entry.keys())

  @staticmethod
  def latlon_to_local_meters(origin_latitude, origin_longitude, latitude, longitude):
    average_latitude = ((origin_latitude + latitude) / 2) * CV.DEG_TO_RAD
    x = (longitude - origin_longitude) * METERS_PER_DEG_LAT * math.cos(average_latitude)
    y = (latitude - origin_latitude) * METERS_PER_DEG_LAT
    return x, y

  def clear_live_speed_limits(self):
    self.params_memory.remove("MapSpeedLimit")
    self.params_memory.remove("NextMapSpeedLimit")

  def refresh_filtered_dataset(self, filtered_dataset=None):
    if filtered_dataset is None:
      filtered_dataset = self.params.get("SpeedLimitsFiltered") or []

    if filtered_dataset and not any(self.has_live_match_fields(entry) for entry in filtered_dataset):
      dataset = self.cleanup_dataset(self.params.get("SpeedLimits"))
      filtered_dataset = self.enrich_filtered_dataset(dataset, filtered_dataset)

    self.filtered_dataset = [entry for entry in filtered_dataset if self.has_live_match_fields(entry)]

  @staticmethod
  def get_entry_identity(entry):
    start_coordinates = entry.get("start_coordinates")
    if not isinstance(start_coordinates, dict):
      return None

    try:
      return (
        bool(entry.get("incorrect_limit")),
        round(float(start_coordinates["latitude"]), 6),
        round(float(start_coordinates["longitude"]), 6),
        str(entry.get("source", "")),
        round(float(entry.get("speed_limit", 0)), 3),
      )
    except (KeyError, TypeError, ValueError):
      return None

  def enrich_filtered_dataset(self, dataset, filtered_dataset):
    dataset_by_identity = {}
    for entry in dataset:
      if not self.has_live_match_fields(entry):
        continue

      identity = self.get_entry_identity(entry)
      if identity is None or identity in dataset_by_identity:
        continue

      dataset_by_identity[identity] = entry

    if not dataset_by_identity:
      return filtered_dataset

    filtered_entries = list(filtered_dataset)
    updated = False
    for index, entry in enumerate(filtered_entries):
      if self.has_live_match_fields(entry):
        continue

      identity = self.get_entry_identity(entry)
      matching_entry = dataset_by_identity.get(identity)
      if matching_entry is None:
        continue

      filtered_entries[index] = {
        **entry,
        "bearing": matching_entry["bearing"],
        "end_coordinates": matching_entry["end_coordinates"],
        "road_name": matching_entry["road_name"],
        "road_width": matching_entry["road_width"],
      }
      updated = True

    if not updated:
      return filtered_dataset

    return self.cleanup_dataset(filtered_entries)

  def get_speed_limit_source(self):
    vision_speed_limit = self.params_memory.get_float("VisionSpeedLimit") if self.params.get_bool("VisionSpeedLimitDetection") else 0
    sources = [
      (vision_speed_limit, "Vision"),
      (self.sm["starpilotPlan"].slcMapboxSpeedLimit, "Mapbox"),
      (self.sm["starpilotCarState"].dashboardSpeedLimit, "Dashboard")
    ]
    for speed_limit, source in sources:
      if speed_limit > 0:
        return speed_limit, source
    return None

  def get_live_match_candidates(self, road_name, current_bearing):
    return [
      entry for entry in self.filtered_dataset
      if self.has_live_match_fields(entry)
      and entry["road_name"] == road_name
      and entry["speed_limit"] > 0
      and self.bearing_delta(entry["bearing"], current_bearing) <= MAX_BEARING_DELTA
    ]

  def is_in_cached_box(self, latitude, longitude):
    if self.cached_box is None:
      return False
    return self.cached_box["min_latitude"] <= latitude <= self.cached_box["max_latitude"] and \
           self.cached_box["min_longitude"] <= longitude <= self.cached_box["max_longitude"]

  def record_overpass_request(self, content_bytes):
    self.overpass_requests["total_bytes"] += content_bytes
    self.overpass_requests["total_requests"] += 1

  def reset_daily_api_limits(self):
    current_day = datetime.now(timezone.utc).day
    if current_day != self.overpass_requests["day"]:
      self.overpass_requests.update({
        "day": current_day,
        "total_requests": 0,
        "total_bytes": 0,
      })

  def update_params(self, dataset, filtered_dataset):
    self.params.put("OverpassRequests", self.overpass_requests)
    self.params.put("SpeedLimits", list(dataset))
    self.params.put("SpeedLimitsFiltered", list(filtered_dataset))

  def flush_pending_dataset_additions(self, force=False):
    if not self.dataset_additions:
      if force:
        self.last_dataset_flush = time.monotonic()
      return

    now = time.monotonic()
    should_flush = force
    should_flush |= len(self.dataset_additions) >= PENDING_FLUSH_BATCH_SIZE
    should_flush |= (now - self.last_dataset_flush) >= PENDING_FLUSH_INTERVAL_S
    if not should_flush:
      return

    existing_dataset = self.params.get("SpeedLimits") or []
    existing_dataset.extend(self.dataset_additions)

    new_dataset = self.cleanup_dataset(existing_dataset)
    self.params.put("SpeedLimits", list(new_dataset))

    self.dataset_additions.clear()
    self.last_dataset_flush = now

  def find_current_speed_limit_entry(self, latitude, longitude, road_name, current_bearing):
    best_match = None
    best_score = None

    for entry in self.get_live_match_candidates(road_name, current_bearing):
      start_latitude = entry["start_coordinates"]["latitude"]
      start_longitude = entry["start_coordinates"]["longitude"]
      end_latitude = entry["end_coordinates"]["latitude"]
      end_longitude = entry["end_coordinates"]["longitude"]

      segment_x, segment_y = self.latlon_to_local_meters(start_latitude, start_longitude, end_latitude, end_longitude)
      segment_length_sq = segment_x ** 2 + segment_y ** 2
      if segment_length_sq < 1:
        continue

      point_x, point_y = self.latlon_to_local_meters(start_latitude, start_longitude, latitude, longitude)
      projection_ratio = (point_x * segment_x + point_y * segment_y) / segment_length_sq
      segment_length = math.sqrt(segment_length_sq)
      along_track = projection_ratio * segment_length

      clamped_ratio = min(max(projection_ratio, 0.0), 1.0)
      closest_x = segment_x * clamped_ratio
      closest_y = segment_y * clamped_ratio
      cross_track = math.hypot(point_x - closest_x, point_y - closest_y)

      longitudinal_buffer = max(entry["speed_limit"] * 2, ACTIVE_SEGMENT_BUFFER_METERS)
      lateral_buffer = max(entry["road_width"] * 2, MIN_LATERAL_MATCH_BUFFER)
      if -longitudinal_buffer <= along_track <= segment_length + longitudinal_buffer and cross_track <= lateral_buffer:
        score = (cross_track, abs(projection_ratio - 0.5))
        if best_score is None or score < best_score:
          best_match = entry
          best_score = score

    return best_match

  def find_next_speed_limit_entry(self, latitude, longitude, road_name, current_bearing, current_entry=None):
    best_match = None
    best_score = None

    heading_rad = current_bearing * CV.DEG_TO_RAD
    heading_x = math.sin(heading_rad)
    heading_y = math.cos(heading_rad)

    current_segment_id = current_entry.get("segment_id") if current_entry else None

    for entry in self.get_live_match_candidates(road_name, current_bearing):
      if current_segment_id is not None and entry.get("segment_id") == current_segment_id:
        continue

      start_latitude = entry["start_coordinates"]["latitude"]
      start_longitude = entry["start_coordinates"]["longitude"]

      distance_to_start = calculate_distance_to_point(latitude, longitude, start_latitude, start_longitude)
      if distance_to_start < 1:
        continue

      delta_x, delta_y = self.latlon_to_local_meters(latitude, longitude, start_latitude, start_longitude)
      forward_distance = delta_x * heading_x + delta_y * heading_y
      if forward_distance <= 0:
        continue

      score = (distance_to_start, self.bearing_delta(entry["bearing"], current_bearing))
      if best_score is None or score < best_score:
        best_match = entry
        best_score = score

    return best_match, best_score[0] if best_score is not None else 0

  def wait_for_api(self):
    while not is_url_pingable(OVERPASS_STATUS_URL):
      print("Waiting for Overpass API to be available...")
      self.sm.update()

      if self.should_stop_processing:
        return False

      time.sleep(5)
    return True

  def fetch_from_overpass(self, latitude, longitude):
    min_lat = latitude - BOUNDING_BOX_RADIUS_DEGREE
    max_lat = latitude + BOUNDING_BOX_RADIUS_DEGREE
    min_lon = longitude - BOUNDING_BOX_RADIUS_DEGREE
    max_lon = longitude + BOUNDING_BOX_RADIUS_DEGREE

    self.cached_box = {"min_latitude": min_lat, "max_latitude": max_lat, "min_longitude": min_lon, "max_longitude": max_lon}
    self.cached_segments.clear()

    query = (
      f"[out:json][timeout:90][maxsize:{MAX_OVERPASS_DATA_BYTES // 10}];"
      f"way({min_lat:.5f},{min_lon:.5f},{max_lat:.5f},{max_lon:.5f})"
      "[highway~'^(motorway|motorway_link|primary|primary_link|residential|"
      "secondary|secondary_link|tertiary|tertiary_link|trunk|trunk_link)$'];"
      "out geom qt;"
    )

    try:
      response = self.session.post(OVERPASS_API_URL, data=query, timeout=90)
      self.record_overpass_request(len(response.content))

      if response.status_code == 429:
        retry_after = int(response.headers.get("Retry-After", 10))
        print(f"Overpass API rate limit hit. Retrying in {retry_after} seconds.")

        time.sleep(retry_after)

        response = self.session.post(OVERPASS_API_URL, data=query, timeout=90)
        self.record_overpass_request(len(response.content))

      response.raise_for_status()
      return response.json().get("elements", [])
    except requests.exceptions.RequestException as exception:
      print(f"Overpass API request failed: {exception}")
      self.cached_segments.clear()
      return []

  def filter_segments_for_entry(self, entry):
    bearing_rad = entry["bearing"] * CV.DEG_TO_RAD
    start_lat, start_lon = entry["start_coordinates"]["latitude"], entry["start_coordinates"]["longitude"]
    end_lat, end_lon = entry["end_coordinates"]["latitude"], entry["end_coordinates"]["longitude"]
    mid_lat = (start_lat + end_lat) / 2

    forward_buffer_lat = self.meters_to_deg_lat(entry["speed_limit"])
    forward_buffer_lon = self.meters_to_deg_lon(entry["speed_limit"], mid_lat)
    side_buffer_lat = self.meters_to_deg_lat(entry["road_width"])
    side_buffer_lon = self.meters_to_deg_lon(entry["road_width"], mid_lat)

    delta_lat_fwd = forward_buffer_lat * math.cos(bearing_rad)
    delta_lon_fwd = forward_buffer_lon * math.sin(bearing_rad)
    delta_lat_side = side_buffer_lat * math.cos(bearing_rad + math.pi / 2)
    delta_lon_side = side_buffer_lon * math.sin(bearing_rad + math.pi / 2)

    min_lat = min(start_lat, end_lat) - abs(delta_lat_fwd) - abs(delta_lat_side)
    max_lat = max(start_lat, end_lat) + abs(delta_lat_fwd) + abs(delta_lat_side)
    min_lon = min(start_lon, end_lon) - abs(delta_lon_fwd) - abs(delta_lon_side)
    max_lon = max(start_lon, end_lon) + abs(delta_lon_fwd) + abs(delta_lon_side)

    relevant_segments = []
    for segment in self.cached_segments.values():
      if not segment or "nodes" not in segment:
        continue

      latitudes = [node[0] for node in segment["nodes"]]
      longitudes = [node[1] for node in segment["nodes"]]

      if not (max(latitudes) < min_lat or min(latitudes) > max_lat or max(longitudes) < min_lon or min(longitudes) > max_lon):
        relevant_segments.append(segment)

    return relevant_segments

  def log_speed_limit(self):
    if not self.sm.updated[self.gps_location_service]:
      return

    gps_location = self.sm[self.gps_location_service]
    current_latitude = gps_location.latitude
    current_longitude = gps_location.longitude

    if current_latitude == 0 and current_longitude == 0:
      return

    if self.previous_coordinates is None:
      self.previous_coordinates = {"latitude": current_latitude, "longitude": current_longitude}
      return

    current_speed_source = self.get_speed_limit_source()
    valid_sources = {source[0] for source in [current_speed_source] if source and source[0] > 0}

    map_speed = self.params_memory.get_float("MapSpeedLimit")
    is_incorrect_limit = bool(map_speed > 0 and valid_sources and all(abs(map_speed - source) > 1 for source in valid_sources))

    if map_speed > 0 and not is_incorrect_limit:
      self.previous_coordinates = None
      return

    road_name = self.sm["mapdOut"].roadName

    if not road_name or not current_speed_source:
      return

    distance = calculate_distance_to_point(
      self.previous_coordinates["latitude"],
      self.previous_coordinates["longitude"],
      current_latitude,
      current_longitude
    )
    if distance < 1:
      return

    speed_limit, source = current_speed_source
    self.dataset_additions.append({
      "bearing": gps_location.bearingDeg,
      "end_coordinates": {"latitude": current_latitude, "longitude": current_longitude},
      "incorrect_limit": is_incorrect_limit,
      "road_name": road_name,
      "road_width": calculate_lane_width(self.sm["modelV2"].laneLines[1], self.sm["modelV2"].laneLines[2]),
      "source": source,
      "speed_limit": speed_limit,
      "start_coordinates": self.previous_coordinates,
    })
    self.flush_pending_dataset_additions()

    self.previous_coordinates = {"latitude": current_latitude, "longitude": current_longitude}

  def update_live_speed_limits(self):
    gps_location = self.sm[self.gps_location_service]
    current_latitude = gps_location.latitude
    current_longitude = gps_location.longitude
    current_bearing = gps_location.bearingDeg
    road_name = self.sm["mapdOut"].roadName

    if (current_latitude == 0 and current_longitude == 0) or not road_name or not self.filtered_dataset:
      self.clear_live_speed_limits()
      return

    current_entry = self.find_current_speed_limit_entry(current_latitude, current_longitude, road_name, current_bearing)
    next_entry, next_distance = self.find_next_speed_limit_entry(
      current_latitude,
      current_longitude,
      road_name,
      current_bearing,
      current_entry=current_entry,
    )

    if current_entry:
      self.params_memory.put_float("MapSpeedLimit", current_entry["speed_limit"])
    else:
      self.params_memory.remove("MapSpeedLimit")

    if next_entry:
      self.params_memory.put("NextMapSpeedLimit", {
        "distance": next_distance,
        "latitude": next_entry["start_coordinates"]["latitude"],
        "longitude": next_entry["start_coordinates"]["longitude"],
        "speedlimit": next_entry["speed_limit"],
      })
    else:
      self.params_memory.remove("NextMapSpeedLimit")

  def process_new_entries(self, dataset, filtered_dataset):
    existing_segment_ids = {entry["segment_id"] for entry in filtered_dataset if "segment_id" in entry}
    entries_to_process = list(dataset)
    total_entries = len(entries_to_process)

    for i, entry in enumerate(entries_to_process):
      self.sm.update()

      if self.should_stop_processing:
        break

      if not self.can_make_overpass_request:
        self.params_memory.put("UpdateSpeedLimitsStatus", "Hit API limit...")
        time.sleep(5)
        break

      self.params_memory.put("UpdateSpeedLimitsStatus", f"Processing: {i + 1} / {total_entries}")

      start_coords = entry["start_coordinates"]
      self.update_cached_segments(start_coords["latitude"], start_coords["longitude"])
      segments = self.filter_segments_for_entry(entry)

      dataset.remove(entry)

      for segment in segments:
        segment_id = segment["segment_id"]
        if segment_id in existing_segment_ids:
          continue
        if segment["maxspeed"] and not entry.get("incorrect_limit"):
          continue
        if segment["road_name"] != entry.get("road_name"):
          continue

        filtered_dataset.append({
          "bearing": entry["bearing"],
          "end_coordinates": entry["end_coordinates"],
          "incorrect_limit": entry.get("incorrect_limit"),
          "last_vetted": datetime.now(timezone.utc).isoformat(),
          "road_name": entry["road_name"],
          "road_width": entry["road_width"],
          "segment_id": segment_id,
          "source": entry["source"],
          "speed_limit": entry["speed_limit"],
          "start_coordinates": entry["start_coordinates"],
        })
        existing_segment_ids.add(segment_id)

      if i % 100 == 0:
        self.update_params(dataset, filtered_dataset)

  def process_speed_limits(self):
    self.reset_daily_api_limits()

    if not self.wait_for_api():
      return

    self.cached_box, self.cached_segments = None, {}

    dataset = self.cleanup_dataset(self.params.get("SpeedLimits"))
    filtered_dataset = self.cleanup_dataset(self.params.get("SpeedLimitsFiltered"))
    filtered_dataset = self.enrich_filtered_dataset(dataset, filtered_dataset)

    filtered_dataset = self.vet_entries(filtered_dataset)
    self.update_params(dataset, filtered_dataset)

    if dataset and not self.should_stop_processing:
      self.cached_box, self.cached_segments = None, {}
      self.params_memory.put("UpdateSpeedLimitsStatus", "Calculating...")
      self.process_new_entries(dataset, filtered_dataset)

    self.update_params(dataset, filtered_dataset)
    self.refresh_filtered_dataset(filtered_dataset)
    self.params_memory.put("UpdateSpeedLimitsStatus", "Completed!")

  def update_cached_segments(self, latitude, longitude, vetting=False):
    if not self.is_in_cached_box(latitude, longitude):
      elements = self.fetch_from_overpass(latitude, longitude)
      for way in elements:
        if way.get("type") == "way" and (segment_id := way.get("id")):
          tags = way.get("tags", {})
          if vetting:
            self.cached_segments[segment_id] = tags.get("maxspeed")
          elif "geometry" in way and (nodes := way["geometry"]):
            self.cached_segments[segment_id] = {
              "maxspeed": tags.get("maxspeed"),
              "nodes": [(node["lat"], node["lon"]) for node in nodes],
              "road_name": tags.get("name"),
              "segment_id": segment_id,
            }

  def vet_entries(self, filtered_dataset):
    dataset_list = list(filtered_dataset)
    total_to_vet = len(filtered_dataset)
    vetted_entries = deque(maxlen=MAX_ENTRIES)

    for i, entry in enumerate(dataset_list):
      self.sm.update()

      if self.should_stop_processing:
        vetted_entries.extend(dataset_list[i:])
        break

      if not self.can_make_overpass_request:
        self.params_memory.put("UpdateSpeedLimitsStatus", "Hit API limit...")
        time.sleep(5)
        vetted_entries.extend(dataset_list[i:])
        break

      self.params_memory.put("UpdateSpeedLimitsStatus", f"Vetting: {i + 1} / {total_to_vet}")

      last_vetted_time = datetime.fromisoformat(entry["last_vetted"])
      if datetime.now(timezone.utc) - last_vetted_time < timedelta(days=VETTING_INTERVAL_DAYS):
        vetted_entries.append(entry)
        continue

      start_coords = entry["start_coordinates"]
      self.update_cached_segments(start_coords["latitude"], start_coords["longitude"], vetting=True)

      current_maxspeed = self.cached_segments.get(entry["segment_id"])
      if current_maxspeed is None or (entry.get("incorrect_limit") and current_maxspeed != entry.get("speed_limit")):
        entry["last_vetted"] = datetime.now(timezone.utc).isoformat()
        vetted_entries.append(entry)

    return self.cleanup_dataset(list(vetted_entries))

def main():
  logger = MapSpeedLogger()

  previously_started = False

  while True:
    logger.sm.update()

    if logger.sm["deviceState"].started:
      logger.log_speed_limit()
      logger.update_live_speed_limits()

      previously_started = True
    elif previously_started:
      logger.flush_pending_dataset_additions(force=True)

      if logger.sm["deviceState"].networkType in (NetworkType.ethernet, NetworkType.wifi):
        logger.params_memory.put_bool("UpdateSpeedLimits", True)

      logger.clear_live_speed_limits()

      previously_started = False
    elif logger.params_memory.get_bool("UpdateSpeedLimits"):
      logger.process_speed_limits()

      logger.params_memory.remove("UpdateSpeedLimits")
    else:
      logger.clear_live_speed_limits()
      time.sleep(5)

if __name__ == "__main__":
  main()
