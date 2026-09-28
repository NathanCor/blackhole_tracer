# Schwarzschild-style Black Hole Ray Tracer

A stylized C++20 ray tracer simulating light bending around a compact mass and a Keplerian accretion disk with relativistic Doppler shading, rendered via matrix dithering.

<p align="center">
  <img src="blackhole.png" alt="Schwarzschild Black Hole Render" width="100%">
</p>

---

## Overview

This project traces rays backward from a camera through a strong gravitational field, reproducing the visual signature of a black hole: a photon-sphere-like shadow and a lensed, Doppler-shaded accretion disk. It favors a **fast, visually convincing approximation** over a full general-relativistic integration — see "Physical Model" below for exactly what is and isn't implemented.

The output pipeline renders the accumulated radiance into a stylized, dithered matrix look tailored for dark environments (`#0d1117`).

---

## Physical Model

### 1. Light bending — pseudo-Newtonian approximation

Rays are **not** integrated as true null geodesics of the Schwarzschild metric. Instead, `GeodesicIntegrator` applies a Newtonian force derived from the **Paczyński–Wiita pseudo-potential**:

$$\Phi(r) = -\frac{M}{r - r_s}, \qquad r_s = 2M$$

$$\vec{a}(\vec{r}) = -\nabla\Phi = -\frac{M}{(r - r_s)^2}\,\hat{r}$$

integrated with a standard 4th-order Runge-Kutta scheme. This potential is a well-known trick for reproducing the correct **ISCO at $r = 6M$** for massive-particle orbits, and it gives a qualitatively black-hole-like deflection and shadow for rendering purposes. It is **not** the real photon geodesic equation. Benchmarked against the exact Schwarzschild solution (see *Validation & Benchmarks*), it reproduces the capture threshold and the closest-approach radius of null geodesics to better than $0.3\%$, but it deflects light only about **half** as much as general relativity. Treat the render as an artistic approximation, not a physically exact simulation.

- **Event horizon (cutoff):** $r_s = 2M$
- **Disk inner edge:** set to the ISCO of the pseudo-potential, $r = 6M$ (configurable)

### 2. Accretion disk — Doppler-shaded, hand-styled brightness

The disk crosses the equatorial plane $z=0$ between a configurable `rIn`/`rOut`. Its base brightness profile is **not derived from a physical emissivity law** (e.g. Shakura–Sunyaev or Novikov–Thorne); it's a hand-tuned sum of Gaussian rings chosen to look good (bright core near the inner edge, a few concentric bands, a fading outer tail). Treat it as a stylistic stand-in for a real emissivity profile.

What *is* physically modeled is the relativistic shading on top of that base brightness. The disk material moves at the Newtonian circular (Keplerian) speed:

$$v_\phi = \sqrt{\frac{M}{r}}$$

and each sample is shaded by the special-relativistic Doppler factor toward the camera:

$$g = \frac{1}{\gamma\,(1 - \vec{v}\cdot\vec{k})}\sqrt{1 - \frac{2M}{r}}, \qquad \gamma = \frac{1}{\sqrt{1-v^2}}$$

The final intensity uses an **artistic exponent**, not the physical bolometric beaming law ($I_\text{obs} \propto g^4 I_\text{em}$ for an ideal blackbody disk):

$$I_{\text{obs}} = I_{\text{base}} \cdot g^{1.35}$$

The lower exponent was chosen to keep the far (redshifted) side of the disk visible rather than crushing it to black, at the cost of physical accuracy.

### 3. Rendering pipeline
- Fixed-step RK4 backward ray marching from the camera, one ray per pixel (OpenMP-parallelized over rows).
- Rays are stopped at the horizon cutoff ($r \le 1.008\,r_s$) or once they escape past $r = 48M$.
- Disk contribution is accumulated by sampling every plane crossing along each ray.
- Output is tone-mapped (gamma + boost), then dithered with Floyd–Steinberg and written out as a `#0d1117`-styled PPM/PNG.

---

## Project Structure

```text
.
├── CMakeLists.txt
├── include/
│   ├── AccretionDisk.hpp
│   ├── Dither.hpp
│   ├── Font8x8.hpp
│   ├── Geodesic.hpp
│   └── Vec3.hpp
├── src/
│   ├── AccretionDisk.cpp
│   ├── Dither.cpp
│   ├── Geodesic.cpp
│   └── main.cpp
└── benchmarks/
    ├── test_ray.cpp
    └── validate_benchmark.py
```

---

## Build & Usage

