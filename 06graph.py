import numpy as np
import signal_envelope as se
from scipy import interpolate
import plotly.graph_objects as go

for file in listdir("test_samples"):
  name = file.split(".")[0]
  print(name)


  W, _ = se.read_wav(f"test_samples/{name}.wav")
  amp = np.max(np.abs(W))
  W = 4 * W / amp
  X = np.arange(W.size)
  n = W.size

  Xpos, Xneg = se.get_frontiers(W, 0)
  E = se.get_frontiers(W, 1)

  ext = (np.abs(W[E])[0], np.abs(W[E])[-1])
  f = interpolate.interp1d(E, np.abs(W[E]), kind="linear", fill_value=ext, bounds_error=False)
  E = f(X)

  for i in range(E.size):
    if E[i] < 0.1:
      E[i] = 0.1

  C = W / E

  '''============================================================================'''
  '''                                    PLOT                                    '''
  '''============================================================================'''

  fig = go.Figure()
  fig.layout.template ="simple_white"
  fig.update_layout(
    xaxis_title="<b>Sample <i>i</i></b>",
    yaxis_title="<b>Amplitude</b>",
    legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1),
    margin=dict(l=5, r=5, b=5, t=90),
    # font=dict(
    # family="Computer Modern",
    # color="black",
    # size=18
    # )
    title=dict(text=f"<b>{name}</b>", y=.97, x=0.5)
  )
  fig.update_xaxes(showline=False, showgrid=False, zeroline=False)
  fig.update_yaxes(showline=False, showgrid=False, zeroline=False)



  # , zeroline=False, showgrid=True, gridwidth=1, gridcolor='silver', tickvals=[-3,-2,-1, 0, 1,2,3,4]
  light_blue = "rgb(132, 180, 196)"

  fig.add_trace(
    go.Scattergl(
      name="Grid",
      x=[   0, n, None,
            0, n, None,
            0, n, None,
            0, n, None,
            0, n, None,
            0, n, None,
            0, n, None,
            0, n, None,
            0, n, None
      ],
      y=[ -4, -4, None,
          -3, -3, None,
          -2, -2, None,
          -1, -1, None,
          0, 0, None,
          1, 1, None,
          2, 2, None,
          3, 3, None,
          4, 4, None
      ],
      mode="lines",
      line=dict(width=1, color="silver"),
      visible = "legendonly"
    )
  )


  fig.add_trace(
    go.Scattergl(
      name="Signal (<b>w = e</b> ⊙ <b>c</b>)",
      # x=np.arange(W.size),
      y=W,
      mode="lines",
      line=dict(width=1, color=light_blue),
    )
  )

  fig.add_trace(
    go.Scattergl(
      name="Carrier (<b>c</b>)",
      # x=np.arange(W.size),
      y=C,
      mode="lines",
      line=dict(width=1, color="rgba(0,0,0,0.6)",),
    )
  )

  fig.add_trace(
    go.Scattergl(
      name="Envelope (<b>e</b>)",
      # x=E,
      y=E,
      mode="lines",
      line=dict(width=2, color="blue"),
    )
  )

  fig.add_trace(
    go.Scattergl(
      name="Positive Frontier",
      x=Xpos,
      y=W[Xpos],
      mode="lines",
      line=dict(width=2, color="red"),
      visible = "legendonly"
    )
  )

  fig.add_trace(
    go.Scattergl(
      name="Negative Frontier",
      x=Xneg,
      y=W[Xneg],
      mode="lines",
      line=dict(width=2, color="red"),
      visible = "legendonly"
    )
  )


  fig.write_html(f"site/{name}.html", full_html=True, include_mathjax="cdn", include_plotlyjs="cdn")
  fig.write_image(f"site/{name}.svg", width=1000, height=400, engine="kaleido", format="svg")

# fig.show(config=dict({'scrollZoom': True}))
