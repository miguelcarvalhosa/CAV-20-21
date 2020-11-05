/**
 * \file point6.cpp
 *
 * \brief Contains the resolution of points 6 and 8 of Deliverable 1.
 *        This program reads an audio file in .wav format, applies a quantization algorithm to reduce the number of bits
 *        used to represent each audio sample and saves the quantized file.
 *        To use the program, the user must pass three arguments: the input file path, the output file path and the
 *        number of bits to reduce each audio sample (The number of bits to remove of the sample, not the final number
 *        of bits).
 *
 *        Usage: point6 <input file> <output file> <nbits>
 *
 *        At the end, the program prints the signal-to-noise ratio (SNR), as well as the maximum per sample absolute
 *        error of each audio channel.
 *
 * \author Miguel Carvalhosa
 * \author Tânia Ferreira
 * \author Gonçalo Cardoso
 */


#include <iostream>
#include <vector>
#include <math.h>
#include <sndfile.hh>


using namespace std;


/*
 * Constants
 */
constexpr size_t FRAMES_BUFFER_SIZE = 65536;    // Buffer for reading/writing frames


/*
 * Function Prototypes
 */
void printFileInfo(SndfileHandle sndfilein);
short quantizer(short value, unsigned char nbits);


int main(int argc, char *argv[]) {

    // Validate the number of arguments
    if(argc < 4) {
        cerr << "Usage: point6 <input file> <output file> <nbits>" << endl;
        return 1;
    }

    // Validate the input file
    SndfileHandle sndFileIn { argv[argc-3] };
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


    // Print some info about the input file
    cout << "Input File - ";
    printFileInfo(sndFileIn);

    short nBits = stoi(argv[argc-1]);
    cout << "Number of bits to use: " << nBits << endl;

    // Create the output file handler
    SndfileHandle sndFileOut { argv[argc-2], SFM_WRITE, sndFileIn.format(),
                               sndFileIn.channels(), sndFileIn.samplerate() };

    // Validate the output file
    if(sndFileOut.error()) {
        cerr << "Error: invalid output file" << endl;
        return 1;
    }


    size_t nFrames;     // Number of frames to read at each iteration
    long energyInL = 0;      // Left channel energy value
    long energyNoiseL = 0;   // Left channel noise energy value
    long energyInR = 0;      // Right channel energy value
    long energyNoiseR = 0;   // Right channel noise energy value
    int maxLError = 0;       // Maximum per left channel sample absolute error
    int maxRError = 0;       // Maximum per right channel sample absolute error
    int nchannels = sndFileIn.channels();       // Number of channels of the input file

    vector<short> samplesIn(FRAMES_BUFFER_SIZE * nchannels);        // Vector to hold the input file samples
    vector<short> samplesL(FRAMES_BUFFER_SIZE);                     // Vector to hold the input file left channel samples
    vector<short> samplesR(FRAMES_BUFFER_SIZE);                     // Vector to hold the input file right channel samples
    vector<short> samplesQuant(FRAMES_BUFFER_SIZE * nchannels);     // Vector to hold the quantized samples

    // Read the input file, FRAMES_BUFFER_SIZE samples at a time
    while((nFrames = sndFileIn.readf(samplesIn.data(), FRAMES_BUFFER_SIZE))) {
        for(size_t i = 0; i < nFrames*nchannels; i+=2) {             // Iterate through each read sample
            // The frames are ordered in the file as L1 R1 L2 R2 L3 R3 ...
            samplesL[i/2] = samplesIn[i];           // Store each left frame
            samplesR[i/2] = samplesIn[i+1];         // Store each right frame (not needed but might come in handy in the future! )
            // Reduce the number of bits in each sample
            short reducedL = quantizer(samplesL[i/2], nBits);
            short reducedR = quantizer(samplesR[i/2], nBits);
            // Store the reduced samples
            samplesQuant[i] = reducedL;
            samplesQuant[i+1] = reducedR;
            int noiseL = abs(samplesL[i/2] - reducedL);    // Calculate the left channel noise value
            int noiseR = abs(samplesR[i/2] - reducedR);    // Calculate the right channel noise value
            if(noiseL > maxLError) {
                maxLError = noiseL;
            }
            if(noiseR > maxRError) {
                maxRError = noiseR;
            }
            energyInL += pow(samplesL[i/2], 2);       // Accumulate the left channel signal energy
            energyNoiseL += pow(noiseL, 2);           // Accumulate the left channel noise energy
            energyInR += pow(samplesR[i/2], 2);       // Accumulate the right channel signal energy
            energyNoiseR += pow(noiseR, 2);           // Accumulate the right channel noise energy
        }
        sndFileOut.writef(samplesQuant.data(), nFrames);        // Write the quantized samples to the output file
    }


    // Print the results
    double SNR_L = 10*log10((double)(energyInL) / (double)(energyNoiseL));
    cout << "SNR (Left channel): " << SNR_L << " dB" << endl;
    cout << "Max per sample absolute error (left channel): " << maxLError << endl;
    double SNR_R = 10*log10((double)(energyInR) / (double)(energyNoiseR));
    cout << "SNR (Right channel): " << SNR_R << " dB" << endl;
    cout << "Max per sample absolute error (right channel): " << maxRError << endl;


    return 0;
}


/**
 * \brief A function to print some information  about the audio file.
 *
 * \param file          Handler for the audio file
 *
 */
void printFileInfo(SndfileHandle file) {
    cout << "File has:" << endl;
    cout << '\t' << file.frames() << " frames" << endl;
    cout << '\t' << file.samplerate() << " samples per second" << endl;
    cout << '\t' << file.channels() << " channels" << endl;
}


/**
 * \brief A function to quantize a sample.
 *
 * \param value         Sample to be quantized
 * \param nbits         Number of bits to reduce in the quantized sample
 *
 * \return The quantized value
 *
 */
short quantizer(short value, unsigned char nbits) {
    return ((value >> nbits) << nbits);
}