// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h"
#include "Header.h"


// Python Interface

extern "C" __declspec(dllexport) int test(int p) {
	return p;
}

std::vector<size_t> posF, negF, E;
int pos_n, neg_n;

extern "C" __declspec(dllexport) int compute_raw_envelope(double* cW, unsigned int n, unsigned int mode = 0) {

	std::vector<double> W(cW, cW + n);
	std::vector<size_t> posX, negX;
	get_pulses(W, posX, negX);
	if (posX.size() == 0 || negX.size() == 0) {
		return 1;    // no pulses found
	}

	if (mode == 0) { // Frontiers mode
		posF = get_frontier(W, posX);
		negF = get_frontier(W, negX);
	} else 	{        // Envelope mode
		E.resize(posX.size() + negX.size());
		std::set_union(posX.begin(), posX.end(), negX.begin(), negX.end(), E.begin());
		for (size_t i = 0; i < W.size(); i++) {
			W[i] = std::abs(W[i]);
		}
		posF = get_frontier(W, E);
	}
	return 0;
}

extern "C" __declspec(dllexport) int get_pos_size() {
	pos_n = posF.size();
	//std::cout << "C : pos n = " << pos_n << "\n";
	return pos_n;
}

extern "C" __declspec(dllexport) int get_neg_size() {
	neg_n = negF.size();
	//std::cout << "C : neg n  = " << neg_n << "\n";
	return neg_n;
}

extern "C" __declspec(dllexport) size_t * get_pos_X() {
	size_t* ptr = &posF[0];
	return ptr;
}

extern "C" __declspec(dllexport) size_t * get_neg_X() {
	size_t* ptr = &negF[0];
	return ptr;
}

//extern "C" __declspec(dllexport) size_t * get_X() {
//	size_t* ptr = &F[0];
//	return ptr;
//}