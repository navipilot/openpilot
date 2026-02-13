#!/usr/bin/env python3
"""
Copyright (c) 2026, Rick Lan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sublicense,
for non-commercial purposes only, subject to the following conditions:

- The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
- Commercial use (e.g. use in a product, service, or activity intended to
  generate revenue) is prohibited without explicit written permission from
  the copyright holder.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Dashy State Aggregation Daemon

Aggregates all cereal topics needed by dashy UI into a single dashyState message.
This reduces CPU overhead in webrtcd by doing JSON serialization once here
instead of serializing 14+ topics separately.

All display formatting (units, distances, times) is done here so the frontend
can be a pure display layer with no conversion logic.

Publishes: dashyState (pre-serialized JSON at 15Hz)
"""
import json
import cereal.messaging as messaging
from openpilot.common.params import Params
from openpilot.common.realtime import Ratekeeper
from openpilot.common.swaglog import cloudlog
from opendbc.car.common.conversions import Conversions

# Main loop rate
LOOP_RATE = 15  # Hz

# Downsample factor for modelV2 arrays (33 points -> 17 points)
DOWNSAMPLE_FACTOR = 2

# Unit conversion constants
M_TO_KM = 0.001
M_TO_MI = 0.000621371
M_TO_FT = 3.28084

# Global state (refreshed periodically)
_is_metric = True
_params = None
_car_params_cache = None


def _ensure_params():
    """Ensure Params instance exists."""
    global _params
    if _params is None:
        _params = Params()
    return _params


def refresh_metric_preference():
    """Refresh metric preference from params (called periodically)."""
    global _is_metric
    try:
        _is_metric = _ensure_params().get_bool("IsMetric")
    except Exception:
        _is_metric = True


def get_car_params_from_params():
    """Read carParams from Params storage (for immediate availability at startup)."""
    global _car_params_cache
    if _car_params_cache is not None:
        return _car_params_cache
    try:
        from cereal import car
        cp_bytes = _ensure_params().get("CarParams")
        if cp_bytes:
            with car.CarParams.from_bytes(cp_bytes) as cp:
                _car_params_cache = {
                    'openpilotLongitudinalControl': bool(cp.openpilotLongitudinalControl),
                }
                return _car_params_cache
    except Exception:
        pass
    return {'openpilotLongitudinalControl': False}


def format_speed(speed_ms: float) -> str:
    """Format speed for display (m/s -> km/h or mph)."""
    if _is_metric:
        return f"{max(0, speed_ms * Conversions.MS_TO_KPH):.0f}"
    return f"{max(0, speed_ms * Conversions.MS_TO_MPH):.0f}"


def format_speed_value(speed_ms: float) -> float:
    """Convert speed to display units (m/s -> km/h or mph)."""
    if _is_metric:
        return max(0, speed_ms * Conversions.MS_TO_KPH)
    return max(0, speed_ms * Conversions.MS_TO_MPH)


def format_distance(meters: float) -> str:
    """Format distance for display."""
    if meters <= 0:
        return ""
    if _is_metric:
        if meters >= 1000:
            return f"{meters * M_TO_KM:.1f} km"
        return f"{meters:.0f} m"
    else:
        miles = meters * M_TO_MI
        if miles >= 0.1:
            return f"{miles:.1f} mi"
        return f"{meters * M_TO_FT:.0f} ft"


def format_time(seconds: float) -> str:
    """Format time duration for display."""
    if seconds <= 0:
        return ""
    minutes = int(seconds / 60)
    if minutes < 60:
        return f"{minutes} min"
    hours = minutes // 60
    mins = minutes % 60
    if mins == 0:
        return f"{hours} hr"
    return f"{hours} hr {mins} min"


def get_speed_unit() -> str:
    """Get current speed unit string."""
    return "km/h" if _is_metric else "mph"


def get_distance_unit() -> str:
    """Get current distance unit string."""
    return "km" if _is_metric else "mi"


SET_SPEED_NA = 255


