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

    /* delete later, for debug purposes */
    originalFrame.open ("originalFrame_v.txt", std::ofstream::out | std::ofstream::app);

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

            /* delete later, for debug purposes */
            for(int r=0; r<inFileData.height; r++) {
                for(int c=0; c<inFileData.width; c++) {
                    if (c < inFileData.uv_width && r < inFileData.uv_height) {
                        //originalFrame << "[c][r] [Y,U,V] " << c << " " << r << " " << (int)frameBuf[r*inFileData.width + c] << " "
                          //            << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height] << " " << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height] << std::endl;
                    } else {
                        //originalFrame << "[c][r] [Y] " << c << " " << r << " " << (int)frameBuf[r*inFileData.width + c] << std::endl;
                    }
                    originalFrame << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height] << std::endl;
                }
            }
        } else {
            encodeInter(encoder, frameBuf, blockSize, searchArea);
            /* delete later, for debug purposes */
            /* for(int i=0; i<40; i++) {
                std::cout << (int)frameBuf[i] << ", ";
            } */
        }
        nFrames++;
        lastFrameBuf = frameBuf;
        // restore frame dimensions in order to read the next 444 frame
        inFileData.uv_width = inFileData.width;
        inFileData.uv_height = inFileData.height;
    }
    encoder.close();
    originalFrame.close();
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

    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3 / 2];

    /* delete later, for debug purposes */
    int sum_elem= 0;
    decodedFrame.open ("decodedFrame_v.txt", std::ofstream::out | std::ofstream::app);
    printf("---------------decoding------------\n");

    unsigned int i=0;
    while(i<2) {
        if((i%intraFramePeriodicity)==0) {
            frameBuf = decodeIntra(decoder);
            std::cout << "decoded frames: " << i << std::endl;
            /* for debug purposes */
            for(int r=0; r<inFileData.height; r++) {
                for(int c=0; c<inFileData.width; c++) {
                    if (c < inFileData.uv_width && r < inFileData.uv_height) {
                        //decodedFrame << "[c][r] [Y,U,V] " << c << " " << r << " " << (int)frameBuf[r*inFileData.width + c] << " "
                        //              << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height] << " " << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height] << std::endl;
                    } else {
                        //decodedFrame << "[c][r] [Y] " << c << " " << r << " " << (int)frameBuf[r*inFileData.width + c] << std::endl;
                    }
                    decodedFrame << (int)frameBuf[r*inFileData.uv_width + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height] << std::endl;
                }
            }
        } else {
            frameBuf = decodeInter(decoder, blockSize, searchArea);
            /* delete later, for debug purposes */
            /* for(int i=0; i<40; i++) {
                std::cout << (int)frameBuf[i] << ", ";
            }
            std::cout << std::endl; */
            /* for(int i=0; i<inFileData.width * inFileData.height; i++) {
                //std::cout << (int)frameBuf[i] << ", ";
                if(frame2Buf[i] == frameBuf[i]) {
                    sum_elem = sum_elem + 1;
                } else {
                    std::cout << "diferent em " << i << std::endl;
                }
            }*/
            /* if(sum_elem == (inFileData.width * inFileData.height * 3 / 2)-1) {
            //printf("sum %d, element_number %d\n", sum_elem,inFileData.width * inFileData.height);
            } */
        }
        writeFrame(&outFile, frameBuf);
        i++;
        lastFrameBuf = frameBuf;
    }
    decoder.close();
    outFile.close();
    decodedFrame.close();
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
    blockData block;
    blockEstimationData bestBlockMatch;

    /* delete later , for debug purposes */
    static int nBlock = 0;
    originalBlockFrameValue.open ("originalBlockFrameValue.txt", std::ofstream::out | std::ofstream::app);
    file.open ("encodedBestBlock.txt", std::ofstream::out | std::ofstream::app);
    encodedMotion.open ("encodedMotion.txt", std::ofstream::out | std::ofstream::app);
    encodedRefBlockPos.open ("encodedRefBlockPos.txt", std::ofstream::out | std::ofstream::app);
    encodedBlockPos.open("encodedBlockPos.txt", std::ofstream::out | std::ofstream::app);

    for(int r=0; r<inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize ) {

            if(r < inFileData.uv_height && c < inFileData.uv_width) {
                block = getFrameBlock(frameBuf, c, r, blockSize,YUV);
                bestBlockMatch = motionEstimation(block, searchArea, YUV, INTERSPERSED);
                encodeFrameBlock(encoder, bestBlockMatch, YUV);
            } else {
                block = getFrameBlock(frameBuf, c, r, blockSize,Y);
                bestBlockMatch = motionEstimation(block, searchArea, Y, INTERSPERSED);
                encodeFrameBlock(encoder, bestBlockMatch, Y);
            }

            /* delete later , for debug purposes */
            nBlock = nBlock + 1;
            if((nBlock>24 && nBlock < 30)) {
                printf("encoded: block(%d,%d) vetorY(%d,%d) vetorU(%d,%d) vetorV(%d,%d) refY(%d,%d) refU(%d,%d) refV(%d,%d)\n", c, r, bestBlockMatch.motionVector_y.x, bestBlockMatch.motionVector_y.y,bestBlockMatch.motionVector_u.x,bestBlockMatch.motionVector_u.y,bestBlockMatch.motionVector_v.x,bestBlockMatch.motionVector_v.y, bestBlockMatch.ref_y.x, bestBlockMatch.ref_y.y, bestBlockMatch.ref_u.x, bestBlockMatch.ref_u.y, bestBlockMatch.ref_v.x, bestBlockMatch.ref_v.y);
            }

            originalBlockFrameValue << "frameBlockValue YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16*16; i++) {
                if (block.x < inFileData.uv_width && block.y < inFileData.uv_height) {
                    originalBlockFrameValue << "(Y,U,V)" << " (" << (int)block.blockBuf_y[i] << ", "
                                           << (int)block.blockBuf_u[i] << ", " << (int)block.blockBuf_v[i] << ") ";
                } else {
                    originalBlockFrameValue << "(Y)" << " (" << (int)block.blockBuf_y[i] << ") ";
                }
            }
            originalBlockFrameValue << "\n";

            file << "Best block YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16; i++) {
                if(c < inFileData.uv_width && r < inFileData.uv_height) {
                    file << "(" <<  (int)bestBlockMatch.blockBuf_y[i] << ", " << (int)bestBlockMatch.blockBuf_u[i] << ", " << (int)bestBlockMatch.blockBuf_v[i]  << ") ";
                } else {
                    file << "("  <<  (int)bestBlockMatch.blockBuf_y[i] << ") ";
                }
            }
            file << "\n";

            encodedMotion << "motion vector YUV: nBlock-> " << nBlock << " ";
            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                encodedMotion << "Y(" <<  bestBlockMatch.motionVector_y.x << ", " << bestBlockMatch.motionVector_y.y << ") " <<"U(" <<  bestBlockMatch.motionVector_u.x << ", " << bestBlockMatch.motionVector_u.y << ") " << "V(" <<  bestBlockMatch.motionVector_v.x << ", " << bestBlockMatch.motionVector_v.y << ") ";
            } else {
                encodedMotion << "Y(" <<  bestBlockMatch.motionVector_y.x << ", " << bestBlockMatch.motionVector_y.y << ") " ;
            }
            encodedMotion << "\n";

            encodedRefBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";

            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                encodedRefBlockPos << "Y(" <<  bestBlockMatch.ref_y.x << ", " << bestBlockMatch.ref_y.y << ") " <<"U(" <<  bestBlockMatch.ref_u.x << ", " << bestBlockMatch.ref_u.y << ") " << "V(" <<  bestBlockMatch.ref_v.x << ", " << bestBlockMatch.ref_v.y << ") ";
            } else {
                encodedRefBlockPos << "Y(" <<  bestBlockMatch.ref_y.x << ", " << bestBlockMatch.ref_y.y << ") " ;
            }
            encodedRefBlockPos << "\n";

            encodedBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
            encodedBlockPos << "coordenadas(" <<  c << ", " << r <<") \n ";
        }

    }
    /* delete later , for debug purposes */
    originalBlockFrameValue.close();
    file.close();
    encodedMotion.close();
    encodedRefBlockPos.close();
    encodedBlockPos.close();
}

