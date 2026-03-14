using Cxx = import "./include/c++.capnp";
$Cxx.namespace("cereal");

using Car = import "car.capnp";

@0xb526ba661d550a59;

# custom.capnp: a home for empty structs reserved for custom forks
# These structs are guaranteed to remain reserved and empty in mainline
# cereal, so use these if you want custom events in your fork.

# DO rename the structs
# DON'T change the identifier (e.g. @0x81c2f05a394cf4af)

struct FrogPilotCarControl @0x81c2f05a394cf4af {
  hudControl @0 :HUDControl;

  struct HUDControl {
    audibleAlert @0 :AudibleAlert;

    enum AudibleAlert {
      none @0;

      engage @1;
      disengage @2;
      refuse @3;

      warningSoft @4;
      warningImmediate @5;

      prompt @6;
      promptRepeat @7;
      promptDistracted @8;

      # Random Events
      angry @9;
      continued @10;
      dejaVu @11;
      doc @12;
      fart @13;
      firefox @14;
      goat @15;
      hal9000 @16;
      mail @17;
      nessie @18;
      noice @19;
      startup @20;
      thisIsFine @21;
      uwu @22;
    }
  }
}

struct FrogPilotCarParams @0xaedffd8f31e7b55d {
  alternativeExperience @0 :Int16;
  canUsePedal @1 :Bool;
  canUseSDSU @2 :Bool;
  flags @3 :UInt32;
  isHDA2 @4 :Bool;
  openpilotLongitudinalControlDisabled @5 :Bool;
  safetyConfigs @6 :List(SafetyConfig);

  struct SafetyConfig {
    safetyParam @0 :UInt16;
  }
}

struct FrogPilotCarState @0xf35cc4560bbf6ec2 {
  accelPressed @0 :Bool;
  alwaysOnLateralEnabled @1 :Bool;
  brakeLights @2 :Bool;
  dashboardSpeedLimit @3 :Float32;
  decelPressed @4 :Bool;
  distancePressed @5 :Bool;
  distanceLongPressed @6 :Bool;
  distanceVeryLongPressed @7 :Bool;
  ecoGear @8 :Bool;
  forceCoast @9 :Bool;
  isParked @10 :Bool;
  pauseLateral @11 :Bool;
  pauseLongitudinal @12 :Bool;
  sportGear @13 :Bool;
  trafficModeEnabled @14 :Bool;
}

struct FrogPilotDeviceState @0xda96579883444c35 {
  freeSpace @0 :Int16;
  usedSpace @1 :Int16;
}

struct FrogPilotModelDataV2 @0x80ae746ee2596b11 {
  turnDirection @0 :TurnDirection;

  enum TurnDirection {
    none @0;
    turnLeft @1;
    turnRight @2;
  }
}

struct FrogPilotOnroadEvent @0xa5cd762cd951a455 {
  name @0 :EventName;

  enable @1 :Bool;
  noEntry @2 :Bool;
  warning @3 :Bool;
  userDisable @4 :Bool;
  softDisable @5 :Bool;
  immediateDisable @6 :Bool;
  preEnable @7 :Bool;
  permanent @8 :Bool;
  overrideLateral @9 :Bool;
  overrideLongitudinal @10 :Bool;

  enum EventName {
    blockUser @0;
    customStartupAlert @1;
    forcingStop @2;
    goatSteerSaturated @3;
    greenLight @4;
    holidayActive @5;
    laneChangeBlockedLoud @6;
    leadDeparting @7;
    noLaneAvailable @8;
    nnffLoaded @9;
    openpilotCrashed @10;
    pedalInterceptorNoBrake @11;
    speedLimitChanged @12;
    trafficModeActive @13;
    trafficModeInactive @14;
    turningLeft @15;
    turningRight @16;

    # Random Events
    accel30 @17;
    accel35 @18;
    accel40 @19;
    dejaVuCurve @20;
    firefoxSteerSaturated @21;
    hal9000 @22;
    openpilotCrashedRandomEvent @23;
    thisIsFineSteerSaturated @24;
    toBeContinued @25;
    vCruise69 @26;
    yourFrogTriedToKillMe @27;
    youveGotMail @28;
  }
}