def get_cruise_speed(v_cruise_cluster: float) -> int:
    """Get cruise speed value for display.

    Returns the set speed in display units (km/h or mph), or 255 if not set.
    """
    if not (0 < v_cruise_cluster < SET_SPEED_NA):
        return SET_SPEED_NA

    set_speed = v_cruise_cluster
    if not _is_metric:
        set_speed *= Conversions.KPH_TO_MPH

    return round(set_speed)


def downsample(arr):
    """Downsample list by factor."""
    if not arr:
        return []
    return list(arr[::DOWNSAMPLE_FACTOR])


def safe_get(obj, attr, default=None):
    """Safely get attribute from object."""
    try:
        return getattr(obj, attr, default)
    except Exception:
        return default


def extract_car_state(sm):
    """Extract carState fields used by dashy."""
    cs = sm['carState']
    v_ego = float(cs.vEgo)
    v_ego_cluster = float(cs.vEgoCluster)

    # Get set speed from controlsState.vCruiseDEPRECATED (fallback to carState.vCruiseCluster)
    v_cruise = float(cs.vCruiseCluster)
    if 'controlsState' in sm.updated and sm.updated['controlsState']:
        v_cruise = float(sm['controlsState'].vCruiseDEPRECATED)
    set_speed = get_cruise_speed(v_cruise)

    return {
        'vEgo': v_ego,
        'vEgoCluster': v_ego_cluster,
        'gearShifter': str(cs.gearShifter),
        'aEgo': float(cs.aEgo),
        'steeringAngleDeg': float(cs.steeringAngleDeg),
        'leftBlinker': bool(cs.leftBlinker),
        'rightBlinker': bool(cs.rightBlinker),
        'leftBlindspot': bool(cs.leftBlindspot),
        'rightBlindspot': bool(cs.rightBlindspot),
        # Pre-formatted display values
        'speedDisplay': format_speed(v_ego),
        'speedClusterDisplay': format_speed(v_ego_cluster) if v_ego_cluster > 0 else format_speed(v_ego),
        'setSpeed': set_speed,  # 255 = not set, otherwise display value
        'speedUnit': get_speed_unit(),
    }


def extract_selfdrive_state(sm):
    """Extract selfdriveState fields used by dashy."""
    ss = sm['selfdriveState']
    return {
        'enabled': bool(ss.enabled),
        'activeOverride': int(safe_get(ss, 'activeOverride', 0)),
        'experimentalMode': bool(ss.experimentalMode),
        'alertText1': str(ss.alertText1),
        'alertText2': str(ss.alertText2),
        'alertSize': str(ss.alertSize),
        'alertStatus': str(ss.alertStatus),
    }


def extract_device_state(sm):
    """Extract deviceState fields used by dashy."""
    ds = sm['deviceState']
    temp_c = float(safe_get(ds, 'maxTempC', 0))
    # Pre-format temperature for display
    if _is_metric:
        temp_display = f"{temp_c:.0f}°" if temp_c > 0 else "--"
    else:
        temp_f = temp_c * 9 / 5 + 32
        temp_display = f"{temp_f:.0f}°" if temp_c > 0 else "--"
    return {
        'cpuUsagePercent': list(ds.cpuUsagePercent) if ds.cpuUsagePercent else [],
        'memoryUsagePercent': int(ds.memoryUsagePercent),
        'maxTempC': temp_c,
        'deviceType': str(ds.deviceType),
        'tempDisplay': temp_display,
    }


def extract_lead(lead, sm):
    """Extract lead vehicle data."""
    d_rel = float(lead.dRel)
    v_rel = float(lead.vRel)
    y_rel = float(lead.yRel)
    has_lead = bool(lead.status)

    # Pre-format lead display values
    if has_lead:
        v_ego = float(sm['carState'].vEgo) if sm.valid['carState'] else 0
        lead_speed_ms = max(0, v_ego + v_rel)
        speed_display = format_speed(lead_speed_ms)
        distance_display = f"{d_rel:.1f}m" if _is_metric else f"{d_rel * M_TO_FT:.0f}ft"
    else:
        speed_display = "--"
        distance_display = "--"

    return {
        'status': has_lead,
        'dRel': d_rel,
        'yRel': y_rel,
        'vRel': v_rel,
        'speedDisplay': speed_display,
        'distanceDisplay': distance_display,
    }


