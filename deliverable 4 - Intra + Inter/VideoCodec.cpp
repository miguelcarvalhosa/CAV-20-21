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

    /* file.clear();
    file2.clear();
    decodedMotion.clear();
    encodedMotion.clear();
    decodedRefBlockPos.clear();
    encodedRefBlockPos.clear();
    decodedBlockFrameValue.clear();
    originalBlockFrameValue.clear();

    decodedBlockPos.clear();
    encodedBlockPos.clear();
    originalFrame.clear();
    decodedFrame.clear(); */


    std::ifstream inFile(inputFile);
    std::ofstream outFile(compressedFile);

    int blockSize = 16;
    int searchArea = 16;
    int intraFramePeriodicity = 20; // a cada 20 frames 1 é codificada como intra
    GolombEncoder encoder(initial_m, compressedFile);   // ATENCAO AO M!! 40

    std::string headerStr;
    getline(inFile, headerStr);
    inFileData = parseHeader(headerStr);
    encoder.encodeHeader(headerStr.substr(0,34));

    unsigned char* frameBuf;
    unsigned int nFrames=0;
 // !inFile.eof()
    while(nFrames<2) {
        if(inFileData.format == VIDEO_FORMAT_444) {
            frameBuf = convertFrame_444to420(readFrame(&inFile, VIDEO_FORMAT_444));
            // new frame dimensions
            inFileData.uv_width = inFileData.width/2;
            inFileData.uv_height = inFileData.height/2;
        } else {
            frameBuf = readFrame(&inFile, VIDEO_FORMAT_420);
        }
        if((nFrames%intraFramePeriodicity) == 0) {
            encodeIntra(encoder, frameBuf);
            std::cout << "encoded frames: " << nFrames << std::endl;
        } else {
            frame2Bufencode = frameBuf;
            encodeInter(encoder, frameBuf, blockSize, searchArea);
            for(int i=0; i<40; i++) {
                std::cout << (int)frameBuf[i + inFileData.width * inFileData.height + inFileData.uv_width * inFileData.uv_height ] << ", ";
            }
            std::cout << std::endl;
        }
        nFrames++;
        lastFrameBuf = frameBuf;
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

    int intraFramePeriodicity = 20; // a cada 20 frames 1 é codificada como intra
    int blockSize = 16, searchArea = 16;
    long int sum_elem=0, detect;
    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3 / 2];
    printf("---------------decoding------------\n");
    unsigned int i=0;
    while(i<2) { // FRAME NUMBER NEEDS TO BE DETERMINED AND BE PASSED AS AN ARGUMENT
        if((i%intraFramePeriodicity)==0) {
            frameBuf = decodeIntra(decoder);
            std::cout << "decoded frames: " << i << std::endl;
        } else {
            frameBuf = decodeInter(decoder, blockSize);
           /*  frame2Bufdecode = frameBuf;
           for(int i=0; i<40; i++) {
                std::cout << (int)frameBuf[i + inFileData.width * inFileData.height + inFileData.uv_width * inFileData.uv_height ] << ", ";
            }
            std::cout << std::endl;
            for(int i=0; i<inFileData.width * inFileData.height; i++) {
                //std::cout << (int)frameBuf[i] << ", ";
                if(frame2Bufencode[i] == frame2Bufdecode[i]) {
                    sum_elem = sum_elem + 1;
                } else {
                    std::cout << "diferent em " << i << std::endl;
                }
            }
            //if(sum_elem == (inFileData.width * inFileData.height * 3 / 2)-1) {
                printf("sum %d, element_number %d\n", sum_elem,inFileData.width * inFileData.height);
            //} */
        }
        i++;
        lastFrameBuf = frameBuf;
        writeFrame(&outFile, frameBuf);
    }
    decoder.close();
    outFile.close();
}

void VideoCodec::encodeIntra(GolombEncoder& encoder, unsigned char* frameBuf) {

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
}

