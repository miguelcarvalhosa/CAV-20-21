
#include "VideoCodec.h"

VideoCodec::VideoCodec() {

}

VideoCodec::~VideoCodec() {

}

void VideoCodec::setIntraCodingParameters(predictorType predictor, unsigned int estimationBlockSize) {
    this->predictor = predictor;
    this->estimationBlockSize = estimationBlockSize;
}

void VideoCodec::compress( std::string &inputFile, std::string &compressedFile, unsigned int initial_m, parameterEstimationMode estimation) {

    std::ifstream inFile(inputFile);

    GolombEncoder encoder(40, compressedFile);

    std::string headerStr;
    getline(inFile, headerStr);
    inFileData = parseHeader(headerStr);

    this->initial_m = initial_m;
    this->estimation = estimation;

    unsigned int numFrames = calcNFrames(inputFile, inFileData);
    compressedHeaderBuild(headerStr, this -> initial_m, numFrames);
    std::cout << "Tamanho do cabeçalho: " <<headerStr.size() << std::endl;
    encoder.encode(headerStr.size());                                   // encodes the header size at the begining
    encoder.encodeHeader(headerStr.substr(0,headerStr.size())); // encodes the header
    encoder.update(initial_m);

    unsigned char* lastFrameBuf = new unsigned char[inFileData.width*inFileData.height * 3 / 2];
    unsigned char* frameBuf = NULL;
    unsigned int nFrames = 0;

    while(!inFile.eof()) {
        if(inFileData.format == VIDEO_FORMAT_444) {
            delete frameBuf;
            frameBuf = convertFrame_444to420(readFrame(&inFile));
            /* new frame dimensions of 420 format */
            inFileData.uv_width = inFileData.width/2;
            inFileData.uv_height = inFileData.height/2;
        } else if(inFileData.format == VIDEO_FORMAT_422) {
            delete frameBuf;
            frameBuf = convertFrame_422to420(readFrame(&inFile));
            /* new frame dimensions of 420 format */
            inFileData.uv_width = inFileData.width/2;
            inFileData.uv_height = inFileData.height/2;
        } else {
            delete frameBuf;
            frameBuf = readFrame(&inFile);
        }

        encodeIntra(encoder, frameBuf);
        printf("encoded frame %d -> INTRA\n", nFrames);

        memcpy(lastFrameBuf, frameBuf, inFileData.width * inFileData.height * 3 / 2 );
        if(inFileData.format == VIDEO_FORMAT_444) {
            /* restore frame dimensions to 444 frame dimensions */
            inFileData.uv_width = inFileData.width;
            inFileData.uv_height = inFileData.height;
        } else if (inFileData.format == VIDEO_FORMAT_422) {
            /* restore frame dimensions to 422 frame dimensions */
            inFileData.uv_height = inFileData.height;
            inFileData.uv_width = inFileData.width/2;
        }
        nFrames++;
    }
    encoder.close();
}

void VideoCodec::decompress(std::string &outputFile, std::string &compressedFile) {

    std::ofstream outFile(outputFile);

    GolombDecoder decoder(40, compressedFile);

    unsigned int headerSize = decoder.decode();
    std::string headerStr = decoder.decodeHeader(headerSize);
    inFileData = parseCompressedHeader(headerStr);

    this->initial_m = inFileData.golombM;
    this->estimationBlockSize = inFileData.estimationBlockSize;
    this->estimation = inFileData.estimation;
    this->predictor = inFileData.predictor;

    outFile << headerStr.substr(headerStr.find("Y"),headerStr.find("m") - headerStr.find("Y")-1) << std::endl;

    inFileData.uv_width = inFileData.width/2;
    inFileData.uv_height = inFileData.height/2;

    decoder.update(initial_m);


    unsigned char* frameBuf = NULL;
    unsigned char* lastFrameBuf = new unsigned char[inFileData.width*inFileData.height* 3 / 2];

    unsigned int i=0;
    while(i<inFileData.frameCount) {
        delete frameBuf;
        frameBuf = decodeIntra(decoder);
        printf("decoded frame %d -> INTRA\n", i);
        memcpy(lastFrameBuf, frameBuf, inFileData.width * inFileData.height * 3 / 2);
        writeFrame(&outFile, lastFrameBuf);
        i++;
    }
    decoder.close();
    outFile.close();
}

void VideoCodec::encodeIntra(GolombEncoder& encoder, unsigned char* &frameBuf) {
    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    short y, u, v, pred_y, pred_u, pred_v, res_y, res_u, res_v;
    short left_sample, top_sample, top_left_sample;

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

            if(estimation == ESTIMATION_ADAPTATIVE) {
                estimatedBlocks++;
                /* convert samples values to a positive range in order to perform the M estimation */
                res_y_mod = res_y > 0    ?   2 * res_y - 1  :   -2 * res_y;
                sum_y = sum_y + res_y_mod;
                if(estimatedBlocks == estimationBlockSize)
                {
                    m = estimateM_fromBlock(sum_y, estimationBlockSize);
                    encoder.update(m);
                    sum_y = 0;
                    estimatedBlocks = 0;
                }
            }
        }
    }
}

unsigned char* VideoCodec::decodeIntra(GolombDecoder& decoder) {
    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3 / 2];

    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    short y, u, v, pred_y, pred_u, pred_v, res_y, res_u, res_v;
    short left_sample, top_sample, top_left_sample;

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
                if(estimatedBlocks == estimationBlockSize)
                {
                    m = estimateM_fromBlock(sum_y, estimationBlockSize);
                    decoder.update(m);
                    sum_y = 0;
                    estimatedBlocks = 0;
                }
            }
        }
    }
    return frameBuf;
}

