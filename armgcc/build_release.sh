#!/bin/sh
set -e
cmake --preset release
cmake --build --preset release
