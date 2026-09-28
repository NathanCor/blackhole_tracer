#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include "../include/Vec3.hpp"
#include "../include/Geodesic.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <impact_parameter_b> [r_start]\n";
        return 1;
    }

    double b = std::stod(argv[1]);
    double r_start = (argc >= 3) ? std::stod(argv[2]) : 50.0;

    if (r_start <= b) {
        r_start = b + 10.0;
    }

    GeodesicIntegrator integrator(1.0);
    RayState ray;
    ray.pos = Vec3(-std::sqrt(r_start * r_start - b * b), b, 0.0);
    ray.vel = Vec3(1.0, 0.0, 0.0).normalized();

    double r_min = r_start;
    bool captured = false;
    constexpr double ds = 0.01;
    constexpr int max_steps = 30000;
    int step = 0;

    while (step++ < max_steps) {
        double r = ray.pos.norm();
        if (r < r_min) {
            r_min = r;
        }

        // Arrêt si capture sous l'horizon
        if (r <= integrator.getRs() + 0.01) {
            captured = true;
            r_min = integrator.getRs();
            break;
        }

        // Arrêt si le rayon s'éloigne à l'infini après déviation
        if (r > r_start && ray.pos.x > 0.0) {
            break;
        }

        ray = integrator.stepRK4(ray, ds);
    }

    // Sortie JSON lue par le script Python
    std::cout << "{\"b\": " << b
              << ", \"r_min\": " << r_min
              << ", \"captured\": " << (captured ? "true" : "false")
              << "}" << std::endl;

    return 0;
}