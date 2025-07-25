// Copyright 2024 Luka Mihajlovic, written to pass the cpplint check
#include "libFilmMaster2000.h"
#include <iostream>
#include <fstream>
// For swap, copy, and similar operations for our frames
#include <algorithm>
// For processing and working with argv[] as strings
#include <string>

// For threading and keeping track of threads
#include <thread>
#include <vector>

using namespace std;

int loadFile(videoData* dummyVid, char* filePath) {
    std::ifstream binFile;
    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(filePath, std::ios::binary | std::ios::in);

    if (!binFile) {
        std::cout
        << "Failed to open the file for metadata reading."
        << std::endl;
        return 1;
    }
    // file.read sends sizeof(int64_t) bytes to the first variable,
    // which properly casts and sets them to numFrames' address
    binFile.read(reinterpret_cast<char*>(&dummyVid->numFrames),
    sizeof(int64_t));

    binFile.read(reinterpret_cast<char*>(&dummyVid->channels),
    sizeof(unsigned char));

    binFile.read(reinterpret_cast<char*>(&dummyVid->height),
    sizeof(unsigned char));

    binFile.read(reinterpret_cast<char*>(&dummyVid->width),
    sizeof(unsigned char));

    dummyVid->frameSize = (dummyVid->width*dummyVid->height*dummyVid->channels);

    binFile.close();
    return 0;
}

int loadFrames(videoData* dummyVid, char* filePath) {
    std::ifstream binFile;
    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(filePath, std::ios::binary | std::ios::in);

    if (!binFile) {
        std::cout << "Failed to open the file for frame reading." << std::endl;
        return 1;
    }

    binFile.seekg(sizeof(int64_t)
    + sizeof(unsigned char) +
    sizeof(unsigned char) +
    sizeof(unsigned char));  // Skip the data we've collected before

    // Set the reading buffer to 8MB to speed up reading with memory in mind
    binFile.rdbuf()->
    pubsetbuf(0, 8 * 1024 * 1024);
    dummyVid->frameSize = (dummyVid->width*dummyVid->height*dummyVid->channels);

    // https://stackoverflow.com/questions/381244/purpose-of-memory-alignment
    dummyVid->fullFrame = reinterpret_cast<unsigned char*>
    (aligned_alloc(64, dummyVid->frameSize * dummyVid->numFrames));

    binFile.read(reinterpret_cast<char*>
    (dummyVid->fullFrame), (dummyVid->frameSize*dummyVid->numFrames));

    binFile.close();
    return 0;
}

char getDisplayChar(int pixel) {
    if (pixel < 32)     return('.');
    if (pixel < 64)     return(':');
    if (pixel < 96)     return('*');
    if (pixel < 128)    return('+');
    if (pixel < 160)    return('#');
    if (pixel < 192)    return('%');
    if (pixel < 224)    return('&');
    return('@');
}

