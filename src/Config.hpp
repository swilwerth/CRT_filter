//
// Created by sebastian on 18/2/20.
//

#ifndef SDL_CRT_FILTER_CONFIG_HPP
#define SDL_CRT_FILTER_CONFIG_HPP
#include <ResourceRoller.hpp>

class Config {
public:
    static constexpr auto sampleBitmapsFolder = "resources/images";
    static const nint samples = 6;
    const std::string sampleBitmapsNames[6] = {
            "standby.png",
            "RCA_Indian_Head_Test_Pattern.png",
            "standby640.bmp",
            "testCardRGB.bmp",
            "marcosvtar.bmp",
            "alf.bmp"
    };

    static const int SCREEN_WIDTH  = 640;
    static const int SCREEN_HEIGHT = 480;
    static const int TARGET_WIDTH  = 1024;
    static const int TARGET_HEIGHT = 768;

    nint initResources(ResourceRoller& channels) {
        const std::string path  = sampleBitmapsFolder;
        const std::string type = "image";
        auto str = sampleBitmapsNames;
        nint count = samples;
        for(nint i = 0; i < count; ++i)
            channels.Add(str[i],  path + "/" + str[i], type);
        return count;
    }
};

#endif //SDL_CRT_FILTER_CONFIG_HPP