### Prerequisites
- C++20 compliant compiler (`gcc >= 11` or `clang >= 13`)
- CMake >= 3.20
- OpenMP (optional, enables CPU multithreading if found)
- FFmpeg (optional — only needed for the `render` target's automatic PPM → PNG conversion)

### Build with CMake

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

This produces the `blackhole_tracer` executable and the `test_ray` benchmark helper. `blackhole_tracer` writes `blackhole.ppm` when run:

```bash
./blackhole_tracer
```

If FFmpeg is found on your system, an additional `render` target is generated that runs the tracer and converts the output to PNG in one step:

```bash
cmake --build . --target render
```

Without FFmpeg, convert `blackhole.ppm` to PNG with any tool you like, e.g.:

```bash
ffmpeg -y -i blackhole.ppm blackhole.png
# or, with Python/Pillow:
python3 -c "from PIL import Image; Image.open('blackhole.ppm').save('blackhole.png')"
```

> Note: there is no Makefile in this project — only CMake is supported.

---

## Configuration

Camera perspective, inclination angle, step size, and disk radii can be adjusted directly in `src/main.cpp`:

```cpp
constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr double FOV = 0.58;

Vec3 camPos(0.0, -36.0, 6.2);
constexpr double tiltAngle = -0.38; // Inclination angle
```

---

## Validation & Benchmarks

### Theoretical Framework

In Schwarzschild spacetime ($G = c = 1$, $M = 1$), the radial equation governing equatorial null geodesics ($\theta = \pi/2$) is parameterized by the impact parameter $b \equiv L/E$:

$$\left(\frac{dr}{d\lambda}\right)^2 = \frac{1}{b^2} - V_{\text{eff}}(r) \quad \text{where} \quad V_{\text{eff}}(r) = \frac{1}{r^2}\left(1 - \frac{2M}{r}\right)$$

* **Critical impact parameter**: The effective potential attains its maximum at the photon sphere $r_{\text{ph}} = 3M$, defining the capture threshold:
  $$b_c = 3\sqrt{3}M \approx 5.196152\,M$$
* **Plunge regime ($b < b_c$)**: Photons overcome the centrifugal barrier and cross the event horizon ($r \to 2M$).
* **Scattering regime ($b > b_c$)**: Photons reach a turning point $r_{\text{min}}$ (periastron), the largest root of $1/b^2 - V_{\text{eff}}(r) = 0$, before escaping to infinity. The total deflection angle is
  $$\hat{\alpha}(b) = 2\int_0^{u_0} \frac{du}{\sqrt{1/b^2 - u^2(1 - 2Mu)}} - \pi, \qquad u_0 = 1/r_{\text{min}}$$
  which behaves as $4M/b$ at large $b$.

### Methodology

`benchmarks/validate_benchmark.py` compares the C++ RK4 solver (`test_ray`) with the exact analytical solution above, computed with `scipy.integrate.quad` and `scipy.optimize.brentq`. No output from a third-party ray-tracing code (such as GYOTO) is used: the reference is the analytical Schwarzschild result.

Each ray is launched from $r = 2000\,M$ with unit velocity and impact parameter $b$. The launch radius matters: starting much closer (for example at $50\,M$) changes the ray's energy at infinity in the pseudo-Newtonian potential and inflates the apparent errors on $r_{\text{min}}$ and $b_c$ by one to two orders of magnitude.

### Results

| $b/M$ | Exact regime | Solver regime | Exact $r_{\text{min}}$ | Solver $r_{\text{min}}$ | Relative error | Solver deflection (rad) | Exact deflection (rad) | Solver / exact |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `4.50` | Plunge | Plunge | $2.0000$ | $2.0000$ | — | — | — | — |
| `5.00` | Plunge | Plunge | $2.0000$ | $2.0000$ | — | — | — | — |
| `5.15` | Plunge | Plunge | $2.0000$ | $2.0000$ | — | — | — | — |
| `5.20` | Scattering | Scattering | $3.0687$ | $3.0766$ | $2.6 \times 10^{-3}$ | $3.703$ | $6.810$ | $0.544$ |
| `5.35` | Scattering | Scattering | $3.5072$ | $3.5094$ | $6.4 \times 10^{-4}$ | $1.733$ | $3.183$ | $0.544$ |
| `6.00` | Scattering | Scattering | $4.4534$ | $4.4554$ | $4.7 \times 10^{-4}$ | $0.912$ | $1.719$ | $0.531$ |
| `8.00` | Scattering | Scattering | $6.7005$ | $6.7035$ | $4.5 \times 10^{-4}$ | $0.445$ | $0.859$ | $0.518$ |
| `10.00` | Scattering | Scattering | $8.7889$ | $8.7928$ | $4.5 \times 10^{-4}$ | $0.303$ | $0.590$ | $0.513$ |
| `20.00` | Scattering | Scattering | $18.9130$ | $18.9220$ | $4.8 \times 10^{-4}$ | $0.119$ | $0.236$ | $0.506$ |
| `50.00` | Scattering | Scattering | $48.9683$ | $48.9923$ | $4.9 \times 10^{-4}$ | $0.043$ | $0.085$ | $0.502$ |

Critical impact parameter: $b_c = 5.19525\,M$ (solver, by bisection) versus $5.19615\,M$ (exact), a relative difference of $1.7 \times 10^{-4}$.

### Interpretation

* The **capture threshold and the periastron radius** are reproduced closely: the regime is correct for every tested $b$, $b_c$ agrees to $1.7 \times 10^{-4}$, and $r_{\text{min}}$ agrees to about $5 \times 10^{-4}$ (up to $2.6 \times 10^{-3}$ very close to $b_c$).
* The **deflection angle is about half of the general-relativistic value** (solver/exact ratio between $0.50$ and $0.54$). This is the known limitation of a Newtonian force law: it yields a deflection of $2M/b$ where general relativity gives $4M/b$. Gravitational lensing is therefore systematically underestimated, and the renders should be read as qualitatively, not quantitatively, correct.

### Reproducing the Benchmark

Build the project (see above), then run from the repository root:

```bash
python benchmarks/validate_benchmark.py build
```

The optional argument is the folder containing `test_ray` (or the path to the executable). Without it, the script looks in `build/`, `build/Release/`, `cmake-build-release/`, `cmake-build-debug/`, the repository root and the current directory. It exits with a non-zero code if the executable is not found or a run fails.

## License

This project is open-source under the MIT License.
