//
// Created by miguel on 17/12/20.
//

#include "VideoCodec.h"

VideoCodec::VideoCodec() {

}

VideoCodec::~VideoCodec() {

}

void VideoCodec::setIntraCodingParameters(predictorType predictor, unsigned int intraFramePeriodicity, unsigned int estimationBlockSize) {
    this->predictor = predictor;
    this->estimationBlockSize = estimationBlockSize;
    this->intraFramePeriodicity = intraFramePeriodicity;
}
void VideoCodec::setInterCodingParameters(blockSearchMode searchMode, unsigned int blockSize, unsigned int searchArea) {
    if(log2(blockSize)==ceil(log2(blockSize))) {
        this->blockSize = blockSize;
    } else {
        // round to the nearest power of two that minimizes the diff between the specified value and the used one
        if(abs(blockSize-pow(2,floor(log2(blockSize)))) < abs(blockSize-pow(2,ceil(log2(blockSize))))) {
            this->blockSize = pow(2,floor(log2(blockSize)));
        } else {
            this->blockSize = pow(2,ceil(log2(blockSize)));
        }
        std::wcerr << "The specified block size value is not a power of 2, the value was rounded to the neareast power of 2 -> " << this->blockSize << std::endl;
    }
    this->searchArea = searchArea;
    this->searchMode = searchMode;
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

        if((nFrames%intraFramePeriodicity) == 0) {
            encodeIntra(encoder, frameBuf);
            printf("encoded frame %d -> INTRA\n", nFrames);
        } else {
            encodeInter(encoder, frameBuf, lastFrameBuf, blockSize, searchArea);
            //printf("encoded frame %d -> INTER\n", nFrames);
        }

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
    this->intraFramePeriodicity = inFileData.intraFramePeriodicity;
    this->searchMode = inFileData.searchMode;
    this->blockSize = inFileData.blockSize;
    this->searchArea = inFileData.searchArea;

    outFile << headerStr.substr(headerStr.find("Y"),headerStr.find("m") - headerStr.find("Y")-1) << std::endl;

    inFileData.uv_width = inFileData.width/2;
    inFileData.uv_height = inFileData.height/2;

    decoder.update(initial_m);

    unsigned int nFrames = 50;
    unsigned char* frameBuf = NULL;
    unsigned char* lastFrameBuf = new unsigned char[inFileData.width*inFileData.height* 3 / 2];

    unsigned int i=0;
    while(i<inFileData.frameCount) {
        delete frameBuf;
        if((i%intraFramePeriodicity)==0) {
            frameBuf = decodeIntra(decoder);
            printf("decoded frame %d -> INTRA\n", i);
        } else {
            frameBuf = decodeInter(decoder, lastFrameBuf, blockSize);
            //printf("decoded frame %d -> INTER\n", i);
        }
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

void VideoCodec::encodeInter(GolombEncoder& encoder, unsigned char* &frameBuf, unsigned char* &lastFrameBuf, int blockSize, int searchArea) {

    blockEstimationData bestBlockMatch;
    bestBlockMatch.setSize(blockSize);

    unsigned char* frameBlockBuf = NULL;

    for(int r=0; r<inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize ) {
            if(r < inFileData.uv_height && c < inFileData.uv_width) {
                delete frameBlockBuf;
                frameBlockBuf = getFrameBlock(frameBuf, c, r, blockSize);
                bestBlockMatch = motionEstimation(frameBlockBuf, lastFrameBuf, c, r, blockSize, searchArea, YUV, searchMode);
                encodeFrameBlock(encoder, bestBlockMatch, YUV);
            } else {
                delete frameBlockBuf;
                frameBlockBuf = getFrameBlock(frameBuf, c, r, blockSize);
                bestBlockMatch = motionEstimation(frameBlockBuf, lastFrameBuf, c, r, blockSize, searchArea, Y, searchMode);
                encodeFrameBlock(encoder, bestBlockMatch, Y);
            }
        }
    }
}

void VideoCodec::encodeFrameBlock(GolombEncoder& encoder, blockEstimationData& bestMatchData, planeComponent plane) {
    unsigned int sum_y = 0, m, res_y_mod;
    int blockSize = bestMatchData.size;

    switch (plane) {
        case Y:
            encoder.encode(bestMatchData.motionVector_y.x);
            encoder.encode(bestMatchData.motionVector_y.y);

            for (int i=0; i< blockSize*blockSize; i++) {
                encoder.encode(bestMatchData.residuals_y[i]);
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod = bestMatchData.residuals_y[i] > 0    ?   2 * bestMatchData.residuals_y[i] - 1  :   -2 * bestMatchData.residuals_y[i];
                    sum_y = sum_y + res_y_mod;
                    if(i == blockSize*blockSize - 1)
                    {
                        m = estimateM_fromBlock(sum_y, blockSize*blockSize);
                        encoder.update(m);
                        sum_y = 0;
                    }
                }
            }
            break;
        case YUV:
            encoder.encode(bestMatchData.motionVector_y.x);
            encoder.encode(bestMatchData.motionVector_y.y);

            encoder.encode(bestMatchData.motionVector_u.x);
            encoder.encode(bestMatchData.motionVector_u.y);

            encoder.encode(bestMatchData.motionVector_v.x);
            encoder.encode(bestMatchData.motionVector_v.y);

            for (int i=0; i< blockSize*blockSize; i++) {
                encoder.encode(bestMatchData.residuals_y[i]);
                encoder.encode(bestMatchData.residuals_u[i]);
                encoder.encode(bestMatchData.residuals_v[i]);
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod = bestMatchData.residuals_y[i] > 0    ?   2 * bestMatchData.residuals_y[i] - 1  :   -2 * bestMatchData.residuals_y[i];
                    sum_y = sum_y + res_y_mod;
                    if(i == blockSize*blockSize - 1)
                    {
                        m = estimateM_fromBlock(sum_y, blockSize*blockSize);
                        encoder.update(m);
                        sum_y = 0;
                    }
                }
            }
            break;
        default:
            break;
    }
}

