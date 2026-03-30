#!/usr/bin/env python3
from __future__ import annotations

import argparse
import math
import random

from dataclasses import dataclass
from pathlib import Path

import cv2
import numpy as np

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_SPEED_VALUES, DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_SPEED_VALUES, DEFAULT_WORKSPACE, ensure_dir, resolve_workspace


DEFAULT_BACKGROUND_DIR = DEFAULT_WORKSPACE / "backgrounds"
HEADER_FONT_CANDIDATES = (
  "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
  "/System/Library/Fonts/ArialHB.ttc",
  "/System/Library/Fonts/Supplemental/Arial.ttf",
)
NUMBER_FONT_CANDIDATES = (
  "/System/Library/Fonts/Supplemental/DIN Condensed Bold.ttf",
  "/System/Library/Fonts/Supplemental/Arial Black.ttf",
  "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
)
KNOWN_REAL_CROPS = (
  (".tmp/live_c4_capture/stopped_sign_crop_manual.png", 15),
  (".tmp/route_vision/frame_041_sign_tight.jpg", 20),
  (".tmp/route_vision/frame_041_sign_manual.jpg", 20),
  (".tmp/route_12c_seg9_10/seg10_real30_crop.png", 30),
  (".tmp/speed_route_frames_seg2_10_20/t12_sign_crop.png", 40),
)


@dataclass(frozen=True)
class SignSpec:
  detector_class: int
  style: str
  speed_value: int | None


def load_font(candidates: tuple[str, ...], size: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
  for candidate in candidates:
    if Path(candidate).exists():
      return ImageFont.truetype(candidate, size=size)
  return ImageFont.load_default()


def draw_centered(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], text: str, font, fill: str):
  left, top, right, bottom = box
  bbox = draw.multiline_textbbox((0, 0), text, font=font, align="center", spacing=2)
  width = bbox[2] - bbox[0]
  height = bbox[3] - bbox[1]
  x = left + (right - left - width) / 2
  y = top + (bottom - top - height) / 2
  draw.multiline_text((x, y), text, font=font, fill=fill, align="center", spacing=2)


