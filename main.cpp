// Copyright 2024 Luka Mihajlovic, written to pass the cpplint check
#include <iostream>
// For processing and working with argv[] as strings
#include <string>
#include "libFilmMaster2000.h"

// MAIN FUNCTIONS
int handleFunctions(int argc, char* argv[]) {
    unsigned char offset = 0;
    std::string flagSetting = argv[3];
    if ((flagSetting == "-S") || (flagSetting == "-M")) {
        offset = 1;
    }

    // Slight overhead with structs (padding), but much more readable
    videoData inVid;
    if (loadFile(&inVid, argv[1]) == 1) {
        return 1;
    }

    // Wanted to use a switch, using if-else instead: https://cplusplus.com/forum/beginner/70619/
    std::string command = argv[3 + offset];

    if (command == "reverse") {
        if (argc != 4 + offset) {
            std::cout << "Invalid number of parameters." << std::endl;
            std::cout
            << "reverse takes 3 mandatory arguments in the type:"
            << " input output -S/-M(OPTIONAL) reverse"
            << std::endl;
            return 1;
        }
        // From now on, for checking flags, I'll check the offset number
        if (offset == 1) {
            if (std::string(argv[3]) == "-M") {
                memory_reverse(inVid, argv[2], argv[1]);
            } else {
                loadFrames(&inVid, argv[1]);
                speed_reverse(inVid, argv[2]);
            }
        } else {
            loadFrames(&inVid, argv[1]);
            reverse(inVid, argv[2]);
        }

    } else if (command == "swap_channel") {
        if (argc != 5 + offset) {  // 5 or 6 due to optional flags
            std::cout << "Invalid number of parameters:" << argc << std::endl;
            std::cout << "swap_channel takes 5 mandatory arguments in the type:"
            <<" input output -S/-M(OPTIONAL) swap_channel channel_a,channel_b"
            << std::endl;
            return 1;
        }

        // Find where the comma is, take everything before/after as min/max
        size_t splitter = (std::string(argv[4 + offset]).find(','));
        if (splitter == std::string::npos) {
            std::cout
            << "Range format incorrect. [min, max] should be of type x,y"
            << std::endl;
            return 1;
        }

        // Everything after position 2 requires offset for optional flags
        int channelAInput = std::stoi(std::string(argv[4 + offset])
        .substr(0, splitter));

        int channelBInput = std::stoi(std::string(argv[4 + offset])
        .substr(splitter+1));

        // Convert the channel to an int, compare with channel number
        if (channelAInput >= inVid.channels ||
            channelBInput >= inVid.channels ||
            channelAInput < 0 ||
            channelBInput < 0) {
            std::cout
            << "Channel out of bounds error. "
            << static_cast<unsigned int>(inVid.channels)-1
            << " is the highest channel index in the video. Input was "
            << channelAInput << " and " << channelBInput << std::endl;
            return 1;
        }
        if (offset == 1) {
            if (std::string(argv[3]) == "-M") {
                memory_swap(inVid, channelAInput,
                channelBInput, argv[2], argv[1]);
            } else {
                loadFrames(&inVid, argv[1]);
                speed_swap(inVid, channelAInput, channelBInput, argv[2]);
            }

        } else {
            loadFrames(&inVid, argv[1]);
            swap_channel(inVid, channelAInput, channelBInput, argv[2]);
        }

    } else if (command == "clip_channel") {
        if (argc != 6 + offset) {
            std::cout << "Invalid number of parameters." << std::endl;
            std::cout
            << "clip_channel takes 5 mandatory arguments in the type:"
            << "input output -S/-M(OPTIONAL) clip_channel channel [min, max]"
            << std::endl;
            return 1;
        }

        int channelInput = std::stoi(argv[4 + offset]);
        // Convert the channel to an int, compare with channel number
        if (channelInput >= inVid.channels || channelInput < 0) {
            std::cout
            << "Channel out of bounds error. "
            << static_cast<unsigned int>(inVid.channels)-1
            << "  is the highest channel index in the video. Input was "
            << channelInput << std::endl;
            return 1;
        }

        std::string rangeInput = argv[5 + offset];
        if (rangeInput.at(0) != '[' ||
        rangeInput.at(rangeInput.size()-1) != ']') {
            std::cout
            << "Missing brackets on range. Input should be of type [min, max]"
            << std::endl;
            return 1;
        }

        // Find where the comma is, take everything before/after as the min/max
        size_t splitter = (rangeInput.find(','));
        if (splitter == std::string::npos || rangeInput.size() < 5) {
            std::cout
            << "Range format incorrect. [min, max] should be of type [x,y]"
            << std::endl;
            return 1;
        }

        // https://stackoverflow.com/questions/23834624/remove-first-and-last-character-c
        rangeInput = rangeInput.substr(1, rangeInput.size() - 2);
        int min = stoi((rangeInput).substr(0, splitter));
        int max = stoi((rangeInput).substr(splitter));
        if (min < 0 || max > 255) {
            std::cout
            << "Range format incorrect. [min, max] range "
            << "should be between 0 and 255"
            << std::endl;
        }
        if (offset == 1) {
            if (std::string(argv[3]) == "-M") {
                memory_clip(inVid, channelInput, min, max, argv[2], argv[1]);
            } else {
                loadFrames(&inVid, argv[1]);
                speed_clip(inVid, channelInput, min, max, argv[2]);
            }
        } else {
            loadFrames(&inVid, argv[1]);
            clip_channel(inVid, channelInput, min, max, argv[2]);
        }

    } else if (command == "scale_channel") {
        if (argc != 6 + offset) {
            std::cout << "Invalid number of parameters." << std::endl;
            std::cout
            << "scale_channel takes 5 mandatory arguments in the type:"
            << "input output -S/-M(OPTIONAL) scale_channel channel factor"
            << std::endl;
            return 1;
        }
        int channelInput = std::stoi(argv[4 + offset]);
        if (channelInput >= inVid.channels ||
            channelInput < 0) {
            std::cout
            << "Channel out of bounds error. "
            << static_cast<unsigned int>(inVid.channels)-1
            << "  is the highest channel index in the video. Input was "
            << channelInput << std::endl;
            return 1;
        }
        float scaleFactor = std::stof(argv[5+offset]);
        if (scaleFactor < 0) {
            std::cout
            << "Can not scale channel by negative number!"
            << std::endl;
            return 1;
        }
        if (offset == 1) {
            if (std::string(argv[3]) == "-M") {
                memory_scale(inVid, channelInput, scaleFactor,
                argv[2], argv[1]);
            } else {
                loadFrames(&inVid, argv[1]);
                speed_scale(inVid, channelInput, scaleFactor, argv[2]);
            }
        } else {
            loadFrames(&inVid, argv[1]);
            scale_channel(inVid, channelInput, scaleFactor, argv[2]);
        }
    } else if (command == "show_video") {
        loadFrames(&inVid, argv[1]);
        for (int i=0; i < inVid.numFrames; i++) {
            std::cout << "FRAME #" << i << std::endl;
            int initialOffset = (i*inVid.frameSize);
            printFrame(inVid, initialOffset);
        }
    // } else if (command == "sepia") {
    //     if (argc != 4+offset) {
    //         std::cout << "Invalid number of parameters." << std::endl;
    //         std::cout
    //         << "sepia takes 3 mandatory arguments in the type:"
    //         << "input output -S/-M(OPTIONAL) sepia"
    //         << std::endl;
    //         return 1;
    //     }

    //     if (inVid.channels < 3) {
    //         std::cout
    //         << "Sepia command requires three channels, input video contains "
    //         << inVid.channels
    //         << "channels."
    //         << std::endl;
    //         return 1;
    //     }

    //     if (offset == 1) {
    //         if (std::string(argv[3]) == "-M") {
    //             memory_sepia(inVid, argv[2], argv[1]);
    //         } else {
    //             loadFrames(&inVid, argv[1]);
    //             speed_sepia(inVid, argv[2]);
    //         }
    //     } else {
    //         loadFrames(&inVid, argv[1]);
    //         sepia_filter(inVid, argv[2]);
    //     }
    } else {
        std::cout << "Invalid command." << std::endl;
        std::cout << "Valid commands are:" << std::endl;
        std::cout
        << "reverse, swap_channel, clip_channel, "
        << "scale_channel"
        << std::endl;
        return 1;
    }

    free(inVid.fullFrame);
    return 0;
}

int main(int argc, char* argv[]) {
    if (handleFunctions(argc, argv) == 1) {
        return 1;
    } else {
        return 0;
    }

    return 0;
}
