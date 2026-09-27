#pragma once
#include "Vec3.hpp"

class AccretionDisk {
public:
    AccretionDisk(double r_in = 6.0, double r_out = 24.0, double mass = 1.0)
        : rIn(r_in), rOut(r_out), M(mass) {}

    // Évalue la traversée du plan z = 0 par le rayon
    // Retourne l'intensité apparente perçue par le détecteur
    double sample(const Vec3& pOld, const Vec3& pNew, const Vec3& rayDir) const;

private:
    double rIn;
    double rOut;
    double M;
};