void VideoCodec::encodeFrameBlock(GolombEncoder& encoder, blockEstimationData bestMatchData, planeComponent plane) {
    unsigned int i=0, sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
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

VideoCodec::blockEstimationData VideoCodec::motionEstimation(blockData& currentFrameBlock, int searchArea, planeComponent plane, blockSearchMode searchMode) {
    Point topLeftPos, topRightPos, bottomLeftPos;
    Point motionVector, bestMotionVector_y, bestMotionVector_u, bestMotionVector_v;
    /* variable to store each block of the previous frame found within the search area */
    blockData block;
    blockEstimationData bestBlockMatch;
    bestBlockMatch.setSize(currentFrameBlock.size);

    static int nBlock = 0;
    nBlock = nBlock + 1;

    /* block delimeters */
    blockLimits limits = calculateBlockLimits(currentFrameBlock, searchArea);
    topLeftPos = limits.topLeftPos;
    topRightPos = limits.topRightPos;
    bottomLeftPos = limits.bottomLeftPos;

    std::vector<int> bestRes_y(currentFrameBlock.size*currentFrameBlock.size), res_y(currentFrameBlock.size*currentFrameBlock.size);
    std::vector<int> bestRes_u(currentFrameBlock.size*currentFrameBlock.size), res_u(currentFrameBlock.size*currentFrameBlock.size);
    std::vector<int> bestRes_v(currentFrameBlock.size*currentFrameBlock.size), res_v(currentFrameBlock.size*currentFrameBlock.size);

    double bestError_y = std::numeric_limits<double>::max(), blockError_y=0;
    double bestError_u = std::numeric_limits<double>::max(), blockError_u=0;
    double bestError_v = std::numeric_limits<double>::max(), blockError_v=0;

    if(searchMode == EXHAUSTIVE) {
        for(int y=topLeftPos.y; y<bottomLeftPos.y; y=y++) {
            for(int x=topLeftPos.x; x<topRightPos.x; x=x++) {
                /* get each block from the specified previous frame' search area  */
                block = getFrameBlock(lastFrameBuf, x, y, currentFrameBlock.size, plane);
                /* determine each block motion vector */
                motionVector.setPos(currentFrameBlock.x - x, currentFrameBlock.y - y);

                for(int i=0; i < currentFrameBlock.size*currentFrameBlock.size; i++) {
                    res_y[i] = currentFrameBlock.blockBuf_y[i] - block.blockBuf_y[i];
                    blockError_y = blockError_y + std::abs(res_y[i]);
                }
                blockError_y = blockError_y/(currentFrameBlock.size*currentFrameBlock.size);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;
                    bestRes_y = res_y;
                    bestMotionVector_y = motionVector;

                    /* delete later , for debug purposes */
                    bestBlockMatch.ref_y.setX(x);
                    bestBlockMatch.ref_y.setY(y);
                    bestBlockMatch.setBlockBuf(block.blockBuf_y, Y);
                }
                if(plane == YUV) {
                    for(int i=0; i< currentFrameBlock.size*currentFrameBlock.size; i++) {
                        res_u[i] = currentFrameBlock.blockBuf_u[i] - block.blockBuf_u[i];
                        res_v[i] = currentFrameBlock.blockBuf_v[i] - block.blockBuf_v[i];
                        blockError_u = blockError_u + std::abs(res_u[i]);
                        blockError_v = blockError_v + std::abs(res_v[i]);
                    }
                    blockError_u = blockError_u/(currentFrameBlock.size*currentFrameBlock.size);
                    blockError_v = blockError_v/(currentFrameBlock.size*currentFrameBlock.size);
                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;
                        bestRes_u = res_u;
                        bestMotionVector_u = motionVector;

                        /* delete later */
                        bestBlockMatch.ref_u.x = x;
                        bestBlockMatch.ref_u.y = y;
                        bestBlockMatch.blockBuf_u = block.blockBuf_u;

                        bestBlockMatch.ref_u.setX(x);
                        bestBlockMatch.ref_u.setY(y);
                        bestBlockMatch.setBlockBuf(block.blockBuf_u, U);
                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;
                        bestRes_v = res_v;
                        bestMotionVector_v = motionVector;

                        /* delete later */
                        bestBlockMatch.ref_v.setX(x);
                        bestBlockMatch.ref_v.setY(y);
                        bestBlockMatch.setBlockBuf(block.blockBuf_v, V);
                    }
                }
            }
        }
    } else {
        for(int y=topLeftPos.y; y<bottomLeftPos.y; y=y+4) {
            for(int x=topLeftPos.x; x<topRightPos.x; x=x+4) {
                /* get each block from the specified previous frame' search area  */
                block = getFrameBlock(lastFrameBuf, x, y, currentFrameBlock.size, plane);
                /* determine each block motion vector */
                motionVector.setPos(currentFrameBlock.x - x, currentFrameBlock.y - y);

                for(int i=0; i < currentFrameBlock.size*currentFrameBlock.size; i++) {
                    res_y[i] = currentFrameBlock.blockBuf_y[i] - block.blockBuf_y[i];
                    blockError_y = blockError_y + std::abs(res_y[i]);
                }
                blockError_y = blockError_y/(currentFrameBlock.size*currentFrameBlock.size);
                if(blockError_y < bestError_y){
                    bestError_y = blockError_y;
                    bestRes_y = res_y;
                    bestMotionVector_y = motionVector;
                    /* delete later , for debug purposes */
                    bestBlockMatch.ref_y.setX(x);
                    bestBlockMatch.ref_y.setY(y);
                    bestBlockMatch.setBlockBuf(block.blockBuf_y, Y);
                }
                if(plane == YUV) {
                    for(int i=0; i< currentFrameBlock.size*currentFrameBlock.size; i++) {
                        res_u[i] = currentFrameBlock.blockBuf_u[i] - block.blockBuf_u[i];
                        res_v[i] = currentFrameBlock.blockBuf_v[i] - block.blockBuf_v[i];
                        blockError_u = blockError_u + std::abs(res_u[i]);
                        blockError_v = blockError_v + std::abs(res_v[i]);
                    }
                    blockError_u = blockError_u/(currentFrameBlock.size*currentFrameBlock.size);
                    blockError_v = blockError_v/(currentFrameBlock.size*currentFrameBlock.size);
                    if(blockError_u < bestError_u){
                        bestError_u = blockError_u;
                        bestRes_u = res_u;
                        bestMotionVector_u = motionVector;

                        /* delete later , for debug purposes */
                        bestBlockMatch.ref_u.setX(x);
                        bestBlockMatch.ref_u.setY(y);
                        bestBlockMatch.setBlockBuf(block.blockBuf_u, U);
                    }
                    if(blockError_v < bestError_v){
                        bestError_v = blockError_v;
                        bestRes_v = res_v;
                        bestMotionVector_v = motionVector;

                        /* delete later , for debug purposes */
                        bestBlockMatch.ref_v.setX(x);
                        bestBlockMatch.ref_v.setY(y);
                        bestBlockMatch.setBlockBuf(block.blockBuf_v, V);
                    }
                }
            }
        }
    }
    bestBlockMatch.setResiduals(bestRes_y, bestRes_u, bestRes_v);
    bestBlockMatch.setMotionVectors(bestMotionVector_y, bestMotionVector_u, bestMotionVector_v);
    return bestBlockMatch;
}


