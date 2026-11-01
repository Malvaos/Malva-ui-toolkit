"""A real pointer for an X server, through the XTest extension (libX11 and libXtst, by ctypes: no
packages to install). Used to drive the dock under Xvfb the way a person would."""
import ctypes
import math
import time

_x11 = ctypes.CDLL("libX11.so.6")
_xtst = ctypes.CDLL("libXtst.so.6")
_x11.XOpenDisplay.restype = ctypes.c_void_p
_x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
_x11.XFlush.argtypes = [ctypes.c_void_p]
_x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
_xtst.XTestFakeMotionEvent.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_ulong]
_xtst.XTestFakeButtonEvent.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_int, ctypes.c_ulong]

LEFT, MIDDLE, RIGHT = 1, 2, 3


def ease(t):
    """A hand's movement: slow start, slow stop."""
    return t * t * (3 - 2 * t)


class Pointer:
    def __init__(self, display=None):
        self.display = _x11.XOpenDisplay(display.encode() if display else None)
        if not self.display:
            raise RuntimeError("cannot open the X display")
        self.x = 0.0
        self.y = 0.0

    def close(self):
        _x11.XCloseDisplay(self.display)

    def move(self, x, y):
        self.x, self.y = float(x), float(y)
        _xtst.XTestFakeMotionEvent(self.display, -1, int(round(x)), int(round(y)), 0)
        _x11.XFlush(self.display)

    def button(self, which, down):
        _xtst.XTestFakeButtonEvent(self.display, which, 1 if down else 0, 0)
        _x11.XFlush(self.display)

    def click(self, which=LEFT, hold=0.06):
        self.button(which, True)
        time.sleep(hold)
        self.button(which, False)

    def glide(self, x, y, seconds, hz=100, easing=ease):
        """Moves in a smooth line, in real time."""
        x0, y0 = self.x, self.y
        steps = max(1, int(seconds * hz))
        start = time.monotonic()
        for k in range(1, steps + 1):
            t = easing(k / steps)
            self.move(x0 + (x - x0) * t, y0 + (y - y0) * t)
            wait = start + seconds * k / steps - time.monotonic()
            if wait > 0:
                time.sleep(wait)
