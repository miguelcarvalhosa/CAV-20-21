//
// Created by miguel on 17/12/20.
//

#include "VideoCodec.h"

VideoCodec::VideoCodec(predictorType predictor, unsigned int initial_m, parameterEstimationMode estimation, unsigned int estimation_block_size, lossMode loss, unsigned int lostBits) {
    this->predictor = predictor;
    this->initial_m = initial_m;
    this->estimation = estimation;
    this->estimation_block_size = estimation_block_size;
    this->loss = loss;
    this->lostBits = lostBits;
}

VideoCodec::~VideoCodec() {

}

void VideoCodec::compress(std::string inputFile, std::string compressedFile) {

    std::ifstream inFile(inputFile);
    std::ofstream outFile(compressedFile);

    GolombEncoder encoder(initial_m, compressedFile);   // ATENCAO AO M!! 40

    std::string headerStr;
    getline(inFile, headerStr);
    inFileData = parseHeader(headerStr);
    encoder.encodeHeader(headerStr.substr(0,34));

    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3/2];

    unsigned int nFrames=0, sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    short y, u, v, pred_y, pred_u, pred_v, res_y, res_u, res_v;
    short left_sample, top_sample, top_left_sample;

    while(!inFile.eof()) {
        if(inFileData.format == VIDEO_FORMAT_444) {
            unsigned char *frameBuf444 = readFrame(&inFile, VIDEO_FORMAT_444);
            frameBuf = convertFrame_444to420(frameBuf444);
            // new frame dimensions
            inFileData.uv_width = inFileData.width/2;
            inFileData.uv_height = inFileData.height/2;
        } else {
            frameBuf = readFrame(&inFile, VIDEO_FORMAT_420);
        }
        // FALTA IMPLEMENTAR A CONVERSÃO DE 422 PARA 420

        for(int r=0; r<inFileData.height; r++) {
            for(int c=0; c<inFileData.width; c++) {
                y = frameBuf[r*inFileData.width + c];
                if(r == 0 && c == 0) {
                    left_sample = 0;
                    top_sample = 0;
                    top_left_sample = 0;
                }
                else if(r == 0) {
                    left_sample = frameBuf[r*inFileData.width + c - 1];
                    top_sample = 0;
                    top_left_sample = 0;
                }
                else if(c == 0) {
                    left_sample = 0;
                    top_sample = frameBuf[(r-1)*inFileData.width + c];
                    top_left_sample = 0;
                }
                else {
                    left_sample = frameBuf[r*inFileData.width + c - 1];
                    top_sample = frameBuf[(r-1)*inFileData.width + c];
                    top_left_sample = frameBuf[(r-1)*inFileData.width + c - 1];
                }
                pred_y = predict(left_sample, top_sample, top_left_sample, predictor);
                res_y = y - pred_y;
                encoder.encode(res_y);
                if(r < inFileData.uv_height && c < inFileData.uv_width) {
                    u = frameBuf[r*inFileData.uv_width + c + inFileData.height*inFileData.width];
                    if(r == 0 && c == 0) {
                        left_sample = 0;
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(r == 0) {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width];
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(c == 0) {
                        left_sample = 0;
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width];
                        top_left_sample = 0;
                    }
                    else {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width];
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width];
                        top_left_sample = frameBuf[(r-1)*inFileData.uv_width+ c - 1 + inFileData.height*inFileData.width];
                    }
                    pred_u = predict(left_sample, top_sample, top_left_sample, predictor);
                    res_u = u - pred_u;
                    encoder.encode(res_u);
                    //std::cout << "u" << u << "-" << res_u << std::endl;
                    v = frameBuf[r*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                    if(r == 0 && c == 0) {
                        left_sample = 0;
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(r == 0) {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(c == 0) {
                        left_sample = 0;
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_left_sample = 0;
                    }
                    else {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_left_sample = frameBuf[(r-1)*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                    }
                    pred_v = predict(left_sample, top_sample, top_left_sample, predictor);
                    res_v = v - pred_v;
                    encoder.encode(res_v);
                }
                /* if(nFrames<1 && r>=358 && r<365 && c >= 638 && c < 645) {
                    std::cout << "y: " << y << " u: " << u << " v: " << v << std::endl;
                }*/
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    estimatedBlocks++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod = res_y > 0    ?   2 * res_y - 1  :   -2 * res_y;
                    sum_y = sum_y + res_y_mod;
                    if(estimatedBlocks == estimation_block_size)
                    {
                        m = estimateM_fromBlock(sum_y, estimation_block_size);
                        //std::cout << "m: " << m << " sum: " << sum_y << std::endl;
                        encoder.update(m);
                        sum_y = 0;
                        estimatedBlocks = 0;
                    }

                }
            }
        }
        nFrames++;
        // restore frame dimensions in order to read the next 444 frame
        inFileData.uv_width = inFileData.width;
        inFileData.uv_height = inFileData.height;
    }
    encoder.close();

}
void VideoCodec::decompress(std::string outputFile, std::string compressedFile) {
    std::ifstream inFile(compressedFile);
    std::ofstream outFile(outputFile);

    GolombDecoder decoder(initial_m, compressedFile);   // ATENCAO AO M!! 40

    std::string headerStr;
    headerStr = decoder.decodeHeader(34);//34 420  e 39 chars se for 444 ou 422

    inFileData = parseHeader(headerStr);

    outFile << headerStr << std::endl;

    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3 / 2];

    unsigned int i=0, sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    short y, u, v, pred_y, pred_u, pred_v, res_y, res_u, res_v;
    short left_sample, top_sample, top_left_sample;

    while(i<500) { // FRAME NUMBER NEEDS TO BE DETERMINED AND BE PASSED AS AN ARGUMENT
        for(int r=0; r<inFileData.height; r++) {
            for(int c=0; c<inFileData.width; c++) {
                if(r == 0 && c == 0) {
                    left_sample = 0;
                    top_sample = 0;
                    top_left_sample = 0;
                }
                else if(r == 0) {
                    left_sample = frameBuf[r*inFileData.width + c - 1];
                    top_sample = 0;
                    top_left_sample = 0;
                }
                else if(c == 0) {
                    left_sample = 0;
                    top_sample = frameBuf[(r-1)*inFileData.width + c];
                    top_left_sample = 0;
                }
                else {
                    left_sample = frameBuf[r*inFileData.width + c - 1];
                    top_sample = frameBuf[(r-1)*inFileData.width + c];
                    top_left_sample = frameBuf[(r-1)*inFileData.width + c - 1];
                }
                res_y = decoder.decode();
                pred_y = predict(left_sample, top_sample, top_left_sample, predictor);
                y = res_y + pred_y;
                frameBuf[r*inFileData.width + c] = y;
                //std::cout << "y" << y << "-" << res_y << std::endl;

                if(r < inFileData.uv_height && c < inFileData.uv_width) {
                    if(r == 0 && c == 0) {
                        left_sample = 0;
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(r == 0) {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width];
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(c == 0) {
                        left_sample = 0;
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width];
                        top_left_sample = 0;
                    }
                    else {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width];
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width];
                        top_left_sample = frameBuf[(r-1)*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width];
                    }
                    res_u = decoder.decode();
                    pred_u = predict(left_sample, top_sample, top_left_sample, predictor);
                    u = res_u + pred_u;
                    frameBuf[r*inFileData.uv_width + c + inFileData.height*inFileData.width] = u;
                    //std::cout << "u" << u << "-" << res_u << std::endl;
                    if(r == 0 && c == 0) {
                        left_sample = 0;
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(r == 0) {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_sample = 0;
                        top_left_sample = 0;
                    }
                    else if(c == 0) {
                        left_sample = 0;
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_left_sample = 0;
                    }
                    else {
                        left_sample = frameBuf[r*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_sample = frameBuf[(r-1)*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                        top_left_sample = frameBuf[(r-1)*inFileData.uv_width + c - 1 + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width];
                    }
                    res_v = decoder.decode();
                    pred_v = predict(left_sample, top_sample, top_left_sample, predictor);
                    v = res_v + pred_v;
                    frameBuf[r*inFileData.uv_width + c + inFileData.height*inFileData.width + inFileData.uv_height*inFileData.uv_width] = v;
                }
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    estimatedBlocks++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod = res_y > 0    ?   2 * res_y - 1  :   -2 * res_y;
                    sum_y = sum_y + res_y_mod;
                    if(estimatedBlocks == estimation_block_size)
                    {
                        m = estimateM_fromBlock(sum_y, estimation_block_size);
                        decoder.update(m);
                        sum_y = 0;
                        estimatedBlocks = 0;
                    }
                }
                 /* if(i<1 && r>=358 && r<365 && c >= 638 && c < 645) {
                    std::cout << "y: " << y << " u: " << u << " v: " << v << std::endl;
                }*/
            }
        }
        i++;
        writeFrame(&outFile, frameBuf);
    }
    decoder.close();
    outFile.close();
}


