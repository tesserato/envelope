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

//#define v // for verbose

typedef unsigned long long pint;
typedef long long          inte;
typedef double             real;

//typedef size_t pint;
//typedef int          inte;
//typedef double             real;

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

struct mode_abdm { // mode & average absolute deviation from mode
	pint mode;
	real abdm;
	mode_abdm() {
		mode = 0;
		abdm = 0.0;
	}
	mode_abdm(pint m, real a) {
		mode = m;
		abdm = a;
	}
};

class Chronograph {
private:
	std::chrono::time_point< std::chrono::steady_clock> start, end;
	double duration{ 0.0 };
public:
	Chronograph() {
		start = std::chrono::high_resolution_clock::now();
	}
	double stop(std::string message = "Time = ") {
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		if (duration < 1000) {
			std::cout << message << duration << " milliseconds\n";
			return duration;
		}
		if (duration < 60000) {
			duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
			std::cout << message << duration << " seconds\n";
			return duration;
		}
		duration = std::chrono::duration_cast<std::chrono::minutes>(end - start).count();
		std::cout << message << duration << " minutes\n";
		return duration;
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

void ft_smooth(v_real& in, real ndevs = 3.0) {
	/* Configure a Descriptor */
	std::vector<std::complex<double>> out(in.size() / 2 + 1);

	write_vector(in, "00before.csv");
	DFTI_DESCRIPTOR_HANDLE hand;
	MKL_LONG status;
	status = DftiCreateDescriptor(&hand, DFTI_DOUBLE, DFTI_REAL, 1, in.size());
	status = DftiSetValue(hand, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
	status = DftiSetValue(hand, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);
	status = DftiSetValue(hand, DFTI_BACKWARD_SCALE, 1.0f / in.size());
	//status = DftiSetValue(hand, DFTI_PACKED_FORMAT, DFTI_CCE_FORMAT);

	//status = DftiSetValue(hand, DFTI_INPUT_STRIDES, <real_strides>);
	//status = DftiSetValue(hand, DFTI_OUTPUT_STRIDES,<complex_strides>);
	status = DftiCommitDescriptor(hand);
	/* Compute an FFT */
	status = DftiComputeForward(hand, in.data(), out.data());

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
		if (std::abs(out[i]) <= avg + ndevs * dev) {
			out[i] = { 0.0,0.0 };
		}
	}

	/* Compute an inverse FFT */
	//std::fill(in.begin(), in.end(), 0.0);
	status = DftiComputeBackward(hand, out.data(), in.data());
	DftiFreeDescriptor(&hand);
	write_vector(in, "01after.csv");
}

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

class Compressed {
public:
	v_inte X_PCs; // start of each pulse
	v_real Envelope;
	v_real Waveform;
	v_real W_reconstructed;
	pint fps;

	Compressed(const v_inte& Xp, const v_real& W, const v_real& E, const v_real& R, pint f) {
		X_PCs = Xp;
		Envelope = E;
		Waveform = W;
		W_reconstructed = R;
		fps = f;
	}
	
	static Compressed smooth(v_inte& Xp, v_real& W, v_real& E, pint n, pint f = 44100) {
		ft_smooth(W, 0.0);
		pint first{ 0 };
		//pint last{ 0 };
		real min_distance{ 2.0 };
		real distance;
		for (pint i = 0; i < W.size() - 1; i++)	{
			distance = std::abs(W[i]) + std::abs(W[i + 1]);
			if (distance < min_distance) {
				min_distance = distance;
				first = i + 1;
			}
		}

		std::rotate(W.begin(), W.begin() + first, W.end());

		for (pint i = 0; i < Xp.size(); i++) {
			Xp[i] = Xp[i] + first;
		}


		//ft_smooth(E, 1.0);
		pint itens{ 5 };
		smooth_XPCs(Xp);

		real avg_t{ 0.0 };
		real avg_e{ 0.0 };
		while (Xp[0] > 0) {
			for (pint i = 0; i < itens; i++) {
				avg_t += Xp[i + 1] - Xp[i];
				avg_e += E[i];
			}
			Xp.insert(Xp.begin(), Xp[0] - std::round(avg_t / itens));
			E.insert(E.begin(), avg_e / itens);
		}

		avg_t = 0.0;
		avg_e = 0.0;
		while (Xp.back() < n) {
			for (pint i = Xp.size() - itens-1; i < Xp.size()-1; i++) {
				avg_t += Xp[i + 1] - Xp[i];
				avg_e += E[i];
			}
			Xp.push_back(Xp.back() + std::round(avg_t / itens));
			E.push_back(avg_e / itens);
		}

		boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin(), W.end(), 0.0, 1.0 / real(W.size()));
		inte x0{ Xp[0] };
		inte x1{ Xp[1] };
		v_real W_reconstructed;
		real step{ 1.0 / (x1 - x0) };
		for (pint i = 0; i < Xp.size() - 1; i++) {
			x0 = Xp[i];
			x1 = Xp[i + 1];
			if (x1 - x0 > 3) {
				step = 1.0 / real(x1 - x0);
				for (inte j = x0; j < x1; j++) {
					W_reconstructed.push_back(spline((j - x0) * step) * E[i]);
				}
			}
			else {
				std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
			}
		}

		auto start = W_reconstructed.begin() - Xp[0];
		v_real Wf(start, start + n);
		return Compressed(Xp, W, E, Wf, f);
	}

	static Compressed raw(const v_inte& Xp, const v_real& W, const v_real& S, pint f = 44100) {
#ifdef v
		std::cout << "inside Compressed::raw\n";
#endif
		boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin(), W.end(), 0.0, 1.0 / (real)W.size());
		v_real W_reconstructed(S.size(), 0.0);
		v_real Envelope;

		inte x0{ Xp[0] };
		inte x1{ Xp[1] };
		real step{ 1.0 / (real)(x1 - x0) };
		real e;// { std::abs(*std::max_element(S.begin()), S.begin() + Xp[1], abs_compare)) }
		for (pint i = 1; i < Xp.size() - 1; i++) {
			x0 = Xp[i];
			x1 = Xp[i + 1];
			e = std::abs(*std::max_element(S.begin() + x0, S.begin() + x1, abs_compare));
			Envelope.push_back(e);
			step = 1.0 / real(x1 - x0);
			for (inte j = x0; j < x1; j++) {
				W_reconstructed[j] = spline((j - x0) * step) * e;
			}
			if (x1 - x0 <= 3) {
				std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
			}
		}
		Envelope.push_back(std::abs(*std::max_element(S.begin() + Xp.back(), S.end(), abs_compare)));
#ifdef v
		std::cout << "inside Compressed::raw-1\n";
#endif
		v_inte Xp_new = Xp;
		v_real En_new = Envelope;
		pint itens{ 5 };
		real avg_t{ 0.0 };
		real avg_e{ 0.0 };
		while (Xp_new[0] > 0) {
			for (pint i = 0; i < itens; i++) {
				avg_t += Xp_new[i + 1] - Xp_new[i];
				avg_e += En_new[i];
			}
			Xp_new.insert(Xp_new.begin(), Xp_new[0] - (inte)std::round(avg_t / itens));
			En_new.insert(En_new.begin(), avg_e / (real)itens);
		}
#ifdef v
		std::cout << "inside Compressed::raw-2\n";
#endif
		avg_t = 0.0;
		avg_e = 0.0;
		while (Xp_new.back() < S.size()) {
			for (pint i = Xp_new.size() - itens - 1; i < Xp_new.size() - 1; i++) {
				avg_t += Xp_new[i + 1] - Xp_new[i];
				avg_e += En_new[i];
			}
			Xp_new.push_back(Xp_new.back() + (inte)std::round(avg_t / itens));
			En_new.push_back(avg_e / (real)itens);
		}
#ifdef v
		std::cout << "inside Compressed::raw-3\n";
#endif
		// Filling from 0 to Xp[1]
		pint ctr = 0;
		x0 = Xp_new[ctr];
		x1 = Xp_new[ctr + 1];
		while (x1 <= Xp[1]) {
			if (x1 < 0)	{
				continue;
			}
			else {
				step = 1.0 / (real)(x1 - x0);
				for (inte j = x0; j < x1; j++) {
					if (j >= 0) {
					W_reconstructed[j] = spline((j - x0) * step) * En_new[ctr];
					}
				}
			}
			ctr++;
			x0 = Xp_new[ctr];
			x1 = Xp_new[ctr + 1];
		}
#ifdef v
		std::cout << "inside Compressed::raw-4\n";
#endif
		x0 = Xp.back();
		x1 = Xp_new.back();
		step = 1.0 / (real)(x1 - x0);
		for (inte j = x0; j < x1; j++) {
			if (j < S.size()) {
				W_reconstructed[j] = spline((j - x0) * step) * En_new.back();
			}
		}
#ifdef v
		std::cout << "inside Compressed::raw-5\n";
#endif
		return Compressed(Xp, W, Envelope, W_reconstructed, f);
	}
};

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

