"""Assert that all functions are effectively using no python.
We don't set `nopython=True` in the decorators because we don't want to introduce possible bugs in production.
But for debugging and testing, we want to make sure that the functions are effectively using no python.
"""
from numba import njit

from signal_envelope.envelope import _get_envelope, _get_average_radius, _get_pulses, _get_circle


def test_get_frontiers_py():
    # Try to compile the function with nopthon=True
    njit(_get_envelope)
    njit(_get_average_radius)
    njit(_get_pulses)
    njit(_get_circle)
