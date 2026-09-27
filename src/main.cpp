#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "../include/Vec3.hpp"
#include "../include/Geodesic.hpp"
#include "../include/AccretionDisk.hpp"
#include "../include/Dither.hpp"
#include "../include/Font8x8.hpp"

int main() {
    constexpr int WIDTH = 1920;
    constexpr int HEIGHT = 1080;
    constexpr double FOV = 0.58;

    // Angle rasant et perspective
    Vec3 camPos(0.0, -36.0, 6.2);
    Vec3 lookAt(0.0, 0.0, -0.2);

    Vec3 forward = (lookAt - camPos).normalized();
    Vec3 rawRight = forward.cross({0.0, 0.0, 1.0}).normalized();
    Vec3 rawUp = rawRight.cross(forward).normalized();

    // Inclinaison accentuée et orientée vers la gauche (-0.38 rad ~ -22 degrés)
    constexpr double tiltAngle = -0.38;
    Vec3 right = rawRight * std::cos(tiltAngle) + rawUp * std::sin(tiltAngle);
    Vec3 up = rawRight * (-std::sin(tiltAngle)) + rawUp * std::cos(tiltAngle);

    GeodesicIntegrator integrator(1.0);
    // Disque concentré de 5.2M à 19.5M pour un ratio d'aspect idéal
    AccretionDisk disk(5.0, 19.5, 1.0);

    std::vector<float> luminanceBuffer(WIDTH * HEIGHT, 0.0f);

    std::cout << "Calcul des geodesiques relativistes (" << WIDTH << "x" << HEIGHT << ")..." << std::endl;

    #pragma omp parallel for schedule(dynamic, 8)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double u = (x - WIDTH * 0.5) / (HEIGHT * 0.5) * std::tan(FOV * 0.5);
            double v = -(y - HEIGHT * 0.5) / (HEIGHT * 0.5) * std::tan(FOV * 0.5);

            Vec3 rayDir = (forward + right * u + up * v).normalized();
            RayState state{camPos, rayDir};

            double accumulatedIntensity = 0.0;
            constexpr double stepSize = 0.09; // Pas plus fin pour des contours très nets

            for (int step = 0; step < 1100; ++step) {
                double r = state.pos.norm();

                // Horizon des événements (ombre du trou noir)
                if (r <= integrator.getRs() * 1.008) {
                    break;
                }

                // Rayon échappé à l'infini
                if (r > 48.0) {
                    break;
                }

                RayState nextState = integrator.stepRK4(state, stepSize);

                double dI = disk.sample(state.pos, nextState.pos, state.vel);
                if (dI > 0.0) {
                    accumulatedIntensity += dI;
                }

                state = nextState;
            }

            luminanceBuffer[y * WIDTH + x] = static_cast<float>(accumulatedIntensity);
        }
    }

    // Recherche du maximum pour normalisation
    float maxVal = 0.0f;
    for (float v : luminanceBuffer) {
        if (v > maxVal) maxVal = v;
    }

    if (maxVal > 0.0f) {
        for (float& v : luminanceBuffer) {
            if (v <= 0.0f) continue;
            float norm = v / maxVal;
            // Gamma plus doux (0.42) + boost : fait ressortir tous les anneaux secondaires
            float boosted = std::pow(norm, 0.42f) * 1.25f;
            v = std::clamp(boosted, 0.0f, 1.0f);
        }
    }

    // Incrustation de la citation en bas à droite
    drawText(luminanceBuffer.data(), WIDTH, HEIGHT, WIDTH - 520, HEIGHT - 140,
             "Nothing escapes the pull of clean code.");

    // Dithering matriciel Floyd-Steinberg
    std::cout << "Application du tramage matriciel..." << std::endl;
    std::vector<uint8_t> binaryImage;
    DitherProcessor::applyFloydSteinberg(luminanceBuffer, WIDTH, HEIGHT, binaryImage);

    std::cout << "Export vers blackhole.ppm..." << std::endl;
    DitherProcessor::writePPM("blackhole.ppm", binaryImage, WIDTH, HEIGHT);

    std::cout << "Termine !" << std::endl;
    return 0;
}