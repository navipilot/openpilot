#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path

DEBUG_BASE_DIR = Path("/data/media/0/vision_speed_limit_debug")


def load_events(session_path: Path):
  events_path = session_path / "events.jsonl"
  if not events_path.is_file():
    raise FileNotFoundError(f"Missing events file: {events_path}")

  events = []
  with events_path.open("r", encoding="utf-8") as log_file:
    for line in log_file:
      line = line.strip()
      if not line:
        continue
      events.append(json.loads(line))
  return events


def resolve_session(session_arg: str | None) -> Path | None:
  if session_arg:
    session_path = Path(session_arg)
    if session_path.is_dir():
      return session_path
    candidate = DEBUG_BASE_DIR / session_arg
    if candidate.is_dir():
      return candidate
    raise FileNotFoundError(f"Session not found: {session_arg}")

  if not DEBUG_BASE_DIR.is_dir():
    return None

  sessions = sorted((path for path in DEBUG_BASE_DIR.iterdir() if path.is_dir()), reverse=True)
  return sessions[0] if sessions else None


def list_sessions():
  if not DEBUG_BASE_DIR.is_dir():
    print(f"No debug sessions found in {DEBUG_BASE_DIR}")
    return

  sessions = sorted((path for path in DEBUG_BASE_DIR.iterdir() if path.is_dir()), reverse=True)
  if not sessions:
    print(f"No debug sessions found in {DEBUG_BASE_DIR}")
    return

  for session_path in sessions:
    events_path = session_path / "events.jsonl"
    event_count = 0
    bookmark_count = 0
    if events_path.is_file():
      with events_path.open("r", encoding="utf-8") as log_file:
        for line in log_file:
          if not line.strip():
            continue
          event_count += 1
          if '"event":"bookmark"' in line:
            bookmark_count += 1
    print(f"{session_path.name}: {event_count} events, {bookmark_count} bookmarks")


def print_event(event: dict):
  fields = [
    event.get("wallTime", ""),
    event.get("event", ""),
  ]
  if event.get("sessionSeconds") is not None:
    fields.append(f"t+{event['sessionSeconds']}s")

  if event.get("roadName"):
    fields.append(f"road={event['roadName']}")
  if event.get("speedLimitMph"):
    fields.append(f"speed={event['speedLimitMph']} mph")
  if event.get("candidateSpeedLimitMph"):
    fields.append(f"candidate={event['candidateSpeedLimitMph']} mph")
  if event.get("confidence"):
    fields.append(f"conf={event['confidence']}")
  if event.get("candidateConfidence"):
    fields.append(f"candidateConf={event['candidateConfidence']}")
  if event.get("statusText"):
    fields.append(f"status={event['statusText']}")
  elif event.get("status"):
    fields.append(f"status={event['status']}")
  if event.get("snapshot"):
    fields.append(f"snapshot={event['snapshot']}")
  print(" | ".join(str(field) for field in fields if field != ""))


def summarize_session(session_path: Path, window: int):
  events = load_events(session_path)
  print(f"Session: {session_path}")
  print(f"Events: {len(events)}")

  bookmarks = [idx for idx, event in enumerate(events) if event.get("event") == "bookmark"]
  if not bookmarks:
    print("Bookmarks: none")
    return

  print(f"Bookmarks: {len(bookmarks)}")
  for bookmark_number, event_idx in enumerate(bookmarks, start=1):
    print(f"\nBookmark {bookmark_number}")
    start = max(event_idx - window, 0)
    end = min(event_idx + window + 1, len(events))
    for idx in range(start, end):
      prefix = "->" if idx == event_idx else "  "
      print(prefix, end="")
      print_event(events[idx])


def main():
  parser = argparse.ArgumentParser(description="Summarize StarPilot speed-limit vision debug sessions.")
  parser.add_argument("session", nargs="?", help="Session id or full path. Defaults to the latest session.")
  parser.add_argument("--list", action="store_true", help="List available sessions and exit.")
  parser.add_argument("--window", type=int, default=5, help="How many events before/after each bookmark to print.")
  args = parser.parse_args()

  if args.list:
    list_sessions()
    return

  session_path = resolve_session(args.session)
  if session_path is None:
    print(f"No debug sessions found in {DEBUG_BASE_DIR}")
    return

  summarize_session(session_path, max(args.window, 0))


if __name__ == "__main__":
  main()