unsigned char* VideoCodec::decodeIntra(GolombDecoder& decoder) {

    unsigned char* frameBuf = new unsigned char[inFileData.width * inFileData.height * 3 / 2];

    unsigned int i=0, sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
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

unsigned char* VideoCodec::decodeInter(GolombDecoder& decoder, int blockSize, int searchArea) {

    estimation_block_size = blockSize*blockSize;
    blockEstimationData encodedBlock;
    unsigned char* frameBuf = new unsigned char[inFileData.width*inFileData.height*(3/2)];

    /* delete later , for debug purposes */
    static int nBlock = 0;
    decodedBlockFrameValue.open ("decodedFrameValue.txt", std::ofstream::out | std::ofstream::app);
    file2.open ("decodedBestBlock.txt", std::ofstream::out | std::ofstream::app);
    decodedMotion.open ("decodedMotion.txt", std::ofstream::out | std::ofstream::app);
    decodedRefBlockPos.open ("decodedRefBlockPos.txt", std::ofstream::out | std::ofstream::app);
    decodedBlockPos.open("decodedBlockPos.txt", std::ofstream::out | std::ofstream::app);


    int nLines = blockSize, u_offset = blockSize*blockSize, v_offset = 2*blockSize*blockSize;

    for(int r = 0; r < inFileData.height; r=r+blockSize) {
        for (int c = 0; c < inFileData.width; c=c+blockSize) {
            encodedBlock = decodeFrameBlock(decoder, blockSize, c, r);
            for(int n = 0; n < nLines; n++) {
                memcpy(frameBuf + inFileData.width*(n+r) + c, encodedBlock.blockBuf_y + n*blockSize, blockSize);
                if(r<=inFileData.uv_height && c<=inFileData.uv_width) {
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height, encodedBlock.blockBuf_u + n*blockSize, blockSize);
                    memcpy(frameBuf + inFileData.uv_width*(n+r) + c + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height, encodedBlock.blockBuf_v + n*blockSize, blockSize);
                }
            }
            /* delete later , for debug purposes */
            nBlock = nBlock + 1;
            decodedBlockFrameValue << "frameBlockValue YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16*16; i++) {
                if (c < inFileData.uv_width && r < inFileData.uv_height) {
                    decodedBlockFrameValue << "(Y,U,V)" << " (" << (int)encodedBlock.blockBuf_y[i] << ", "
                                            << (int)encodedBlock.blockBuf_u[i] << ", " << (int)encodedBlock.blockBuf_v[i] << ") ";
                } else {
                    decodedBlockFrameValue << "(Y)" << " (" << (int)encodedBlock.blockBuf_y[i] << ") ";
                }
            }
            decodedBlockFrameValue << "\n";

            file2 << "Used reference block YUV: nBlock-> " << nBlock << " ";
            for(int i=0; i< 16; i++) {
                if(c < inFileData.uv_width && r < inFileData.uv_height) {
                    file2 << "(" <<  (int)encodedBlock.refBlockBuf_y[i] << ", " << (int)encodedBlock.refBlockBuf_u[i] << ", " << (int)encodedBlock.refBlockBuf_v[i]  << ") ";
                } else {
                    file2 << "("  <<  (int)encodedBlock.refBlockBuf_y[i] << ") ";
                }
            }
            file2 << "\n";

            decodedMotion << "motion vector YUV: nBlock-> " << nBlock << " ";
            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                decodedMotion << "Y(" <<  encodedBlock.motionVector_y.x << ", " << encodedBlock.motionVector_y.y << ") " <<"U(" <<  encodedBlock.motionVector_u.x << ", " << encodedBlock.motionVector_u.y << ") " << "V(" <<  encodedBlock.motionVector_v.x << ", " << encodedBlock.motionVector_v.y << ") ";
            } else {
                decodedMotion << "Y(" <<  encodedBlock.motionVector_y.x << ", " << encodedBlock.motionVector_y.y << ") " ;
            }
            decodedMotion << "\n";

            decodedRefBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";

            if(c < inFileData.uv_width && r < inFileData.uv_height) {
                decodedRefBlockPos << "Y(" <<  encodedBlock.ref_y.x << ", " << encodedBlock.ref_y.y << ") " <<"U(" <<  encodedBlock.ref_u.x << ", " << encodedBlock.ref_u.y << ") " << "V(" <<  encodedBlock.ref_v.x << ", " << encodedBlock.ref_v.y << ") ";
            } else {
                decodedRefBlockPos << "Y(" <<  encodedBlock.ref_y.x << ", " << encodedBlock.ref_y.y << ") " ;
            }
            decodedRefBlockPos << "\n";

            decodedBlockPos << "refBlockPos YUV: nBlock-> " << nBlock << " ";
            decodedBlockPos << "coordenadas(" <<  c << ", " << r <<") \n ";
        }
    }
    /* delete later , for debug purposes */
    decodedRefBlockPos.close();
    decodedMotion.close();
    file2.close();
    decodedRefBlockPos.close();
    decodedBlockPos.close();
    return frameBuf;
}