VideoCodec::blockEstimationData VideoCodec::motionEstimation(unsigned char* &frameBlockBuf, unsigned char* &lastFrameBuf, int block_x, int block_y, int blockSize, int searchArea, planeComponent plane, blockSearchMode searchMode) {
    Point motionVector;

    blockEstimationData bestBlockMatch;
    bestBlockMatch.setSize(blockSize);

    /* block delimeters */
    blockLimits limits = calculateBlockLimits(block_x,block_y,blockSize,searchArea);

    std::vector<short>  res_y(blockSize*blockSize);
    std::vector<short>  res_u(blockSize*blockSize);
    std::vector<short>  res_v(blockSize*blockSize);

    double bestError_y = std::numeric_limits<double>::max(), blockError_y=0;
    double bestError_u = std::numeric_limits<double>::max(), blockError_u=0;
    double bestError_v = std::numeric_limits<double>::max(), blockError_v=0;

    unsigned char* refBlockBuf_y = NULL;
    unsigned char* refBlockBuf_u = NULL;
    unsigned char* refBlockBuf_v = NULL;

    if(searchMode == EXHAUSTIVE) {
        for(int y=limits.topLeftPos.y; y<limits.bottomLeftPos.y; y++) {
            for(int x=limits.topLeftPos.x; x<limits.topRightPos.x; x++) {
                delete refBlockBuf_y;
                delete refBlockBuf_u;
                delete refBlockBuf_v;
                /* get each block from the specified previous frame' search area  */
                refBlockBuf_y = getFrameBlock_component(lastFrameBuf, x, y, blockSize, Y);
                if(plane == YUV) {
                    refBlockBuf_u = getFrameBlock_component(lastFrameBuf, x, y, blockSize, U);
                    refBlockBuf_v = getFrameBlock_component(lastFrameBuf, x, y, blockSize, V);
                }
                /* determine each block motion vector */
                motionVector.setPos(block_x - x, block_y - y);

                for(int r=0; r < blockSize; r++) {
                    for(int c=0; c < blockSize; c++) {
                        res_y[r*blockSize + c] = frameBlockBuf[r*blockSize + c] - refBlockBuf_y[r*blockSize + c];
                        blockError_y = blockError_y + std::abs(res_y[r*blockSize + c]);
                        if(plane == YUV) {
                            res_u[r*blockSize + c] = frameBlockBuf[r*blockSize + c + blockSize*blockSize] - refBlockBuf_u[r*blockSize + c];
                            res_v[r*blockSize + c] = frameBlockBuf[r*blockSize + c + 2*blockSize*blockSize] - refBlockBuf_v[r*blockSize + c];
                            blockError_u = blockError_u + std::abs(res_u[r*blockSize + c]);
                            blockError_v = blockError_v + std::abs(res_v[r*blockSize + c]);
                        }
                    }
                }
                blockError_y = blockError_y/(blockSize*blockSize);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;
                    bestBlockMatch.setResiduals(res_y, Y);
                    bestBlockMatch.setMotionVector(motionVector, Y);
                }
                if(plane == YUV) {
                    blockError_u = blockError_u/(blockSize*blockSize);
                    blockError_v = blockError_v/(blockSize*blockSize);

                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;

                        bestBlockMatch.setResiduals(res_u, U);
                        bestBlockMatch.setMotionVector(motionVector, U);
                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;

                        bestBlockMatch.setResiduals(res_v, V);
                        bestBlockMatch.setMotionVector(motionVector, V);
                    }
                }
            }
        }
    } else {
        for(int y=limits.topLeftPos.y; y<limits.bottomLeftPos.y; y=y+4) {
            for(int x=limits.topLeftPos.x; x<limits.topRightPos.x; x=x+4) {
                delete refBlockBuf_y;
                delete refBlockBuf_u;
                delete refBlockBuf_v;
                /* get each block from the specified previous frame' search area  */
                refBlockBuf_y = getFrameBlock_component(lastFrameBuf, x, y, blockSize, Y);
                if(plane == YUV) {
                    refBlockBuf_u = getFrameBlock_component(lastFrameBuf, x, y, blockSize, U);
                    refBlockBuf_v = getFrameBlock_component(lastFrameBuf, x, y, blockSize, V);
                }
                /* determine each block motion vector */
                motionVector.setPos(block_x - x, block_y - y);

                for(int r=0; r < blockSize; r++) {
                    for(int c=0; c < blockSize; c++) {
                        res_y[r*blockSize + c] = frameBlockBuf[r*blockSize + c] - refBlockBuf_y[r*blockSize + c];
                        blockError_y = blockError_y + std::abs(res_y[r*blockSize + c]);
                        if(plane == YUV) {
                            res_u[r*blockSize + c] = frameBlockBuf[r*blockSize + c + blockSize*blockSize] - refBlockBuf_u[r*blockSize + c];
                            res_v[r*blockSize + c] = frameBlockBuf[r*blockSize + c + 2*blockSize*blockSize] - refBlockBuf_v[r*blockSize + c];
                            blockError_u = blockError_u + std::abs(res_u[r*blockSize + c]);
                            blockError_v = blockError_v + std::abs(res_v[r*blockSize + c]);
                        }
                    }
                }
                blockError_y = blockError_y/(blockSize*blockSize);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;

                    bestBlockMatch.setResiduals(res_y, Y);
                    bestBlockMatch.setMotionVector(motionVector, Y);
                }
                if(plane == YUV) {
                    blockError_u = blockError_u/(blockSize*blockSize);
                    blockError_v = blockError_v/(blockSize*blockSize);

                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;

                        bestBlockMatch.setResiduals(res_u, U);
                        bestBlockMatch.setMotionVector(motionVector, U);
                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;
                        bestBlockMatch.setResiduals(res_v, V);
                        bestBlockMatch.setMotionVector(motionVector, V);

                    }
                }
            }
        }
    }
    return bestBlockMatch;
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

