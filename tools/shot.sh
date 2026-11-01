#!/bin/sh
# Renders one component of a .slint file to a PNG with slint-viewer under Xvfb.
#   tools/shot.sh file.slint Component out.png [WxH]
VIEWER=${SLINT_VIEWER:-slint-viewer}
export SLINT_BACKEND=${SLINT_BACKEND:-winit-femtovg}
xvfb-run -a -s "-screen 0 ${4:-1600x1200}x24" "$VIEWER" "$1" --component "$2" --screenshot "$3"