void refine_frontier(std::vector<pulse>& Pulses, const v_pint& Xp, const v_real& W, inte avgL, real stdL, inte n_stds = 3) {
#ifdef v
	std::cout << "inside refine_frontier\n";
#endif
	std::vector<pulse> Pulses_to_split;
	std::vector<pulse> Pulses_to_test;
	v_pint Xzeroes;
	std::function<inte(const v_real& V, const inte x0, const inte x1)> arg;
	if (W[Xp[0]] >= 0) {
		arg = argmax;
	}
	else {
		arg = argmin;
	}
	inte currsign, x;
#ifdef v
	std::cout << "Refining frontier; TH=" << avgL + n_stds * stdL << "\n";
#endif
	for (pint i = 1; i < Xp.size(); i++) {
		if (Xp[i] - Xp[i - 1] >= avgL + n_stds * stdL) {
			Pulses_to_split.push_back(pulse(Xp[i - 1], Xp[i]));
		}
	}
	while (Pulses_to_split.size() > 0) {		
		for (pulse p : Pulses_to_split) {
			Xzeroes.clear();
			currsign = sgn(W[p.start]) ;
			for (pint i = p.start + 1; i < p.end; i++)	{
				//if (currsign == 0) {
				//	Xzeroes.push_back(i - 1);
				//	currsign = sgn(W[i]);
				//}
				if (currsign != sgn(W[i])){
					Xzeroes.push_back(i);
					currsign = sgn(W[i]);
				}
			}
			if (Xzeroes.size() > 1) {
				x = arg(W, Xzeroes[0], Xzeroes.back());
				Pulses_to_test.push_back(pulse(p.start, x));
				Pulses_to_test.push_back(pulse(x, p.end));
			}
		}
		//std::cout << "Pulses_to_split size:" << Pulses_to_split.size() << "\n";
		Pulses_to_split.clear();


		for (pulse p : Pulses_to_test) {
			if (p.end - p.start >= avgL + n_stds * stdL) {
				Pulses_to_split.push_back(p);
			} else {
				Pulses.push_back(p);
			}
		}
		//std::cout << "Pulses_to_test size:" << Pulses_to_test.size() << "\n";
		Pulses_to_test.clear();
	}
}