def extract_radar_state(sm):
    """Extract radarState fields used by dashy."""
    rs = sm['radarState']
    return {
        'leadOne': extract_lead(rs.leadOne, sm),
        'leadTwo': extract_lead(rs.leadTwo, sm),
    }


def extract_live_tracks(sm):
    """Extract liveTracks radar points for bird's eye view.

    Filters out tracks that are already shown as leadOne or leadTwo.
    Uses radarTrackId matching: when radarState matches a liveTrack to a lead,
    radarTrackId changes from -1 (vision-only) to the track's ID.
    """
    try:
        lt = sm['liveTracks']
        points = []

        # Get lead vehicle radar track IDs to filter them out
        # radarTrackId = -1 means vision-only (no radar match)
        # radarTrackId >= 0 means matched to a radar track
        lead_track_ids = set()
        if sm.valid.get('radarState', False):
            rs = sm['radarState']
            if rs.leadOne.status and rs.leadOne.radarTrackId >= 0:
                lead_track_ids.add(rs.leadOne.radarTrackId)
            if rs.leadTwo.status and rs.leadTwo.radarTrackId >= 0:
                lead_track_ids.add(rs.leadTwo.radarTrackId)

        if hasattr(lt, 'points'):
            for pt in lt.points:
                # Skip if this track is already shown as a lead vehicle
                if pt.trackId in lead_track_ids:
                    continue

                points.append({
                    'd': float(pt.dRel),
                    'y': float(pt.yRel),
                    'v': float(pt.vRel),
                    'm': bool(pt.measured),
                })
        return {'points': points}
    except Exception as e:
        cloudlog.warning(f"extract_live_tracks error: {e}")
        return {'points': []}


def extract_live_gps(sm):
    """Extract liveGPS fields used by dashy."""
    gps = sm['liveGPS']
    # Skip if no coordinates yet
    if gps.latitude == 0 and gps.longitude == 0:
        return None
    return {
        'latitude': float(gps.latitude),
        'longitude': float(gps.longitude),
        'bearingDeg': float(gps.bearingDeg),
        'speed': float(gps.speed),
        'gpsOK': bool(gps.gpsOK),
        'horizontalAccuracy': float(gps.horizontalAccuracy),
        'status': str(gps.status),
    }


def extract_nav_instruction(sm):
    """Extract navInstruction fields used by dashy."""
    nav = sm['navInstruction']
    maneuver_dist = float(safe_get(nav, 'maneuverDistance', 0))
    dist_remaining = float(safe_get(nav, 'distanceRemaining', 0))
    time_remaining = float(safe_get(nav, 'timeRemaining', 0))
    speed_limit_ms = float(safe_get(nav, 'speedLimit', 0))

    return {
        'valid': sm.valid['navInstruction'],
        'maneuverDistance': maneuver_dist,
        'maneuverPrimaryText': str(safe_get(nav, 'maneuverPrimaryText', '')),
        'maneuverSecondaryText': str(safe_get(nav, 'maneuverSecondaryText', '')),
        'maneuverType': str(safe_get(nav, 'maneuverType', '')),
        'maneuverModifier': str(safe_get(nav, 'maneuverModifier', '')),
        'distanceRemaining': dist_remaining,
        'timeRemaining': time_remaining,
        'timeRemainingTypical': float(safe_get(nav, 'timeRemainingTypical', 0)),
        'speedLimit': speed_limit_ms,
        'speedLimitSign': str(safe_get(nav, 'speedLimitSign', '')),
        # Pre-formatted display values
        'maneuverDistanceDisplay': format_distance(maneuver_dist),
        'distanceRemainingDisplay': format_distance(dist_remaining),
        'timeRemainingDisplay': format_time(time_remaining),
        'speedLimitDisplay': format_speed(speed_limit_ms) if speed_limit_ms > 0 else '',
    }


