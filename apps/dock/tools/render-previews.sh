#!/bin/sh
# Renders the preview scenes into screenshots/. Needs slint-viewer (and python3 with numpy and
# pillow, to blur the backdrop). Run from the project's root:  tools/render-previews.sh
set -e
# The kit's Slint library is imported as "@malva/malva.slint"; the viewer finds it through -L.
VIEWER="${SLINT_VIEWER:-slint-viewer} -L malva=${MALVA_SLINT_DIR:-../../slint}"
OUT=${1:-screenshots}
mkdir -p "$OUT" preview/assets
[ -f preview/assets/wallpaper-light.png ] || python3 tools/make-wallpapers.py preview/assets
# 1. the desktops alone, 2. blurred, for the glass to show, 3. the scenes.
for mode in light dark; do
  comp=Desktop$(printf %s "$mode" | sed 's/^./\U&/')
  $VIEWER preview/preview.slint --component "$comp" --screenshot "preview/assets/desktop-$mode.png"
  python3 tools/make-wallpapers.py blur "preview/assets/desktop-$mode.png" "preview/assets/backdrop-$mode.png" "$mode"
  python3 tools/make-wallpapers.py blur "preview/assets/wallpaper-$mode.png" "preview/assets/wallblur-$mode.png" "$mode"
done
for scene in Light Dark Menu MenuDark Left Right Bin; do
  name=$(printf %s "$scene" | sed 's/MenuDark/menu-dark/; s/.*/\L&/')
  $VIEWER preview/preview.slint --component "Preview$scene" --screenshot "$OUT/preview-$name.png"
done
