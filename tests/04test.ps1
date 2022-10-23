python -m venv 0testenv         # create virtual environment
.\0testenv\Scripts\Activate.ps1 #activate it
pip list
pip install numpy
pip install signal-envelope
pip install plotly
pip list
python 05graph_single_wav.py
deactivate