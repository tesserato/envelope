import builtins
import ctypes
import os
import sys

from .envelope import read_wav, save_wav

test_int = 5
cppmode = True

try:
    directory = os.path.dirname(sys.modules["signal_envelope"].__file__)
    path = os.path.join(directory, "envelope.dll")
except:
    print("Could not resolve path to 'envelope.dll'")
    cppmode = False

try:
    lib = ctypes.CDLL(path)
    try:
        if test_int != lib.test(test_int):
            cppmode = False
            print("Sent integer different from returned integer")
    except:
        print("Could not communicate with 'envelope.dll'")
except:
    print("Could not load 'envelope.dll'")
    cppmode = False

if cppmode:
    # print("Running in C++ mode (faster)")
    builtins.lib = lib
    from .envelope import get_frontiers_cpp as get_frontiers
else:
    print("Running in Python mode (with Numba)")
    from .envelope import get_frontiers_py as get_frontiers
