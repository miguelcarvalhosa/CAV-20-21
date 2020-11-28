
#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <math.h>
#include <numeric>
#include <fstream>

#include "GolombEncoder.h"
#include "GolombDecoder.h"
using namespace std;

vector<float> audioEntropy(string fileName);
float residualsEntropy(string fileName);
void calculateAudioHist(string histFileName, string audioFileName);
void calculateResHist(string fileName_hist_res, vector<short> res);

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {
    /* vectors to store the samples from each channel and the calculated mono version */
    vector<short> buf_r(FRAMES_BUFFER_SIZE), buf_l(FRAMES_BUFFER_SIZE), buf_mono(FRAMES_BUFFER_SIZE);   // TIREI UNSIGNED Q ESTAVA !!!
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);
    /* vectors to store the element count of each channel and the mono version */
    vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    vector<short> r_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_0(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_1(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_2(FRAMES_BUFFER_SIZE * 2);
    vector<short> res_3(FRAMES_BUFFER_SIZE * 2);
    /* identifies the first frame of data */
    int frame = 0;


    if(argc < 3) {
        cerr << "Usage: wavcp <input file.wav> <output file.txt>" << endl;
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

    GolombEncoder encoder(1024,"Residuals.bin");
    int expected1, expected2;
    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            buf_r[i/2] = samples[i];      // Retirei passagem para indicies positivos   !!!
            buf_l[i/2]= samples[i+1];
            buf_mono[i/2] = ((buf_r[i/2] + buf_l[i/2])/2);
            if(frame == 0) {
                if (i == 0) {
                    res_0[i] = buf_r[i / 2];
                    res_0[i + 1] = buf_l[i / 2];
                    encoder.encode(res_0[i]);
                    encoder.encode(res_0[i + 1]);
                    expected1 = res_0[i];
                    expected2 = res_0[i + 1];

                } else if (i == 2) {

                    res_0[i] = buf_r[i / 2];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    encoder.encode(res_1[i]);

                    res_0[i + 1] = buf_l[i / 2];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    encoder.encode(res_1[i + 1]);
                } else if (i == 4) {

                    res_0[i] = buf_r[i / 2];
                    res_1[i] = res_0[i] - res_0[i - 2];
                    res_2[i] = res_1[i] - res_1[i - 2];
                    encoder.encode(res_2[i]);

                    res_0[i + 1] = buf_l[i / 2];
                    res_1[i + 1] = res_0[i + 1] - res_0[i - 1];
                    res_2[i + 1] = res_1[i + 1] - res_1[i - 1];
                    encoder.encode(res_2[i + 1]);
                    frame = 1;
                }
            }
            else if (i == 0) {

                res_0[i] = buf_r[i / 2];
                res_1[i] = res_0[i] - res_0[samples.size() - 2];
                res_2[i] = res_1[i] - res_1[samples.size() - 2];
                res_3[i] = res_2[i] - res_2[samples.size() - 2];
                encoder.encode(res_3[i]);

                res_0[i + 1] = buf_l[i / 2];
                res_1[i + 1] = res_0[i + 1] - res_0[samples.size() - 1];
                res_2[i + 1] = res_1[i + 1] - res_1[samples.size() - 1];
                res_3[i + 1] = res_2[i + 1] - res_2[samples.size() - 1];
                encoder.encode(res_3[i + 1]);
            }
            else {

                res_0[i] = buf_r[i/2];
                res_1[i] = res_0[i] - res_0[i-2];
                res_2[i] = res_1[i] - res_1[i-2];
                res_3[i] = res_2[i] - res_2[i-2];
                encoder.encode(res_3[i]);

                res_0[i+1] = buf_l[i/2];
                res_1[i+1] = res_0[i+1] - res_0[i-1];
                res_2[i+1] = res_1[i+1] - res_1[i-1];
                res_3[i+1] = res_2[i+1] - res_2[i-1];
                encoder.encode(res_3[i+1]);
            }

        }
    }
    encoder.close();
    GolombDecoder decoder(8192,"Residuals.bin");

    /* ORIGINAL AUDIO ENTROPY */
    calculateAudioHist("hist_2.txt", argv[1]);
    vector<float> ent = audioEntropy("hist_2.txt");
    cout << "Right channel entropy: " << ent[0] << endl;
    cout << "Left channel entropy: " << ent[1] << endl;
    cout << "Mono channel entropy: " << ent[2] << endl;

    calculateResHist("hist_res_0.txt", res_0);
    calculateResHist("hist_res_1.txt", res_1);
    calculateResHist("hist_res_2.txt", res_2);
    calculateResHist("hist_res_3.txt", res_3);

    float ent_res_0 = residualsEntropy("hist_res_0.txt");
    float ent_res_1 = residualsEntropy("hist_res_1.txt");
    float ent_res_2 = residualsEntropy("hist_res_2.txt");
    float ent_res_3 = residualsEntropy("hist_res_3.txt");

    cout << "Temporal predictior order 0: " << ent_res_0 << endl;
    cout << "Temporal predictior order 1: " << ent_res_1 << endl;
    cout << "Temporal predictior order 2: " << ent_res_2 << endl;
    cout << "Temporal predictior order 3: " << ent_res_3 << endl;

    return 0;
}

