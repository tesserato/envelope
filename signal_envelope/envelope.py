"""Main functions."""
import ctypes
import wave
from typing import Union, Tuple

import numpy as np
from numba import jit

Envelope_None = np.array([-1], dtype=np.int64)

def read_wav(path):
    """Reads a mono WAV file from disk, returning a signal as a NumPy array and fps"""
    wav = wave.open(path, 'r')
    signal = np.frombuffer(wav.readframes(-1), np.int16).astype(np.double)
    fps = wav.getframerate()
    return signal, fps


def save_wav(signal, name='test.wav', fps=44100):
    """save .wav file to program folder"""
    o = wave.open(name, 'wb')
    o.setframerate(fps)
    o.setnchannels(1)
    o.setsampwidth(2)
    o.writeframes(np.int16(signal))  # Int16
    o.close()


###############################
#    Python implementation    #
###############################

@jit()
def _get_circle(x0, y0, x1, y1, r):
    """Given the coordinates of two points and a radius, returns the center of the circle that passes through the points and possesses the given radius."""
    q = np.sqrt((x1 - x0) ** 2 + (y1 - y0) ** 2)
    c = np.sqrt(r * r - (q / 2) ** 2)
    x3 = (x0 + x1) / 2
    y3 = (y0 + y1) / 2
    if y0 + y1 >= 0:
        xc = x3 + c * (y0 - y1) / q
        yc = y3 + c * (x1 - x0) / q
    else:
        xc = x3 - c * (y0 - y1) / q
        yc = y3 - c * (x1 - x0) / q

    return xc, yc


@jit()
def _get_pulses(W, minPeriod=4):
    """Given a vector, returns the indices of the absolute maximum values of the positive and negative pulses."""
    sign = np.sign(W[0])
    posX = []
    negX = []
    x0 = 0
    for x in range(1, W.size):
        if np.sign(W[x]) != sign:  # Prospective pulse
            sign = np.sign(W[x])
            if x - x0 > minPeriod:  # Not noise
                xp = x0 + np.argmax(np.abs(W[x0: x]))
                x0 = x
                if np.sign(W[xp]) >= 0:
                    posX.append(xp)
                else:
                    negX.append(xp)
    return np.array(posX), np.array(negX)


@jit()
def _get_average_radius(X, Y):
    """Gets the average radius of pulses described by X, Y"""
    k_sum = 0
    for i in range(len(X) - 1):
        x = (X[i + 1] - X[i])
        y = (Y[i + 1] - Y[i])
        k = y / (x * np.sqrt(x * x + y * y))
        k_sum += k
    r = np.abs(1 / (k_sum / (len(X) - 1)))
    return r


@jit()
def _get_envelope(X, Y, scaleRadiusBy=1.0):
    """Extracts the envelope of pulses described by X, Y"""
    scaling = ((X[-1] - X[0]) / 2) / np.sum(Y)
    Y = Y * scaling

    r = _get_average_radius(X, Y) * scaleRadiusBy
    id1 = 0
    id2 = 1
    envelope_X = [X[0]]
    n = len(X)
    while id2 < n:
        xc, yc = _get_circle(X[id1], Y[id1], X[id2], Y[id2], r)
        empty = True
        for i in range(id2 + 1, n):
            if np.sqrt((xc - X[i]) ** 2 + (yc - Y[i]) ** 2) < r:
                empty = False
                id2 += 1
                break
        if empty:
            envelope_X.append(X[id2])
            id1 = id2
            id2 += 1
    envelope_X = np.array(envelope_X)
    return envelope_X


def get_frontiers_py(W: np.ndarray, mode: int = 0, minPeriod: int = 0,
                     scaleRadiusBy: float = 1.0, use_numba: bool = True) -> Union[Tuple[np.ndarray, np.ndarray], np.ndarray, None]:
    """
    Use this function to get the envelope of a discrete signal
    :param W: the signal
    :param mode: If mode == 0: Returns positive and negative indices frontiers of a signal.
                 If mode == 1: Returns indices of the envelope of a signal
    :param minPeriod: Ignores pulses with length < minPeriod
    :param scaleRadiusBy: The radius of the circle used to determine the envelope is scaled by scaleRadiusBy
    :return: If mode == 0: Returns positive and negative indices frontiers of a signal.
             If mode == 1: Returns indices of the envelope of a signal

    Note: This function is not faster with numba, and numba doesn't support Union of return types.
    """
    PosX, NegX = _get_pulses(W, minPeriod) if use_numba else _get_pulses.py_func(W, minPeriod)
    if PosX.size == 0 or NegX.size == 0:
        print("Error: nonperiodic signal, no pulses found")
        return
    if mode == 0:
        PosFrontierX = _get_envelope(PosX, W[PosX], scaleRadiusBy) if use_numba else _get_envelope.py_func(PosX, W[PosX], scaleRadiusBy)
        NegFrontierX = _get_envelope(NegX, W[NegX], scaleRadiusBy) if use_numba else _get_envelope.py_func(NegX, W[NegX], scaleRadiusBy)
        return PosFrontierX, NegFrontierX
    else:
        X = np.unique(np.hstack((PosX, NegX)))
        FrontierX = _get_envelope(X, np.abs(W[X]), scaleRadiusBy) if use_numba else _get_envelope.py_func(X, np.abs(W[X]), scaleRadiusBy)
        return FrontierX


###############################
#      C++ implementation     #
###############################

def get_frontiers_cpp(W, mode=0):
    """Uses the functions exposed by the DLL to extract the envelope of a Wav file faster """
    if mode == 0:  # Frontiers mode
        result = lib.compute_raw_envelope(W.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ctypes.c_size_t(W.size),
                                          ctypes.
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

    else:  # Envelope mode
        result = lib.compute_raw_envelope(W.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), ctypes.c_size_t(W.size),
                                          ctypes.c_size_t(mode))
        if result == 1:
            print("Error: nonperiodic signal, no pulses found")
            return
        pos_n = lib.get_pos_size()
        lib.get_pos_X.restype = np.ctypeslib.ndpointer(dtype=ctypes.c_size_t, shape=(pos_n,))
        pos_X = lib.get_pos_X()
        return np.copy(pos_X)