struct FrogPilotPlan @0xf98d843bfd7004a3 {
  accelerationJerk @0 :Float32;
  cscControllingSpeed @1 :Bool;
  cscSpeed @2 :Float32;
  cscTraining @3 :Bool;
  dangerFactor @4 :Float32;
  dangerJerk @5 :Float32;
  desiredFollowDistance @6 :Int64;
  experimentalMode @7 :Bool;
  forcingStop @8 :Bool;
  forcingStopLength @9 :Float32;
  frogpilotEvents @10 :List(FrogPilotOnroadEvent);
  frogpilotToggles @11 :Text;
  increasedStoppedDistance @12 :Float32;
  lateralCheck @13 :Bool;
  laneWidthLeft @14 :Float32;
  laneWidthRight @15 :Float32;
  maxAcceleration @16 :Float32;
  minAcceleration @17 :Float32;
  redLight @18 :Bool;
  roadCurvature @19 :Float32;
  slcMapSpeedLimit @20 :Float32;
  slcMapboxSpeedLimit @21 :Float32;
  slcNextSpeedLimit @22 :Float32;
  slcOverriddenSpeed @23 :Float32;
  slcSpeedLimit @24 :Float32;
  slcSpeedLimitOffset @25 :Float32;
  slcSpeedLimitSource @26 :Text;
  speedJerk @27 :Float32;
  speedLimitChanged @28 :Bool;
  tFollow @29 :Float32;
  themeUpdated @30 :Bool;
  unconfirmedSlcSpeedLimit @31 :Float32;
  vCruise @32 :Float32;
  weatherDaytime @33 :Bool;
  weatherId @34 :Int16;
}

struct FrogPilotRadarState @0xb86e6369214c01c8 {
  leadLeft @0 :LeadData;
  leadRight @1 :LeadData;

  struct LeadData {
    dRel @0 :Float32;
    yRel @1 :Float32;
    vRel @2 :Float32;
    aRel @3 :Float32;
    vLead @4 :Float32;
    dPath @6 :Float32;
    vLat @7 :Float32;
    vLeadK @8 :Float32;
    aLeadK @9 :Float32;
    fcw @10 :Bool;
    status @11 :Bool;
    aLeadTau @12 :Float32;
    modelProb @13 :Float32;
    radar @14 :Bool;
    radarTrackId @15 :Int32 = -1;

    aLeadDEPRECATED @5 :Float32;
  }
}

struct FrogPilotSelfdriveState @0xf416ec09499d9d19 {
  alertText1 @0 :Text;
  alertText2 @1 :Text;
  alertStatus @2 :AlertStatus;
  alertSize @3 :AlertSize;
  alertType @4 :Text;
  alertSound @5 :Car.CarControl.HUDControl.AudibleAlert;

  enum AlertStatus {
    normal @0;
    userPrompt @1;
    critical @2;
    frogpilot @3;
  }

  enum AlertSize {
    none @0;
    small @1;
    mid @2;
    full @3;
  }
}

struct CustomReserved9 @0xa1680744031fdb2d {
}

struct CustomReserved10 @0xcb9fd56c7057593a {
}

struct CustomReserved11 @0xc2243c65e0340384 {
}

struct CustomReserved12 @0x9ccdc8676701b412 {
}

struct CustomReserved13 @0xcd96dafb67a082d0 {
}

struct CustomReserved14 @0xb057204d7deadf3f {
}

struct CustomReserved15 @0xbd443b539493bc68 {
}

struct CustomReserved16 @0xfc6241ed8877b611 {
}

struct MapdDownloadLocationDetails @0xff889853e7b0987f {
  downloadedFiles @0 :UInt32;
  location @1 :Text;
  totalFiles @2 :UInt32;
}

