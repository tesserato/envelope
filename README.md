[![DOI](https://zenodo.org/badge/297107471.svg)](https://zenodo.org/badge/latestdoi/297107471)

# Quick start

Module to extract the envelope of signals.

Interactive visualizations [here](https://envelope.netlify.app/).

More details can be found in [this paper](https://doi.org/10.1016/j.dsp.2021.103229).

A command line application for Win64 machines can be found in the releases section. For usage info run the executable
with the -h flag.
A Python module implementing the same functionality can be found [here](https://pypi.org/project/signal-envelope/).
A number of test Wav files can be found at the /test_samples folder.

# Documentation

## C++

The C++ documentation is more extensive and can be found directly in the source code
or [here](https://tesserato.github.io/envelope/html/index.html).

## Python

The Python implementation consists of a thin wrapper around the DLL, used in 64 bits Windows 10 machines and a native
implementation, used in other systems. The native Python implementation completely mirrors the C++ implementation, and
is documented via comments in
the [source code](https://github.com/tesserato/envelope/blob/master/signal_envelope/envelope.py).

Usage information can be found [here](https://pypi.org/project/signal-envelope/).

Python implementation is accelerated via [Numba](https://pypi.org/project/numba/).

# Used Libraries

[Libsndfile](http://www.mega-nerd.com/libsndfile/)

[Boost](https://www.boost.org/)

[Intel Math Kernel Library](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onemkl.html#gs.9syxj0)