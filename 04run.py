import numpy as np
from signal_envelope import read_wav, get_frontiers

W, _ = read_wav("./test_samples/piano33.wav")

Xpos, Xneg = get_frontiers(W)

print(Xpos[0 : 10])


'''============================================================================'''
'''                                    PLOT                                    '''
'''============================================================================'''

import plotly.graph_objects as go
fig = go.Figure()
fig.layout.template ="plotly_white"
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
    x=np.arange(W.size),
    y=W,
    mode="lines",
    line=dict(color="silver",),
  )
)

fig.add_trace(
  go.Scatter(
    name="+Frontier",
    x=Xpos,
    y=np.abs(W[Xpos]),
    mode="lines",
    line=dict(width=1, color="red"),
  )
)

fig.add_trace(
  go.Scatter(
    name="-Frontier",
    x=Xneg,
    y=W[Xneg],
    mode="lines",
    line=dict(width=1, color="red"),
  )
)

fig.show(config=dict({'scrollZoom': True}))
