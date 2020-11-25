# Envelope
Python module to extract the envelope of signals.

More information in this [paper](https://arxiv.org/abs/2009.02860).

# Functions

This module implements 3 functions:

**read_wav("path/to/mono/signal.wav")**, that returns a tuple (W, fps), where W is a numpy array and fps is an int with the value of the frame rate of the file.

**save_wav(signal, path = "test.wav", fps = 44100)**, that saves a numpy array as a .wav file.

**get_frontiers(W)**, that returns a tuple (pos_ids, neg_ids) with the indices of the positive and negative frontiers of a numpy array representing the samples of a discrete function.
If this function is called from a compatible setup (currently, Windows 64bit), it computes the frontiers faster via specialized native code. A fall-back version in pure Python is also provided, in which case a warning is printed indicating that a slower mode is being used. In both cases, the usage is the same, as are the results.

# Usage
install the module: `pip install signal-envelope`

A minimal example would then be:

    import signal_envelope as se

    W, _ = se.read_wav("path/to/signal.wav")
    Xpos, Xneg = se.get_frontiers(W)
    print(Xpos, W[Xpos])

A number of test wav files can be found at the Github repository for the project.

# Source
The code for this repository (except, for now, the C++ source code of the envelope.dll) is available at [Github](https://github.com/tesserato/envelope).