unsigned char* VideoCodec::readFrame(std::ifstream* fp, videoFormat format) {
    unsigned char* frameBuf = frameBuf = new unsigned char[inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height)];

    std::string foo;
    getline (*fp, foo); // Skipping word FRAME

    fp->read((char *)frameBuf, inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height));

    return frameBuf;
}

void VideoCodec::writeFrame(std::ofstream* fp, unsigned char* frameBuf) {
    *fp << "FRAME" << std::endl;
    *fp << frameBuf;
    //fp->write(reinterpret_cast<const char*>(frameBuf), inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height));
}

unsigned char* VideoCodec::convertFrame_422to444(unsigned char* frameBuf) {
    unsigned char* frameBuf444 = new unsigned char[inFileData.width * inFileData.height * 3];
    memcpy(frameBuf444, frameBuf, inFileData.width * inFileData.height);
    for(int r = 0 ; r < inFileData.uv_height ; r++) {
        for(int c = 0 ; c < inFileData.uv_width ; c++) {
            for(int j = 0 ; j < 2 ; j++) {
                frameBuf444[(r * inFileData.width + (c * 2) + j + inFileData.width * inFileData.height)] = frameBuf[(r * inFileData.uv_width + c) + inFileData.width * inFileData.height];
                frameBuf444[(r * inFileData.width + (c * 2) + j + (inFileData.width * inFileData.height) * 2)] = frameBuf[(r * inFileData.uv_width + c) + inFileData.width * inFileData.height + inFileData.uv_width * inFileData.uv_height];
            }
        }
    }
    return frameBuf444;
}