// SUPPORT FUNCTIONS
void printFrame(videoData inVid,
int initialOffset) {
    for (int z=0; z < inVid.channels; z++) {
        for (int j=0; j < inVid.height; j++) {
            for (int k=0; k < inVid.width; k++) {
                unsigned char pixel = static_cast<int>
                (inVid.fullFrame[
                    initialOffset+(z*inVid.height*inVid.width)+j*inVid.width+k
                ]);
                std::cout << getDisplayChar(pixel) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl << "=-=-=-=" << std::endl;
    }
}

// Write inFile to the filepath, similarly to how we read data
void writeFile(videoData& inFile,
const char filePath[]) {
    std::cout << "Writing file as " << filePath;
    std::ofstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    // every frame consists of pixels with 1 as size
    outFile.write(reinterpret_cast<const char*>
    (inFile.fullFrame), (inFile.frameSize * inFile.numFrames));

    outFile.close();
}

// REVERSE FUNCTIONS
void reverse(videoData& inFile,
const char* filePath) {
    for (int64_t i=0; i < (inFile.numFrames/2); i++) {
        unsigned char* frontFrame =  // Leftmost frame
        inFile.fullFrame + (i*inFile.frameSize);

        unsigned char* backFrame =  // Rightmost frame
        inFile.fullFrame + ((inFile.numFrames - 1 - i)*inFile.frameSize);

        std::swap_ranges(frontFrame,
        (frontFrame + inFile.frameSize), backFrame);
    }

    writeFile(inFile, filePath);
}

void memory_reverse(videoData& inFile,
const char* filePath, const char* sourcePath) {
    unsigned char* frontFrame = new unsigned char[inFile.frameSize];
    unsigned char* backFrame = new unsigned char[inFile.frameSize];

    // Opening this file only once to reduce overhead of non-stop open/close
    std::fstream binFile;
    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(sourcePath, std::ios::binary | std::ios::in | std::ios::out);

    std::fstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    int metadataSize =
    sizeof(int64_t) +
    sizeof(unsigned char)+
    sizeof(unsigned char) +
    sizeof(unsigned char);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    for (int i=0; i < inFile.numFrames/2; i++) {
        int64_t frontPos = (metadataSize+(inFile.frameSize*i));
        int64_t backPos =
        (metadataSize+inFile.frameSize*(inFile.numFrames-1-i));

        binFile.seekg(frontPos);  // Seek to the next frame (from the front)
        binFile.read(reinterpret_cast<char*>(frontFrame), inFile.frameSize);

        binFile.seekg(backPos);  // Seek to the next frame (from the back)
        binFile.read(reinterpret_cast<char*>(backFrame), inFile.frameSize);

        // Write the front and back together, this way we don't store frames
        outFile.seekp(frontPos);

        outFile.write(reinterpret_cast<char*>
        (backFrame), inFile.frameSize);
        // If we're not at the middle frame, write the back frame as well
        if (i != (inFile.numFrames-1-i)) {
            outFile.seekp(backPos);
            outFile.write(reinterpret_cast<char*>
            (frontFrame), inFile.frameSize);
        }
    }

    binFile.close();
    outFile.close();
    delete[] frontFrame;
    delete[] backFrame;
}

// Callable instance for speed_reverse threading, essentially same as reverse()
void reverseChunk(videoData& inFile,
int64_t start, int64_t end) {
    unsigned char* frontFrame =
    inFile.fullFrame + (start * inFile.frameSize);

    unsigned char* backFrame =
    inFile.fullFrame + ((inFile.numFrames - 1 - start) * inFile.frameSize);
    while (start < end) {
        std::swap_ranges(frontFrame, frontFrame + inFile.frameSize, backFrame);
        start++;
        frontFrame += inFile.frameSize;
        backFrame -= inFile.frameSize;
    }
}

void speed_reverse(videoData& inFile,
const char* filePath) {
     // Get the max number of threads we can have running at once
    int totalThreads = std::thread::hardware_concurrency();
    // If can't split the frames, run the regular function to avoid overhead
    if (inFile.numFrames < totalThreads) {
        reverse(inFile, filePath);
        return;
    }
    // Reverse functions in pairs of frames, so we go up to frames/2
    int64_t framesInThread = (inFile.numFrames/2)/totalThreads;
    // Vector for the threads so that we can easily join them
    std::vector<std::thread> threads;

    for (int i=0; i < totalThreads; i++) {
        int64_t chunkStart = i*framesInThread;
        int64_t chunkEnd;
        if (i == (totalThreads-1)) {
            // Last thread handles all remaining frames
            chunkEnd = inFile.numFrames / 2;
        } else {
            chunkEnd = chunkStart + framesInThread;
        }
        // Inspired by https://en.cppreference.com/w/cpp/container/vector/emplace_back
        threads.emplace_back
        (reverseChunk, std::ref(inFile), chunkStart, chunkEnd);
    }

    // https://stackoverflow.com/questions/15027282/c-for-each-pulling-from-vector-elements
    for (auto& thread : threads) {
        thread.join();
    }
    writeFile(inFile, filePath);
}

// SWAP CHANNEL FUNCTIONS
void swap_channel(videoData& inFile,
unsigned char ch1, unsigned char ch2, const char filePath[]) {
    // Move through frames, swapping their chanenls
    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        unsigned char* ch1Start =
        (inFile.fullFrame + frame*inFile.frameSize)
        + (ch1*inFile.width*inFile.height);

        unsigned char* ch2Start =
        (inFile.fullFrame + frame*inFile.frameSize)
        + (ch2*inFile.width*inFile.height);

        std::swap_ranges(ch1Start,
        (ch1Start+(inFile.width*inFile.height)),
        ch2Start);
    }
    writeFile(inFile, filePath);
}

void memory_swap(videoData& inFile,
unsigned char ch1, unsigned char ch2,
const char filePath[], const char sourcePath[]) {
    unsigned char* tempFrame = new unsigned char[inFile.frameSize];

    // Opening this file once to reduce overhead of non-stop calling open/close
    std::fstream binFile;

    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(sourcePath, std::ios::binary | std::ios::in);

    std::fstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    int metadataSize =
    sizeof(int64_t) +
    sizeof(unsigned char) +
    sizeof(unsigned char) +
    sizeof(unsigned char);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    for (int i=0; i < inFile.numFrames; i++) {
        int64_t framePos = (metadataSize+(inFile.frameSize*i));
        binFile.seekg(framePos);  // Seek to the current frame
        binFile.read(reinterpret_cast<char*>(tempFrame), inFile.frameSize);

        unsigned char* ch1Start = tempFrame + (ch1*inFile.width*inFile.height);
        unsigned char* ch2Start = tempFrame + (ch2*inFile.width*inFile.height);

        std::swap_ranges(ch1Start,
        (ch1Start+(inFile.width*inFile.height)),
        ch2Start);

        outFile.seekp(framePos);  // Write the frame to the output file
        outFile.write(reinterpret_cast<char*>
        (tempFrame), inFile.frameSize);
    }

    delete[] tempFrame;
    binFile.close();
    outFile.close();
}

// Same logic as the functionality of the base function, with a chunk of frames
void swapChunk(videoData& inFile,
int64_t chunkStart, int64_t chunkEnd,
unsigned char ch1, unsigned char ch2) {
    for (int64_t frame=chunkStart; frame < chunkEnd; frame++) {
        unsigned char* ch1Start =
        (inFile.fullFrame + frame * inFile.frameSize)
        + (ch1 * inFile.width * inFile.height);

        unsigned char* ch2Start =
        (inFile.fullFrame
        + frame * inFile.frameSize) + (ch2 * inFile.width * inFile.height);

        std::swap_ranges(ch1Start,
        ch1Start + (inFile.width * inFile.height),
        ch2Start);
    }
}

void speed_swap(videoData& inFile,
unsigned char ch1, unsigned char ch2, const char filePath[]) {
    // Hardware_concurrency(); gets the number of threads we can max run at once
    int totalThreads = std::thread::hardware_concurrency();
    // If we can't split the frames, run regular function to avoid overhead
    if (inFile.numFrames < totalThreads) {
        swap_channel(inFile, (ch1+1), (ch2+1), filePath);
        return;
    }

    // Swap functions in pairs of frames, so we go up to frames/2
    int64_t framesInThread = (inFile.numFrames/2/totalThreads);
    // Vector for the threads so that we can easily join them in the end
    std::vector<std::thread> threads;

    for  (int i=0; i < totalThreads; i++) {
        int64_t chunkStart = i*framesInThread;
        int64_t chunkEnd;
        if (i == (totalThreads-1)) {
            // Last thread handles all remaining frames
            chunkEnd = inFile.numFrames;
        } else {
            chunkEnd = chunkStart + framesInThread;
        }
        // Inspired by https://en.cppreference.com/w/cpp/container/vector/emplace_back
        threads.emplace_back
        (swapChunk, std::ref(inFile),
        chunkStart, chunkEnd,
        ch1, ch2);
    }

    // https://stackoverflow.com/questions/15027282/c-for-each-pulling-from-vector-elements
    for (auto& thread : threads) {
        thread.join();
    }
    writeFile(inFile, filePath);
}

// CLIP CHANNEL FUNCTIONS
void clip_channel(videoData& inFile,
int targetChannel, unsigned char minimum,
unsigned char maximum, const char filePath[]) {
    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        unsigned char* channelStart =
        (inFile.fullFrame + frame*inFile.frameSize) +
        (targetChannel*inFile.width*inFile.height);

        // Loop through each pixel in a channel to manipulate them
        for (int i=0; i < (inFile.width*inFile.height); i++) {
            // Adapted from https://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
            channelStart[i] = std::clamp(
                channelStart[i],
                minimum,
                maximum);
        }
    }
    writeFile(inFile, filePath);
}

