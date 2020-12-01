### signal-envelope ###
import numpy as np
import wave
import numpy as np
import ctypes

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


###############################
###  Python implementation  ###
###############################

def _get_circle(x0, y0, x1, y1, r):
  '''returns center of circle that passes through two points'''  
  q = np.sqrt((x1 - x0)**2 + (y1 - y0)**2)
  c = np.sqrt(r * r - (q / 2)**2)
  x3 = (x0 + x1) / 2
  y3 = (y0 + y1) / 2 
  if y0 + y1 >= 0:
    xc = x3 + c * (y0 - y1) / q
    yc = y3 + c * (x1 - x0) / q
  else:
    xc = x3 - c * (y0 - y1) / q
    yc = y3 - c * (x1 - x0) / q

  return xc, yc

def _get_pulses(W):
  '''Sorts a signal into pulses, returning positive and negative X, Y coordinates, and filtering out noise'''
  n = W.size
  sign = np.sign(W[0])
  x = 1
  while np.sign(W[x]) == sign:
    x += 1
  x0 = x + 1
  sign = np.sign(W[x0])
  posX = []
  # posY = []
  negX = []
  # negY = []
  for x in range(x0, n):
    if np.sign(W[x]) != sign: # Prospective pulse
      if x - x0 > 2:          # Not noise
        xp = x0 + np.argmax(np.abs(W[x0 : x]))
        yp = W[xp]
        if np.sign(yp) >= 0:
          posX.append(xp)
          # posY.append(yp)
        else:
          negX.append(xp)
          # negY.append(yp)
      x0 = x
      sign = np.sign(W[x])
  # return np.array(posX), np.array(posY), np.array(negX), np.array(negY)
  return np.array(posX), np.array(negX)

def _get_average_radius(X, Y):
  k_sum = 0
  for i in range(len(X) - 1):
    x = (X[i + 1] - X[i])
    y = (Y[i + 1] - Y[i])
    k = y / (x * np.sqrt(x*x + y*y))
    k_sum += k
  r = np.abs(1 / (k_sum / (len(X) - 1)))
  return r

def _get_envelope(X, Y):
  '''extracts the frontier via snowball method'''
  scaling = (np.sum(X[1:] - X[:-1]) / 2) / np.sum(Y)
  # scaling = ((X[-1] - X[0]) / (2 * (len(X) - 1))) / np.average(Y)
  Y = Y * scaling
  
  r = _get_average_radius(X, Y)
  id1 = 0
  id2 = 1
  envelope_X = [X[0]]
  n = len(X)
  while id2 < n:
    xc, yc = _get_circle(X[id1], Y[id1], X[id2], Y[id2], r)
    empty = True
    for i in range(id2 + 1, n):
      if np.sqrt((xc - X[i])**2 + (yc - Y[i])**2) < r:
        empty = False
        id2 += 1
        break
    if empty:
      envelope_X.append(X[id2])
      id1 = id2
      id2 += 1
  envelope_X = np.array(envelope_X)
  return envelope_X

def get_frontiers_py(W, mode=0):
  "If mode == 0: Returns positive and negative indices frontiers of a signal"
  "If mode == 1: Returns indices of the envelope of a signal"
  PosX, NegX = _get_pulses(W)
  if PosX.size == 0 or NegX.size == 0:
    print("Error: nonperiodic signal, no pulses found")
    return
  if mode == 0:    
    PosFrontierX = _get_envelope(PosX, W[PosX])
    NegFrontierX = _get_envelope(NegX, W[NegX])
    return PosFrontierX, NegFrontierX
  else:
    X = np.unique(np.hstack([PosX, NegX]))
    FrontierX = _get_envelope(X, np.abs(W[X]))
    return FrontierX

###############################
###  C++ implementation  ######
###############################

def get_frontiers_cpp(W, mode=0):
  if mode == 0: # Frontiers mode
    result = lib.compute_raw_envelope(W.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ctypes.c_size_t(W.size), ctypes.
    c_size_t(mode))
    if result == 1:
      print("Error: nonperiodic signal, no pulses found")
      return
    pos_n = lib.get_pos_size()
    neg_n = lib.get_neg_size()

    lib.get_pos_X.restype = np.ctypeslib.ndpointer(dtype=ctypes.c_size_t, shape=(pos_n,))
    lib.get_neg_X.restype = np.ctypeslib.ndpointer(dtype=ctypes.c_size_t, shape=(neg_n,))

    pos_X = lib.get_pos_X()
    neg_X = lib.get_neg_X()
    return np.copy(pos_X), np.copy(neg_X)

  else:        # Envelope mode
    result = lib.compute_raw_envelope(W.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ctypes.c_size_t(W.size), ctypes.c_size_t(mode))
    if result == 1:
      print("Error: nonperiodic signal, no pulses found")
      return
    pos_n = lib.get_pos_size()
    lib.get_pos_X.restype = np.ctypeslib.ndpointer(dtype=ctypes.c_size_t, shape=(pos_n,))
    pos_X = lib.get_pos_X()
    return np.copy(pos_X)



