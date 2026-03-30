#!/bin/zsh
set -euo pipefail

REPO_ROOT="/Users/dominickthompson/starpilot"
WORKSPACE="/Volumes/T5/starpilot_speed_limit/workspace/speed_limit_training_clean"
LOG_DIR="$WORKSPACE/logs"
BASE_DETECTOR_NAME="yolo11n-comma-us-clean-v1"
GLARE_DETECTOR_NAME="yolo11n-comma-us-clean-glare-v1"
CLASSIFIER_NAME="yolo11n-cls-speed-limit-us-clean-v1"
EXPORT_DIR="$WORKSPACE/exports/overnight_latest"

mkdir -p "$LOG_DIR"
cd "$REPO_ROOT"

echo "[$(date)] waiting for GLARE raw download to finish"
while true; do
  if pgrep -f "download_glare_raw.py --workspace $WORKSPACE" >/dev/null; then
    sleep 30
  else
    break
  fi
done

echo "[$(date)] importing completed GLARE images"
.venv/bin/python scripts/speed_limit_vision/import_glare_images.py --workspace "$WORKSPACE" --overwrite
.venv/bin/python scripts/speed_limit_vision/build_value_dataset.py --workspace "$WORKSPACE" --overwrite

echo "[$(date)] waiting for base detector run to finish"
while true; do
  if pgrep -f "train_detector.py --workspace $WORKSPACE .*--name $BASE_DETECTOR_NAME" >/dev/null; then
    sleep 30
  else
    break
  fi
done

BASE_DETECTOR_WEIGHTS="$WORKSPACE/runs/detector/$BASE_DETECTOR_NAME/weights/best.pt"
if [[ ! -f "$BASE_DETECTOR_WEIGHTS" ]]; then
  echo "[$(date)] missing base detector weights: $BASE_DETECTOR_WEIGHTS" >&2
  exit 1
fi

echo "[$(date)] starting GLARE-augmented detector fine-tune"
.venv/bin/python scripts/speed_limit_vision/train_detector.py \
  --workspace "$WORKSPACE" \
  --device mps \
  --epochs 20 \
  --batch 24 \
  --workers 4 \
  --model "$BASE_DETECTOR_WEIGHTS" \
  --name "$GLARE_DETECTOR_NAME" \
  --exist-ok

DETECTOR_WEIGHTS="$WORKSPACE/runs/detector/$GLARE_DETECTOR_NAME/weights/best.pt"
if [[ ! -f "$DETECTOR_WEIGHTS" ]]; then
  DETECTOR_WEIGHTS="$BASE_DETECTOR_WEIGHTS"
fi

echo "[$(date)] training value classifier"
.venv/bin/python scripts/speed_limit_vision/train_value_classifier.py \
  --workspace "$WORKSPACE" \
  --device mps \
  --epochs 40 \
  --batch 64 \
  --workers 4 \
  --name "$CLASSIFIER_NAME" \
  --exist-ok

CLASSIFIER_WEIGHTS="$WORKSPACE/runs/classifier/$CLASSIFIER_NAME/weights/best.pt"
if [[ ! -f "$CLASSIFIER_WEIGHTS" ]]; then
  echo "[$(date)] missing classifier weights: $CLASSIFIER_WEIGHTS" >&2
  exit 1
fi

echo "[$(date)] exporting ONNX models into repo assets"
.venv/bin/python scripts/speed_limit_vision/export_models.py \
  --workspace "$WORKSPACE" \
  --detector-weights "$DETECTOR_WEIGHTS" \
  --classifier-weights "$CLASSIFIER_WEIGHTS" \
  --output-dir "$EXPORT_DIR" \
  --install-repo-assets

echo "[$(date)] evaluating runtime saved-frame cases"
.venv/bin/python scripts/speed_limit_vision/evaluate_runtime_cases.py \
  --models-dir "$EXPORT_DIR" \
  --strict | tee "$LOG_DIR/runtime_cases_overnight.txt"

echo "[$(date)] evaluating bookmarked lead-ins"
.venv/bin/python scripts/speed_limit_vision/evaluate_bookmark_leadins.py \
  --models-dir "$EXPORT_DIR" \
  --lead-in 7 \
  --sample-fps 5 \
  --json-out "$LOG_DIR/bookmark_windows_overnight.json" | tee "$LOG_DIR/bookmark_windows_overnight.txt"

echo "[$(date)] overnight clean pipeline complete"