mode_abdm get_mode_and_abdm(v_pint& T) {
	std::sort(T.begin(), T.end());
	pint curr_value = T[0];
	pint curr_count = 0;
	pint max_count = 0;
	pint mode = 0;
	for (auto t: T) {
		if (t == curr_value){
			curr_count++;
		} 
		else {
			if (curr_count > max_count) {
				max_count = curr_count;
				mode = curr_value;
			}
			curr_value = t;
			curr_count = 1;
		}
	}
	if (curr_count > max_count) {
		max_count = curr_count;
		mode = curr_value;
	}
	inte abdm{ 0 };
	for (auto t : T) {
		abdm += std::abs(inte(t) - inte(mode));
	}
	
	return mode_abdm(mode, real(abdm) / real(T.size()));
}

void refine_frontier_iter(v_pint& Xp, const v_real& W) {
	v_pint T(Xp.size() - 1);
	for (pint i = 0; i < Xp.size() - 1; i++)	{
		T[i] = Xp[i + 1] - Xp[i];
	}

	mode_abdm modeabdm = get_mode_and_abdm(T);
	pint mde{ modeabdm.mode };
	real std{ modeabdm.abdm };

	std::vector<pulse> Pulses;
	refine_frontier(Pulses, Xp, W, mde, std);

	pint psize{ 0 };
	real std_c{ 0.0 };
	while (Pulses.size() > psize) {
#ifdef v
		std::cout << "Xp size before:" << Xp.size() << "\n";
#endif
		psize = Pulses.size();
		for (pulse p:Pulses)	{
			Xp.push_back(p.start);
			Xp.push_back(p.end);
		}
		v_pint::iterator it=std::unique(Xp.begin(), Xp.end());
		Xp.resize(std::distance(Xp.begin(), it));
#ifdef v
		std::cout << "Xp size after:" << Xp.size() << "\n";
#endif
		T.resize(Xp.size() - 1);
		for (pint i = 0; i < Xp.size() - 1; i++) {
			T[i] = Xp[i + 1] - Xp[i];
		}

		modeabdm = get_mode_and_abdm(T);
		mde = modeabdm.mode;
		std_c = modeabdm.abdm;

		if (std_c < std) {
			std = std_c;
			refine_frontier(Pulses, Xp, W, mde, std);
		}
		else {
#ifdef v
			std::cout << "std0=" << std << " , std1=" << std_c << "\n";
#endif

			break;
		}
	}
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

mode_abdm average_pc_waveform(v_real& pcw,  v_inte& Xp, const v_real& W) {
#ifdef v
	std::cout << "average_pc_waveform\n";
#endif
	v_pint T(Xp.size() - 1);
	for (pint i = 0; i < Xp.size() - 1; ++i) {
		T[i] = Xp[i + 1] - Xp[i];
	}

	mode_abdm modeabdm = get_mode_and_abdm(T);
	pint mode{ modeabdm.mode };

	inte x0{ 0 };
	inte x1{ 0 };
	real step{ 1.0 / real(mode) };
	pcw.resize(mode);
	std::fill(pcw.begin(), pcw.end(), 0.0);
	real amp;
	for (pint i = 1; i < Xp.size() - 1; i++) {
		x0 = 0;
		if (Xp[i] > 0) {
			x0 = Xp[i];
		}
		x1 = Xp[i + 1];

#ifdef v
		assert(x1 - x0 > 5);
#endif
		if (x1 - x0 > 5) {
			//amp = std::abs(*std::max_element(W.begin() + x0, W.begin() + x1, abs_compare));
			boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin() + x0, W.begin() + x1, 0, 1.0 / real(x1 - x0));
			for (pint j = 0; j < mode; j++) {
				pcw[j] += spline(j * step);// *amp* amp;
			}			
		}
	}
	amp = std::abs(*std::max_element(pcw.begin(), pcw.end(), abs_compare));
	for (pint i = 0; i < mode; i++) {
		pcw[i] = pcw[i] / amp;
	}

	pint first{ 0 };
	//pint last{ 0 };
	real min_distance{ 2.0 };
	real distance;
	for (pint i = 0; i < pcw.size() - 1; i++) {
		distance = std::abs(pcw[i]) + std::abs(pcw[i + 1]);
		if (distance < min_distance) {
			min_distance = distance;
			first = i + 1;
		}
	}

	std::rotate(pcw.begin(), pcw.begin() + first, pcw.end());

	for (pint i = 0; i < Xp.size(); i++) {
		Xp[i] = Xp[i] + first;
	}

	while (Xp.back() >= W.size()) {
		Xp.pop_back();
	}

	return modeabdm;
}

