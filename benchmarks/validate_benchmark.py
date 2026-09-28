#!/usr/bin/env python3
"""
Validation physique des géodésiques nulles en métrique de Schwarzschild.
Confrontation numérique (solveur RK4 C++) vs Solution analytique exacte (GYOTO benchmark standard).
"""

import json
import os
import subprocess
import sys
from pathlib import Path

import numpy as np
from scipy.integrate import quad
from scipy.optimize import brentq

# Unités géométrisées G = c = 1, M = 1
M = 1.0
R_S = 2.0 * M
B_CRIT = 3.0 * np.sqrt(3.0) * M
R_START = 2000.0
TEST_B = [4.50, 5.00, 5.15, 5.20, 5.35, 6.00, 8.00, 10.00, 20.00, 50.00]


def theoretical_r_min(b):
    """
    Calcule la racine réelle r_min > 3M de l'équation :
    1/b^2 - (1/r^2)*(1 - 2M/r) = 0 pour b > b_crit.
    """
    if b <= B_CRIT:
        return R_S

    # Polynôme en u = 1/r : 2M*u^3 - u^2 + 1/b^2 = 0
    def eq(u):
        return 2.0 * M * u**3 - u**2 + 1.0 / b**2

    return 1.0 / brentq(eq, 0.0, 1.0 / (3.0 * M) - 1e-9)


def theoretical_deflection(b):
    if b <= B_CRIT:
        return None

    u0 = 1.0 / theoretical_r_min(b)

    def integrand(t):
        u = u0 * (1.0 - t * t)
        return 4.0 * u0 * t / np.sqrt(1.0 / b**2 - u**2 * (1.0 - 2.0 * M * u))

    value, _ = quad(integrand, 0.0, 1.0, limit=400)
    return value - np.pi


def find_executable():
    root = Path(__file__).resolve().parent.parent
    names = ["test_ray", "test_ray.exe"]
    folders = [
        Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else None,
        root / "build",
        root / "build" / "Release",
        root / "cmake-build-release",
        root / "cmake-build-debug",
        root,
        Path.cwd(),
    ]
    for folder in folders:
        if folder is None:
            continue
        if folder.is_file():
            return str(folder)
        for name in names:
            candidate = folder / name
            if candidate.is_file() and os.access(candidate, os.X_OK):
                return str(candidate)
    return None


def run_ray(exe, b):
    res = subprocess.run(
        [exe, f"{b:.6f}", f"{R_START}"],
        capture_output=True,
        text=True,
        check=True,
    )
    return json.loads(res.stdout.strip().splitlines()[-1])


def numerical_critical_b(exe):
    lo, hi = 4.0, 6.0
    for _ in range(40):
        mid = 0.5 * (lo + hi)
        if run_ray(exe, mid)["captured"]:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


def fmt(value, spec):
    return "--" if value is None else format(value, spec)


def main():
    exe = find_executable()
    if exe is None:
        print("test_ray executable not found. Build the project and pass its path or folder as argument.", file=sys.stderr)
        return 2

    print("=" * 108)
    print("Schwarzschild null geodesics: pseudo-Newtonian RK4 solver vs exact GR solution")
    print(f"Executable: {exe}")
    print(f"Start radius: {R_START:g} M")
    print("=" * 108)
    header = (
        f"{'b/M':>6} | {'regime exact':<12} | {'regime code':<11} | {'r_min exact':>11} | "
        f"{'r_min code':>10} | {'rel. err':>9} | {'defl. code':>10} | {'defl. GR':>9} | {'code/GR':>7}"
    )
    print(header)
    print("-" * len(header))

    failures = 0
    ratios = []

    for b in TEST_B:
        try:
            data = run_ray(exe, b)
        except (subprocess.CalledProcessError, json.JSONDecodeError, IndexError) as exc:
            print(f"{b:>6.2f} | execution failed: {type(exc).__name__}")
            failures += 1
            continue

        exact_capture = b <= B_CRIT
        r_exact = theoretical_r_min(b)
        d_exact = theoretical_deflection(b)
        d_code = data["deflection"]
        rel_err = abs(data["r_min"] - r_exact) / r_exact
        ratio = None if (d_code is None or d_exact is None) else d_code / d_exact
        if ratio is not None and b >= 10.0:
            ratios.append(ratio)

        print(
            f"{b:>6.2f} | {'capture' if exact_capture else 'scattering':<12} | "
            f"{'capture' if data['captured'] else 'scattering':<11} | {r_exact:>11.4f} | "
            f"{data['r_min']:>10.4f} | {rel_err:>9.2e} | {fmt(d_code, '10.5f'):>10} | "
            f"{fmt(d_exact, '9.5f'):>9} | {fmt(ratio, '7.3f'):>7}"
        )

    try:
        b_code = numerical_critical_b(exe)
    except (subprocess.CalledProcessError, json.JSONDecodeError, IndexError) as exc:
        print(f"critical impact parameter search failed: {type(exc).__name__}", file=sys.stderr)
        return 1

    print()
    print(f"Critical impact parameter, solver : {b_code:.6f} M")
    print(f"Critical impact parameter, exact  : {B_CRIT:.6f} M")
    print(f"Relative difference               : {abs(b_code - B_CRIT) / B_CRIT:.2e}")
    if ratios:
        print(f"Mean deflection ratio solver/GR (b >= 10 M) : {np.mean(ratios):.3f}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