void VideoCodec::encodeInter(GolombEncoder& encoder, unsigned char* frameBuf, int blockSize, int searchArea) {
    estimation_block_size = blockSize*blockSize;

    blockEstimationData bestBlockMatch;
    unsigned char* frameBlockBuf = new unsigned char [blockSize*blockSize*3];
    /* delete later */
    static int nBlock = 0;
    originalBlockFrameValue.open ("originalBlockFrameValue.txt", std::ofstream::out | std::ofstream::app);
    //file.open ("encodedBestBlock.txt", std::ofstream::out | std::ofstream::app);
    encodedMotion.open ("encodedMotion.txt", std::ofstream::out | std::ofstream::app);
    //encodedRefBlockPos.open ("encodedRefBlockPos.txt", std::ofstream::out | std::ofstream::app);

    for(int r=0; r<inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize ) {

            if(r < inFileData.uv_height && c < inFileData.uv_width) {
                frameBlockBuf = getFrameBlock(frameBuf, c, r, blockSize,YUV);
                bestBlockMatch = motionEstimation(frameBlockBuf, c, r, blockSize, searchArea, YUV, INTERSPERSED);
                encodeFrameBlock(encoder, bestBlockMatch, YUV);
            } else {
                frameBlockBuf = getFrameBlock(frameBuf, c, r, blockSize,Y);
                bestBlockMatch = motionEstimation(frameBlockBuf, c, r, blockSize, searchArea, Y, INTERSPERSED);
                encodeFrameBlock(encoder, bestBlockMatch, Y);
            }

            nBlock = nBlock + 1;
            originalBlockFrameValue << "frameBlockValue YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16*16; i++) {
                if (c < inFileData.uv_width && r < inFileData.uv_height) {
                    originalBlockFrameValue << "(Y,U,V)" << " (" << (int)frameBlockBuf[i] << ", "
                                           << (int)frameBlockBuf[i + blockSize*blockSize] << ", " << (int)frameBlockBuf[i + 2*blockSize*blockSize]  << ") ";
                } else {
                    originalBlockFrameValue << "(Y)" << " (" << (int)frameBlockBuf[i] << ") ";
                }
            }
            originalBlockFrameValue << "\n";


            encodedMotion << "motion vector YUV: nBlock-> " << nBlock << " ";
            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                encodedMotion << "Y(" <<  bestBlockMatch.motionVector_y.x << ", " << bestBlockMatch.motionVector_y.y << ") " <<"U(" <<  bestBlockMatch.motionVector_u.x << ", " << bestBlockMatch.motionVector_u.y << ") " << "V(" <<  bestBlockMatch.motionVector_v.x << ", " << bestBlockMatch.motionVector_v.y << ") ";
            } else {
                encodedMotion << "Y(" <<  bestBlockMatch.motionVector_y.x << ", " << bestBlockMatch.motionVector_y.y << ") " ;
            }
            encodedMotion << "\n";

        }
    }
    originalBlockFrameValue.close();
    encodedMotion.close();
}

