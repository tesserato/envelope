"""Test numba implementation of envelope.get_frontiers_py against pure python implementation"""
import timeit

import numpy as np

from signal_envelope.envelope import get_frontiers_py, read_wav

# Load signal
W, _ = read_wav("../test_samples/alto.wav")

# Use the original python implementation
Xpos_py, Xneg_py = get_frontiers_py(W, use_numba=False)

# Use the numba implementation
Xpos_numba, Xneg_numba = get_frontiers_py(W)

# Compare results
assert np.allclose(Xpos_py, Xpos_numba)
assert np.allclose(Xneg_py, Xneg_numba)

n_times_py = 3
n_times_numba = 1_000
# Time pure python implementation
t_py = timeit.timeit(
    "get_frontiers_py(W, use_numba=False)",
    setup="from __main__ import get_frontiers_py, W",
    number=n_times_py,
) / n_times_py

# Time numba implementation
t_numba = timeit.timeit(
    "get_frontiers_py(W)",
    setup="from __main__ import get_frontiers_py, W",
    number=n_times_numba,
) / n_times_numba

print(f"Pure python implementation: {t_py:.3f} s")
print(f"Numba implementation: {t_numba:.3f} s")
