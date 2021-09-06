/** @file
This file builds the DLL used in the Python module.
**/
#include "pch.h"
#include "Header.h"


/** Receives and returns an int, Used to test Python communication via C foreign function interface. **/
extern "C" __declspec(dllexport) int test(int p) {
	return p;
}


std::vector<size_t> posF /** Positive frontier indices. **/, negF /** Negative frontier indices. **/, E /** Envelope indices. **/;
int pos_n /** Positive frontier size. **/, neg_n /** Negative frontier size. **/;

/** Given a pointer to an array of doubles and its size, Computes its Frontiers or Envelope, according to mode. **/
extern "C" __declspec(dllexport) int compute_raw_envelope(
	double* cW /** Pointer to an array of doubles, representing a signal. **/,
	unsigned int n /** Size of the array. **/,
	unsigned int mode = 0 /** mode = 0 for frontiers and mode = 1 for envelope. Envelope is returned as the positive frontier, in case of mode =1. **/
) 
{
	std::vector<double> W(cW, cW + n);
	std::vector<size_t> posX, negX;
	get_pulses(W, posX, negX);
	if (posX.size() == 0 || negX.size() == 0) {
		return 1;    // No pulses found
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


/** Returns the size of the positive frontier. **/
extern "C" __declspec(dllexport) int get_pos_size() {
	pos_n = posF.size();
	//std::cout << "C : pos n = " << pos_n << "\n";
	return pos_n;
}


/** Returns the size of the negative frontier. **/
extern "C" __declspec(dllexport) int get_neg_size() {
	neg_n = negF.size();
	//std::cout << "C : neg n  = " << neg_n << "\n";
	return neg_n;
}


/** Returns a pointer to the array containing the positive frontier indices. **/
extern "C" __declspec(dllexport) size_t * get_pos_X() {
	size_t* ptr = &posF[0];
	return ptr;
}


/** Returns a pointer to the array containing the negative frontier indices. **/
extern "C" __declspec(dllexport) size_t * get_neg_X() {
	size_t* ptr = &negF[0];
	return ptr;
}