void memory_clip(videoData& inFile,
int targetChannel, unsigned char minimum,
unsigned char maximum, const char filePath[], const char sourcePath[]) {
    unsigned char* tempFrame = new unsigned char[inFile.frameSize];

    // Opening this file once to reduce overhead of non-stop calling open/close
    std::fstream binFile;

    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(sourcePath, std::ios::binary | std::ios::in);

    std::fstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    int metadataSize =
    sizeof(int64_t) +
    sizeof(unsigned char) +
    sizeof(unsigned char) +
    sizeof(unsigned char);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        int64_t framePos = metadataSize + (inFile.frameSize*frame);
        binFile.seekg(framePos);  // Seek to the current frame
        binFile.read(reinterpret_cast<char*>(tempFrame), inFile.frameSize);

        unsigned char* channelStart =
        tempFrame + (targetChannel*inFile.width*inFile.height);

        for (int i=0; i < (inFile.width*inFile.height); i++) {
            channelStart[i] = std::clamp(channelStart[i], minimum, maximum);
        }

        // Seek back to where the frame would be located, for output
        outFile.seekp(framePos);
        outFile.write(reinterpret_cast<char*>
        (tempFrame), inFile.frameSize);
    }

    delete[] tempFrame;
    binFile.close();
    outFile.close();
}