def extract_nav_instruction_ext(sm):
    """Extract navInstructionExt fields used by dashy (extended nav data)."""
    nav_ext = sm['navInstructionExt']
    # Extract allManeuvers list (with name field and formatted distance)
    all_maneuvers = []
    if hasattr(nav_ext, 'allManeuvers'):
        for m in nav_ext.allManeuvers:
            dist = float(safe_get(m, 'distance', 0))
            all_maneuvers.append({
                'distance': dist,
                'distanceDisplay': format_distance(dist),
                'type': str(safe_get(m, 'type', '')),
                'modifier': str(safe_get(m, 'modifier', '')),
                'name': str(safe_get(m, 'name', '')),
            })
    return {
        'turnAngle': float(safe_get(nav_ext, 'turnAngle', 0)),
        'turnCurvature': float(safe_get(nav_ext, 'turnCurvature', 0)),
        'allManeuvers': all_maneuvers,
    }


def extract_nav_route(sm):
    """Extract navRoute coordinates used by dashy."""
    route = sm['navRoute']
    coords = []
    if hasattr(route, 'coordinates'):
        for c in route.coordinates:
            coords.append([float(c.longitude), float(c.latitude)])
    return {
        'coordinates': coords,
    }


def extract_model_v2(sm):
    """Extract modelV2 fields used by dashy (downsampled)."""
    model = sm['modelV2']

    # Position
    pos = model.position
    position = {
        'x': downsample(list(pos.x)),
        'y': downsample(list(pos.y)),
        'z': downsample(list(pos.z)),
    }

    # Lane lines (4 lines)
    lane_lines = []
    for line in model.laneLines:
        lane_lines.append({
            'x': downsample(list(line.x)),
            'y': downsample(list(line.y)),
            'z': downsample(list(line.z)),
        })

    # Road edges (2 edges)
    road_edges = []
    for edge in model.roadEdges:
        road_edges.append({
            'x': downsample(list(edge.x)),
            'y': downsample(list(edge.y)),
            'z': downsample(list(edge.z)),
        })

    return {
        'position': position,
        'laneLines': lane_lines,
        'laneLineProbs': list(model.laneLineProbs) if hasattr(model, 'laneLineProbs') else [0, 0, 0, 0],
        'roadEdges': road_edges,
        'roadEdgeStds': list(model.roadEdgeStds) if hasattr(model, 'roadEdgeStds') else [1, 1],
    }


def extract_live_calibration(sm):
    """Extract liveCalibration fields used by dashy."""
    cal = sm['liveCalibration']
    return {
        'rpyCalib': list(cal.rpyCalib) if hasattr(cal, 'rpyCalib') and cal.rpyCalib else [],
        'calStatus': str(cal.calStatus) if hasattr(cal, 'calStatus') else 'uncalibrated',
        'height': list(cal.height) if hasattr(cal, 'height') else [],
    }


def extract_longitudinal_plan(sm):
    """Extract longitudinalPlan fields used by dashy."""
    lp = sm['longitudinalPlan']
    return {
        'allowThrottle': bool(safe_get(lp, 'allowThrottle', True)),
    }


def extract_controls_state_ext(sm):
    """Extract controlsStateExt fields used by dashy."""
    cse = sm['controlsStateExt']
    return {
        'alkaActive': bool(safe_get(cse, 'alkaActive', False)),
    }


def extract_car_params(sm):
    """Extract carParams fields used by dashy."""
    cp = sm['carParams']
    return {
        'openpilotLongitudinalControl': bool(safe_get(cp, 'openpilotLongitudinalControl', False)),
    }