void calculateAudioHist(string histFileName, string audioFileName) {
    vector<short> samples(FRAMES_BUFFER_SIZE * 2);

    /* vectors to store the element count of each channel and the mono version */
    vector<long unsigned> hist_r((int)pow(2,16)), hist_l((int)pow(2,16)), hist_mono((int)pow(2,16));

    /* file for storing calculated frequencies */
    ofstream outfile(histFileName);

    SndfileHandle sndFileIn { audioFileName };
    if(sndFileIn.error()) {
        cerr << "Error: invalid input file" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
        cerr << "Error: file is not in WAV format" << endl;
    }

    if((sndFileIn.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
        cerr << "Error: file is not in PCM_16 format" << endl;
    }

    /* reads all frame from source audio file and computes for each channel the element count */
    while (sndFileIn.readf(samples.data(), FRAMES_BUFFER_SIZE)) {
        for(long unsigned int i=0; i< samples.size(); i=i+2) {
            /* NOTE: The addition of the 32767 constant is only needed to convert the negative
             *       samples into positive indexes that can be accessed in the hist vectors to
             *       count each element occurrence */
            hist_r[samples[i]+32767] += 1;
            hist_l[samples[i+1]+32767] += 1;
            hist_mono[((samples[i] + samples[i+1])/2) + 32767] += 1;
        }
    }
    /* write the computed element count */
    for (long unsigned int k=0; k< hist_r.size(); k++) {
        outfile << hist_r [k] << " " << hist_l[k] << " " << hist_mono[k] << endl;
    }
}

void calculateResHist(string fileName, vector<short> res) {
    /* vector to store the element count of the histogram */
    vector<long unsigned> hist_res((int)pow(2,16));
    /* file for writing the residuals histograms for each predictor order */
    ofstream outfile(fileName);
    for(unsigned int i=0; i< res.size(); i++) {
        hist_res[res[i]+32767] += 1;
    }
    for(unsigned int k=0; k < hist_res.size(); k++) {
        outfile << hist_res[k] << endl;
    }
}
vector<float> audioEntropy(string fileName) {
    // nº de bits médio/simbolo
    float Pi_r, Pi_l, Pi_mono;
    float ent_r = 0, ent_l = 0, ent_mono = 0;
    unsigned long int count_r = 0, count_l = 0, count_mono = 0;
    vector<float> ent(3);

    vector<unsigned int> hist_r(FRAMES_BUFFER_SIZE), hist_l(FRAMES_BUFFER_SIZE), hist_mono(FRAMES_BUFFER_SIZE);

    ifstream hist_file(fileName);

    unsigned int i=0;
    while(!hist_file.eof()){
        hist_file >> hist_r[i];
        hist_file >> hist_l[i];
        hist_file >> hist_mono[i];
        i++;
    }
    count_r = accumulate(hist_r.begin(), hist_r.end(), 0);
    count_l = accumulate(hist_l.begin(), hist_l.end(), 0);
    count_mono = accumulate(hist_mono.begin(), hist_mono.end(), 0);

    for (i=0;i<FRAMES_BUFFER_SIZE;i++)
    {
        if(hist_r[i]> 0) {
            Pi_r = (float)(hist_r[i])/count_r;
            ent_r -= Pi_r*log2(Pi_r);
        }
        if(hist_l[i]> 0) {
            Pi_l = (float)(hist_l[i])/count_l;
            ent_l -= Pi_l*log2(Pi_l);
        }
        if(hist_mono[i]> 0) {
            Pi_mono= (float)(hist_mono[i])/count_mono;
            ent_mono -= Pi_mono*log2(Pi_mono);
        }
    }
    ent[0] = ent_r;
    ent[1] = ent_l;
    ent[2] = ent_mono;
    return ent;
}

float residualsEntropy(string fileName) {
    float Pi;
    float ent = 0;
    unsigned long int count = 0;

    vector<unsigned int> hist_res(FRAMES_BUFFER_SIZE);

    ifstream hist_file(fileName);

    unsigned int i=0;
    while(!hist_file.eof()){
        hist_file >> hist_res[i];
        i++;
    }
    count = accumulate(hist_res.begin(), hist_res.end(), 0);

    for (i=0;i<FRAMES_BUFFER_SIZE;i++) {
        if (hist_res[i] > 0) {
            Pi = (float) (hist_res[i]) / count;
            ent -= Pi * log2(Pi);
        }
    }
    return ent;
}