unsigned char* VideoCodec::decodeInter(GolombDecoder& decoder, unsigned char* &lastFrameBuf, int blockSize) {

    unsigned char * frameBuf = new unsigned char[inFileData.height * inFileData.width * 3 /2];
    unsigned char * frameBlockBuf = NULL;

    int nLines = blockSize;

    for(int r = 0; r < inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize) {
            delete frameBlockBuf;
            frameBlockBuf = decodeFrameBlock(decoder, lastFrameBuf, blockSize, c, r);
            for(int n = 0; n < nLines; n++) {
                memcpy(frameBuf + inFileData.width*(n+r) + c, frameBlockBuf + n*blockSize, blockSize);
                if(r < inFileData.uv_height && c < inFileData.uv_width) {
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height, frameBlockBuf + blockSize*blockSize + n*blockSize, blockSize);
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height, frameBlockBuf + 2*blockSize*blockSize + n*blockSize, blockSize);
                }
            }
        }
    }
    return frameBuf;
}

unsigned char* VideoCodec::decodeFrameBlock(GolombDecoder& decoder, unsigned char* &lastFrameBuf, int blockSize, int x, int y) {

    Point motionVector_y, motionVector_u, motionVector_v, refPos_y, refPos_u, refPos_v;

    unsigned char * frameBlockBuf = new unsigned char[blockSize*blockSize*3];
    unsigned char * refBlockBuf_y = NULL;
    unsigned char * refBlockBuf_u = NULL;
    unsigned char * refBlockBuf_v = NULL;

    int res_y, res_u, res_v;

    motionVector_y.x = decoder.decode();
    motionVector_y.y = decoder.decode();

    refPos_y.x =  x - motionVector_y.x;
    refPos_y.y =  y - motionVector_y.y;

    delete refBlockBuf_y;
    delete refBlockBuf_u;
    delete refBlockBuf_v;

     if(x<inFileData.uv_width && y< inFileData.uv_height) {
         motionVector_u.x = decoder.decode();
         motionVector_u.y = decoder.decode();
         motionVector_v.x = decoder.decode();
         motionVector_v.y = decoder.decode();

         refPos_u.x =  x - motionVector_u.x;
         refPos_u.y =  y - motionVector_u.y;
         refPos_v.x =  x - motionVector_v.x;
         refPos_v.y =  y - motionVector_v.y;

         refBlockBuf_y = getFrameBlock_component(lastFrameBuf, refPos_y.x, refPos_y.y, blockSize,Y);
         refBlockBuf_u = getFrameBlock_component(lastFrameBuf, refPos_u.x, refPos_u.y, blockSize,U);
         refBlockBuf_v = getFrameBlock_component(lastFrameBuf, refPos_v.x, refPos_v.y, blockSize,V);

     } else {
         refBlockBuf_y = getFrameBlock_component(lastFrameBuf, refPos_y.x, refPos_y.y, blockSize,Y);
     }

    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    for (int r = 0; r < blockSize; r++) {
        for (int c = 0; c < blockSize; c++) {
            res_y = decoder.decode();
            frameBlockBuf[r*blockSize + c] = res_y + refBlockBuf_y[r*blockSize + c];

            if(x < inFileData.uv_width && y <  inFileData.uv_height) {
                res_u = decoder.decode();
                res_v = decoder.decode();
                frameBlockBuf[r*blockSize + c + blockSize*blockSize] = res_u + refBlockBuf_u[r*blockSize + c];
                frameBlockBuf[r*blockSize + c + 2*blockSize*blockSize] = res_v + refBlockBuf_v[r*blockSize + c];

            }
            if(estimation == ESTIMATION_ADAPTATIVE) {
                /* convert residual values to a positive range in order to perform the M estimation */
                res_y_mod = res_y > 0    ?   2 * res_y - 1  :   -2 * res_y;
                sum_y = sum_y + res_y_mod;
                if((r == blockSize -1) && (c == blockSize-1))
                {
                    m = estimateM_fromBlock(sum_y, blockSize*blockSize);
                    decoder.update(m);
                    sum_y = 0;
                    estimatedBlocks = 0;
                }
            }
        }
    }
    return frameBlockBuf;
}
unsigned char* VideoCodec::getFrameBlock_component(unsigned char* &frameBuf, int x, int y, int blockSize, planeComponent plane){
    unsigned char* frameBlockBuf = new unsigned char [blockSize*blockSize];

    for (int r = 0; r < blockSize; r++) {
        for (int c = 0; c < blockSize; c++) {
            if(plane == Y) {
                frameBlockBuf[r * blockSize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
            } else if(plane == U) {
                frameBlockBuf[r* blockSize + c] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
            } else if(plane == V) {
                frameBlockBuf[r * blockSize + c ] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height];
            }
        }
    }
    return frameBlockBuf;
}