unsigned char* VideoCodec::convertFrame_420to444(unsigned char* frameBuf) {
    unsigned char* frameBuf444 = new unsigned char[inFileData.width * inFileData.height * 3];
    memcpy(frameBuf444, frameBuf, inFileData.width * inFileData.height);
    for (int r = 0; r < inFileData.uv_height; r++) {
        for (int c = 0; c < inFileData.uv_width; c++) {
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    frameBuf444[(r * 2 + i) * inFileData.width + (c * 2 + j) + inFileData.width * inFileData.height] = frameBuf[(r * inFileData.uv_width + c) + inFileData.width * inFileData.height];
                    frameBuf444[(r * 2 + i) * inFileData.width + (c * 2 + j) + (inFileData.width * inFileData.height) * 2] = frameBuf[(r * inFileData.uv_width + c) + inFileData.width * inFileData.height + inFileData.uv_width * inFileData.uv_height];
                }
            }
        }
    }
    return frameBuf444;
}

unsigned char* VideoCodec::convertFrame_444to420(unsigned char* frameBuf) {
    unsigned char* frameBuf420 = new unsigned char[inFileData.width * inFileData.height * 3/2];
    memcpy(frameBuf420, frameBuf, inFileData.width * inFileData.height);
    /*for(int i=0; i<(inFileData.width*inFileData.height); i+=4) {
        frameBuf420[i/4 + inFileData.width*inFileData.height] = frameBuf[i + inFileData.width*inFileData.height];
        frameBuf420[i/4 + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height] = frameBuf[i + inFileData.width*inFileData.height*2];
    }*/
    for (int r = 0; r < inFileData.height; r+=2) {
        for (int c = 0; c < inFileData.width; c+=2) {
            frameBuf420[(r/2)*(inFileData.width/2)+(c/2) + inFileData.width*inFileData.height] = frameBuf[r*inFileData.width + c + inFileData.width*inFileData.height];
            frameBuf420[(r/2)*(inFileData.width/2)+(c/2) + inFileData.width*inFileData.height + inFileData.width*inFileData.height/4] = frameBuf[r*inFileData.width + c + inFileData.width*inFileData.height*2];
        }
    }
    return frameBuf420;
}

