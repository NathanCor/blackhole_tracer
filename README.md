# Schwarzschild Black Hole Ray Tracer

A high-performance C++20 relativistic ray tracer simulating gravitational lensing, null geodesics around a Schwarzschild black hole, and an accretion disk with relativistic Doppler effects, rendered via matrix dithering.

<p align="center">
  <img src="blackhole.png" alt="Schwarzschild Black Hole Render" width="100%">
</p>

---

## Overview

Light rays propagating in the vicinity of a compact mass follow null geodesics defined by the metric of curved spacetime. This project integrates photon trajectories in the **Schwarzschild metric** backward from a distant camera to their source, reproducing the photon sphere, the shadow of the black hole, and the distorted geometry of a thin Keplerian accretion disk.

The output pipeline renders the accumulated radiance into a stylized, dithered matrix look tailored for dark environments.

---

## Physical Foundations

### 1. Metric and Equations of Motion
In geometrized units ($G = c = 1$), the Schwarzschild metric in spherical coordinates $(t, r, \theta, \phi)$ with signature $(-, +, +, +)$ is:

$$ds^2 = -\left(1 - \frac{2M}{r}\right)dt^2 + \left(1 - \frac{2M}{r}\right)^{-1}dr^2 + r^2(d\theta^2 + \sin^2\theta \, d\phi^2)$$

Light rays follow null geodesics ($ds^2 = 0$). By taking advantage of spherical symmetry, the trajectory of a photon can be integrated using the effective geodesic equations with a 4th-order Runge-Kutta scheme (RK4):

$$\frac{d^2 x^i}{d\lambda^2} + \Gamma^i_{\mu\nu} \frac{dx^\mu}{d\lambda} \frac{dx^\nu}{d\lambda} = 0$$

- **Event Horizon:** $r_s = 2M$
- **Photon Sphere:** $r_{ph} = 3M$
- **ISCO (Innermost Stable Circular Orbit):** $r_{isco} = 6M$

### 2. Relativistic Doppler & Redshift
The plasma orbiting within the disk moves at Keplerian velocity:

$$v_\phi = \sqrt{\frac{M}{r}}$$

The emitted intensity is shifted according to the relativistic factor $g$:

$$g = \frac{\nu_{\text{obs}}}{\nu_{\text{em}}} = \frac{1}{\gamma (1 - \vec{v} \cdot \vec{k})} \sqrt{1 - \frac{2M}{r}}$$

where $\gamma = (1 - v^2)^{-1/2}$ is the Lorentz factor, and $\vec{k}$ is the photon direction unit vector in the emitter frame. The observed specific intensity scales with:

$$I_{\text{obs}} \propto g^4 I_{\text{em}}$$

### 3. Rendering Pipeline
- **Numerical Integration:** Adaptive RK4 step backward from the observer.
- **Accretion Disk Crossing:** Continuous boundary sampling on the equatorial plane $z = 0$.
- **Dithering:** Floyd-Steinberg and ordered thresholding mapped over a `#0d1117` GitHub dark canvas.

---

## Project Structure

```text
.
├── CMakeLists.txt
├── Makefile
├── include/
│   ├── AccretionDisk.hpp
│   ├── Dither.hpp
│   ├── Font8x8.hpp
│   ├── Geodesic.hpp
│   └── Vec3.hpp
└── src/
    ├── AccretionDisk.cpp
    ├── Dither.cpp
    ├── Geodesic.cpp
    └── main.cpp
