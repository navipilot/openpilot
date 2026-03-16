class CoopSteering:
  # Torque threshold must be well above EPAS actuation feedback (~0.5 Nm)
  # Real driver steering input is typically 1.5+ Nm
  TORQUE_THRESHOLD = 1.5  # Nm - deadzone for driver input detection
  RESUME_DELAY = 0.5      # seconds before openpilot resumes after driver releases
  RESUME_BLEND = 1.5      # seconds to blend from physical angle to OP desired after resume
  FRAME_RATE = 50          # Hz (steering sent every 2 frames at 100Hz main loop)

  def __init__(self):
    self.override_active = False
    self.frames_since_override = 0
    self.resume_frames = int(self.RESUME_DELAY * self.FRAME_RATE)
    self.blend_frames = int(self.RESUME_BLEND * self.FRAME_RATE)
    self._blend_counter = 0
    self._blending = False

  def update(self, steering_torque, hands_on_level, lat_active):
    """Call every steering frame (50Hz). Returns True if driver is overriding."""
    if not lat_active:
      self.override_active = False
      self.frames_since_override = 0
      self._blending = False
      self._blend_counter = 0
      return False

    # hands_on_level >= 3 is the EPAS's own filtered driver detection (most reliable)
    # torque threshold is a backup for quicker response on strong driver input
    driver_input = hands_on_level >= 3 or abs(steering_torque) > self.TORQUE_THRESHOLD

    if driver_input:
      self.override_active = True
      self.frames_since_override = 0
      self._blending = False
      self._blend_counter = 0
    elif self.override_active:
      self.frames_since_override += 1
      if self.frames_since_override >= self.resume_frames:
        self.override_active = False
        self.frames_since_override = 0
        # Start blending from physical angle to OP desired
        self._blending = True
        self._blend_counter = 0
    elif self._blending:
      self._blend_counter += 1
      if self._blend_counter >= self.blend_frames:
        self._blending = False

    return self.override_active

  @property
  def blend_factor(self):
    """0.0 = use physical angle, 1.0 = use OP desired angle.
    Ramps linearly over RESUME_BLEND seconds after override ends."""
    if self.override_active:
      return 0.0
    if self._blending:
      return min(1.0, self._blend_counter / self.blend_frames)
    return 1.0