def render_regulatory_sign(speed_value: int, school_zone: bool, seed: int) -> Image.Image:
  rng = random.Random(seed)
  sign_w = rng.randint(260, 320)
  sign_h = rng.randint(390, 470)
  image = Image.new("RGBA", (sign_w, sign_h), (255, 255, 255, 0))
  draw = ImageDraw.Draw(image)

  border_radius = max(int(sign_w * 0.08), 16)
  draw.rounded_rectangle((0, 0, sign_w - 1, sign_h - 1), border_radius, fill="white", outline="black", width=max(sign_w // 38, 5))

  header_font = load_font(HEADER_FONT_CANDIDATES, max(sign_w // 9, 26))
  number_font = load_font(NUMBER_FONT_CANDIDATES, max(sign_w // 3, 84))
  footer_font = load_font(HEADER_FONT_CANDIDATES, max(sign_w // 12, 18))

  if school_zone:
    draw_centered(draw, (int(sign_w * 0.10), int(sign_h * 0.06), int(sign_w * 0.90), int(sign_h * 0.24)), "SCHOOL", header_font, "black")
    draw_centered(draw, (int(sign_w * 0.10), int(sign_h * 0.20), int(sign_w * 0.90), int(sign_h * 0.42)), "SPEED\nLIMIT", header_font, "black")
    draw_centered(draw, (int(sign_w * 0.12), int(sign_h * 0.42), int(sign_w * 0.88), int(sign_h * 0.78)), str(speed_value), number_font, "black")
    draw_centered(draw, (int(sign_w * 0.08), int(sign_h * 0.76), int(sign_w * 0.92), int(sign_h * 0.94)), "WHEN FLASHING", footer_font, "black")
  else:
    draw_centered(draw, (int(sign_w * 0.10), int(sign_h * 0.08), int(sign_w * 0.90), int(sign_h * 0.34)), "SPEED\nLIMIT", header_font, "black")
    draw_centered(draw, (int(sign_w * 0.12), int(sign_h * 0.40), int(sign_w * 0.88), int(sign_h * 0.84)), str(speed_value), number_font, "black")

  if school_zone and rng.random() < 0.65:
    lamp_y = int(sign_h * 0.12)
    lamp_r = max(sign_w // 18, 12)
    for lamp_x in (int(sign_w * 0.16), int(sign_w * 0.84)):
      draw.ellipse((lamp_x - lamp_r, lamp_y - lamp_r, lamp_x + lamp_r, lamp_y + lamp_r), fill=(255, 192, 0), outline="black", width=3)

  return image


def render_advisory_sign(speed_value: int, seed: int) -> Image.Image:
  rng = random.Random(seed)
  size = rng.randint(240, 320)
  image = Image.new("RGBA", (size, size), (255, 255, 255, 0))
  base = Image.new("RGBA", (size, size), (255, 255, 255, 0))
  draw = ImageDraw.Draw(base)

  draw.polygon(((size / 2, 0), (size, size / 2), (size / 2, size), (0, size / 2)), fill=(255, 214, 10), outline="black")
  base = base.rotate(45, expand=True, resample=Image.Resampling.BICUBIC)
  bbox = base.getbbox()
  if bbox is not None:
    base = base.crop(bbox)

  draw = ImageDraw.Draw(base)
  number_font = load_font(NUMBER_FONT_CANDIDATES, max(base.size[0] // 3, 72))
  footer_font = load_font(HEADER_FONT_CANDIDATES, max(base.size[0] // 12, 18))
  draw_centered(draw, (0, int(base.size[1] * 0.18), base.size[0], int(base.size[1] * 0.68)), str(speed_value), number_font, "black")
  if rng.random() < 0.7:
    draw_centered(draw, (0, int(base.size[1] * 0.68), base.size[0], int(base.size[1] * 0.92)), "MPH", footer_font, "black")
  return base


def add_motion_blur(image: Image.Image, radius: int) -> Image.Image:
  if radius <= 1:
    return image
  kernel_size = radius * 2 + 1
  kernel = np.zeros((kernel_size, kernel_size), dtype=np.float32)
  kernel[kernel_size // 2, :] = 1.0 / kernel_size
  array = cv2.filter2D(np.array(image), -1, kernel)
  return Image.fromarray(array)


def augment_sign(sign: Image.Image, rng: random.Random) -> Image.Image:
  image = sign.copy()
  if rng.random() < 0.7:
    image = image.filter(ImageFilter.GaussianBlur(radius=rng.uniform(0.2, 1.8)))
  if rng.random() < 0.35:
    image = add_motion_blur(image, radius=rng.randint(2, 5))

  brightness = ImageEnhance.Brightness(image)
  image = brightness.enhance(rng.uniform(0.75, 1.18))
  contrast = ImageEnhance.Contrast(image)
  image = contrast.enhance(rng.uniform(0.85, 1.25))
  return image


def choose_sign_spec(rng: random.Random, speed_values: tuple[int, ...]) -> SignSpec:
  roll = rng.random()
  if roll < 0.16:
    return SignSpec(detector_class=1, style="advisory", speed_value=rng.choice((20, 25, 30, 35, 40, 45)))
  if roll < 0.34:
    school_choices = tuple(value for value in speed_values if value in (15, 20, 25))
    return SignSpec(detector_class=2, style="school_zone", speed_value=rng.choice(school_choices or speed_values))
  return SignSpec(detector_class=0, style="regulatory", speed_value=rng.choice(speed_values))


def paste_transformed(background_bgr: np.ndarray, sign_rgba: Image.Image, rng: random.Random):
  background = background_bgr.copy()
  bg_h, bg_w = background.shape[:2]
  sign = np.array(sign_rgba)
  sign_h, sign_w = sign.shape[:2]

  target_h = int(rng.uniform(bg_h * 0.045, bg_h * 0.17))
  scale = target_h / max(sign_h, 1)
  target_w = max(int(sign_w * scale), 12)
  resized = cv2.resize(sign, (target_w, target_h), interpolation=cv2.INTER_LINEAR)
  sign_h, sign_w = resized.shape[:2]

  center_x = int(rng.uniform(bg_w * 0.58, bg_w * 0.92))
  center_y = int(rng.uniform(bg_h * 0.10, bg_h * 0.58))

  src = np.float32([[0, 0], [sign_w - 1, 0], [sign_w - 1, sign_h - 1], [0, sign_h - 1]])
  skew_x = sign_w * rng.uniform(0.04, 0.18)
  skew_y = sign_h * rng.uniform(0.02, 0.12)
  dst = np.float32([
    [center_x - sign_w * rng.uniform(0.35, 0.55), center_y - sign_h * rng.uniform(0.55, 0.70)],
    [center_x + sign_w * rng.uniform(0.35, 0.55), center_y - sign_h * rng.uniform(0.45, 0.70)],
    [center_x + sign_w * rng.uniform(0.28, 0.52), center_y + sign_h * rng.uniform(0.30, 0.58)],
    [center_x - sign_w * rng.uniform(0.26, 0.48), center_y + sign_h * rng.uniform(0.34, 0.62)],
  ])
  dst += np.float32([
    [rng.uniform(-skew_x, skew_x), rng.uniform(-skew_y, skew_y)],
    [rng.uniform(-skew_x, skew_x), rng.uniform(-skew_y, skew_y)],
    [rng.uniform(-skew_x, skew_x), rng.uniform(-skew_y, skew_y)],
    [rng.uniform(-skew_x, skew_x), rng.uniform(-skew_y, skew_y)],
  ])

  matrix = cv2.getPerspectiveTransform(src, dst)
  warped = cv2.warpPerspective(resized, matrix, (bg_w, bg_h), flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT, borderValue=(0, 0, 0, 0))
  alpha = warped[:, :, 3:4].astype(np.float32) / 255.0
  if alpha.max() <= 0.01:
    return background, None, None

  warped_rgb = warped[:, :, :3].astype(np.float32)
  composite = background.astype(np.float32) * (1.0 - alpha) + warped_rgb * alpha
  composite = composite.astype(np.uint8)

  ys, xs = np.where(alpha[:, :, 0] > 0.05)
  if len(xs) == 0 or len(ys) == 0:
    return background, None, None

  x1, x2 = int(xs.min()), int(xs.max())
  y1, y2 = int(ys.min()), int(ys.max())
  bbox = (x1, y1, x2, y2)
  crop = composite[y1:y2 + 1, x1:x2 + 1]
  return composite, bbox, crop


def detector_label_line(detector_class: int, bbox: tuple[int, int, int, int], image_shape: tuple[int, int, int]) -> str:
  image_h, image_w = image_shape[:2]
  x1, y1, x2, y2 = bbox
  x_center = ((x1 + x2) / 2) / image_w
  y_center = ((y1 + y2) / 2) / image_h
  width = (x2 - x1) / image_w
  height = (y2 - y1) / image_h
  return f"{detector_class} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n"


def save_classifier_crop(base_dir: Path, split: str, speed_value: int, image_bgr: np.ndarray, stem: str):
  output_dir = ensure_dir(base_dir / split / str(speed_value))
  output_path = output_dir / f"{stem}.jpg"
  cv2.imwrite(str(output_path), image_bgr, [cv2.IMWRITE_JPEG_QUALITY, 92])


def collect_backgrounds(background_dir: Path) -> list[Path]:
  if not background_dir.is_dir():
    return []
  return sorted(path for path in background_dir.iterdir() if path.suffix.lower() in {".jpg", ".jpeg", ".png"})


def main():
  parser = argparse.ArgumentParser(description="Generate a synthetic U.S. speed-limit detector/classifier dataset.")
  parser.add_argument("--workspace", default=str(DEFAULT_WORKSPACE), help="Training workspace root.")
  parser.add_argument("--background-dir", default=str(DEFAULT_BACKGROUND_DIR), help="Background image directory.")
  parser.add_argument("--train-count", type=int, default=9000, help="Number of synthetic training detector images.")
  parser.add_argument("--val-count", type=int, default=1200, help="Number of synthetic validation detector images.")
  parser.add_argument("--negative-ratio", type=float, default=0.18, help="Share of detector images with no sign.")
  parser.add_argument("--seed", type=int, default=20260330, help="Random seed.")
  args = parser.parse_args()

  workspace = resolve_workspace(args.workspace)
  background_dir = Path(args.background_dir).expanduser().resolve()
  backgrounds = collect_backgrounds(background_dir)
  if not backgrounds:
    raise FileNotFoundError(f"No backgrounds found in {background_dir}")

  detector_image_dir = workspace / "detector" / "images"
  detector_label_dir = workspace / "detector" / "labels"
  classifier_dir = workspace / "classifier"
  speed_values = tuple(DEFAULT_SPEED_VALUES)
  rng = random.Random(args.seed)

  for split, count in (("train", max(args.train_count, 0)), ("val", max(args.val_count, 0))):
    ensure_dir(detector_image_dir / split)
    ensure_dir(detector_label_dir / split)
    ensure_dir(classifier_dir / split)

    for index in range(count):
      background_path = rng.choice(backgrounds)
      background = cv2.imread(str(background_path))
      if background is None:
        continue

      stem = f"{split}_{index:06d}"
      image_path = detector_image_dir / split / f"{stem}.jpg"
      label_path = detector_label_dir / split / f"{stem}.txt"
      detector_lines: list[str] = []

      if rng.random() >= args.negative_ratio:
        sign_spec = choose_sign_spec(rng, speed_values)
        if sign_spec.style == "advisory":
          sign_image = render_advisory_sign(sign_spec.speed_value or 25, seed=rng.randint(0, 1_000_000))
        else:
          sign_image = render_regulatory_sign(sign_spec.speed_value or 25, school_zone=sign_spec.style == "school_zone", seed=rng.randint(0, 1_000_000))
        sign_image = augment_sign(sign_image, rng)
        composite, bbox, crop = paste_transformed(background, sign_image, rng)
        if bbox is not None:
          detector_lines.append(detector_label_line(sign_spec.detector_class, bbox, composite.shape))
          background = composite
          if crop is not None and sign_spec.speed_value is not None and sign_spec.detector_class != 1:
            save_classifier_crop(classifier_dir, split, sign_spec.speed_value, crop, stem)

      if rng.random() < 0.45:
        alpha = rng.uniform(0.05, 0.20)
        overlay = np.full_like(background, int(rng.uniform(180, 240)))
        background = cv2.addWeighted(background, 1.0 - alpha, overlay, alpha, 0)
      if rng.random() < 0.3:
        background = cv2.GaussianBlur(background, (3, 3), rng.uniform(0.1, 0.9))
      if rng.random() < 0.25:
        noise = rng.normalvariate(0, 6)
        background = np.clip(background.astype(np.int16) + noise, 0, 255).astype(np.uint8)

      cv2.imwrite(str(image_path), background, [cv2.IMWRITE_JPEG_QUALITY, 92])
      label_path.write_text("".join(detector_lines), encoding="utf-8")

  repo_root = Path(__file__).resolve().parents[2]
  imported_real = 0
  for relative_path, speed_value in KNOWN_REAL_CROPS:
    crop_path = repo_root / relative_path
    if not crop_path.is_file():
      continue
    image = cv2.imread(str(crop_path))
    if image is None:
      continue
    split = "val" if imported_real % 4 == 0 else "train"
    save_classifier_crop(classifier_dir, split, speed_value, image, f"real_{speed_value}_{imported_real:03d}")
    imported_real += 1

  print(f"Generated synthetic detector data in {workspace / 'detector'}")
  print(f"Generated synthetic classifier data in {workspace / 'classifier'}")
  print(f"Backgrounds used: {len(backgrounds)}")
  print(f"Imported real crops: {imported_real}")


if __name__ == "__main__":
  main()