// Callable instance for speed_reverse threading, essentially same as reverse()
void clipChunk(videoData& inFile,
int64_t chunkStart, int64_t chunkEnd,
int targetChannel, unsigned char minimum, unsigned char maximum) {
    for (int64_t i=chunkStart; i < chunkEnd; i++) {
        unsigned char* channelStart =
        inFile.fullFrame + (i * inFile.frameSize) +
        (targetChannel * inFile.width * inFile.height);

        // Loop through each pixel in a channel to manipulate them
        for (int j = 0; j < (inFile.width * inFile.height); j++) {
            // Adapted from https://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
            channelStart[j] = std::clamp(
                channelStart[j],
                minimum,
                maximum);
        }
    }
}

void speed_clip(videoData& inFile,
int targetChannel, unsigned char minimum,
unsigned char maximum, const char filePath[]) {
    // Hardware_concurrency(); gets the number of threads we can max run at once
    int totalThreads = std::thread::hardware_concurrency();
    // If we can't split the frames, run regular function to avoid overhead
    if (inFile.numFrames < totalThreads) {
        clip_channel(inFile, (targetChannel+1), minimum, maximum, filePath);
        return;
    }

    int64_t framesInThread = (inFile.numFrames/totalThreads);
    // Vector for the threads so that we can easily join them in the end
    std::vector<std::thread> threads;

    for (int i=0; i < totalThreads; i++) {
        int64_t chunkStart = i*framesInThread;
        int64_t chunkEnd;
        if (i == (totalThreads-1)) {
            // Last thread handles all remaining frames
            chunkEnd = inFile.numFrames;
        } else {
            chunkEnd = chunkStart + framesInThread;
        }
        // Inspired by https://en.cppreference.com/w/cpp/container/vector/emplace_back
        threads.emplace_back
        (clipChunk, std::ref(inFile),
        chunkStart, chunkEnd,
        targetChannel, minimum, maximum);
    }

    // https://stackoverflow.com/questions/15027282/c-for-each-pulling-from-vector-elements
    for (auto& thread : threads) {
        thread.join();
    }
    writeFile(inFile, filePath);
}