v_inte refine_Xpcs(const v_real& W, const v_real& avgpc, pint min_size, pint max_size) {
#ifdef v
	std::cout << "refine_Xpcs: min_size=" << min_size << ", max_size" << max_size << "\n";
#endif
	if (avgpc.size() <= 5) {
		std::cout << "Average Pseudo Cycle waveform size <= 5";
		throw std::invalid_argument("Average Pseudo Cycle waveform size <= 5");
	}
	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(avgpc.begin(), avgpc.end(), 0, 1.0 / avgpc.size());
	v_real Wpadded(W.size() + 2 * min_size, 0.0);
	for (pint i = min_size; i < W.size() + min_size; i++) {
		Wpadded[i] = W[i - min_size];
	}
	pint best_size{ 0 };
	inte best_x0{ 0 };
	real best_val{ 0.0 };
	real curr_val{ 0.0 };
	real step{ 0.0 };
	real amp{0.0};
	v_inte Xpc;
	std::vector<v_real> interps(max_size - min_size + 1, v_real(max_size + 1));
	for (pint size = min_size; size <= max_size; size++) {
		step = 1.0 / real(size);
		for (pint x0 = 1; x0 < min_size; x0++) {
			curr_val = 0.0;
			//amp = std::abs(*std::max_element(Wpadded.begin() + x0, Wpadded.begin() + x0 + size, abs_compare));
			for (pint i = 0; i <= size; i++) {
				interps[size - min_size][i] = spline(i * step);
				curr_val += Wpadded[x0 + i] * interps[size - min_size][i];// / amp;

			}
			if (curr_val > best_val) {
				best_val = curr_val;
				best_x0 = x0;
				best_size = size;
			}
		}
	}
	Xpc.push_back(best_x0 - inte(min_size));
	inte curr_x0{ best_x0 + inte(best_size) };
	Xpc.push_back(curr_x0 - inte(min_size));

#ifdef v
	std::cout << "refine_Xpcs: before while loop\n";
#endif
	while (curr_x0 + max_size - min_size < W.size()) {
		best_val = 0.0;
		for (pint size = min_size; size <= max_size; size++) {
			curr_val = 0.0;
			//amp = std::abs(*std::max_element(Wpadded.begin() + curr_x0, Wpadded.begin() + curr_x0 + size, abs_compare));
			for (pint i = 0; i <= size; i++) {
				curr_val += Wpadded[curr_x0 + i] * interps[size - min_size][i];
			}
			if (curr_val >= best_val) {
				best_val = curr_val;
				best_size = size;
			}
		}
		curr_x0 += best_size;
#ifdef v
		std::cout << "c:" << curr_x0 << " ";
#endif
		Xpc.push_back(curr_x0 - inte(min_size));
	}
#ifdef v
	assert(Xpc[1] >= 0);
#endif
	return Xpc;
}