void VideoCodec::encodeFrameBlock(GolombEncoder& encoder, blockEstimationData bestMatchData, planeComponent plane) {
    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    static int nBlock=0;
    nBlock = nBlock + 1;
    switch (plane) {
        case Y:
            encoder.encode(bestMatchData.motionVector_y.x);
            encoder.encode(bestMatchData.motionVector_y.y);

            for (int i=0; i< bestMatchData.size*bestMatchData.size; i++) {
                encoder.encode(bestMatchData.residuals_y[i]);
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    estimatedBlocks++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod = bestMatchData.residuals_y[i] > 0    ?   2 * bestMatchData.residuals_y[i] - 1  :   -2 * bestMatchData.residuals_y[i];
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
            break;
        case YUV:
            encoder.encode(bestMatchData.motionVector_y.x);
            encoder.encode(bestMatchData.motionVector_y.y);

            encoder.encode(bestMatchData.motionVector_u.x);
            encoder.encode(bestMatchData.motionVector_u.y);

            encoder.encode(bestMatchData.motionVector_v.x);
            encoder.encode(bestMatchData.motionVector_v.y);

            for (int i=0; i< bestMatchData.size*bestMatchData.size; i++) {
                encoder.encode(bestMatchData.residuals_y[i]);
                /*if(nBlock < 20) {
                    std::cout << "res_y res_u res_v: " << bestMatchData.residuals_y[i] << " " << bestMatchData.residuals_u[i] << " "<< bestMatchData.residuals_v[i] << std::endl;
                }*/
                encoder.encode(bestMatchData.residuals_u[i]);
                encoder.encode(bestMatchData.residuals_v[i]);
                if(estimation == ESTIMATION_ADAPTATIVE) {
                    estimatedBlocks++;
                    /* convert samples values to a positive range in order to perform the M estimation */
                    res_y_mod =bestMatchData.residuals_y[i] > 0    ?   2 * bestMatchData.residuals_y[i] - 1  :   -2 * bestMatchData.residuals_y[i];
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
            break;
        default:
            break;
    }
}

VideoCodec::blockEstimationData VideoCodec::motionEstimation(unsigned char* frameBlockBuf, int block_x, int block_y, int blockSize, int searchArea, planeComponent plane, blockSearchMode searchMode) {
    Point motionVector, bestMotionVector_y, bestMotionVector_u, bestMotionVector_v;

    /* delete later */
    Point refPos_y, refPos_u, refPos_v;
    unsigned char * refBlock_y = new unsigned char[blockSize*blockSize];
    unsigned char * refBlock_u = new unsigned char[blockSize*blockSize];
    unsigned char * refBlock_v = new unsigned char[blockSize*blockSize];
    static int nBlock = 0;
    nBlock = nBlock + 1;
    file.open ("encodedBestBlock.txt", std::ofstream::out | std::ofstream::app);
    encodedRefBlockPos.open ("encodedRefBlockPos.txt", std::ofstream::out | std::ofstream::app);
    encodedBlockPos.open("encodedBlockPos.txt", std::ofstream::out | std::ofstream::app);

    /* variable to store each block of the previous frame found within the search area */
    blockEstimationData bestBlockMatch;
    bestBlockMatch.setSize(blockSize);

    /* block delimeters */
    blockLimits limits = calculateBlockLimits(block_x,block_y,blockSize,searchArea);

    std::vector<int> bestRes_y(blockSize*blockSize), res_y(blockSize*blockSize);
    std::vector<int> bestRes_u(blockSize*blockSize), res_u(blockSize*blockSize);
    std::vector<int> bestRes_v(blockSize*blockSize), res_v(blockSize*blockSize);

    double bestError_y = std::numeric_limits<double>::max(), blockError_y=0;
    double bestError_u = std::numeric_limits<double>::max(), blockError_u=0;
    double bestError_v = std::numeric_limits<double>::max(), blockError_v=0;

    unsigned char* refBlockBuf = new unsigned char[blockSize*blockSize*3];
    //printf("topleft(%d,%d) topright(%d,%d) bottomLeft(%d,%d)\n", limits.topLeftPos.x,limits.topLeftPos.y, limits.topRightPos.x, limits.topRightPos.y, limits.bottomLeftPos.x, limits.bottomLeftPos.y);

    if(searchMode == EXHAUSTIVE) {
        for(int y=limits.topLeftPos.y; y<limits.bottomLeftPos.y; y++) {
            for(int x=limits.topLeftPos.x; x<limits.topRightPos.x; x++) {
                /* get each block from the specified previous frame' search area  */
                refBlockBuf = getFrameBlock(lastFrameBuf, x, y, blockSize, plane);
                /* determine each block motion vector */
                motionVector.setPos(block_x - x, block_y - y);

                for(int i=0; i < blockSize*blockSize; i++) {
                    res_y[i] = frameBlockBuf[i] - refBlockBuf[i];
                    blockError_y = blockError_y + std::abs(res_y[i]);
                }
                blockError_y = blockError_y/(blockSize*blockSize);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;
                    bestRes_y = res_y;
                    bestMotionVector_y = motionVector;

                }
                if(plane == YUV) {
                    for(int i=0; i< blockSize*blockSize; i++) {
                        res_u[i] = frameBlockBuf[i + blockSize*blockSize] - refBlockBuf[i + blockSize*blockSize];
                        res_v[i] = frameBlockBuf[i + 2*blockSize*blockSize] - refBlockBuf[i + 2*blockSize*blockSize];
                        blockError_u = blockError_u + std::abs(res_u[i]);
                        blockError_v = blockError_v + std::abs(res_v[i]);
                    }
                    blockError_u = blockError_u/(blockSize*blockSize);
                    blockError_v = blockError_v/(blockSize*blockSize);
                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;
                        bestRes_u = res_u;
                        bestMotionVector_u = motionVector;

                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;
                        bestRes_v = res_v;
                        bestMotionVector_v = motionVector;
                    }
                }
            }
        }
    } else {
        for(int y=limits.topLeftPos.y; y<limits.bottomLeftPos.y; y=y+4) {
            for(int x=limits.topLeftPos.x; x<limits.topRightPos.x; x=x+4) {
                /* get each block from the specified previous frame' search area  */
                refBlockBuf = getFrameBlock(lastFrameBuf, x, y, blockSize, plane);
                /* determine each block motion vector */
                motionVector.setPos(block_x - x, block_y - y);

                for(int i=0; i < blockSize*blockSize; i++) {
                    res_y[i] = frameBlockBuf[i] - refBlockBuf[i];
                    blockError_y = blockError_y + std::abs(res_y[i]);
                }
                blockError_y = blockError_y/(blockSize*blockSize);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;
                    bestRes_y = res_y;
                    bestMotionVector_y = motionVector;

                    /* delete later */
                    memcpy(refBlock_y, refBlockBuf, blockSize*blockSize);
                    refPos_y.x = x;
                    refPos_y.y = y;
                }
                if(plane == YUV) {
                    for(int i=0; i< blockSize*blockSize; i++) {
                        res_u[i] = frameBlockBuf[i + blockSize*blockSize] - refBlockBuf[i + blockSize*blockSize];
                        res_v[i] = frameBlockBuf[i + 2*blockSize*blockSize] - refBlockBuf[i + 2*blockSize*blockSize];
                        blockError_u = blockError_u + std::abs(res_u[i]);
                        blockError_v = blockError_v + std::abs(res_v[i]);
                    }
                    blockError_u = blockError_u/(blockSize*blockSize);
                    blockError_v = blockError_v/(blockSize*blockSize);
                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;
                        bestRes_u = res_u;
                        bestMotionVector_u = motionVector;

                        /* delete later */
                        memcpy(refBlock_u, refBlockBuf + blockSize*blockSize, blockSize*blockSize);
                        refPos_u.x = x;
                        refPos_u.y = y;
                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;
                        bestRes_v = res_v;
                        bestMotionVector_v = motionVector;

                        /* delete later */
                        memcpy(refBlock_v, refBlockBuf + 2*blockSize*blockSize, blockSize*blockSize);
                        refPos_v.x = x;
                        refPos_v.y = y;
                    }
                }
            }
        }
    }
    bestBlockMatch.setResiduals(bestRes_y, bestRes_u, bestRes_v);
    bestBlockMatch.setMotionVectors(bestMotionVector_y, bestMotionVector_u, bestMotionVector_v);

    file << "Used reference block YUV: nBlock-> " << nBlock << " ";
    encodedRefBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
    for(int r=0; r < blockSize; r++) {
        for(int c=0; c < blockSize; c++) {
            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                file << "(" <<  (int)refBlock_y[r*blockSize + c] << ", " << (int)refBlock_u[r*blockSize + c] << ", " << (int)refBlock_v[r*blockSize + c]  << ") ";
            } else {
                file << "("  <<  (int)refBlock_y[r*blockSize + c] << ") ";
            }
        }
    }
    file << "\n";
    if(block_x < inFileData.uv_width && block_y < inFileData.uv_height) {
        encodedRefBlockPos << "Y(" <<  refPos_y.x << ", " << refPos_y.y << ") " <<"U(" <<  refPos_u.x << ", " << refPos_u.y << ") " << "V(" <<  refPos_v.x << ", " << refPos_v.y << ") ";
    } else {
        encodedRefBlockPos << "Y(" <<  refPos_y.x << ", " << refPos_y.y << ") " ;
    }
    encodedRefBlockPos << "\n";

    encodedBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
    encodedBlockPos << "coordenadas(" <<  block_x << ", " << block_y <<") \n ";

    file.close();
    encodedRefBlockPos.close();
    encodedBlockPos.close();

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
        }
    }
    return frameBuf;
}

