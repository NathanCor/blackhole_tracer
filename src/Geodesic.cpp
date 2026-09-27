#include "../include/Geodesic.hpp"

Vec3 GeodesicIntegrator::computeAcceleration(const Vec3& pos) const {
    double r = pos.norm();
    if (r <= rs) return {0, 0, 0};

    // Potentiel effectif avec précession et capture relativiste
    double factor = -M / (std::pow(r - rs, 2.0) * r);
    return pos * factor;
}

RayState GeodesicIntegrator::stepRK4(const RayState& s, double h) const {
    auto deriv = [this](const RayState& st) -> RayState {
        return {st.vel, computeAcceleration(st.pos)};
    };

    RayState k1 = deriv(s);

    RayState s2{s.pos + k1.pos * (0.5 * h), s.vel + k1.vel * (0.5 * h)};
    RayState k2 = deriv(s2);

    RayState s3{s.pos + k2.pos * (0.5 * h), s.vel + k2.vel * (0.5 * h)};
    RayState k3 = deriv(s3);

    RayState s4{s.pos + k3.pos * h, s.vel + k3.vel * h};
    RayState k4 = deriv(s4);

    RayState next;
    next.pos = s.pos + (k1.pos + k2.pos * 2.0 + k3.pos * 2.0 + k4.pos) * (h / 6.0);
    next.vel = s.vel + (k1.vel + k2.vel * 2.0 + k3.vel * 2.0 + k4.vel) * (h / 6.0);
    return next;
}