real error(const v_real& W1, const v_real& W2) {

	real err{ 0.0 };
	inte n = std::min(W1.size(), W2.size());
	for (pint i = 0; i < n; i++) {
		err += std::abs(W1[i] - W2[i]);
	}
	return err / n;
}

std::vector<float> fpa_from_FT(std::vector<std::complex<double>>& FT, std::string path = "") {
	int n = FT.size();
	int f;
	float p;
	float a;
	float val = 0.0;
	float max_val = 0.0;
	if (path == "") {
		for (std::size_t i = 0; i < n; ++i) {
			val = std::pow(FT[i].real(), 2.0) + std::pow(FT[i].imag(), 2.0);
			if (val > max_val) {
				max_val = val;
				f = i;
				p = std::arg(FT[i]);
				a = std::abs(FT[i]);
			}
		}
	}
	else {
		std::ofstream outfile(path);
		outfile << "Real, Imag\n";
		for (std::size_t i = 0; i < n; ++i) {
			outfile << FT[i].real() << "," << FT[i].imag() << "\n";
			val = std::pow(FT[i].real(), 2.0) + std::pow(FT[i].imag(), 2.0);
			if (val > max_val) {
				max_val = val;
				f = i;
				p = std::arg(FT[i]);
				a = std::abs(FT[i]);
			}
		}
		outfile.close();
		std::cout << "saved FT @" << path << "\n";
	}
	std::cout << "f=" << f << ", p=" << p << ", a=" << a << ", n=" << n << "\n";
	return { float(f), p, a };
}

