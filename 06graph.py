'''To be used AFTER signal_envelope is installed via pip'''
import sys
sys.path.append("signal_envelope/")
import numpy as np
import signal_envelope as se
import plotly.graph_objects as go




W, _ = se.read_wav("test_samples/piano33.wav")
Xpos, Xneg = se.get_frontiers(W)



fig = go.Figure()
fig.layout.template ="plotly_white"
fig.update_layout(
  xaxis_title="i",
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
    x=np.arange(W.size),
    y=W,
    mode="lines",
    line=dict(color="silver",),
  )
)

fig.add_trace(
  go.Scatter(
    name="Positive Frontier",
    x=Xpos,
    y=np.abs(W[Xpos]),
    mode="lines",
    line=dict(width=1, color="red"),
  )
)

fig.add_trace(
  go.Scatter(
    name="Negative Frontier",
    x=Xneg,
    y=W[Xneg],
    mode="lines",
    line=dict(width=1, color="red"),
  )
)

fig.show(config=dict({'scrollZoom': True}))
