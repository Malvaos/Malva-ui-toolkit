#!/bin/sh
# Renders the preview scenes into screenshots/. Needs slint-viewer, Xvfb (xvfb-run), and python3 with numpy
# and pillow (to blur the backdrop). Run from the project's root:  tools/render-previews.sh [out-dir]
set -e
# The kit's Slint library is imported as "@malva/malva.slint"; the viewer finds it through -L.
VIEWER="${SLINT_VIEWER:-slint-viewer} -L malva=${MALVA_SLINT_DIR:-../../slint}"
OUT=${1:-screenshots}
mkdir -p "$OUT" preview/assets
export SLINT_BACKEND=winit-femtovg
shot() { xvfb-run -a -s "-screen 0 1600x1000x24" $VIEWER "$1" --component "$2" --screenshot "$3"; }
python3 tools/make-sample.py
[ -f preview/assets/wallpaper-light.png ] || python3 tools/make-wallpapers.py preview/assets
# 1. the desktops alone, 2. blurred, for the glass to show, 3. the scenes.
for mode in light dark; do
  comp=Desktop$(printf %s "$mode" | sed 's/^./\U&/')
  shot preview/preview.slint "$comp" "preview/assets/desktop-$mode.png"
  python3 tools/make-wallpapers.py blur "preview/assets/desktop-$mode.png" "preview/assets/backdrop-$mode.png" "$mode"
  python3 tools/make-wallpapers.py blur "preview/assets/wallpaper-$mode.png" "preview/assets/wallblur-$mode.png" "$mode"
done
for scene in Rest RestDark FileMenu FileMenuDark SystemMenu TrayMenu LayoutMenu Wifi Bluetooth Battery BatteryLow Volume Calendar CalendarDark Control ControlDark Notifications NotificationsDark Toasts ToastsDark Osd Power PowerCountdown; do
  name=$(printf %s "$scene" | sed 's/\([a-z]\)\([A-Z]\)/\1-\2/g' | tr 'A-Z' 'a-z')
  shot preview/preview.slint "Preview$scene" "$OUT/preview-$name.png"
done
# 4. the elements sheets: every part on one page, light and dark.
shot preview/elements.slint ElementsLight "$OUT/elements-light.png"
shot preview/elements.slint ElementsDark "$OUT/elements-dark.png"