unsigned char* VideoCodec::getFrameBlock(unsigned char* &frameBuf, int x, int y, int blockSize) {
    unsigned char* frameBlockBuf = new unsigned char [blockSize*blockSize*3];

    for (int r = 0; r < blockSize; r++) {
        for (int c = 0; c < blockSize; c++) {
                frameBlockBuf[r * blockSize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
                frameBlockBuf[r* blockSize + c + blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
                frameBlockBuf[r* blockSize + c + 2*blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height];
        }
    }
    return frameBlockBuf;
}

VideoCodec::blockLimits VideoCodec::calculateBlockLimits(int x, int y, int blockSize, int searchArea) {

    blockLimits limits;
    Point topLeftPos, topRightPos, bottomLeftPos;
    if((x - searchArea < 0) && (y - searchArea < 0)) {
        topLeftPos.x = x;
        topLeftPos.y = y;

        topRightPos.x = x + blockSize + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = y + blockSize + searchArea - 1;
    } else if( (x - searchArea < 0) && (y - searchArea >= 0)) {
        topLeftPos.x = x;
        topLeftPos.y = y - searchArea;

        topRightPos.x = x + blockSize + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = y + blockSize + searchArea - 1;
    } else if( (x - searchArea >= 0) && (y - searchArea < 0)) {
        topLeftPos.x = x - searchArea;
        topLeftPos.y = y;

        topRightPos.x = x + blockSize + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = y + blockSize + searchArea - 1;
    } else {
        topLeftPos.x = x - searchArea;
        topLeftPos.y = y - searchArea;

        topRightPos.x = x + blockSize + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = y + blockSize + searchArea - 1;
    }
    limits.setBlockLimits(topLeftPos, topRightPos, bottomLeftPos);

    return limits;
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
    std::string str_intraFramePeriodicity  = " IFP" + std::to_string(this->intraFramePeriodicity);
    compressedHeader += str_intraFramePeriodicity;
    std::string str_searchMode  = " SM" + std::to_string(this->searchMode);
    compressedHeader += str_searchMode;
    std::string str_blockSize  = " BS" + std::to_string(this->blockSize);
    compressedHeader += str_blockSize;
    std::string str_searchArea  = " SA" + std::to_string(this->searchArea) + " Z\n";
    compressedHeader += str_searchArea;
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

    data.frameCount = stoi(data.header.substr(data.header.find(" N") + 3, data.header.find(" N") - data.header.find(" B") - 2));
    std::cout << "N frames: " << data.frameCount << std::endl;

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

    data.intraFramePeriodicity = stoi(data.header.substr(data.header.find("IFP") + 3, data.header.find("IFP") - data.header.find("SM") - 1));
    std::cout << "Intra Frame Periodicity: " << data.intraFramePeriodicity << std::endl;

    data.searchMode = (blockSearchMode) stoi(data.header.substr(data.header.find("SM") + 2, data.header.find("SM") - data.header.find("BS") - 1));
    std::cout << "Search Mode: " << data.searchMode << std::endl;

    data.blockSize = stoi(data.header.substr(data.header.find(" B") + 3, data.header.find(" B") - data.header.find(" E") - 2));
    std::cout << "Block Size: " << data.blockSize << std::endl;

    data.searchArea = stoi(data.header.substr(data.header.find("SA") + 2, data.header.find("SA") - data.header.find("Z") - 1));
    std::cout << "Search Area: " << data.searchArea << std::endl;



    return data;
}