unsigned char* VideoCodec::decodeInter(GolombDecoder& decoder, int blockSize) {

    estimation_block_size = blockSize*blockSize;

    unsigned char* frameBuf = new unsigned char[inFileData.width*inFileData.height*(3/2)];
    unsigned char * frameBlockBuf = new unsigned char[blockSize*blockSize*3];

    /* delete later */
    static int nBlock = 0;
    decodedBlockFrameValue.open ("decodedFrameValue.txt", std::ofstream::out | std::ofstream::app);

    int nLines = blockSize;
    int u_offset = blockSize*blockSize, v_offset = 2*blockSize*blockSize;

    for(int r = 0; r < inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize) {
            frameBlockBuf = decodeFrameBlock(decoder, blockSize, c, r);
            for(int n = 0; n < nLines; n++) {
                memcpy(frameBuf + inFileData.width*(n+r) + c, frameBlockBuf + n*blockSize, blockSize);
                if(r<=inFileData.uv_height && c<=inFileData.uv_width) {
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height, frameBlockBuf + u_offset + n*blockSize, blockSize);
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height, frameBlockBuf + v_offset + n*blockSize, blockSize);
                }
            }
            /* delete later */
            nBlock = nBlock + 1;

            decodedBlockFrameValue << "frameBlockValue YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16*16; i++) {
                if (c < inFileData.uv_width && r < inFileData.uv_height) {
                    decodedBlockFrameValue << "(Y,U,V)" << " (" << (int)frameBlockBuf[i] << ", "
                                            << (int)frameBlockBuf[i + blockSize*blockSize] << ", " << (int)frameBlockBuf[i + 2*blockSize*blockSize] << ") ";
                } else {
                    decodedBlockFrameValue << "(Y)" << " (" << (int)frameBlockBuf[i] << ") ";
                }
            }
            decodedBlockFrameValue << "\n";
        }
    }
    decodedBlockFrameValue.close();
    return frameBuf;
}

