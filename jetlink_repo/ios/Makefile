# Jetlink for iPhone and iPad.
#
#   make project   regenerate Jetlink.xcodeproj from project.yml (needs xcodegen)
#   make build     compile for a device without signing (needs Xcode's iOS platform)
#   make typecheck compile and link the app's sources for iOS through SwiftPM,
#                  which needs only the SDK Xcode ships with
#   make open      open the project in Xcode; pick a team, then run on a phone
#   make testflight archive, upload to App Store Connect and wait while Apple
#                  processes it (scripts/testflight.sh lists what it needs)
SHELL := /usr/bin/env bash
.SHELLFLAGS := -euo pipefail -c

IOS_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD := $(IOS_DIR)/build

.PHONY: project build typecheck open testflight clean

project:
	JETLINK_VERSION=0.0.0 JETLINK_BUILD=1 xcodegen generate --spec $(IOS_DIR)/project.yml --project $(IOS_DIR)

build: project
	xcodebuild build -project $(IOS_DIR)/Jetlink.xcodeproj -scheme Jetlink \
	  -destination 'generic/platform=iOS' -derivedDataPath $(BUILD)/DerivedData \
	  CODE_SIGNING_ALLOWED=NO

typecheck:
	$(IOS_DIR)/scripts/typecheck.sh

open: project
	open $(IOS_DIR)/Jetlink.xcodeproj

testflight:
	$(IOS_DIR)/scripts/testflight.sh

clean:
	rm -rf $(BUILD)
