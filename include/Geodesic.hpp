#pragma once
#include "Vec3.hpp"

struct RayState {
    Vec3 pos; // Position en coordonnées pseudo-cartésiennes
    Vec3 vel; // Vitesse d'impulsion du photon (dr/dλ)
};

class GeodesicIntegrator {
public:
    explicit GeodesicIntegrator(double mass = 1.0) : M(mass), rs(2.0 * mass) {}

    // Dérivée d²x/dλ² selon l'approximation de champ fort de Paczynski-Wiita
    // reproduisant exactement r_ISCO = 6M et la sphère photon à 3M
    Vec3 computeAcceleration(const Vec3& pos) const;

    RayState stepRK4(const RayState& s, double h) const;

    double getMass() const { return M; }
    double getRs() const { return rs; }

private:
    double M;
    double rs;
};