// SCALE CHANNEL FUNCTIONS
void scale_channel(videoData& inFile,
int targetChannel, float scaleFactor, const char filePath[]) {
    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
            unsigned char* channelStart =
            (inFile.fullFrame + frame*inFile.frameSize) +
            (targetChannel*inFile.width*inFile.height);

            // Loop through each pixel in a channel to manipulate them
            for (int i=0; i < (inFile.width*inFile.height); i++) {
                // Adapted from https://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
                channelStart[i] = std::clamp(
                    (channelStart[i]) * scaleFactor,
                    0.0f,
                    255.0f);
            }
        }
    writeFile(inFile, filePath);
}

void memory_scale(videoData& inFile,
int targetChannel, float scaleFactor,
const char filePath[], const char sourcePath[]) {
    unsigned char* tempFrame = new unsigned char[inFile.frameSize];

    // Opening this file once to reduce overhead of non-stop calling open/close
    std::fstream binFile;
    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(sourcePath, std::ios::binary | std::ios::in);

    std::fstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    int metadataSize =
    sizeof(int64_t) +
    sizeof(unsigned char) +
    sizeof(unsigned char) +
    sizeof(unsigned char);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        int64_t framePos = metadataSize + (inFile.frameSize*frame);
        binFile.seekg(framePos);  // Seek to the current frame
        binFile.read(reinterpret_cast<char*>(tempFrame), inFile.frameSize);

        unsigned char* channelStart =
        tempFrame + (targetChannel*inFile.width*inFile.height);

        // Adapted from https://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
        for (int i=0; i < (inFile.width*inFile.height); i++) {
            channelStart[i] = std::clamp((
                channelStart[i]) * scaleFactor,
                0.0f,
                255.0f);
        }

        // Seek back to where the frame would be located, for output
        outFile.seekp(framePos);

        outFile.write(reinterpret_cast<char*>
        (tempFrame), inFile.frameSize);
    }

    delete[] tempFrame;
    binFile.close();
    outFile.close();
}

// Callable instance for speed_reverse threading, essentially same as reverse()
void scaleChunk(videoData& inFile,
int64_t chunkStart, int64_t chunkEnd,
int targetChannel, float scaleFactor) {
    for (int64_t i=chunkStart; i < chunkEnd; i++) {
        unsigned char* channelStart =
        inFile.fullFrame +
        (i * inFile.frameSize) +
        (targetChannel * inFile.width * inFile.height);

        // Loop through each pixel in a channel to manipulate them
        for (int j = 0; j < (inFile.width * inFile.height); j++) {
            // Adapted from https://stackoverflow.com/questions/9323903/most-efficient-elegant-way-to-clip-a-number
            channelStart[j] = std::clamp(
                (channelStart[j]) * scaleFactor,
                0.0f,
                255.0f);
        }
    }
}

