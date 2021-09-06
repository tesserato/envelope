#pragma once
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <chrono> 
#include <fstream>
#include <algorithm>
#include <sndfile.hh> // Wav in and out
#include "mkl_dfti.h" // Intel MKL
#include <tuple>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <filesystem> // to list all files in a directory


typedef unsigned long long pint;
typedef long long          inte;
typedef double             real;

typedef std::vector<pint> v_pint;
typedef std::vector<inte> v_inte;
typedef std::vector<real> v_real;


const real PI = 3.14159265358979323846;

template <typename T> void write_vector(const std::vector<T>& V, std::string path = "teste.csv") {
	std::ofstream out(path);
	for (pint i = 0; i < V.size() - 1; ++i) {
		out << V[i] << ",";
	}
	out << V.back();
	out.close();
}

static bool abs_compare(real a, real b) {
	return (std::abs(a) < std::abs(b));
}

struct point {
	real x;
	real y;
};

struct pulse {
	pint start;
	pint end;
	pulse(inte s, inte e) {
		if (s < 0) {
			std::cout << "negative start for Pulse\n";
		}
		if (e < 0) {
			std::cout << "negative end for Pulse\n";
		}
		start = s;
		end = e;
	}
};

class Wav {
public:
	pint fps;
	v_real W;

