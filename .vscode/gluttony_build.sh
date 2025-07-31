#!/usr/bin/env bash
set -e

build_config="Debug"
timestamp=$(date '+%Y-%m-%d--%H:%M:%S')
stage_name="Gluttony_${build_config}_${timestamp}"
STAGE_DIR="/home/mich/workspace/application_template/bin/Debug-linux-x86_64/${stage_name}"

echo "------ Clearing previous artifacts (trash at: $STAGE_DIR) ------"
mkdir -p "$STAGE_DIR"

# move all previous artifacts into staging (ignore missing)
mv "/home/mich/workspace/application_template/bin/Debug-linux-x86_64/gluttony"             "$STAGE_DIR/" 2>/dev/null || true
mv "/home/mich/workspace/application_template/bin/Debug-linux-x86_64/gluttony_editor"      "$STAGE_DIR/" 2>/dev/null || true
mv "/home/mich/workspace/application_template/bin/Debug-linux-x86_64/shaders"              "$STAGE_DIR/" 2>/dev/null || true

# trash the staging directory and Makefiles
gio trash "$STAGE_DIR" --force || true
cd "/home/mich/workspace/application_template"
find . -name "Makefile" -delete

echo "------ Regenerating Makefiles and rebuilding ------"
./vendor/premake/premake5 gmake2
gmake -j

echo "------ Done ------"
