/**
 * \file point1.cpp
 *
 * \brief Contains the resolution of point 1 of Deliverable 1.
 *        This program reads an audio file sample by sample and writes it to a new audio file.
 *        To use the program, the user must pass two arguments: the input file path and the output file path
 *
 *        Usage: point1 <input file> <output file>
 *
 *
 * \author António Neves
 */


#include <iostream>
#include <vector>
#include <sndfile.hh>

using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {

	if(argc < 3) {
		cerr << "Usage: wavcp <input file> <output file>" << endl;
		return 1;
	}

	SndfileHandle sndFileIn { argv[argc-2] };
	if(sndFileIn.error()) {
		cerr << "Error: invalid input file" << endl;
		return 1;
    }

	if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format" << endl;
		return 1;
	}

	if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format" << endl;
		return 1;
	}

	cout << "Input file has:" << endl;
	cout << '\t' << sndFileIn.frames() << " frames" << endl;
	cout << '\t' << sndFileIn.samplerate() << " samples per second" << endl;
	cout << '\t' << sndFileIn.channels() << " channels" << endl;

	SndfileHandle sndFileOut { argv[argc-1], SFM_WRITE, sndFileIn.format(),
	  sndFileIn.channels(), sndFileIn.samplerate() };
	if(sndFileOut.error()) {
		cerr << "Error: invalid output file" << endl;
		return 1;
    }

	size_t nFrames;
	vector<short> samples(FRAMES_BUFFER_SIZE * sndFileIn.channels());
	// reads nFrames from source audio file to destination file
	while((nFrames = sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)))
		sndFileOut.writef(samples.data(), nFrames);

	return 0;
}

