#include "Header.h"
#include <filesystem>

#define t // for time

//class layer {
//private:
//	v_real W;
//	bool reconstructed = false;
//public:
//	v_inte X_PCs; // start of each pulse
//	v_real Envelope;
//	v_real Waveform;
//	pint n;
//	pint fps;
//
//
//	layer(v_inte X, v_real E, v_real W, pint n_, pint fps_) {
//		X_PCs = X;
//		Envelope = E;
//		Waveform = W;
//		n = n_;
//		fps = fps_;
//	}
//	pint size() {
//		return X_PCs.size() + Envelope.size() + Waveform.size();
//	}
//	v_real reconstruct() {
//		if (reconstructed) {
//			return W;
//		}
//		boost::math::interpolators::cardinal_cubic_b_spline<real> spline(Waveform.begin(), Waveform.end(), 0.0, 1.0 / real(Waveform.size()));
//		v_real W_reconstructed(n, 0.0);
//
//		inte x0{ X_PCs[0] };
//		inte x1{ X_PCs[1] };
//		real step{ 1.0 / (x1 - x0) };
//		for (pint i = 1; i < X_PCs.size() - 1; i++) {
//			x0 = X_PCs[i];
//			x1 = X_PCs[i + 1];
//			step = 1.0 / real(x1 - x0);
//			for (inte j = x0; j < x1; j++) {
//				W_reconstructed[j] = spline((j - x0) * step) * Envelope[i];
//			}
//			if (x1 - x0 <= 3) {
//				std::cout << "Warning: Pseudo cycle with period < 4 between " << x0 << " and " << x1 << "\n";
//			}
//		}
//
//		inte X_PCs_start = X_PCs[1];
//		inte X_PCs_end = X_PCs.back();
//		pint itens{ 5 };
//		real avg_t{ 0.0 };
//		real avg_e{ 0.0 };
//		while (X_PCs[0] > 0) {
//			avg_t = 0.0;
//			avg_e = 0.0;
//			for (pint i = 0; i < itens; i++) {
//				avg_t += X_PCs[i + 1] - X_PCs[i];
//				avg_e += Envelope[i];
//			}
//			X_PCs.insert(X_PCs.begin(), X_PCs[0] - std::round(avg_t / itens));
//			Envelope.insert(Envelope.begin(), avg_e / itens);
//		}
//
//		while (X_PCs.back() < n) {
//			avg_t = 0.0;
//			avg_e = 0.0;
//			for (pint i = X_PCs.size() - itens - 1; i < X_PCs.size() - 1; i++) {
//				avg_t += X_PCs[i + 1] - X_PCs[i];
//				avg_e += Envelope[i];
//			}
//			X_PCs.push_back(X_PCs.back() + std::round(avg_t / itens));
//			Envelope.push_back(avg_e / itens);
//		}
//
//		// Filling from 0 to Xp[1]
//		pint ctr = 0;
//		x0 = X_PCs[ctr];
//		x1 = X_PCs[ctr + 1];
//		while (x1 <= X_PCs_start) {
//			if (x1 < 0) {
//				continue;
//			}
//			else {
//				step = 1.0 / real(x1 - x0);
//				for (inte j = x0; j < x1; j++) {
//					if (j >= 0) {
//						W_reconstructed[j] = spline((j - x0) * step) * Envelope[ctr];
//					}
//				}
//			}
//			ctr++;
//			x0 = X_PCs[ctr];
//			x1 = X_PCs[ctr + 1];
//		}
//
//		ctr = X_PCs.size() - 1;
//		x0 = X_PCs[ctr - 1];
//		x1 = X_PCs[ctr];
//		while (x0 >= X_PCs_end) {
//			if (x0 >= n) {
//				continue;
//			}
//			else {
//				step = 1.0 / real(x1 - x0);
//				for (inte j = x0; j < x1; j++) {
//					if (j < n) {
//						W_reconstructed[j] = spline((j - x0) * step) * Envelope.back();
//					}
//				}
//			}
//			ctr--;
//			x0 = X_PCs[ctr - 1];
//			x1 = X_PCs[ctr];
//		}
//		reconstructed = true;
//		W = W_reconstructed;
//		return W_reconstructed;
//	}
//};
//
//Compressed compress(const v_real& W, inte max_seconds = 100, pint max_iterations = std::numeric_limits<pint>::max(), inte mult = 3) {
//#ifdef v
//	std::cout << "inside compress\n";
//#endif
//	v_pint posX, negX;
//	get_pulses(W, posX, negX);
//
//	if (posX.empty() || negX.empty()) {
//		std::cout << "Unable to get pulses\n";
//		throw "Unable to get pulses";
//	}
//
//	auto posF = get_frontier(W, posX);
//	auto negF = get_frontier(W, negX);
//	//write_vector(posF, name + "_pos.csv");
//	//write_vector(negF, name + "_neg.csv");
//
//	refine_frontier_iter(posF, W);
//	refine_frontier_iter(negF, W);
//	//write_vector(posF, name + "_pos_refined.csv");
//	//write_vector(negF, name + "_neg_refined.csv");
//
//	v_inte best_Xpcs = get_Xpcs(posF, negF);
//	//write_vector(best_Xpcs, "Xpcs.csv");
//
//	v_real best_avg;
//	mode_abdm ma = average_pc_waveform(best_avg, best_Xpcs, W);
//	//write_vector(best_avg, "avgpcw.csv");
//
//	Compressed Wave_rep = Compressed::raw(best_Xpcs, best_avg, W);
//#ifdef v
//	std::cout << "inside compress-1\n";
//#endif
//	real lower_error = error(Wave_rep.W_reconstructed, W);// average_pc_metric(best_avg, best_Xpcs, W);
//	inte min_size = std::max(inte(10), inte(ma.mode) - inte(mult * ma.abdm));
//	inte max_size = ma.mode + inte(mult * ma.abdm);
//#ifdef v
//	std::cout << "inside compress-2\n";
//#endif
//
//	// get sinusoid waveform
//	auto fpa = rfft(W);
//#ifdef v
//	std::cout << "inside compress-3\n";
//	std::cout << "f=" << fpa[0] << " p=" << fpa[1] << " a=" << fpa[2] << "\n";
//#endif
//
//	if (fpa[0] > 0) {
//		pint T = pint(real(W.size()) / fpa[0]);
//		v_real sinusoid(T, 0.0);
//		for (pint i = 0; i < T; i++) {
//			sinusoid[i] = fpa[2] * cos(fpa[1] + 2 * PI * fpa[0] * real(i) / W.size());
//		}
//
//#ifdef v
//		std::cout << "inside compress-4\n";
//		write_vector(W, "W.csv");
//		write_vector(sinusoid, "sinusoid.csv");
//#endif
//
//		v_inte Xpcs_sinusoid = refine_Xpcs(W, sinusoid, T / 2, T * 2);
//		Compressed Wave_rep_sin = Compressed::raw(Xpcs_sinusoid, sinusoid, W);
//
//		auto sinusoid_error = error(Wave_rep_sin.W_reconstructed, W);
//		std::cout << "Sinusoid error= " << sinusoid_error << ", Envelope error=" << lower_error << "\n";
//		if (sinusoid_error < lower_error) {
//			std::cout << "Using sinusoid as initial waveform:\n";
//			std::cout << "f=" << fpa[0] << " p=" << fpa[1] << " a=" << fpa[2] << "\n";
//			best_avg = sinusoid;
//			min_size = T / 2;
//			max_size = 2 * T;
//		}
//	}
//
//	std::cout
//		<< "\nInitial Info:\n"
//		<< "Number of samples in the original signal:" << W.size() << "\n"
//		<< "Appr. compressed size:" << best_Xpcs.size() + best_avg.size() << "\n"
//		<< "Xpcs size:            " << best_Xpcs.size() << "\n"
//		<< "Waveform length mode: " << ma.mode << "\n"
//		<< "Waveform length abdm: " << ma.abdm << "\n"
//		<< "Min waveform length:  " << min_size << "\n"
//		<< "Max waveform length:  " << max_size << "\n"
//		<< "Initial average error:" << lower_error << "\n";
//
//	real avdv;
//	v_inte Xpcs;
//	v_real avg(best_avg);
//
//	const std::chrono::time_point< std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
//	real duration = 0.0;
//	pint ctr = 0;
//	while (duration <= max_seconds && ctr < max_iterations) {
//		Xpcs = refine_Xpcs(W, avg, min_size, max_size);
//		//std::cout << " refine_Xpcs";
//		ma = average_pc_waveform(avg, Xpcs, W);
//		//std::cout << " average_pc_waveform";
//		inte min_size = std::max(inte(10), inte(ma.mode) - inte(mult * ma.abdm));
//		inte max_size = ma.mode + inte(mult * ma.abdm);
//
//		Wave_rep = Compressed::raw(Xpcs, avg, W);
//
//		avdv = error(Wave_rep.W_reconstructed, W);
//
//		if (avdv < lower_error) {
//			lower_error = avdv;
//			best_Xpcs = Xpcs;
//			best_avg = avg;
//
//			pint cn = 2 * (best_Xpcs.size() + best_avg.size()) + 1;
//			std::cout
//				<< "\n"
//				<< "Iteration " << ctr << "\n"
//				<< "Number of samples in the original signal:" << W.size() << "\n"
//				<< "Appr. compressed size:" << cn << " (" <<  float(cn) / float(W.size()) << ")\n"
//				<< "Waveform length mode: " << ma.mode << "\n"
//				<< "Waveform length abdm: " << ma.abdm << "\n"
//				<< "Min waveform length:  " << min_size << "\n"
//				<< "Max waveform length:  " << max_size << "\n"
//				<< "Average error:        " << lower_error << "\n";
//		}
//		else {
//			std::cout << "Iteration=" << ctr << " time=" << duration << " error=" << avdv << " m=" << min_size << " M=" << max_size << " | ";
//		}
//		ctr++;
//		duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count();
//	}
//
//	return Compressed::raw(best_Xpcs, best_avg, W);
//}
//
//v_real get_residue(const v_real& W0, const v_real& W1) {
//	v_real R(W0.size(), 0.0);
//	for (pint i = 0; i < R.size(); i++)	{
//		R[i] = W0[i] - W1[i];
//	}
//	return R;
//}
//
inline bool ends_with(std::string const& value, std::string const& ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
//
//void write_bin(std::string path, pint orig_n, pint fps, const v_inte& beg_of_pseudo_cycles, const v_real& waveform, const v_real& amp_of_pseudo_cycles) {
//	std::cout << "n=" << orig_n << "\n";
//
//	std::ofstream data_file;      // pay attention here! ofstream
//
//	pint max_t = 0;
//	v_pint T_pint(beg_of_pseudo_cycles.size());
//	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
//		T_pint[i] = beg_of_pseudo_cycles[i] - beg_of_pseudo_cycles[i - 1];
//		if (T_pint[i] > max_t) {
//			max_t = T_pint[i];
//		}
//	}
//
//
//	v_pint pint_data = { orig_n, fps, amp_of_pseudo_cycles.size(), waveform.size(), max_t }; // header
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::trunc);
//	data_file.write((char*) &pint_data[0], pint_data.size() * sizeof(pint));
//	data_file.close();
//
//	
//	std::vector<char> T(beg_of_pseudo_cycles.size());
//	T[0] = beg_of_pseudo_cycles[0];
//	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
//		T[i] = 127.0 * (real)T_pint[i] / (real)max_t;
//	}
//
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&T[0], T.size() * sizeof(char));
//	data_file.close();
//
//
//	std::vector<char> wv(waveform.size());
//	for (size_t i = 0; i < waveform.size(); i++) {
//		wv[i] = waveform[i] * 127.0;
//	}
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&wv[0], wv.size() * sizeof(char));
//	data_file.close();
//
//	std::vector<char> en(amp_of_pseudo_cycles.size());
//	for (size_t i = 0; i < amp_of_pseudo_cycles.size(); i++) {
//		en[i] = amp_of_pseudo_cycles[i] * 127.0;
//	}
//	data_file.open(path, std::ios::out | std::ios::binary | std::fstream::app);
//	data_file.write((char*)&en[0], en.size() * sizeof(char));
//	data_file.close();
//}
//
//layer read_bin(std::string path) {
//	std::ifstream  data_file;
//	data_file.open(path, std::ios::in | std::ios::binary);
//
//	pint* header = new pint[5];
//	data_file.read(reinterpret_cast<char*>(&header[0]), 5 * sizeof(pint));
//
//	std::cout << "Decompressing file with n="<<header[0] << " and fps="<< header[1]<<"\n";
//	
//	char* beg_of_pseudo_cycles_buffer = new char[header[2] + 1];
//	data_file.read((char*)&beg_of_pseudo_cycles_buffer[0], (header[2] + 1) * sizeof(char));
//	v_inte beg_of_pseudo_cycles(header[2] + 1);
//
//	beg_of_pseudo_cycles[0] = beg_of_pseudo_cycles_buffer[0];
//	for (size_t i = 1; i < beg_of_pseudo_cycles.size(); i++) {
//		beg_of_pseudo_cycles[i] = beg_of_pseudo_cycles[i - 1] + header[4] * (real)beg_of_pseudo_cycles_buffer[i] / 127.0;
//	}
//
//	char* waveform_buffer = new char[header[3]];
//	data_file.read((char*)&waveform_buffer[0], (header[3]) * sizeof(char));
//	v_real waveform(header[3]);
//	for (size_t i = 0; i < header[3]; i++)	{
//		waveform[i] = (real)waveform_buffer[i] / 127.0;
//	}
//
//	char* envelope_buffer = new char[header[2]];
//	data_file.read((char*)&envelope_buffer[0], (header[2]) * sizeof(char));
//	v_real envelope(header[2]);
//	for (size_t i = 0; i < header[2]; i++) {
//		envelope[i] = (real)envelope_buffer[i] / 127.0;
//	}
//
//	data_file.close();
//
//	return layer(beg_of_pseudo_cycles, envelope, waveform, header[0], header[1]);
//}