VideoCodec::fileData VideoCodec::parseHeader(std::string header) {
    fileData data;        // Create a file data structure
    data.header = header;     // Store the header in the file data structure

    // Extract the frame width from the header and store it in the file data structure
    data.width = stoi(data.header.substr(data.header.find(" W") + 2, data.header.find(" H") - data.header.find(" W") - 2));

    // Extract the frame height from the header and store it in the file data structure
    data.height = stoi(data.header.substr(data.header.find(" H") + 2, data.header.find(" F") - data.header.find(" H") - 2));

    // Extract the framerate from the header and store it in the file data structure
    std::string fps_string = data.header.substr(data.header.find(" F") + 2, data.header.find(" I") - data.header.find(" F") - 2);
    data.fps = (double)stoi(fps_string.substr(0, fps_string.find(":"))) / (double)stoi(fps_string.substr(fps_string.find(":") + 1));

    // Extract the color subsampling format from the header and store it in the file data structure
    if(data.header.find("C") != -1) {
        switch(stoi(data.header.substr(data.header.find("C") + 1, 4))) {
            case 444:
                data.format = VIDEO_FORMAT_444;
                break;
            case 422:
                data.format = VIDEO_FORMAT_422;
                break;
            case 420:
                data.format = VIDEO_FORMAT_420;
                break;
            default:
                std::cerr << "Error: Invalid video format" << std::endl;
                break;
        }
    }
    else {
        data.format = VIDEO_FORMAT_420;
    }

    // Print the data in the terminal
    std::cout << "Header: " << data.header << std::endl;
    std::cout << "Width: " << data.width << std::endl;
    std::cout << "Height: " << data.height << std::endl;
    std::cout << "FPS: " << data.fps << std::endl;

    // Based on the color subsampling format, calculate the U and V frame dimensions and store them in the file data structure
    if(data.format == VIDEO_FORMAT_444) {
        std::cout << "Format: 444" << std::endl;
        data.uv_width = data.width;
        data.uv_height = data.height;
    }
    else if(data.format == VIDEO_FORMAT_422) {
        std::cout << "Format: 422" << std::endl;
        data.uv_width = data.width / 2;
        data.uv_height = data.height;
    }
    else if(data.format == VIDEO_FORMAT_420) {
        std::cout << "Format: 420" << std::endl;
        data.uv_width = data.width / 2;
        data.uv_height = data.height / 2;
    }
    else {
        std::cerr << "Error: Invalid video format" << std::endl;
    }

    return data;
}

int VideoCodec::predict(int left_sample, int top_sample, int top_left_sample, predictorType predictor) {
    int predVal = 0;
    switch(predictor) {
        case PREDICTOR_LINEAR_JPEG_1:
            predVal = left_sample;
            break;
        case PREDICTOR_LINEAR_JPEG_2:
            predVal = top_sample;
            break;
        case PREDICTOR_LINEAR_JPEG_3:
            predVal = top_left_sample;
            break;
        case PREDICTOR_LINEAR_JPEG_4:
            predVal = left_sample + top_sample - top_left_sample;
            break;
        case PREDICTOR_LINEAR_JPEG_5:
            predVal = left_sample + (top_sample - top_left_sample)/2;
            break;
        case PREDICTOR_LINEAR_JPEG_6:
            predVal = top_sample + (left_sample - top_left_sample)/2;
            break;
        case PREDICTOR_LINEAR_JPEG_7:
            predVal = (left_sample + top_sample) / 2;
            break;
        case PREDICTOR_NON_LINEAR_JPEGLS:
            if(top_left_sample >= std::max(left_sample, top_sample)) {
                predVal = std::min(left_sample, top_sample);
            }
            else if(top_left_sample <= std::min(left_sample, top_sample)) {
                predVal = std::max(left_sample, top_sample);
            }
            else {
                predVal = left_sample + top_sample - top_left_sample;
            }
            break;
    }
    return predVal;
}

unsigned int VideoCodec::estimateM_fromBlock(unsigned int sum, unsigned int blockSize) {
    double mean, alfa;
    unsigned int m;

    /* compute the mean value of blockSize samples */
    mean = (sum/(double)(blockSize));
    /* estimate the geometric distribution parameter alfa */
    alfa = mean/(1+mean);
    /* compute the optimal m for a geometric distribution with parameter alfa */
    m = ceil(-1/log2(alfa));
    /* guarantee that the value of the m parameter is at least two */
    if(m<2) {
        m = 2;
    }
    return m;
}