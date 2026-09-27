#pragma once
#include <cstdint>

// Polices 8x8 pour les caractères ASCII 32 à 126
extern const uint8_t FONT8X8[95][8];

void drawText(float* buffer, int width, int height, int startX, int startY, const char* text);