/** @file
This file builds the command line application.
*/
#include "Header.h"
#include <filesystem>

#define t // for time

inline bool ends_with(std::string const& value, std::string const& ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


int main(int argc, char** argv) {
	bool frontiers_mode = false;
	std::vector<std::string> wav_paths;
	for (int i = 1; i < argc; ++i) { // parsing args
		if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
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

	for (auto& path : wav_paths) {
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

