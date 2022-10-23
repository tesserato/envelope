"""Test of the main envelope algorithm"""
from unittest import TestCase


class Test(TestCase):
    def test_get_frontiers_py(self):
        """Test of the main envelope algorithm"""
        from signal_envelope import read_wav, get_frontiers
        W, _ = read_wav("test_samples/alto.wav")
        get_frontiers(W)
        get_frontiers(W, 1)
