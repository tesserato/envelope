from os import listdir
# import sys
from timeit import default_timer as timer

import numpy as np
import plotly.graph_objects as go
from scipy.interpolate import interp1d
from scipy.signal import savgol_filter, hilbert, butter, filtfilt

import signal_envelope as se


def butter_lowpass_filter(data, fps, cutoff=10, order=2):
    nyq = 0.5 * fps
    normal_cutoff = cutoff / nyq
    # Get the filter coefficients
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    y = filtfilt(b, a, data)
    return y


def err(R, E):
    return np.average(np.abs(E - R))


def get_reference_envelope(W):
    E = np.copy(W)

    t = int(n / np.argmax(np.abs(np.fft.rfft(E))))

    Wp = np.pad(W, (0, 10 * t))
    Wp_roll = np.copy(Wp)
    conv = np.zeros(10 * t)

    for i in range(t // 2, 10 * t):
        Wp_roll = np.roll(Wp_roll, 1)
        conv[i] = np.sum(Wp_roll * Wp)

    t = int(np.argmax(conv) * 2)
    rolled_W = np.roll(np.pad(E, (t // 2, t // 2 + 2)), -t // 2)

    # print(t)
    for i in range(t):
        rolled_W = np.roll(rolled_W, 1)
        for j in range(E.size):
            E[j] = max(abs(E[j]), abs(rolled_W[t // 2 + j]))
    return E


plot = True
error = "Samples,Present Work,Smoothing,Lowpass,Hilbert\n"
time = "Samples,Reference,Present Work,Smoothing,Lowpass,Hilbert\n"

for file in listdir("test_samples"):
    name = file.split(".")[0]
    error += name
    time += name

    '''==============='''
    ''' Read wav file '''
    '''==============='''
    W, fps = se.read_wav(f"test_samples/{name}.wav")
    W = W - np.average(W)
    amplitude = np.max(np.abs(W))
    W = W / amplitude
    n = W.size
    X = np.arange(n)

    freq = np.argmax(np.abs(np.fft.rfft(W)))
    print(f"{name}: n={n}, f={freq} gf={freq * fps / n}")

    start = timer()
    E = get_reference_envelope(W)
    time += f",{timer() - start}"

    '''==============='''
    '''  Present work '''
    '''==============='''
    start = timer()
    Ex = se.get_frontiers(W, 1)
    f = interp1d(Ex, np.abs(W[Ex]), kind="linear", fill_value="extrapolate", assume_sorted=True)
    envY = f(X)
    time += f",{timer() - start}"
    lms = err(E, envY)
    print(f"Present work: lms ={lms}, time={timer() - start}")
    error += f",{lms}"

    '''==============='''
    '''   Smoothing   '''
    '''==============='''
    start = timer()
    envY_smooth = savgol_filter(np.abs(W), 3000 + 1, 3)
    time += f",{timer() - start}"
    lms_smooth = err(E, envY_smooth)
    print(f"Smoothing: lms ={lms_smooth}, time={timer() - start}")
    error += f",{lms_smooth}"

    '''==============='''
    '''    Lowpass    '''
    '''==============='''
    start = timer()
    envY_lowpass = butter_lowpass_filter(np.abs(W), fps)
    time += f",{timer() - start}"
    lms_lowpass = err(E, envY_lowpass)
    print(f"Lowpass:   lms ={lms_lowpass}, time={timer() - start}")
    error += f",{lms_lowpass}"

    '''==============='''
    '''    Hilbert    '''
    '''==============='''
    start = timer()
    analytic_signal = np.abs(hilbert(W))
    envY_hilbert = butter_lowpass_filter(analytic_signal, fps, (freq * fps / n) / 10)
    time += f",{timer() - start}\n"
    lms_hilbert = err(E, envY_hilbert)
    print(f"Hilbert:   lms ={lms_hilbert}, time={timer() - start}")
    error += f",{lms_hilbert}\n"

    if not plot:
        continue
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
    fig.layout.template = "plotly_white"
    fig.update_layout(
        xaxis_title="<b>Sample <i>i</i></b>",
        yaxis_title="<b>Amplitude</b>",
        legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1, itemsizing='constant'),
        margin=dict(l=0, r=0, b=0, t=0),
        font=FONT,
        titlefont=FONT
    )
    fig.layout.xaxis.title.font = FONT
    fig.layout.yaxis.title.font = FONT

    fig.update_xaxes(showline=False, showgrid=False, zeroline=False)
    fig.update_yaxes(showline=False, showgrid=False, zeroline=False)

    fig.add_trace(
        go.Scatter(
            name="Signal",
            x=X,
            y=W,
            mode="lines",
            line=dict(width=1, color="silver"),
        )
    )

    fig.add_trace(
        go.Scatter(
            name="Reference Envelope",
            x=X,
            y=E,
            # y= WW,
            mode="lines",
            line=dict(width=1, color="rgba(176, 107, 4, 0.7)"),
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

    # fig.show(config=dict({'scrollZoom': True}))
    fig.write_html(f"site/comparisons/{name}.html", full_html=True, include_mathjax="cdn", include_plotlyjs="cdn")
    fig.write_image(f"site/comparisons/{name}.webp", width=2000, height=800, format="webp")
    print("saved:", name, "\n")

f = open("Errors.csv", "w")
f.write(error)
f.close()

f = open("Times_seconds.csv", "w")
f.write(time)
f.close()
