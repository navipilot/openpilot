# Train Speed-Limit Vision

This flow is for replacing the current imported `ayoubsa_best` checkpoint with a U.S.-focused detector plus a separate posted-speed classifier.

The intended recipe is:

1. bootstrap from public traffic-sign data
2. fine-tune on comma-specific bookmarked drives
3. export ONNX models
4. copy them onto the device for runtime testing

## Why Two Models

The detector and the value reader solve different problems.

- detector: find the sign and decide whether it is a regulatory speed-limit sign
- classifier: read the posted value from the cropped sign

This is a better fit for U.S. roads than a single detector with baked-in classes like `Speed Limit 10`, `Speed Limit 20`, `Speed Limit 30`, and so on.

## Suggested Public Data

- `LISA` for U.S. roadside sign geometry and sign style
- `ARTS` for U.S. MUTCD-style sign annotations and values
- `GLARE` for U.S. glare/lighting failures

Use public data to get the model into the right regime, then fine-tune on comma bookmarks and replay clips.

## Install Training Deps

From the repo root:

```bash
uv sync --extra speedvision
```

The runtime device does not need these packages. Only the training machine does.

## Initialize a Workspace

```bash
.venv/bin/python scripts/speed_limit_vision/init_workspace.py
```

This creates `.tmp/speed_limit_training` with:

- detector image/label folders
- classifier crop folders
- review/bookmark manifests
- raw-source manifests
- export and run directories

To keep the raw datasets off the internal disk, point `--workspace` at the SSD-backed workspace, for example:

```bash
.venv/bin/python scripts/speed_limit_vision/init_workspace.py \
  --workspace /Volumes/T5/starpilot_speed_limit/workspace/speed_limit_training_clean
```

The workspace now also tracks:

- `manifests/raw_sources.csv`
- `manifests/public_detector_samples.csv`
- `manifests/public_classifier_samples.csv`

Those manifests are the provenance record for every imported public sample.

## Import Public Datasets

ARTS challenging subset:

```bash
.venv/bin/python scripts/speed_limit_vision/import_arts_challenging.py \
  --workspace /Volumes/T5/starpilot_speed_limit/workspace/speed_limit_training_clean
```

This imports mapped speed-limit signs from the raw `challenging-dev.tar.gz` archive and appends detector/classifier provenance rows into the manifest CSVs.

GLARE and LISA should be downloaded into the SSD raw tree first:

- `/Volumes/T5/starpilot_speed_limit/raw/glare_official`
- `/Volumes/T5/starpilot_speed_limit/raw/lisa_official`

Then import them into the same workspace so the detector/classifier datasets stay source-traceable.

For GLARE, do not pull the whole Drive tree blindly. Use the filtered raw downloader so only the `Images/` and optional `Tracks/` files land on disk:

```bash
.venv/bin/python scripts/speed_limit_vision/download_glare_raw.py \
  --workspace /Volumes/T5/starpilot_speed_limit/workspace/speed_limit_training_clean \
  --output-root /Volumes/T5/starpilot_speed_limit/raw/glare_raw \
  --prefix Images/ \
  --resume
```

Then import the completed image tree:

```bash
.venv/bin/python scripts/speed_limit_vision/import_glare_images.py \
  --workspace /Volumes/T5/starpilot_speed_limit/workspace/speed_limit_training_clean \
  --overwrite
```

## Import Bookmarked Debug Sessions

After a drive, copy or mount the debug session directory locally, then import it:

```bash
.venv/bin/python scripts/speed_limit_vision/import_debug_sessions.py
```

Or point at specific sessions:

```bash
.venv/bin/python scripts/speed_limit_vision/import_debug_sessions.py 20260330_220102 20260330_223355
```

This writes:

- `review/bookmarks.csv`
- snapshot images under `review/images`

That manifest is the shortlist for labeling.

If the route only lives on comma connect, fetch it directly into the same clip layout the bookmark tools expect:

```bash
.venv/bin/python scripts/speed_limit_vision/download_connect_routes.py \
  <dongle_id>/<route_log_id> \
  --streams fcamera,qlog
```

This reads the JWT from `~/.comma/auth.json`, downloads the requested route files from comma connect, and updates:

- `/Volumes/T5/starpilot_speed_limit/live_route_clips/bookmark_windows/data/media/0/realdata/...` when the SSD is mounted, otherwise the same `.tmp/...` paths under the repo
- `/Volumes/T5/starpilot_speed_limit/live_routes_meta/qlog_mtimes.txt` when the SSD is mounted, otherwise `.tmp/live_routes_meta/qlog_mtimes.txt`
- `/Volumes/T5/starpilot_speed_limit/live_routes_meta/files.txt` when the SSD is mounted, otherwise `.tmp/live_routes_meta/files.txt`

For multiple routes, either pass several ids on the command line or use `--routes-file`.

## Evaluate Real Sign Lead-Ins