VideoCodec::blockEstimationData VideoCodec::decodeFrameBlock(GolombDecoder& decoder, int blockSize, int x, int y) {

    Point motionVector_y, motionVector_u, motionVector_v, refPos_y, refPos_u, refPos_v;
    blockData referenceBlock;

    unsigned char * frameBlockBuf = new unsigned char[blockSize*blockSize*3];
    unsigned char * refBlockBuf_y = new unsigned char[blockSize*blockSize];
    unsigned char * refBlockBuf_u = new unsigned char[blockSize*blockSize];
    unsigned char * refBlockBuf_v = new unsigned char[blockSize*blockSize];

    unsigned char * blockBuf_y = new unsigned char[blockSize*blockSize];
    unsigned char * blockBuf_u = new unsigned char[blockSize*blockSize];
    unsigned char * blockBuf_v = new unsigned char[blockSize*blockSize];

    blockEstimationData encodedBlock;

    int res_y, res_u, res_v;

    /* delete later */
    static int nBlock = 0;
    nBlock = nBlock + 1;

    motionVector_y.x = decoder.decode();
    motionVector_y.y = decoder.decode();

    //std::cout << "encoded motion (Y): " << motionVector_y.x << ", " << motionVector_y.y << std::endl;
    refPos_y.x =  x - motionVector_y.x;
    refPos_y.y =  y - motionVector_y.y;


    referenceBlock = getFrameBlock(lastFrameBuf, refPos_y.x, refPos_y.y, blockSize,Y);
    refBlockBuf_y = referenceBlock.blockBuf_y;

    /* delete later , for debug purposes */
    encodedBlock.setMotionVector(motionVector_y, Y);
    encodedBlock.setReferenceBuf(referenceBlock.blockBuf_y, Y);
    encodedBlock.ref_y.setX(refPos_y.x);
    encodedBlock.ref_y.setY(refPos_y.y);
     if(x<inFileData.uv_width && y< inFileData.uv_height) {
        motionVector_u.x = decoder.decode();
        motionVector_u.y = decoder.decode();

        motionVector_v.x = decoder.decode();
        motionVector_v.y = decoder.decode();

        refPos_u.x =  x - motionVector_u.x;
        refPos_u.y =  y - motionVector_u.y;

        refPos_v.x =  x - motionVector_v.x;
        refPos_v.y =  y - motionVector_v.y;

        referenceBlock = getFrameBlock(lastFrameBuf, refPos_u.x, refPos_u.y, blockSize,U);
        refBlockBuf_u = referenceBlock.blockBuf_u;
        encodedBlock.setReferenceBuf(referenceBlock.blockBuf_u, U);
        referenceBlock = getFrameBlock(lastFrameBuf, refPos_v.x, refPos_v.y, blockSize,V);
        refBlockBuf_v = referenceBlock.blockBuf_v;
        encodedBlock.setReferenceBuf(referenceBlock.blockBuf_v, V);
        /* delete later , for debug purposes */
        encodedBlock.setMotionVector(motionVector_u, U);
        encodedBlock.setMotionVector(motionVector_v, V);
        encodedBlock.ref_u.setX(refPos_u.x);
        encodedBlock.ref_u.setY(refPos_u.y);
        encodedBlock.ref_v.setX(refPos_v.x);
        encodedBlock.ref_v.setY(refPos_v.y);
     }


    unsigned int sum_y = 0, estimatedBlocks = 0, m, res_y_mod;
    for (int i = 0; i < blockSize*blockSize; i++) {
        res_y = decoder.decode();
        frameBlockBuf[i] = res_y + refBlockBuf_y[i];
         //delete later
        blockBuf_y[i] = frameBlockBuf[i];
        if(x < inFileData.uv_width && y <  inFileData.uv_height) {
            res_u = decoder.decode();
            res_v = decoder.decode();
            frameBlockBuf[i + blockSize*blockSize] = res_u + refBlockBuf_u[i];
            frameBlockBuf[i + blockSize*blockSize*2] = res_v + refBlockBuf_v[i];

            /* delete later */
            blockBuf_u[i] = frameBlockBuf[i + blockSize*blockSize];
            blockBuf_v[i] = frameBlockBuf[i + 2*blockSize*blockSize];
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
    encodedBlock.setBlockBuf(blockBuf_y, Y);
    if(x<inFileData.uv_width && y< inFileData.uv_height) {
        encodedBlock.setBlockBuf(blockBuf_u, U);
        encodedBlock.setBlockBuf(blockBuf_v, V);
    }
    /* delete later , for debug purposes */
    if(nBlock>24 && nBlock < 30) {
        printf("decoded: block(%d,%d) vetorY(%d,%d) vetorU(%d,%d) vetorV(%d,%d) refY(%d,%d) refU(%d,%d) refV(%d,%d)\n", x,y, motionVector_y.x, motionVector_y.y,motionVector_u.x,motionVector_u.y,motionVector_v.x,motionVector_v.y, encodedBlock.ref_y.x, encodedBlock.ref_y.y, encodedBlock.ref_u.x, encodedBlock.ref_u.y, encodedBlock.ref_v.x, encodedBlock.ref_v.y);
    }
    return encodedBlock;
}

VideoCodec::blockData VideoCodec::getFrameBlock(unsigned char* frameBuf, int x, int y, int blocksize, planeComponent plane) {
    blockData block;
    block.setSize(blocksize);
    block.setBlockPos(x,y);

    for (int r = 0; r < block.size; r++) {
        for (int c = 0; c < block.size; c++) {
            if(plane == Y) {
                block.blockBuf_y[r * blocksize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
            } else if(plane == U) {
                block.blockBuf_u[r*blocksize + c] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
            } else if(plane == V) {
                block.blockBuf_v[r*blocksize + c] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height];
            } else {
                block.blockBuf_y[r*blocksize + c] = frameBuf[(y+r)*inFileData.width + (x+c)];
                block.blockBuf_u[r*blocksize + c] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height];
                block.blockBuf_v[r*blocksize + c] = frameBuf[(y+r)*inFileData.uv_width + (x+c) + inFileData.width*inFileData.height + inFileData.uv_width*inFileData.uv_height];
            }
        }
    }
    return block;
}

VideoCodec::blockLimits VideoCodec::calculateBlockLimits(blockData currentFrameBlock, int searchArea) {

    blockLimits limits;
    Point topLeftPos, topRightPos, bottomLeftPos;
    if((currentFrameBlock.x - searchArea < 0) && (currentFrameBlock.y - searchArea < 0)) {
        topLeftPos.x = currentFrameBlock.x;
        topLeftPos.y = currentFrameBlock.y;

        topRightPos.x = currentFrameBlock.x + currentFrameBlock.size + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = currentFrameBlock.y + currentFrameBlock.size + searchArea - 1;
    } else if( (currentFrameBlock.x - searchArea < 0) && (currentFrameBlock.y - searchArea >= 0)) {
        topLeftPos.x = currentFrameBlock.x;
        topLeftPos.y = currentFrameBlock.y - searchArea;

        topRightPos.x = currentFrameBlock.x + currentFrameBlock.size + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = currentFrameBlock.y + currentFrameBlock.size + searchArea - 1;
    } else if( (currentFrameBlock.x - searchArea >= 0) && (currentFrameBlock.y - searchArea < 0)) {
        topLeftPos.x = currentFrameBlock.x - searchArea;
        topLeftPos.y = currentFrameBlock.y;

        topRightPos.x = currentFrameBlock.x + currentFrameBlock.size + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = currentFrameBlock.y + currentFrameBlock.size + searchArea - 1;
    } else {
        topLeftPos.x = currentFrameBlock.x - searchArea;
        topLeftPos.y = currentFrameBlock.y - searchArea;

        topRightPos.x = currentFrameBlock.x + currentFrameBlock.size + searchArea - 1;
        topRightPos.y = topLeftPos.y;

        bottomLeftPos.x = topLeftPos.x;
        bottomLeftPos.y = currentFrameBlock.y + currentFrameBlock.size + searchArea - 1;
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