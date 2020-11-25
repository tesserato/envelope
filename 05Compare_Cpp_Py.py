'''To be used AFTER signal_envelope is installed via pip'''
import sys
sys.path.append("signal_envelope/")
import numpy as np
import signal_envelope as se


# lib = ctypes.CDLL("signal_envelope/envelope.dll")

from envelope import read_wav, get_frontiers_cpp, get_frontiers_py

W, _ = read_wav("test_samples/piano33.wav")

Xpos_py, Xneg_py = get_frontiers_py(W)

Xpos_cpp, Xneg_cpp = se.get_frontiers(W)

print(np.allclose(Xpos_py, Xpos_cpp), np.allclose(Xneg_py, Xneg_cpp))