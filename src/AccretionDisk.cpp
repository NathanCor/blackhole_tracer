#include "../include/AccretionDisk.hpp"
#include <algorithm>
#include <cmath>

double AccretionDisk::sample(const Vec3& pOld, const Vec3& pNew, const Vec3& rayDir) const {
    if ((pOld.z > 0.0 && pNew.z > 0.0) || (pOld.z < 0.0 && pNew.z < 0.0)) {
        return 0.0;
    }

    double t = -pOld.z / (pNew.z - pOld.z);
    Vec3 hit = pOld + (pNew - pOld) * t;
    double r = std::sqrt(hit.x * hit.x + hit.y * hit.y);

    if (r < rIn || r > rOut) return 0.0;

    // Coeur très éclatant (ruban central)
    double core = std::exp(-std::pow((r - 6.1) / 1.3, 2.0)) * 3.6;

    // Anneaux concentriques nets avec creux marqués
    double ring1 = std::exp(-std::pow((r - 8.6) / 0.85, 2.0)) * 1.5;
    double ring2 = std::exp(-std::pow((r - 11.4) / 1.1, 2.0)) * 1.1;
    double ring3 = std::exp(-std::pow((r - 14.8) / 1.6, 2.0)) * 0.7;

    // Traîne extérieure qui s'estompe
    double tail = std::exp(-(r - rIn) / 4.2) * 0.5;

    double baseProfile = core + ring1 + ring2 + ring3 + tail;

    // Vitesse orbitale
    double v_phi = std::sqrt(M / r);
    Vec3 diskVel{-hit.y / r * v_phi, hit.x / r * v_phi, 0.0};

    // Effet Doppler et redshift
    Vec3 k = rayDir.normalized() * -1.0;
    double gamma = 1.0 / std::sqrt(std::max(0.02, 1.0 - diskVel.norm2()));
    double dop = 1.0 / (gamma * (1.0 - diskVel.dot(k)));
    double red = std::sqrt(std::max(0.02, 1.0 - (2.0 * M) / r));
    double g = dop * red;

    return baseProfile * std::pow(g, 1.35);
}