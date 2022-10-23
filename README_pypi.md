[![DOI](https://zenodo.org/badge/297107471.svg)](https://zenodo.org/badge/latestdoi/297107471)

# Envelope

Python module to extract the envelope of signals.

More information at [Github](https://github.com/tesserato/envelope);

Interactive visualization [here](https://envelope.netlify.app/).

# Functions

This module implements 3 functions:

## **read_wav("path/to/mono/signal.wav")**,

Returns a tuple `(W, fps)`, where W is a numpy array and `fps` is an integer with the value of the frame rate of the
file.

## **save_wav(signal, path = "test.wav", fps = 44100)**,

Saves a NumPy array as a .wav file.

## **get_frontiers(W, mode=0)**,

### When mode=0:

Returns a tuple `(pos_ids, neg_ids)` of NumPy arrays, with the indices of the positive and negative frontiers of a NumPy
array representing a discrete wave `W`.

### When mode=1:

Returns the NumPy array `X_envelope` with the indices of the envelope of the wave `W`. Note that, to obtain the values
of the envelope at those points, `np.abs(W[X_envelope])` is recommended.

If this function is called from a compatible setup (currently, Windows 64bit), it computes the envelope and the
frontiers faster via specialized native code. A fall-back version in pure Python is also provided, in which case a
warning is printed indicating that a slower mode is being used. In both cases, the usage is the same, as are the
results.

# Usage

install the module: `pip install signal-envelope`

A minimal example would then be:

    import signal_envelope as se

    W, _ = se.read_wav("path/to/signal.wav")
    X_pos_frontier, X_neg_frontier = se.get_frontiers(W, 0)
    print(X_pos_frontier, W[X_pos_frontier])

    X_envelope = se.get_frontiers(W, 1)
    print(X_envelope, np.abs(W[X_envelope]))

A number of test wav files can be found at the [Github](https://github.com/tesserato/envelope) repository for the
project.

# Source

The code for this repository is available at [Github](https://github.com/tesserato/envelope).

# Used Libraries

[Libsndfile](http://www.mega-nerd.com/libsndfile/)
[Boost](https://www.boost.org/)
[Intel Math Kernel Library](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onemkl.html#gs.9syxj0)