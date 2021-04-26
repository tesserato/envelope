import plotly.graph_objects as go
import numpy as np
from scipy.interpolate import interp1d
from scipy.signal import savgol_filter, hilbert, butter, filtfilt
import sys
from timeit import default_timer as timer
import signal_envelope as se

def butter_lowpass_filter(data, fps, cutoff = 10, order = 2):
  nyq = 0.5 * fps
  normal_cutoff = cutoff / nyq
  # Get the filter coefficients 
  b, a = butter(order, normal_cutoff, btype='low', analog=False)
  y = filtfilt(b, a, data)
  return y



name = "piano" # change the name here for any of the files in the "test_samples" folder
# name = "sinusoid"
# name = "soprano"

'''==============='''
''' Read wav file '''
'''==============='''
W, fps = se.read_wav(f"test_samples/{name}.wav")
W = W - np.average(W)
amplitude = np.max(np.abs(W))
W = W / amplitude
n = W.size
print(f"n={n}")
X = np.arange(n)

'''==============='''
'''  Present work '''
'''==============='''
start = timer()
Ex = se.get_frontiers(W, 1)
f = interp1d(Ex, np.abs(W[Ex]), kind="linear", fill_value="extrapolate", assume_sorted=False)
envY = f(X)
lms = np.average((np.abs(W) - envY * 0.5)**2)
print(f"Present work: lms ={lms}, time={timer() - start}")


'''==============='''
'''   Smoothing   '''
'''==============='''
start = timer()
envY_smooth = savgol_filter(np.abs(W), 3000 + 1, 3)
lms_smooth = np.average((np.abs(W) - envY_smooth * 0.5)**2)
print(f"Smoothing: lms ={lms_smooth}, time={timer() - start}")


'''==============='''
'''    Lowpass    '''
'''==============='''
start = timer()
envY_lowpass = butter_lowpass_filter(np.abs(W), fps)
lms_lowpass = np.average((np.abs(W) - envY_lowpass * 0.5)**2)
print(f"Lowpass:   lms ={lms_lowpass}, time={timer() - start}")


'''==============='''
'''    Hilbert    '''
'''==============='''
start = timer()
analytic_signal = np.abs(hilbert(W))
envY_hilbert = butter_lowpass_filter(analytic_signal, fps, 100)
lms_hilbert = np.average((np.abs(W) - envY_hilbert * 0.5)**2)

print(f"Hilbert:   lms ={lms_hilbert}, time={timer() - start}")
'''============================================================================'''
'''                                    PLOT                                    '''
'''============================================================================'''
FONT = dict(
    family="Latin Modern Roman",
    color="black",
    size=13.3333
  )

'''Plotting'''
fig = go.Figure()
fig.layout.template ="plotly_white" 
fig.update_layout(
  xaxis_title="<b>Sample <i>i</i></b>",
  yaxis_title="<b>Amplitude</b>",
  legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1),
  margin=dict(l=0, r=0, b=0, t=0),
  font=FONT,
  titlefont=FONT
)
fig.layout.xaxis.title.font=FONT
fig.layout.yaxis.title.font=FONT

fig.update_xaxes(showline=False, showgrid=False, zeroline=False)
fig.update_yaxes(showline=False, showgrid=False, zeroline=False)



fig.add_trace(
  go.Scatter(
    name="Signal",
    x=X,
    y=W,
    mode="lines",
    line=dict(width=.5, color="silver"),
  )
)

fig.add_trace(
  go.Scatter(
    name="Hilbert",
    x=X,
    y=envY_hilbert,
    mode="lines",
    line=dict(width=1, color="red"),
  )
)


fig.add_trace(
  go.Scatter(
    name="Smoothing",
    x=X,
    y=envY_smooth,
    mode="lines",
    line=dict(width=1, color="blue"),
  )
)

fig.add_trace(
  go.Scatter(
    name="Lowpass Filter",
    x=X,
    y=envY_lowpass,
    mode="lines",
    line=dict(width=1, color="green"),
  )
)

fig.add_trace(
  go.Scatter(
    name="Present Work",
    x=X,
    y=envY,
    mode="lines",
    line=dict(width=1, color="black"),
  )
)


fig.show(config=dict({'scrollZoom': True}))

save_name = "./images/" + sys.argv[0].split('/')[-1].replace(".py", "") + "_" + name + ".svg"
fig.write_image(save_name, width=650, height=280, engine="kaleido", format="svg")
print("saved:", save_name)