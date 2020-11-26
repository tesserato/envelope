'''To be used AFTER signal_envelope is installed via pip'''
import sys
# sys.path.append("signal_envelope/")
import numpy as np
import signal_envelope as se
from scipy import interpolate
import plotly.graph_objects as go

name = "spoken_voice"
W, _ = se.read_wav(f"test_samples/{name}.wav")
amp = np.max(np.abs(W))
W = 4 * W / amp
X = np.arange(W.size)

Xpos, Xneg = se.get_frontiers(W, 0)
E = se.get_frontiers(W, 1)

f = interpolate.interp1d(E, np.abs(W[E]), kind="linear", fill_value="extrapolate")
E = f(X)

for i in range(E.size):
  if E[i] < 0.1:
    E[i] = 0.1

C = W / E

'''============================================================================'''
'''                                    PLOT                                    '''
'''============================================================================'''

fig = go.Figure()
fig.layout.template ="plotly_white"
fig.update_layout(
  xaxis_title="<b><i>i</i></b>",
  yaxis_title="<b>Amplitude</b>",
  legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1),
  margin=dict(l=5, r=5, b=5, t=5),
  font=dict(
  family="Computer Modern",
  color="black",
  size=18
  )
)

fig.update_xaxes(showline=False, showgrid=False, zeroline=False)
fig.update_yaxes(showline=False, zeroline=False, showgrid=True, gridwidth=1, gridcolor='silver', tickvals=[-4,-3,-2,-1, 0, 1,2,3,4])

fig.add_trace(
  go.Scatter(
    name="Signal (<b>w = e</b> ⊙ <b>c</b>)",
    # x=np.arange(W.size),
    y=W,
    mode="lines",
    line=dict(color="gray",),
  )
)

fig.add_trace(
  go.Scatter(
    name="Carrier (<b>c</b>)",
    # x=np.arange(W.size),
    y=C,
    mode="lines",
    line=dict(color="rgba(0,0,0,0.4)",),
  )
)

fig.add_trace(
  go.Scatter(
    name="Envelope (<b>e</b>)",
    # x=E,
    y=E,
    mode="lines",
    line=dict(width=2, color="blue"),
  )
)

fig.add_trace(
  go.Scatter(
    name="Positive Frontier",
    x=Xpos,
    y=W[Xpos],
    mode="lines",
    line=dict(width=1, color="red"),
    visible = "legendonly"
  )
)

fig.add_trace(
  go.Scatter(
    name="Negative Frontier",
    x=Xneg,
    y=W[Xneg],
    mode="lines",
    line=dict(width=1, color="red"),
    visible = "legendonly"
  )
)


fig.write_html(f"site/{name}.html", full_html=False, include_mathjax="cdn", include_plotlyjs="cdn")
# fig.show(config=dict({'scrollZoom': True}))
