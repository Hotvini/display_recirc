#!/bin/sh
set -e
"$(dirname "$0")/build_debug.sh"
"$(dirname "$0")/build_release.sh"
