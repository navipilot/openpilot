class CoopSteering:
  TORQUE_THRESHOLD = 0.5  # Nm - deadzone for driver input detection
  RESUME_DELAY = 0.5      # seconds before openpilot resumes after driver releases
  FRAME_RATE = 50          # Hz (steering sent every 2 frames at 100Hz main loop)

  def __init__(self):
    self.override_active = False
    self.frames_since_override = 0
    self.resume_frames = int(self.RESUME_DELAY * self.FRAME_RATE)

  def update(self, steering_torque, hands_on_level, lat_active):
    """Call every steering frame (50Hz). Returns True if driver is overriding."""
    if not lat_active:
      self.override_active = False
      self.frames_since_override = 0
      return False

    driver_input = abs(steering_torque) > self.TORQUE_THRESHOLD or hands_on_level >= 2

    if driver_input:
      self.override_active = True
      self.frames_since_override = 0
    elif self.override_active:
      self.frames_since_override += 1
      if self.frames_since_override >= self.resume_frames:
        self.override_active = False
        self.frames_since_override = 0

    return self.override_active