int main(int argc, char** argv) {
	//pint max_secs = 20;
	//pint max_iters = std::numeric_limits<pint>::max();
	bool frontiers_mode = false;
	std::vector<std::string> wav_paths;
	//std::vector<std::string> cmp_paths;
	//std::string append = "reconstructed";
	for (int i = 1; i < argc; ++i) { // parsing args
		if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
			std::cout << "Size of char: " << sizeof(char) << " byte\n";
			std::cout << "Size of short: " << sizeof(short) << " byte\n";
			std::cout << "Size of int: " << sizeof(int) << " bytes\n";
			std::cout << "Size of float: " << sizeof(float) << " bytes\n";
			std::cout << "Size of double: " << sizeof(double) << " bytes\n\n";
			std::cout
				<< " For more info about the author: carlos-tarjano.web.app\n"
				<< " Usage: \n"
				<< " [-f] [path/to/file1.wav]...[path/to/filen.wav]\n"
				<< " -f or --frontiers: returns frontiers instead of envelope\n"
				<< " If no path is given the root folder will be scanned for .wav files, and those will be processed accordingly\n";
			return 0;
		}
		else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--frontiers") == 0) {
			frontiers_mode = true;
			std::cout << "Frontiers mode enabled\n";
			i++;
		}
		else if (ends_with(argv[i], ".wav")) {
			wav_paths.push_back(argv[i]);
		}
	}

	if (wav_paths.empty()) { // no files found in args, searching root
		for (const auto& entry : std::filesystem::directory_iterator("./")) {
			std::string path = { entry.path().u8string() };
			if (ends_with(path, ".wav")) {
				wav_paths.push_back(path);
			}
		}
	}

	for (auto path : wav_paths) {
		std::cout << "\nProcessing " << path << std::endl;
		Wav WV = read_wav(path);
		std::vector<size_t> posX, negX;
		get_pulses(WV.W, posX, negX);

		try {
			if (frontiers_mode) {
				auto posF = get_frontier(WV.W, posX);
				auto negF = get_frontier(WV.W, negX);
				path.replace(path.end() - 4, path.end(), "_P.csv");
				write_vector(posF, path);
				path.replace(path.end() - 6, path.end(), "_N.csv");
				write_vector(negF, path);
			}
			else
			{
				std::vector<size_t> E(posX.size() + negX.size());
				std::set_union(posX.begin(), posX.end(), negX.begin(), negX.end(), E.begin());
				for (size_t i = 0; i < WV.W.size(); i++) {
					WV.W[i] = std::abs(WV.W[i]);
				}
				auto Envelope = get_frontier(WV.W, E);
				path.replace(path.end() - 4, path.end(), "_E.csv");
				write_vector(Envelope, path);
			}
		}
		catch (...) {
			continue;
		}
	} 
}

