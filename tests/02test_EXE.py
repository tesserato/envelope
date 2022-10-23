import subprocess
import wave

import numpy as np
import plotly.graph_objects as go


def read_wav(path):
    """returns signal & fps"""
    wav = wave.open(path, 'r')
    signal = np.frombuffer(wav.readframes(-1), np.int16).astype(np.double)
    fps = wav.getframerate()
    return signal, fps


name = "alto"

W, _ = read_wav("test_samples/alto.wav")
subprocess.call(f"executable/envelope.exe -h")
subprocess.call(f"executable/envelope.exe test_samples/{name}.wav")
subprocess.call(f"executable/envelope.exe test_samples/{name}.wav -f")

E = np.genfromtxt(f"test_samples/{name}_E.csv", delimiter=",", dtype=int)
P = np.genfromtxt(f"test_samples/{name}_P.csv", delimiter=",", dtype=int)
N = np.genfromtxt(f"test_samples/{name}_N.csv", delimiter=",", dtype=int)

'''============================================================================'''
'''                                    PLOT                                    '''
'''============================================================================'''

fig = go.Figure()
fig.layout.template = "plotly_white"
fig.update_layout(
    xaxis_title="x",
    yaxis_title="Amplitude",
    legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1),
    margin=dict(l=5, r=5, b=5, t=5),
    font=dict(
        family="Computer Modern",
        color="black",
        size=18
    )
)

fig.add_trace(
    go.Scatter(
        name="Signal",
        # x=np.arange(W.size),
        y=W,
        mode="lines",
        line=dict(color="silver", ),
    )
)

fig.add_trace(
    go.Scatter(
        name="Positive Frontier",
        x=P,
        y=W[P],
        mode="lines",
        line=dict(width=1, color="red"),
    )
)

fig.add_trace(
    go.Scatter(
        name="Negative Frontier",
        x=N,
        y=W[N],
        mode="lines",
        line=dict(width=1, color="red"),
    )
)

fig.add_trace(
    go.Scatter(
        name="Envelope",
        x=E,
        y=np.abs(W[E]),
        mode="lines",
        line=dict(width=1, color="blue"),
    )
)

fig.show(config=dict({'scrollZoom': True}))
