# cpp-video-editor
Quick and lightweight video editor written in C++, Programming Paradigms coursework for 2024/25
All content is found in the libFilmMaster2000.cpp file, definitions in the header file libFilmMaster2000.h - These functionalities are imported into the main file main.cpp to be executed based on user input.

The testSuite bash file contains sequences of commands to evaluate efficiency of each possible input and generate the output for those commands. Individual commands may still be tested by running the main.cpp file.

runme.cpp will be created from the makefile, which is to be run in a terminal environment for processing of video frames.
Videos are to be converted into a binary video format, split by channel (as seen in the binary file in the data directory).

# How to install
This is assuming a Linux environment, such as Ubuntu.
You will require GCC, or an equivalent C++ compiler for this project.

After downloading the source files, simply run the makefile and the project will be built.
The makefile supports cleaning and test commands as well, running the test suite and removing all extra files respectively.

# Usage
The runme executable takes the following general format:
./runme [input file] [output file] [-S/-M] [function] [options]

The [-S/-M] flag forces the app to focus on speed or memory efficiency. Leaving this flag out will yield a balanced solution without prioritising one or the other.
The [function] flag specifies the required operation:
- reverse: Reverses the order of video frames
- swap_channel [ch1,ch2]: Swaps colour channels. Specify channels based on a 0-index system (i.e, swapping the 0th and 1st channels for substituting R and G channels of an RGB video)
- clip_channel [channel] [min,max]: Clips pixel values in the selected channel to the min and max values
- scale_channel [channel] [factor]: Scales pixel values in the selected channel by an input value
- sepia: Applies a sepia filter to the video

# Examples
- Reverse a video: ./runme input.bin output.bin reverse
- Swap channels 1, 2: ./runme input.bin output.bin swap_channel 1,2
- Clip channel 1 to pixel values between 10 and 100: ./runme input.bin output.bin clip_channel 1 [10,100]
- Scale channel 0 by a factor of 2: ./runme input.bin output.bin scale_channel 0 2
- Apply a sepia filter to the video: ./runme input.bin output.bin sepia
