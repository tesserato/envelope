"""
Use the algorithm directly in you Python script by copying the code below and calling the function "get_frontiers_py"
"""
import numpy as np


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


def _get_pulses(W, minPeriod=0):
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


def get_frontiers_py(W, mode=0, minPeriod=0, scaleRadiusBy=1.0):
    """
    Use this function to get the envelope of a discrete signal
    If mode == 0: Returns positive and negative indices frontiers of a signal
    If mode == 1: Returns indices of the envelope of a signal
    ignores pulses with length < minPeriod
    The radius of the circle used to determine the envelope is scaled by scaleRadiusBy
    """
    PosX, NegX = _get_pulses(W, minPeriod)
    if PosX.size == 0 or NegX.size == 0:
        print("Error: nonperiodic signal, no pulses found")
        return
    if mode == 0:
        PosFrontierX = _get_envelope(PosX, W[PosX], scaleRadiusBy)
        NegFrontierX = _get_envelope(NegX, W[NegX], scaleRadiusBy)
        return PosFrontierX, NegFrontierX
    else:
        X = np.unique(np.hstack([PosX, NegX]))
        FrontierX = _get_envelope(X, np.abs(W[X]), scaleRadiusBy)
        return FrontierX