void speed_scale(videoData& inFile,
int targetChannel, float scaleFactor, const char filePath[]) {
    // Hardware_concurrency(); gets the number of threads we can max run at once
    int totalThreads = std::thread::hardware_concurrency();
    // If we can't split the frames, run regular function to avoid overhead
    if (inFile.numFrames < totalThreads) {
        scale_channel(inFile, targetChannel + 1, scaleFactor, filePath);
        return;
    }

    int64_t framesInThread = (inFile.numFrames/totalThreads);
    // Vector for the threads so that we can easily join them in the end
    std::vector<std::thread> threads;

    for (int i=0; i < totalThreads; i++) {
        int64_t chunkStart = i*framesInThread;
        int64_t chunkEnd;
        if (i == (totalThreads-1)) {
            // Last thread handles all remaining frames
            chunkEnd = inFile.numFrames;
        } else {
            chunkEnd = chunkStart + framesInThread;
        }
        // Inspired by https://en.cppreference.com/w/cpp/container/vector/emplace_back
        threads.emplace_back
        (scaleChunk, std::ref(inFile),
        chunkStart, chunkEnd, targetChannel, scaleFactor);
    }

    // https://stackoverflow.com/questions/15027282/c-for-each-pulling-from-vector-elements
    for (auto& thread : threads) {
        thread.join();
    }
    writeFile(inFile, filePath);
}

// SEPIA FUNCTIONS
void sepia_filter(videoData& inFile,
const char filePath[]) {
    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        // All values are stored as r,r,r,r,...g,g,g,...b,b,b - so channel1 is R
        int64_t redStart = frame*inFile.frameSize;
        int64_t greenStart = redStart+(inFile.width*inFile.height);
        int64_t blueStart = redStart+2*(inFile.width*inFile.height);

        for (int i=0; i < inFile.width*inFile.height; i++) {
            unsigned char redValue = inFile.fullFrame[redStart+i];
            unsigned char greenValue = inFile.fullFrame[greenStart+i];
            unsigned char blueValue = inFile.fullFrame[blueStart+i];

            // All sepia values are calculated according to findings in https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
            // This post also motivated the creation of this functionality
            unsigned char newRed =
            std::clamp(
                ((redValue*0.393)+(greenValue*0.769)+(blueValue*0.189)),
                0.0,
                255.0);

            unsigned char newGreen =
            std::clamp(
                ((redValue*0.349)+(greenValue*0.686)+(blueValue*0.168)),
                0.0,
                255.0);

            unsigned char newBlue =
            std::clamp(
                ((redValue*0.272)+(greenValue*0.534)+(blueValue*0.131)),
                0.0,
                255.0);

            inFile.fullFrame[redStart + i] = newRed;
            inFile.fullFrame[greenStart + i] = newGreen;
            inFile.fullFrame[blueStart + i] = newBlue;
        }
    }
    writeFile(inFile, filePath);
}

void sepiaChunk(videoData& inFile,
int64_t chunkStart, int64_t chunkEnd) {
    for (int64_t frame = chunkStart; frame < chunkEnd; frame++) {
        // All values are stored as r,r,r,r,...g,g,g,...b,b,b - so channel1 is R
        int64_t redStart = frame*inFile.frameSize;
        int64_t greenStart = redStart+(inFile.width*inFile.height);
        int64_t blueStart = redStart+2*(inFile.width*inFile.height);

        for (int i = 0; i < inFile.width * inFile.height; i++) {
            unsigned char redValue = inFile.fullFrame[redStart+i];
            unsigned char greenValue = inFile.fullFrame[greenStart+i];
            unsigned char blueValue = inFile.fullFrame[blueStart+i];

            // All sepia values are calculated according to findings in https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
            // This post also motivated the creation of this functionality
            unsigned char newRed =
            std::clamp(
                ((redValue*0.393)+(greenValue*0.769)+(blueValue*0.189)),
                0.0,
                255.0);

            unsigned char newGreen =
            std::clamp(
                ((redValue*0.349)+(greenValue*0.686)+(blueValue*0.168)),
                0.0,
                255.0);

            unsigned char newBlue =
            std::clamp(
                ((redValue*0.272)+(greenValue*0.534)+(blueValue*0.131)),
                0.0,
                255.0);

            inFile.fullFrame[redStart + i] = newRed;
            inFile.fullFrame[greenStart + i] = newGreen;
            inFile.fullFrame[blueStart + i] = newBlue;
        }
    }
}

