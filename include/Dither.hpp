#pragma once
#include <vector>
#include <cstdint>
#include <string>

class DitherProcessor {
public:
    static void applyFloydSteinberg(std::vector<float>& src, int width, int height,
                                    std::vector<uint8_t>& out);

    static void writePPM(const std::string& filename, const std::vector<uint8_t>& data,
                         int width, int height);
};