std::vector<float> rfft(const v_real& cin) {
	std::vector<float> in(cin.begin(), cin.end());
	//std::cout << "rfft";
	//auto tp = Chronograph();
	int n = (in.size() + 1) / 2;
	std::vector<std::complex<float>> out(in.size());

	DFTI_DESCRIPTOR_HANDLE descriptor;
	MKL_LONG status;
	// FFT 
	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_REAL, 1, in.size());//Specify size and precision
	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);             //Out of place FFT
	status = DftiCommitDescriptor(descriptor);                                       //Finalize the descriptor
	status = DftiComputeForward(descriptor, in.data(), out.data());                  //Compute the Forward FFT
	status = DftiFreeDescriptor(&descriptor);                                        //Free the descriptor

	//for (size_t i = 0; i < n; i++) {
	//	out[i] = out[i] / double(n);
	//}

	float val = 0.0;
	float max_val = 0.0;
	float f = 0.0;
	float p = 0.0;
	float a = 0.0;
	for (size_t i = 0; i < n; ++i) {
		out[i] = out[i] / (float)n;
		val = std::pow(out[i].real(), 2.0) + std::pow(out[i].imag(), 2.0);
		if (val > max_val) {
			max_val = val;
			f = (float)i;
			p = std::arg(out[i]);
			a = std::abs(out[i]);
		}
	}

	//tp.stop("FFT Time: ");
	//std::cout << "f=" << f << " p=" << p << " a=" << a <<"\n";
	//std::cout << "------------";
	//auto ff = (double)f;
	std::vector<float> result { f,p,a };

	//std::cout << "f=" << result[0] << " p=" << result[1] << " a=" << result[2] << "\n";

	return result;
}


//void make_seamless(v_real& in) {
//
//	//int n = (in.size() + 1) / 2;
//	std::vector<std::complex<float>> out(in.size());
//
//	DFTI_DESCRIPTOR_HANDLE descriptor;
//	MKL_LONG status;
//	// FFT 
//	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_REAL, 1, in.size());//Specify size and precision
//	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);             //Out of place FFT
//	status = DftiCommitDescriptor(descriptor);                                       //Finalize the descriptor
//	status = DftiComputeForward(descriptor, in.data(), out.data());                  //Compute the Forward FFT
//	status = DftiFreeDescriptor(&descriptor);                                        //Free the descriptor
//
//	//float avg{ 0.0 };
//	//for (size_t i = 0; i < out.size(); i++) {
//	//	avg += std::abs(out[i]);
//	//}
//
//	//avg = avg / out.size();
//
//	//for (size_t i = 0; i < out.size(); i++) {
//	//	if (std::abs(out[i]) < avg) {
//	//		out[i] = { 0.0,0.0 };
//	//	}
//	//}
//
//	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_COMPLEX, 1, out.size()); //Specify size and precision
//	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);                  //Out of place FFT
//	status = DftiSetValue(descriptor, DFTI_BACKWARD_SCALE, 1.0f / out.size());            //Scale down the output
//	status = DftiCommitDescriptor(descriptor);                                            //Finalize the descriptor
//	status = DftiComputeBackward(descriptor, out.data(), in.data());                      //Compute the Backward FFT
//	status = DftiFreeDescriptor(&descriptor);                                             //Free the descriptor
//
//}

//real average_pc_metric(const v_real& pcw, const v_inte& Xp, const v_real& W) {
//	pint mode{ pcw.size() };
//
//	inte x0{ 0 };
//	inte x1{ 0 };
//	pint ac{ 0 };
//	real step{ 1.0 / real(mode) };
//	real avdv{ 0.0 };
//	for (pint i = 1; i < Xp.size() - 1; i++) {
//		x0 = Xp[i];
//		x1 = Xp[i + 1];
//		if (x1 - x0 > 5) {
//			boost::math::interpolators::cardinal_cubic_b_spline<real> spline(W.begin() + x0, W.begin() + x1, 0, 1.0 / real(x1 - x0));
//			for (pint i = 0; i <= mode; i++) {
//				avdv += std::abs(spline(i * step) - W[x0 + i]);
//				ac++;
//			}
//		}
//	}
//	avdv += avdv / real(ac);
//	return avdv;
//}

//real average_pc_metric(const v_real& pcw, const v_inte& Xp, const v_real& W) {
//	//pint mode{ pcw.size() };
//	inte x0{ 0 };
//	inte x1{ 0 };
//	pint ac{ 0 };
//	real step;
//	real avdv{ 0.0 };
//	boost::math::interpolators::cardinal_cubic_b_spline<real> spline(pcw.begin(), pcw.end(), 0, 1.0 / real(pcw.size()));
//
//	for (pint i = 1; i < Xp.size() - 1; i++) {
//		x0 = Xp[i];
//		x1 = Xp[i + 1];
//		if (x1 - x0 > 3) {
//			step = 1.0 / real(x1 - x0);
//			for (pint j = 1; j < x1 - x0; j++) {
//				avdv += std::abs(spline(j * step) - W[x0 + j]);
//				ac++;
//			}
//		}
//	}
//	avdv += avdv / real(ac);
//	return avdv;
//}

