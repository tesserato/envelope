'''To be used AFTER signal_envelope is installed via pip'''
import sys

sys.path.append("signal_envelope/")
import numpy as np
import signal_envelope as se

# lib = ctypes.CDLL("signal_envelope/envelope.dll")

from envelope import read_wav, get_frontiers_py

W, _ = read_wav("test_samples/piano.wav")

Xpos_py, Xneg_py = get_frontiers_py(W, 0)

Xpos_cpp, Xneg_cpp = se.get_frontiers(W, 0)

try:
    print(np.allclose(Xpos_py, Xpos_cpp))
except:
    print(f"Xpos_py != Xpos_cpp:")
    print(f"Xpos_py size = {Xpos_py.size}, Xpos_cpp size = {Xpos_cpp.size}\n")

try:
    print(np.allclose(Xneg_py, Xneg_cpp))
except:
    print(f"Xneg_py != Xneg_cpp")
    print(f"Xneg_py size = {Xneg_py.size}, Xneg_cpp size = {Xneg_cpp.size}\n")

XX_py = get_frontiers_py(W, 1)
XX_cpp = se.get_frontiers(W, 1)

try:
    print(np.allclose(XX_py, XX_cpp))
except:
    print(f"XX_py != XX_cpp")
    print(f"XX_py size = {XX_py.size}, XX_cpp size = {XX_cpp.size}\n")
