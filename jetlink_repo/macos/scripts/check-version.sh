#!/usr/bin/env bash
#
# Check that a release tag matches the package version.
#
#   scripts/check-version.sh v0.2.0
#
# The release workflow runs this before it builds anything, so a tag that does
# not match jetlink.__version__ fails in a second instead of after a
# notarization. pyproject.toml reads its version from there too.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"

TAG="${1:-}"
[ -n "$TAG" ] || { echo "usage: check-version.sh vX.Y.Z" >&2; exit 1; }

TAG_VERSION="${TAG#v}"
SOURCE="$REPO_ROOT/jetlink/__init__.py"
[ -f "$SOURCE" ] || { echo "error: no jetlink/__init__.py at $SOURCE" >&2; exit 1; }

# The first `__version__ = '...'`, in either quote. Plain sed rather than
# importing the package so this runs with nothing installed.
PROJECT_VERSION="$(sed -n -E "s/^__version__[[:space:]]*=[[:space:]]*['\"]([^'\"]+)['\"].*/\1/p" "$SOURCE" | head -n 1)"
[ -n "$PROJECT_VERSION" ] || { echo "error: no __version__ in $SOURCE" >&2; exit 1; }

if [ "$TAG_VERSION" != "$PROJECT_VERSION" ]; then
  echo "error: tag and package version disagree" >&2
  echo "  tag                  $TAG (version $TAG_VERSION)" >&2
  echo "  jetlink/__init__.py  $PROJECT_VERSION" >&2
  exit 1
fi

echo "$TAG matches jetlink.__version__ $PROJECT_VERSION"
