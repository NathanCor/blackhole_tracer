#include "../include/Dither.hpp"
#include "../include/Font8x8.hpp"
#include <fstream>
#include <algorithm>
#include <cstring>

// Table de glyphes minimale pour le texte
const uint8_t FONT8X8[95][8] = {
    {0,0,0,0,0,0,0,0},         // space
    {24,60,60,24,24,0,24,0},   // !
    {0,0,0,0,0,0,0,0},         // "
    {0,0,0,0,0,0,0,0},         // #
    {0,0,0,0,0,0,0,0},         // $
    {0,0,0,0,0,0,0,0},         // %
    {0,0,0,0,0,0,0,0},         // &
    {0,0,0,0,0,0,0,0},         // '
    {12,24,48,48,48,24,12,0},  // (
    {48,24,12,12,12,24,48,0},  // )
    {0,0,0,0,0,0,0,0},         // *
    {0,0,0,0,0,0,0,0},         // +
    {0,0,0,0,0,24,24,48},      // ,
    {0,0,0,126,0,0,0,0},       // -
    {0,0,0,0,0,24,24,0},       // .
    {3,6,12,24,48,96,192,0},   // /
    {60,102,110,118,102,60,0,0}, // 0
    {0,0,0,0,0,0,0,0}
};

void drawText(float* buffer, int width, int height, int startX, int startY, const char* text) {
    int curX = startX;
    size_t len = strlen(text);
    for (size_t c = 0; c < len; ++c) {
        char ch = text[c];
        if (ch >= 32 && ch <= 126) {
            const uint8_t* glyph = FONT8X8[ch - 32];
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    if (glyph[row] & (1 << (7 - col))) {
                        // Écriture agrandie x2 pour le texte
                        for (int dy = 0; dy < 2; ++dy) {
                            for (int dx = 0; dx < 2; ++dx) {
                                int px = curX + col * 2 + dx;
                                int py = startY + row * 2 + dy;
                                if (px >= 0 && px < width && py >= 0 && py < height) {
                                    buffer[py * width + px] = 1.0f;
                                }
                            }
                        }
                    }
                }
            }
        }
        curX += 16;
    }
}

void DitherProcessor::applyFloydSteinberg(std::vector<float>& src, int width, int height,
                                         std::vector<uint8_t>& out) {
    // 3 octets par pixel (RGB)
    out.assign(width * height * 3, 0);

    // Couleur de fond : Bleu nuit très foncé style GitHub / espace (#0d1117)
    constexpr uint8_t bgR = 13;
    constexpr uint8_t bgG = 17;
    constexpr uint8_t bgB = 23;

    // Remplissage initial de l'image avec la couleur de fond
    for (int i = 0; i < width * height; ++i) {
        out[i * 3 + 0] = bgR;
        out[i * 3 + 1] = bgG;
        out[i * 3 + 2] = bgB;
    }

    constexpr float bayer4x4[4][4] = {
        {  0.0f/16.0f,  8.0f/16.0f,  2.0f/16.0f, 10.0f/16.0f },
        { 12.0f/16.0f,  4.0f/16.0f, 14.0f/16.0f,  6.0f/16.0f },
        {  3.0f/16.0f, 11.0f/16.0f,  1.0f/16.0f,  9.0f/16.0f },
        { 15.0f/16.0f,  7.0f/16.0f, 13.0f/16.0f,  5.0f/16.0f }
    };

    constexpr int CELL = 2;

    for (int y = 0; y < height; y += CELL) {
        for (int x = 0; x < width; x += CELL) {
            float sum = 0.0f;
            int count = 0;
            for (int dy = 0; dy < CELL && (y + dy) < height; ++dy) {
                for (int dx = 0; dx < CELL && (x + dx) < width; ++dx) {
                    sum += src[(y + dy) * width + (x + dx)];
                    count++;
                }
            }
            float val = (count > 0) ? (sum / count) : 0.0f;

            if (val <= 0.005f) continue;

            float threshold = bayer4x4[(y / CELL) % 4][(x / CELL) % 4];

            if (val > threshold * 0.65f) {
                bool full = (val > 0.60f);

                // Couleur du point
                uint8_t ptR = static_cast<uint8_t>(std::clamp(190 + val * 65.0f, 0.0f, 255.0f));
                uint8_t ptG = static_cast<uint8_t>(std::clamp(215 + val * 40.0f, 0.0f, 255.0f));
                uint8_t ptB = 255;

                for (int dy = 0; dy < CELL && (y + dy) < height; ++dy) {
                    for (int dx = 0; dx < CELL && (x + dx) < width; ++dx) {
                        if (full || (dx == 0 && dy == 0)) {
                            int pIdx = ((y + dy) * width + (x + dx)) * 3;
                            out[pIdx + 0] = ptR;
                            out[pIdx + 1] = ptG;
                            out[pIdx + 2] = ptB;
                        }
                    }
                }
            }
        }
    }
}

void DitherProcessor::writePPM(const std::string& filename, const std::vector<uint8_t>& data,
                              int width, int height) {
    std::ofstream ofs(filename, std::ios::binary);
    // En-tête P6 (RGB binaire)
    ofs << "P6\n" << width << " " << height << "\n255\n";
    // Écriture directe des pixels RGB
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
}