unsigned char* VideoCodec::readFrame(std::ifstream* fp) {
    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height)];

    std::string foo;
    getline (*fp, foo); // Skipping word FRAME

    fp->read((char *)frameBuf, inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height));

    return frameBuf;
}

void VideoCodec::writeFrame(std::ofstream* fp, unsigned char* frameBuf) {
    *fp << "FRAME" << std::endl;
    *fp << frameBuf;
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
    for (int r = 0; r < inFileData.height; r+=2) {
        for (int c = 0; c < inFileData.width; c+=2) {
            frameBuf420[(r/2)*(inFileData.width/2)+(c/2) + inFileData.width*inFileData.height] = frameBuf[r*inFileData.width + c + inFileData.width*inFileData.height];
            frameBuf420[(r/2)*(inFileData.width/2)+(c/2) + inFileData.width*inFileData.height + inFileData.width*inFileData.height/4] = frameBuf[r*inFileData.width + c + inFileData.width*inFileData.height*2];
        }
    }
    return frameBuf420;
}

unsigned char* VideoCodec::convertFrame_422to420(unsigned char* frameBuf) {
    unsigned char* frameBuf420 = new unsigned char[inFileData.width * inFileData.height * 3/2];
    memcpy(frameBuf420, frameBuf, inFileData.width * inFileData.height);
    for (int r = 0; r < inFileData.height; r+=2) {
        for (int c = 0; c < inFileData.uv_width; c++) {
            frameBuf420[(r/2)*(inFileData.uv_width)+(c) + inFileData.width*inFileData.height] = frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height];
            frameBuf420[(r/2)*(inFileData.uv_width)+(c) + inFileData.width*inFileData.height + inFileData.width*inFileData.height/4] = frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height*3/2];
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


// makes the string to be written in the header. Counts with the basic info from the input file already
void VideoCodec::compressedHeaderBuild(std::string& compressedHeader, int m, int nFrames) {

    if(inFileData.format == VIDEO_FORMAT_444){
        compressedHeader.replace(compressedHeader.find("C") + 1, 3, "420");
    }
    else if(inFileData.format == VIDEO_FORMAT_422){
        compressedHeader.replace(compressedHeader.find("C") + 1, 3, "420");
    }
    else{

    }
    std::string str_m = " m" + std::to_string(m);
    compressedHeader += str_m;
    std::string str_nFrames = " NF" + std::to_string(nFrames);
    compressedHeader += str_nFrames;
    std::string str_estimationBlockSize = " EBS" + std::to_string(this->estimationBlockSize);
    compressedHeader += str_estimationBlockSize;
    std::string str_estimation = " ES" + std::to_string(estToInt(estimation));
    compressedHeader += str_estimation;
    std::string str_predictor = " PR" + std::to_string(this->predictor);
    compressedHeader += str_predictor;
    std::cout << compressedHeader;

}

int VideoCodec::calcNFrames(std::string inputFile, fileData inFileData) {
    std::ifstream in_file(inputFile, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    int file_size = in_file.tellg();
    int nFrames;
    if(inFileData.format == VIDEO_FORMAT_444) {
        nFrames = file_size/(inFileData.width*inFileData.height*3);
    }
    else if(inFileData.format == VIDEO_FORMAT_422) {
        nFrames = file_size/(inFileData.width*inFileData.height*2);
    }
    else{
        nFrames = file_size/(inFileData.width*inFileData.height*(3/2));
    }
    return nFrames;
    //std::cout<<"Number of frames is"<<" "<< nFrames<<" "<<"bytes";
}

int VideoCodec::estToInt(parameterEstimationMode estimation) {
    switch(estimation) {
        case ESTIMATION_NONE:
            return 0;
        case ESTIMATION_ADAPTATIVE:
            return 1;
        default:
            return -1;
    }
}

VideoCodec::fileData VideoCodec::parseCompressedHeader(std::string header) {
    fileData data;            // Create a file data structure
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

    data.golombM = stoi(data.header.substr(data.header.find(" m") + 2, data.header.find(" m") - data.header.find(" N") - 2));
    std::cout << "m: " << data.golombM << std::endl;

    /* delete this after bug is corrected*/
    data.frameCount = 500;
            //stoi(data.header.substr(data.header.find(" N") + 3, data.header.find(" N") - data.header.find(" B") - 2));
    //std::cout << "N frames: " << data.frameCount << std::endl;

    data.estimationBlockSize = stoi(data.header.substr(data.header.find("EBS") + 3, data.header.find("EBS") - data.header.find("ES") - 1));
    std::cout << "Estimation Block Size: " << data.estimationBlockSize << std::endl;

    int estimation = stoi(data.header.substr(data.header.find("ES") + 2, data.header.find(" E") - data.header.find(" Z") - 2));
    if(estimation == 0) {
        data.estimation = ESTIMATION_NONE;
    }
    else{
        data.estimation = ESTIMATION_ADAPTATIVE;
    }
    std::cout << "Estimation:" << data.estimation << std::endl;

    data.predictor = (predictorType) stoi(data.header.substr(data.header.find("PR") + 2, data.header.find("PR") - data.header.find("IFP") - 1));
    std::cout << "Predictor: " << data.predictor << std::endl;

    return data;
}