Bookmark stills are often too late. To score what matters, replay the real `fcamera.hevc` footage from the `5` seconds before each bookmark:

```bash
.venv/bin/python scripts/speed_limit_vision/evaluate_bookmark_leadins.py \
  --json-out .tmp/live_route_clips/bookmark_windows_report.json
```

This produces a per-bookmark report of whether the current runtime saw anything in the usable sign approach window.

The evaluator now reads a local session-to-route map from `session_route_map.json` under the same `live_routes_meta` root. Keep that file local or on the SSD so personal route ids never need to live in the repo.

## Import Missed Lead-Ins for Labeling

Turn those lead-in misses into review frames and contact sheets:

```bash
.venv/bin/python scripts/speed_limit_vision/import_bookmark_leadins.py \
  --mode misses
```

This writes:

- `review/bookmark_leadins.csv`
- sampled frames under `review/leadins/frames`
- contact sheets under `review/leadins/contact_sheets`

That review set is the right source for labeling missed `55 mph`, night, and town-sequence failures.

The bookmark/lead-in importers also accept source metadata fields such as region, device, and driver. Use those when importing debug sessions from multiple users so the comma-specific fine-tune can be sliced by contributor or geography instead of becoming one opaque pool.

To shrink that review set to the most promising frames per missed sign window:

```bash
.venv/bin/python scripts/speed_limit_vision/rank_bookmark_leadin_frames.py
```

This writes `review/bookmark_leadin_shortlist.csv` with the top-ranked sampled frames per bookmark.

## Build the Detector Dataset

Take the imported review images and move or copy the ones you want into:

- `detector/images/train`
- `detector/images/val`

Label them in YOLO detect format into:

- `detector/labels/train`
- `detector/labels/val`

Recommended classes:

- `regulatory_speed_limit`
- `advisory_speed_limit`
- `school_zone_speed_limit`

The dataset YAML is already generated at:

- `detector/dataset.yaml`

## Build the Value Classifier Dataset

Fill out:

- `classifier/value_labels.csv`

Columns:

- `image_path`: source image file
- `split`: `train` or `val`
- `speed_limit_mph`: posted value such as `25`, `35`, or `55`
- `bbox_index`: which YOLO box to crop if an image has multiple labeled signs
- `padding`: optional crop padding ratio
- `label_path`: optional explicit path to the YOLO label file

Then generate the classifier crop folders:

```bash
.venv/bin/python scripts/speed_limit_vision/build_value_dataset.py
```

This writes cropped sign images into:

- `classifier/train/<value>/...`
- `classifier/val/<value>/...`

## Train

Detector:

```bash
.venv/bin/python scripts/speed_limit_vision/train_detector.py --device mps
```

Classifier:

```bash
.venv/bin/python scripts/speed_limit_vision/train_value_classifier.py --device mps
```

Use `--device cpu`, `--device mps`, or a CUDA device string depending on the training host.

## Rebalance Toward Real Comma Data

If the detector starts overfitting to synthetic/public data, build a lighter rebalanced dataset that keeps all `real_*` detector images and samples the rest:

```bash
.venv/bin/python scripts/speed_limit_vision/rebalance_detector_dataset.py \
  --workspace .tmp/speed_limit_training \
  --max-other-train 3200
```

Then point the detector trainer at the generated YAML:

```bash
.venv/bin/python scripts/speed_limit_vision/train_detector.py \
  --workspace .tmp/speed_limit_training \
  --data .tmp/speed_limit_training/detector_rebalanced/dataset.yaml \
  --device mps
```

This keeps validation unchanged while making retrains faster and more comma-biased.

## Export ONNX

```bash
.venv/bin/python scripts/speed_limit_vision/export_models.py \
  --detector-weights .tmp/speed_limit_training/runs/detector/yolo11n-speed-limit-us/weights/best.pt \
  --classifier-weights .tmp/speed_limit_training/runs/classifier/yolo11n-cls-speed-limit-us/weights/best.pt \
  --install-repo-assets
```

That writes:

- `.tmp/speed_limit_training/exports/speed_limit_us_detector.onnx`
- `.tmp/speed_limit_training/exports/speed_limit_us_value_classifier.onnx`

And optionally copies them into:

- `starpilot/assets/vision_models`

## Copy to the Device

```bash
.venv/bin/python scripts/speed_limit_vision/install_models.py --host comma@192.168.3.110
```

The runtime already prefers `speed_limit_us_detector.onnx` plus `speed_limit_us_value_classifier.onnx` when both files exist in `starpilot/assets/vision_models`.

## Evaluate the Runtime Path

Run the real StarPilot runtime path, using the installed ONNX pair, against the known saved-frame cases:

```bash
.venv/bin/python scripts/speed_limit_vision/evaluate_runtime_cases.py --strict
```

For temporal behavior on a saved frame directory or route extract, replay the runtime directly:

```bash
.venv/bin/python scripts/replay_speed_limit_vision.py .tmp/vision_iter/seg10_5fps --frames-fps 5
```