	Wav(v_real W_, pint fps_=44100) {
		fps = fps_;
		W = W_;
		std::cout << "New wave with n=" << W.size() << " and fps=" << fps << "\n";
	}
	void write(std::string path = "test.wav") {
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



void smooth_XPCs(v_inte& Xp) {

	v_real T(Xp.size() - 1);
	for (pint i = 0; i < Xp.size() - 1; i++) {
		T[i] = Xp[i + 1] - Xp[i];
	}


	/* Configure a Descriptor */
	std::vector<std::complex<double>> out(T.size() / 2 + 1);

	//write_vector(in, "00before.csv");
	DFTI_DESCRIPTOR_HANDLE hand;
	MKL_LONG status;
	status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_REAL, 1, T.size());
	status = DftiSetValue(hand, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	status = DftiSetValue(hand, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);
	status = DftiSetValue(hand, DFTI_BACKWARD_SCALE, 1.0f / T.size());
	//status = DftiSetValue(hand, DFTI_PACKED_FORMAT, DFTI_CCE_FORMAT);

	//status = DftiSetValue(hand, DFTI_INPUT_STRIDES, <real_strides>);
	//status = DftiSetValue(hand, DFTI_OUTPUT_STRIDES,<complex_strides>);
	status = DftiCommitDescriptor(hand);
	/* Compute an FFT */
	status = DftiComputeForward(hand, T.data(), out.data());

	float avg{ 0.0 };
	for (size_t i = 0; i < out.size(); i++) {
		avg += std::abs(out[i]);
	}

	avg = avg / out.size();

	float dev{ 0.0 };
	for (size_t i = 0; i < out.size(); i++) {
		dev += std::abs(std::abs(out[i]) - avg);
	}

	dev = dev / out.size();


	for (size_t i = 0; i < out.size(); i++) {
		if (std::abs(out[i]) <= avg) {
			out[i] = { 0.0,0.0 };
		}
	}

	/* Compute an inverse FFT */
	//std::fill(in.begin(), in.end(), 0.0);
	status = DftiComputeBackward(hand, out.data(), T.data());
	DftiFreeDescriptor(&hand);
	//write_vector(in, "01after.csv");

	for (pint i = 0; i < Xp.size() - 1; i++) {
		Xp[i + 1] = Xp[i] + std::round(T[i]);
	}
}


Wav read_wav(std::string path) {
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
	return Wav(W, fps);
};

point get_circle(real x0, real y0, real x1, real y1, real r) {
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
	return { xc, yc };
}

template <typename T> inte sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

inte argabsmax(const v_real& V, const inte x0, const inte x1) {
	real max{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i < x1; i++) {
		if (std::abs(V[i]) > max) {
			max = std::abs(V[i]);
			idx = i;
		}
	}
	return idx;
}

inte argmax(const v_real& V, const inte x0, const inte x1) {
	real max{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i <= x1; i++) {
		if (V[i] > max) {
			max = V[i];
			idx = i;
		}
	}
	return idx;
}

inte argmin(const v_real& V, const inte x0, const inte x1) {
	real min{ std::abs(V[x0]) };
	inte idx = x0;
	for (pint i = x0; i <= x1; i++) {
		if (V[i] < min) {
			min = V[i];
			idx = i;
		}
	}
	return idx;
}

v_pint find_zeroes(const v_real& W) {
	inte sign{ sgn(W[0]) };
	v_pint id_of_zeroes;

	for (pint i = 0; i < W.size(); i++) {
		if (sgn(W[i]) != sign) {
			id_of_zeroes.push_back(i);
			sign = sgn(W[i]);
		}
	}
	return id_of_zeroes;
}

void get_pulses(const v_real& W, v_pint& posX, v_pint& negX) {
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

v_pint get_frontier(const v_real& W, const v_pint& X) {
#ifdef v
	std::cout << "inside get_frontier\n";
#endif
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
	return frontierX;
}

v_inte get_Xpcs(const v_pint& Xpos, const v_pint& Xneg) {
	pint min_id{ std::min(Xpos.size(),Xneg.size()) };
	v_inte Xpcs(min_id);
	for (pint i = 0; i < min_id; i++)	{
		Xpcs[i] = std::round((real(Xpos[i]) + real(Xneg[i])) / 2);
	}
	v_inte::iterator it = std::unique(Xpcs.begin(), Xpcs.end());
	Xpcs.resize(std::distance(Xpcs.begin(), it));
	std::sort(Xpcs.begin(), Xpcs.end());
	Xpcs.erase(std::unique(Xpcs.begin(), Xpcs.end()), Xpcs.end());
	return Xpcs;
}

//v_inte refine_Xpcs(const v_real& W, const v_real& avgpc, pint min_size, pint max_size) {
//#ifdef v
//	std::cout << "refine_Xpcs: min_size=" << min_size << ", max_size" << max_size << "\n";
//#endif
//	if (avgpc.size() <= 5) {
//		std::cout << "Average Pseudo Cycle waveform size <= 5";
//		throw std::invalid_argument("Average Pseudo Cycle waveform size <= 5");
//	}
//	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(avgpc.begin(), avgpc.end(), 0, 1.0 / avgpc.size());
//	v_real Wpadded(W.size() + 2 * min_size, 0.0);
//	for (pint i = min_size; i < W.size() + min_size; i++) {
//		Wpadded[i] = W[i - min_size];
//	}
//	pint best_size{ 0 };
//	inte best_x0{ 0 };
//	real best_val{ 0.0 };
//	real curr_val{ 0.0 };
//	real step{ 0.0 };
//	real amp{0.0};
//	v_inte Xpc;
//	std::vector<v_real> interps(max_size - min_size + 1, v_real(max_size + 1));
//	for (pint size = min_size; size <= max_size; size++) {
//		step = 1.0 / real(size);
//		for (pint x0 = 1; x0 < min_size; x0++) {
//			curr_val = 0.0;
//			//amp = std::abs(*std::max_element(Wpadded.begin() + x0, Wpadded.begin() + x0 + size, abs_compare));
//			for (pint i = 0; i <= size; i++) {
//				interps[size - min_size][i] = spline(i * step);
//				curr_val += Wpadded[x0 + i] * interps[size - min_size][i];// / amp;
//
//			}
//			if (curr_val > best_val) {
//				best_val = curr_val;
//				best_x0 = x0;
//				best_size = size;
//			}
//		}
//	}
//	Xpc.push_back(best_x0 - inte(min_size));
//	inte curr_x0{ best_x0 + inte(best_size) };
//	Xpc.push_back(curr_x0 - inte(min_size));
//
//#ifdef v
//	std::cout << "refine_Xpcs: before while loop\n";
//#endif
//	while (curr_x0 + max_size - min_size < W.size()) {
//		best_val = 0.0;
//		for (pint size = min_size; size <= max_size; size++) {
//			curr_val = 0.0;
//			//amp = std::abs(*std::max_element(Wpadded.begin() + curr_x0, Wpadded.begin() + curr_x0 + size, abs_compare));
//			for (pint i = 0; i <= size; i++) {
//				curr_val += Wpadded[curr_x0 + i] * interps[size - min_size][i];
//			}
//			if (curr_val >= best_val) {
//				best_val = curr_val;
//				best_size = size;
//			}
//		}
//		curr_x0 += best_size;
//#ifdef v
//		std::cout << "c:" << curr_x0 << " ";
//#endif
//		Xpc.push_back(curr_x0 - inte(min_size));
//	}
//#ifdef v
//	assert(Xpc[1] >= 0);
//#endif
//	return Xpc;
//}

//real error(const v_real& W1, const v_real& W2) {
//
//	real err{ 0.0 };
//	inte n = std::min(W1.size(), W2.size());
//	for (pint i = 0; i < n; i++) {
//		err += std::abs(W1[i] - W2[i]);
//	}
//	return err / n;
//}



