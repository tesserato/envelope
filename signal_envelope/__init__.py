import numpy as np
import wave
import ctypes
import builtins
import os
import sys

def read_wav(path): 
  """returns signal & fps"""
  wav = wave.open(path , 'r')
  signal = np.frombuffer(wav.readframes(-1) , np.int16).astype(np.double)
  fps = wav.getframerate()
  return signal, fps

def save_wav(signal, name = 'test.wav', fps = 44100): 
  '''save .wav file to program folder'''
  o = wave.open(name, 'wb')
  o.setframerate(fps)
  o.setnchannels(1)
  o.setsampwidth(2)
  o.writeframes(np.int16(signal)) # Int16
  o.close()

test_int = 5
cppmode = True

try:
  dir = os.path.dirname(sys.modules["signal_envelope"].__file__)
  path = os.path.join(dir, "envelope.dll")
except:
  print("Could not resolve the path to 'envelope.dll'")

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
  print("Running in Python mode (significantly slower)")
  from .envelope import get_frontiers_py as get_frontiers


