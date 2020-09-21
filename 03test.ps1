python -m venv 0testenv         # create virtual environment
.\0testenv\Scripts\Activate.ps1 #activate it
pip list
pip install signal-envelope
pip list
python 04run.py # brass: [226 231 259 266 456 476 ...]
deactivate