unsigned char* VideoCodec::decodeFrameBlock(GolombDecoder& decoder, int blockSize, int x, int y) {

    static int nBlock = 0;
    nBlock = nBlock + 1;
    decodedBlockFrameValue.open ("decodedFrameValue.txt", std::ofstream::out | std::ofstream::app);
    file2.open ("decodedBestBlock.txt", std::ofstream::out | std::ofstream::app);
    decodedMotion.open ("decodedMotion.txt", std::ofstream::out | std::ofstream::app);
    decodedRefBlockPos.open ("decodedRefBlockPos.txt", std::ofstream::out | std::ofstream::app);
    decodedBlockPos.open("decodedBlockPos.txt", std::ofstream::out | std::ofstream::app);

    decodedBlockFrameValue << "frameBlockValue YUV: nBlock-> " << nBlock << " ";
    file2 << "Used reference block YUV: nBlock-> " << nBlock << " ";

    Point motionVector_y, motionVector_u, motionVector_v, refPos_y, refPos_u, refPos_v;

    unsigned char * frameBlockBuf = new unsigned char[blockSize*blockSize*3];
    unsigned char * refBlockBuf = new unsigned char[blockSize*blockSize*3];


    int res_y, res_u, res_v;

    motionVector_y.x = decoder.decode();
    motionVector_y.y = decoder.decode();

    //std::cout << "encoded motion (Y): " << motionVector_y.x << ", " << motionVector_y.y << std::endl;
    refPos_y.x =  x - motionVector_y.x;
    refPos_y.y =  y - motionVector_y.y;


     if(x<inFileData.uv_width && y< inFileData.uv_height) {
        motionVector_u.x = decoder.decode();
        motionVector_u.y = decoder.decode();

        motionVector_v.x = decoder.decode();
        motionVector_v.y = decoder.decode();

        refPos_u.x =  x - motionVector_u.x;
        refPos_u.y =  y - motionVector_u.y;

        refPos_v.x =  x - motionVector_v.x;
        refPos_v.y =  y - motionVector_v.y;

        refBlockBuf = getFrameBlock(lastFrameBuf, refPos_y.x, refPos_y.y, blockSize,YUV);
     } else {
         refBlockBuf = getFrameBlock(lastFrameBuf, refPos_y.x, refPos_y.y, blockSize,Y);
     }

    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    for (int r = 0; r < blockSize; r++) {
        for (int c = 0; c < blockSize; c++) {
            res_y = decoder.decode();
            frameBlockBuf[r*blockSize + c] = res_y + refBlockBuf[r*blockSize + c];

            if(x < inFileData.uv_width && y <  inFileData.uv_height) {
                res_u = decoder.decode();
                res_v = decoder.decode();
                frameBlockBuf[r*blockSize + c + blockSize*blockSize] = res_u + refBlockBuf[r*blockSize + c + blockSize*blockSize];
                frameBlockBuf[r*blockSize + c + 2*blockSize*blockSize] = res_v + refBlockBuf[r*blockSize + c + 2*blockSize*blockSize];

                /* delete later */
                decodedBlockFrameValue << "(Y,U,V)" << " (" << (int)frameBlockBuf[r*blockSize + c] << ", "
                                        << (int)frameBlockBuf[r*blockSize + c + blockSize*blockSize] << ", " << (int)frameBlockBuf[r*blockSize + c + 2*blockSize*blockSize]  << ") ";
                file2 << "(" <<  (int)refBlockBuf[r*blockSize + c] << ", " << (int)refBlockBuf[r*blockSize + c + blockSize*blockSize] << ", " << (int)refBlockBuf[r*blockSize + c + 2*blockSize*blockSize]  << ") ";
            } else {
                decodedBlockFrameValue << "(Y)" << " (" << (int)frameBlockBuf[r*blockSize + c]  << ") ";
                file2 << "("  <<  (int)refBlockBuf[r*blockSize + c] << ") ";
            }
        }
        if(estimation == ESTIMATION_ADAPTATIVE) {
            estimatedBlocks++;
             //convert samples values to a positive range in order to perform the M estimation
            res_y_mod = res_y > 0    ?   2 * res_y - 1  :   -2 * res_y;
            sum_y = sum_y + res_y_mod;
            if(estimatedBlocks == estimation_block_size)
            {
                m = estimateM_fromBlock(sum_y, estimation_block_size);
                //std::cout << "m: " << m << " sum: " << sum_y << std::endl;
                decoder.update(m);
                sum_y = 0;
                estimatedBlocks = 0;
            }

        }
    }
    decodedBlockFrameValue << "\n";
    file2 << "\n";


    decodedMotion << "motion vector YUV: nBlock-> " << nBlock << " ";
    if(x < inFileData.uv_width && y < inFileData.uv_height) {
        decodedMotion << "Y(" <<  motionVector_y.x << ", " << motionVector_y.y << ") " <<"U(" <<  motionVector_u.x << ", " << motionVector_u.y << ") " << "V(" <<  motionVector_v.x << ", " << motionVector_v.y << ") ";
    } else {
        decodedMotion << "Y(" <<  motionVector_y.x << ", " << motionVector_y.y << ") " ;
    }
    decodedMotion << "\n";

    decodedRefBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
    if(x < inFileData.uv_width && y < inFileData.uv_height) {
        decodedRefBlockPos << "Y(" <<  refPos_y.x << ", " << refPos_y.y << ") " <<"U(" <<  refPos_u.x << ", " << refPos_u.y << ") " << "V(" <<  refPos_v.x << ", " << refPos_v.y << ") ";
    } else {
        decodedRefBlockPos << "Y(" <<  refPos_y.x << ", " << refPos_y.y << ") " ;
    }
    decodedRefBlockPos << "\n";

    decodedBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
    decodedBlockPos << "coordenadas(" <<  x << ", " << y <<") \n ";

    /* delete later , for debug purposes */
    decodedRefBlockPos.close();
    decodedMotion.close();
    file2.close();
    decodedRefBlockPos.close();
    decodedBlockPos.close();

    return frameBlockBuf;
}

unsigned char* VideoCodec::getFrameBlock(unsigned char* frameBuf, int x, int y, int blockSize, planeComponent plane) {
    unsigned char* frameBlockBuf = new unsigned char [blockSize*blockSize*3];

    for (int r = 0; r < blockSize; r++) {
        for (int c = 0; c < blockSize; c++) {
            if(plane == Y) {
                frameBlockBuf[r * blockSize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
            } else if(plane == U) {
                frameBlockBuf[r* blockSize + c + blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
            } else if(plane == V) {
                frameBlockBuf[r * blockSize + c + 2*blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height];
            } else {
                frameBlockBuf[r * blockSize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
                frameBlockBuf[r* blockSize + c + blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
                frameBlockBuf[r* blockSize + c + 2*blockSize*blockSize] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
            }
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


unsigned char* VideoCodec::readFrame(std::ifstream* fp, videoFormat format) {
    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height + 2 * (inFileData.uv_width * inFileData.uv_height)];

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