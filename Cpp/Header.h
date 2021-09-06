/** @file 
This file contains the declarations and definitions used both in the command line application and the DLL used in the Python module.
Templates are used whenever possible, to streamline the code.
*/
#pragma once
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <chrono> 
#include <fstream>
#include <algorithm>
#include <tuple>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <filesystem>
/** \file sndfile.hh wav in and out, used to read wav files  **/
#include <sndfile.hh>
/** \file mkl_dfti.h Intel MKL, used in the Fourier Transform **/
#include "mkl_dfti.h" 


typedef unsigned long long pint  /** A positive integer **/;
typedef long long          inte  /** An integer **/;
typedef double             real  /** A real number **/;
typedef std::vector<pint> v_pint /** A vector of positive integers **/;
typedef std::vector<inte> v_inte /** A vector of integers **/;
typedef std::vector<real> v_real /** A vector of real numbers **/;


/** Writes a vector to disk as a CSV file. **/
template <typename T> void write_vector(
	const std::vector<T>& V /** Vector to be written. type T must be compatible with "<<" operator */, 
	std::string path = "test.csv" /** Path to save the CSV file. Defaults to "test.csv" */
) 
{
	std::ofstream out(path);
	for (pint i = 0; i < V.size() - 1; ++i) {
		out << V[i] << ",";
	}
	out << V.back();
	out.close();
}


/** Returns the sign of a number **/
template <typename T> inte sgn(T val /** A number (float, double, int, etc) **/) {
	return (T(0) < val) - (val < T(0))/** -1, 0 or 1, representing the sign **/;
}

/** Given a vector, returns the index of its maximum absolute value between two indices **/
template <typename T> inte argabsmax(
	const std::vector<T>& V /** Vector of numbers. **/,
	const inte x0 /** Initial index **/,
	const inte x1 /** Final index **/
) 
{
	real max{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i < x1; i++) {
		if (std::abs(V[i]) > max) {
			max = std::abs(V[i]);
			idx = i;
		}
	}
	return idx /** x0 <= idx < x1, index of the maximum absolute value of vector V between x0 and x1. **/;
}

/** Simple struct to represent the x and y coordinates of a point. **/
struct point {
	real x /** x coordinate */;
	real y /** y coordinate */;
};


/** This class represents a mono WAV file. **/
class Wav {
public:
	pint fps;
	v_real W;
	/** Constructor to manually create a WAV file. **/
	Wav(v_real W_ /** Vector of real values between -1.0 and 1.0 representing the samples. */,
		pint fps_=44100 /** Sampling rate, in frames per second. Defaults to 44100 */) {
		fps = fps_ ;
		W = W_ ;
		std::cout << "New wave with n=" << W.size() << " and fps=" << fps << "\n";
	}
	/** Writes the Wav object to disk, as a mono WAV file. **/
	void write(std::string path = "test.wav" /** Path to save the WAV file. Defaults to "test.wav" */) {
		if (W.size() == 0) {
			std::cout << "size = 0";
			return;
		}
		const char* fname = path.c_str();
		SF_INFO sfinfo;
		sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		sfinfo.channels = 1;
		sfinfo.samplerate = fps;

		SNDFILE* outfile = sf_open(fname, SFM_WRITE, &sfinfo);
		sf_write_double(outfile, &W[0], W.size());
		sf_close(outfile);
		std::cout << "Wave file written. path=" << path << ", fps=" << fps << "\n";
	}
};


/** Reads a mono WAV file from disk, returning a Wav object. **/
Wav read_wav(std::string path /** Path to file. **/) {
	const char* fname = path.c_str();
	SF_INFO sfinfo;
	sfinfo.format = 0;
	SNDFILE* test = sf_open(fname, SFM_READ, &sfinfo);

	if (test == NULL) {
		std::cout << "Couldn't open file at:" << fname;
		std::cout << "\n" << sf_strerror(test);
		sf_close(test);
		exit(1);
	}
	sf_close(test);

	SndfileHandle file = SndfileHandle(fname);

	if (file.channels() != 1) {
		std::cout << "Unexpected number of channels:" << file.channels();
		exit(1);
	}

	const int fps = file.samplerate();
	const int n = file.frames();
	std::cout << "Successfully opened file at:" << path << "\n";

	v_real W(n);
	file.read(&W[0], n);
	return Wav(W, fps) /** Returns a Wav object **/;
};


