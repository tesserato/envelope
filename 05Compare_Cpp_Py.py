'''To be used AFTER signal_envelope is installed via pip'''
import sys
sys.path.append("signal_envelope/")
import numpy as np
import signal_envelope as se


# lib = ctypes.CDLL("signal_envelope/envelope.dll")

from envelope import read_wav, get_frontiers_py

W, _ = read_wav("test_samples/brass.wav")

Xpos_py, Xneg_py = get_frontiers_py(W, 0)

Xpos_cpp, Xneg_cpp = se.get_frontiers(W, 0)

print(np.allclose(Xpos_py, Xpos_cpp), np.allclose(Xneg_py, Xneg_cpp))


XX_py = get_frontiers_py(W, 1)

XX_cpp = se.get_frontiers(W, 1)

print(np.allclose(XX_py, XX_cpp))