//void get_pulses(const v_real& W, std::vector<pos_integer>& posX,  std::vector<pos_integer>& negX) {
//	pos_integer n{ W.size() };
//	int sign{ sgn(W[0]) };
//	int x{ 1 };
//	int x0{ 0 };
//	while (sgn(W[x]) == sign) {
//		x++;
//	}
//	x0 = x + 1;
//	sign = sgn(W[x0]);
//	int xp{ 0 };
//	for (pos_integer x1 = x0; x1 < n; x1++) {
//		if (sgn(W[x1])!=sign) {
//			if (x1 - x0 > 2) {
//				xp = argabsmax(W, x0, x1);
//				if (sgn(W[xp]) >= 0) {
//					//std::cout << "pos " << xp << "\n";
//					posX.push_back(xp);
//				}
//				else {
//					//std::cout << "neg " << xp << "\n";
//					negX.push_back(xp);
//				}
//			}
//			x0 = x1;
//			sign = sgn(W[x1]);
//		}
//	}
//	//std::cout << "opa " << x0 << "\n";
//	return;
//}

//void write_frontier(const v_real& W, const std::vector<pos_integer>& X, std::string path = "teste.csv") {
//	std::ofstream out(path);
//	for (pos_integer i = 0; i < X.size(); ++i) {
//		out << X[i] << "," << W[X[i]] << "\n";
//	}
//	out.close();
//}

//std::vector<float> fpa_from_FT(std::vector<std::complex<float>>& FT, std::string path = "") {
//	int n = FT.size();
//	int f;
//	float p;
//	float a;
//	float val = 0.0;
//	float max_val = 0.0;
//	if (path == "") {
//		for (std::size_t i = 0; i < n; ++i) {
//			val = std::pow(FT[i].real(), 2.0) + std::pow(FT[i].imag(), 2.0);
//			if (val > max_val) {
//				max_val = val;
//				f = i;
//				p = std::arg(FT[i]);
//				a = std::abs(FT[i]);
//			}
//		}
//	}
//	else {
//		std::ofstream outfile(path);
//		outfile << "Real, Imag\n";
//		for (std::size_t i = 0; i < n; ++i) {
//			outfile << FT[i].real() << "," << FT[i].imag() << "\n";
//			val = std::pow(FT[i].real(), 2.0) + std::pow(FT[i].imag(), 2.0);
//			if (val > max_val) {
//				max_val = val;
//				f = i;
//				p = std::arg(FT[i]);
//				a = std::abs(FT[i]);
//			}
//		}
//		outfile.close();
//		std::cout << "saved FT @" << path << "\n";
//	}
//	std::cout << "f=" << f << ", p=" << p << ", a=" << a << ", n=" << n << "\n";
//	return { float(f), p, a };
//}
//
//std::vector<float> rfft(std::vector<float>& in) {
//	auto tp = Chronograph();
//	int n = (in.size() + 1) / 2;
//	std::vector<std::complex<float>> out(n);
//
//	DFTI_DESCRIPTOR_HANDLE descriptor;
//	MKL_LONG status;
//	// FFT 
//	status = DftiCreateDescriptor(&descriptor, DFTI_SINGLE, DFTI_REAL, 1, in.size());//Specify size and precision
//	status = DftiSetValue(descriptor, DFTI_PLACEMENT, DFTI_NOT_INPLACE);             //Out of place FFT
//	status = DftiCommitDescriptor(descriptor);                                       //Finalize the descriptor
//	status = DftiComputeForward(descriptor, in.data(), out.data());                  //Compute the Forward FFT
//	status = DftiFreeDescriptor(&descriptor);                                        //Free the descriptor
//
//	for (size_t i = 0; i < n; i++) {
//		out[i] = out[i] / float(n);
//	}
//
//	auto fpa = fpa_from_FT(out);
//
//	tp.stop("FFT Time: ");
//	std::cout << "\n";
//	return fpa;
//}