void speed_sepia(videoData& inFile,
const char filePath[]) {
    // Hardware_concurrency(); gets the number of threads we can max run at once
    int totalThreads = std::thread::hardware_concurrency();
    // If we can't split the frames, run regular function to avoid overhead
    if (inFile.numFrames < totalThreads) {
        sepia_filter(inFile, filePath);
        return;
    }

    int64_t framesInThread = (inFile.numFrames/totalThreads);
    // Vector for the threads so that we can easily join them in the end
    std::vector<std::thread> threads;

    for (int i=0; i < totalThreads; i++) {
        int64_t chunkStart = i*framesInThread;
        int64_t chunkEnd;
        if (i == (totalThreads-1)) {
             // Last thread handles all remaining frames
            chunkEnd = inFile.numFrames;
        } else {
            chunkEnd = chunkStart + framesInThread;
        }
        // Inspired by https://en.cppreference.com/w/cpp/container/vector/emplace_back
        threads.emplace_back
        (sepiaChunk, std::ref(inFile), chunkStart, chunkEnd);
    }

    // https://stackoverflow.com/questions/15027282/c-for-each-pulling-from-vector-elements
    for (auto& thread : threads) {
        thread.join();
    }
    writeFile(inFile, filePath);
}

void memory_sepia(videoData& inFile,
const char filePath[], const char sourcePath[]) {
    unsigned char* tempFrame = new unsigned char[inFile.frameSize];

    // Opening this file once to reduce overhead of non-stop calling open/close
    std::fstream binFile;
    // Adapted from https://www.eecs.umich.edu/courses/eecs380/HANDOUTS/cppBinaryFileIO-2.html
    binFile.open(sourcePath, std::ios::binary | std::ios::in);

    std::fstream outFile;
    outFile.open(filePath, std::ios::binary | std::ios::out);

    int metadataSize = sizeof(int64_t) +
    sizeof(unsigned char) + sizeof(unsigned char) + sizeof(unsigned char);

    outFile.write(reinterpret_cast<const char*>
    (&inFile.numFrames), sizeof(int64_t));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.channels), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.height), sizeof(unsigned char));

    outFile.write(reinterpret_cast<const char*>
    (&inFile.width), sizeof(unsigned char));

    for (int64_t frame=0; frame < inFile.numFrames; frame++) {
        int64_t framePos = metadataSize + (inFile.frameSize*frame);
        binFile.seekg(framePos);  // Seek to the current frame
        binFile.read(reinterpret_cast<char*>(tempFrame), inFile.frameSize);

        // All values are stored as r,r,r,r,...g,g,g,...b,b,b - so channel1 is R
        int64_t redStart = 0;
        // channel2 is G
        int64_t greenStart = redStart+(inFile.width*inFile.height);
        // channel3 is B
        int64_t blueStart = redStart+2*(inFile.width*inFile.height);

        for (int i = 0; i < inFile.width * inFile.height; i++) {
            unsigned char redValue = tempFrame[redStart+i];
            unsigned char greenValue = tempFrame[greenStart+i];
            unsigned char blueValue = tempFrame[blueStart+i];

            // All sepia values are calculated according to findings in https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
            // This post also motivated the creation of this functionality
            unsigned char newRed =
            std::clamp(
                ((redValue*0.393)+(greenValue*0.769)+(blueValue*0.189)),
                0.0,
                255.0);

            unsigned char newGreen =
            std::clamp(
                ((redValue*0.349)+(greenValue*0.686)+(blueValue*0.168)),
                0.0,
                255.0);

            unsigned char newBlue =
            std::clamp(
                ((redValue*0.272)+(greenValue*0.534)+(blueValue*0.131)),
                0.0,
                255.0);

            tempFrame[redStart + i] = newRed;
            tempFrame[greenStart + i] = newGreen;
            tempFrame[blueStart + i] = newBlue;
        }

        // Seek back to where the frame would be located, for output
        outFile.seekp(framePos);
        outFile.write(reinterpret_cast<char*>(tempFrame), inFile.frameSize);
    }

    delete[] tempFrame;
    binFile.close();
    outFile.close();
}
