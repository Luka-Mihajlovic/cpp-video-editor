// Copyright 2024 Luka Mihajlovic, written to pass the cpplint check
#ifndef LIBFILMMASTER2000_H_
#define LIBFILMMASTER2000_H_

// For int64_t instead of long
#include <cstdint>

// Ordering based on size, to avoid padding out memory assigned in the struct
struct videoData{
    unsigned char* fullFrame = 0;
    int64_t numFrames;
    int frameSize;
    unsigned char channels;
    unsigned char height;
    unsigned char width;
};

// IO
int loadFile(videoData* videoPath, char* filePath);

int loadFrames(videoData* videoPath, char* filePath);

// SUPPORT FUNCTIONS
char getDisplayChar(int pixelValue);

void printFrame(videoData targetVideo, int offset);

void writeFile(videoData& outputVideo, const char outputPath[]);

// REVERSE
void reverse(videoData& inputVideo, const char* outputPath);

void memory_reverse(videoData& inputVideo,
const char* outputPath, const char* fileSourcePath);

void reverseChunk(videoData& inputVideo, int64_t start, int64_t end);

void speed_reverse(videoData& inputVideo, const char* outputPath);

// SWAP
void swap_channel(videoData& inputVideo, unsigned char Channel1,
unsigned char Channel2, const char outputPath[]);

void memory_swap(videoData& inputVideo, unsigned char Channel1,
unsigned char Channel2, const char outputPath[], const char fileSourcePath[]);

void swapChunk(videoData& inputVideo, int64_t chunkStart,
int64_t chunkEnd, unsigned char Channel1, unsigned char Channel2);

void speed_swap(videoData& inputVideo, unsigned char Channel1,
unsigned char Channel2, const char outputPath[]);

// CLIP
void clip_channel(videoData& inputVideo, int targetChannel,
unsigned char minimum, unsigned char maximum, const char outputPath[]);

void memory_clip(videoData& inputVideo, int targetChannel,
unsigned char minimum, unsigned char maximum, const char outputPath[],
const char fileSourcePath[]);

void clipChunk(videoData& inputVideo, int64_t chunkStart,
int64_t chunkEnd, int targetChannel, unsigned char minimum,
unsigned char maximum);

void speed_clip(videoData& inputVideo, int targetChannel,
unsigned char minimum, unsigned char maximum, const char outputPath[]);

// SCALE
void scale_channel(videoData& inputVideo, int targetChannel,
float scaleFactor, const char outputPath[]);

void memory_scale(videoData& inputVideo, int targetChannel,
float scaleFactor, const char outputPath[], const char fileSourcePath[]);

void scaleChunk(videoData& inputVideo, int64_t chunkStart,
int64_t chunkEnd, int targetChannel, float scaleFactor);

void speed_scale(videoData& inputVideo, int targetChannel,
float scaleFactor, const char outputPath[]);

void sepia_filter(videoData& inputVideo, const char* outputPath);

void sepiaChunk(videoData& inputVideo, int64_t chunkStart, int64_t chunkEnd);

void speed_sepia(videoData& inputVideo, const char* outputPath);

void memory_sepia(videoData& inputVideo, const char* outputPath,
const char* fileSourcePath);

#endif  // LIBFILMMASTER2000_H_