struct MapdDownloadProgress @0xfaa35dcac85073a2 {
  active @0 :Bool;
  cancelled @1 :Bool;
  downloadedFiles @2 :UInt32;
  locationDetails @3 :List(MapdDownloadLocationDetails);
  locations @4 :List(Text);
  totalFiles @5 :UInt32;
}

struct MapdPathPoint @0xd6f78acca1bc3939 {
  curvature @0 :Float32;
  latitude @1 :Float64;
  longitude @2 :Float64;
  targetVelocity @3 :Float32;
}

struct MapdExtendedOut @0xa30662f84033036c {
  downloadProgress @0 :MapdDownloadProgress;
  path @1 :List(MapdPathPoint);
  settings @2 :Text;
}

enum MapdInputType {
  acceptSpeedLimit @0;
  cancelDownload @1;
  download @2;
  loadDefaultSettings @3;
  loadPersistentSettings @4;
  loadRecommendedSettings @5;
  reloadSettings @6;
  saveSettings @7;
  setAcceptSpeedLimitTimeout @8;
  setAdjustSetSpeedToAcceptSpeedLimit @9;
  setDefaultLaneWidth @10;
  setEnableSpeed @11;
  setExternalSpeedLimit @12;
  setExternalSpeedLimitControl @13;
  setHoldLastSeenSpeedLimit @14;
  setHoldSpeedLimitWhileChangingSetSpeed @15;
  setLogJson @16;
  setLogLevel @17;
  setLogSource @18;
  setMapCurveSpeedControl @19;
  setMapCurveTargetLatA @20;
  setMapCurveUseEnableSpeed @21;
  setPressGasToAcceptSpeedLimit @22;
  setPressGasToOverrideSpeedLimit @23;
  setSlowDownForNextSpeedLimit @24;
  setSpeedLimitChangeRequiresAccept @25;
  setSpeedLimitControl @26;
  setSpeedLimitOffset @27;
  setSpeedLimitPriority @28;
  setSpeedLimitUseEnableSpeed @29;
  setSpeedUpForNextSpeedLimit @30;
  setTargetLateralAccel @31;
  setTargetSpeedAccel @32;
  setTargetSpeedJerk @33;
  setTargetSpeedTimeOffset @34;
  setVisionCurveMinTargetV @35;
  setVisionCurveSpeedControl @36;
  setVisionCurveTargetLatA @37;
  setVisionCurveUseEnableSpeed @38;
}

enum WaySelectionType {
  current @0;
  extended @1;
  fail @2;
  possible @3;
  predicted @4;
}

enum SpeedLimitOffsetType {
  percent @0;
  static @1;
}

struct MapdIn @0xc86a3d38d13eb3ef {
  bool @0 :Bool;
  float @1 :Float32;
  str @2 :Text;
  type @3 :MapdInputType;
}

enum RoadContext {
  city @0;
  freeway @1;
  unknown @2;
}

struct MapdOut @0xa4f1eb3323f5f582 {
  advisorySpeed @0 :Float32;
  distanceFromWayCenter @1 :Float32;
  estimatedRoadWidth @2 :Float32;
  hazard @3 :Text;
  lanes @4 :UInt8;
  mapCurveSpeed @5 :Float32;
  nextAdvisorySpeed @6 :Float32;
  nextAdvisorySpeedDistance @7 :Float32;
  nextHazard @8 :Text;
  nextHazardDistance @9 :Float32;
  nextSpeedLimit @10 :Float32;
  nextSpeedLimitDistance @11 :Float32;
  oneWay @12 :Bool;
  roadContext @13 :RoadContext;
  roadName @14 :Text;
  speedLimit @15 :Float32;
  speedLimitAccepted @16 :Bool;
  speedLimitSuggestedSpeed @17 :Float32;
  suggestedSpeed @18 :Float32;
  tileLoaded @19 :Bool;
  visionCurveSpeed @20 :Float32;
  wayName @21 :Text;
  wayRef @22 :Text;
  waySelectionType @23 :WaySelectionType;
}