# =============================================================================
# TOPIC CONFIGURATION
# =============================================================================
# Single source of truth for all subscribed topics.
# Comment out a line to disable that topic entirely.
#
# Fields:
#   extractor: function(sm) -> dict, extracts data from message
#   rate: 'fast' = every frame when updated
#         number = slow poll divider (e.g., LOOP_RATE = 1Hz)
#         'valid' = just track valid state, no extraction
#         'subscribe' = subscribed but extracted within other extractors
#   default: initial cache value (None if not specified)
# =============================================================================
TOPICS = {
    # Fast topics - extract every frame when updated
    'carState':         {'extractor': extract_car_state,         'rate': 'fast'},
    'selfdriveState':   {'extractor': extract_selfdrive_state,   'rate': 'fast'},
    'radarState':       {'extractor': extract_radar_state,       'rate': 'fast'},
    'liveTracks':       {'extractor': extract_live_tracks,       'rate': 'fast'},
    'modelV2':          {'extractor': extract_model_v2,          'rate': 'fast'},
    'longitudinalPlan': {'extractor': extract_longitudinal_plan, 'rate': 'fast'},
    'liveGPS':          {'extractor': extract_live_gps,          'rate': 'fast'},

    # Slow topics - poll at fixed intervals
    'deviceState':      {'extractor': extract_device_state,      'rate': LOOP_RATE // 2},
    'liveCalibration':  {'extractor': extract_live_calibration,  'rate': LOOP_RATE},
    'navInstruction':   {'extractor': extract_nav_instruction,   'rate': LOOP_RATE},
    'navInstructionExt': {'extractor': extract_nav_instruction_ext, 'rate': LOOP_RATE},
    'navRoute':         {'extractor': extract_nav_route,         'rate': LOOP_RATE},
    'carParams':        {'extractor': extract_car_params,        'rate': LOOP_RATE * 2},

    # Valid-only topics - just track valid state
    'roadCameraState':  {'rate': 'valid', 'default': False},

    # Subscribe-only topics - subscribed but extracted within other extractors
    'controlsState':    {'rate': 'subscribe'},

    # Optional/dragonpilot-specific topics - comment out to disable
    'controlsStateExt': {'extractor': extract_controls_state_ext, 'rate': 'fast', 'default': {'alkaActive': False}},
}


def main():
    cloudlog.info("dashyd: starting")

    # Initialize metric preference
    refresh_metric_preference()

    # Derive services list from TOPICS config
    services = list(TOPICS.keys())
    sm = messaging.SubMaster(services)
    pm = messaging.PubMaster(['dashyState'])
    rk = Ratekeeper(LOOP_RATE)

    # Initialize cache from TOPICS defaults (exclude subscribe-only topics)
    cache = {t: cfg.get('default') for t, cfg in TOPICS.items() if cfg.get('rate') != 'subscribe'}
    cache['carParams'] = get_car_params_from_params()  # special: init from Params

    # Build topic lists from TOPICS config
    fast_topics = {t: cfg['extractor'] for t, cfg in TOPICS.items() if cfg.get('rate') == 'fast'}
    slow_topics = {t: (cfg['extractor'], cfg['rate']) for t, cfg in TOPICS.items()
                   if isinstance(cfg.get('rate'), int)}
    valid_topics = [t for t, cfg in TOPICS.items() if cfg.get('rate') == 'valid']

    cache_dirty = True
    frame_count = 0

    while True:
        sm.update(0)
        frame_count += 1

        # Refresh metric preference every ~2 seconds
        if frame_count % (LOOP_RATE * 2) == 0:
            refresh_metric_preference()
            cache_dirty = True  # Force re-format with new units

        # Fast topics - extract when updated
        for topic, extractor in fast_topics.items():
            if sm.updated[topic]:
                cache[topic] = extractor(sm)
                cache_dirty = True

        # Slow topics - extract at fixed intervals (ignore sm.updated)
        for topic, (extractor, divider) in slow_topics.items():
            if frame_count % divider == 0:
                cache[topic] = extractor(sm)
                cache_dirty = True

        # Valid-only topics - just track valid state
        for topic in valid_topics:
            if sm.updated[topic]:
                new_val = sm.valid[topic]
                if cache[topic] != new_val:
                    cache[topic] = new_val
                    cache_dirty = True

        # Only serialize and publish if something changed
        if cache_dirty:
            # Only publish when critical data exists (nav data can be null)
            critical_ready = (
                cache.get('carState') is not None and
                cache.get('modelV2') is not None and
                cache.get('selfdriveState') is not None
            )

            if critical_ready:
                state = {
                    'ts': sm.logMonoTime['carState'],
                    'display': {
                        'isMetric': _is_metric,
                        'speedUnit': get_speed_unit(),
                        'distanceUnit': get_distance_unit(),
                    },
                    **cache,  # include all cached topics
                }
                msg = messaging.new_message('dashyState')
                msg.dashyState.json = json.dumps(state).encode()
                pm.send('dashyState', msg)

            cache_dirty = False

        rk.keep_time()


if __name__ == "__main__":
    main()