/** Given the coordinates of two points and a radius, returns the center of the circle that passes through the points and possesses the given radius.**/
point get_circle(
	real x0 /** x coordinate of the first point **/,
	real y0 /** y coordinate of the first point **/,
	real x1 /** x coordinate of the second point **/,
	real y1 /** y coordinate of the second point **/,
	real r /** radius of the circle **/
) 
{
	real q{ sqrt(pow(x1 - x0, 2.0) + pow(y1 - y0, 2.0)) };
	real c{ sqrt(r * r - pow(q / 2.0, 2.0)) };

	real x3{ (x0 + x1) / 2.0 };
	real y3{ (y0 + y1) / 2.0 };
	real xc, yc;

	if (y0 + y1 >= 0) {
		xc = x3 + c * (y0 - y1) / q;
		yc = y3 + c * (x1 - x0) / q;
	}
	else {
		xc = x3 - c * (y0 - y1) / q;
		yc = y3 - c * (x1 - x0) / q;
	}
	return { xc, yc } /** Returns a point representing the center of the circle. **/;
}


/** Given a vector, returns the indices of the absolute maximum values of the positive and negative pulses. **/
void get_pulses(
	const v_real& W /** Vector representing the signal. **/	,
	v_pint& posX /** Vector to be filled with the positive indices. **/,
	v_pint& negX /** Vector to be filled with the negative indices. **/
) 
{
	pint n{ W.size() };
	inte sign{ sgn(W[0]) };
	inte i0{ 0 };
	inte im{ 0 };
	posX.clear();
	negX.clear();
	for (pint i = 1; i < W.size(); i++) {
		if (sgn(W[i]) != sign) {
			sign = sgn(W[i]);
			if (i - i0 > 4) {
				im = argabsmax(W, i0, i);
				i0 = i;
				if (sgn(W[im]) >= 0) {
					posX.push_back(im);
				}
				else {
					negX.push_back(im);
				}
			}
		}
	}	return;
}

/** Given a vector representing the signal and a vector of the indices of its positive or negative pulses, returns the corresponding frontier. **/
v_pint get_frontier(
	const v_real& W /** Vector representing the signal. **/,
	const v_pint& X /** Vector of positive or negative pulses indices. **/
) 
{
	pint n{ X.size() };
	real sumY{ 0.0 };
	real sumY_vec{ W[X[n-1]] - W[X[0]] };
	pint sumX_vec{ X[n-1] - X[0] };

	for (pint i = 0; i < n; i++) {
		sumY += W[X[i]];
	}
	real scaling{ (sumX_vec / 2.0) / sumY };
	real sumk{ 0.0 };
	real x;
	real y;

	v_real Y(n);
	Y[0] = W[X[0]] * scaling;
	for (pint i = 1; i < n; i++) {
		Y[i] = W[X[i]] * scaling;
		x = X[i] - X[i - 1];
		y = Y[i] - Y[i - 1];
		sumk += y / (x * sqrt(x * x + y * y));
	}
	real r{ 1.0 / (sumk / (n - 1)) };
	real rr{ r * r };
	pint idx1{ 0 };
	pint idx2{ 1 };
	v_pint frontierX = { X[0] };
	point pc;
	bool empty;

	while (idx2 < n) {
		pc = get_circle(X[idx1], Y[idx1], X[idx2], Y[idx2], r);
		empty = true;
		for (pint i = idx2 + 1; i < n; i++) {
			if (pow(pc.x - X[i], 2.0) + pow(pc.y - Y[i], 2.0) < rr) {
				empty = false;
				idx2 ++;
				break;
			}
		}
		if (empty) {
			frontierX.push_back(X[idx2]);
			idx1 = idx2;
			idx2 ++;
		}
	}
	return frontierX /** Vector of positive or negative frontier indices. **/;
}





