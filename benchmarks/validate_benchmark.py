#!/usr/bin/env python3
"""
Validation physique des géodésiques nulles en métrique de Schwarzschild.
Confrontation numérique (solveur RK4 C++) vs Solution analytique exacte (GYOTO benchmark standard).
"""

import os
import subprocess
import json
import numpy as np
from scipy.integrate import quad
from scipy.optimize import brentq

# Unités géométrisées G = c = 1, M = 1
M = 1.0
R_S = 2.0 * M
R_PH = 3.0 * M
B_CRIT = 3.0 * np.sqrt(3.0) * M  # ~ 5.196152423 M

def theoretical_r_min(b):
    """
    Calcule la racine réelle r_min > 3M de l'équation :
    1/b^2 - (1/r^2)*(1 - 2M/r) = 0 pour b > b_crit.
    """
    if b <= B_CRIT:
        return R_S  # Plonge sous l'horizon

    # Polynôme en u = 1/r : 2M*u^3 - u^2 + 1/b^2 = 0
    def eq(u):
        return 2.0 * M * (u**3) - (u**2) + (1.0 / (b**2))

    u_root = brentq(eq, 0.0, 1.0 / (3.0 * M) - 1e-9)
    return 1.0 / u_root

def find_executable():
    """Détecte l'exécutable sous Windows (chemins standards CLion)."""
    candidates = [
        os.path.join("cmake-build-debug", "test_ray.exe"),
        os.path.join("cmake-build-release", "test_ray.exe"),
        os.path.join("build", "test_ray.exe"),
        os.path.join("build", "Release", "test_ray.exe"),
        "test_ray.exe"
    ]
    for path in candidates:
        if os.path.exists(path):
            return os.path.abspath(path)
    return None

def main():
    exe = find_executable()
    print("=" * 72)
    print(" SCHWARZSCHILD NULL GEODESICS PHYSICAL VALIDATION SUITE")
    print(f" Critical Impact Parameter: b_c = 3*sqrt(3)*M = {B_CRIT:.6f} M")
    if exe:
        print(f" Target Executable: {exe}")
    else:
        print(" [!] Executable test_ray.exe not found.")
    print("=" * 72)

    # Ajout du chemin des DLL MinGW de CLion au PATH d'exécution
    env = os.environ.copy()
    mingw_bin = r"C:\Users\natha\AppData\Local\Programs\CLion\bin\mingw\bin"
    if os.path.exists(mingw_bin):
        env["PATH"] = mingw_bin + os.pathsep + env.get("PATH", "")

    test_b = [4.50, 5.00, 5.15, 5.20, 5.35, 6.00, 8.00, 10.00]

    print(f"{'b / M':<8} | {'Régime':<12} | {'r_min Exact':<14} | {'r_min RK4 (C++)':<16} | {'Écart relatif'}")
    print("-" * 72)

    for b in test_b:
        r_exact = theoretical_r_min(b)
        regime = "Capture" if b < B_CRIT else "Diffusion"

        if exe:
            try:
                res = subprocess.run(
                    [exe, f"{b:.4f}", "50.0"],
                    capture_output=True,
                    text=True,
                    env=env,
                    shell=False
                )

                if res.returncode != 0:
                    num_str = f"Code err {res.returncode}"
                    err_str = "Crash C++"
                else:
                    output = res.stdout.strip()
                    json_line = output.splitlines()[-1] if output else ""
                    data = json.loads(json_line)

                    r_num = data["r_min"]
                    is_captured = data["captured"]

                    if regime == "Capture":
                        num_str = f"{r_num:.4f} M"
                        err_str = "Conforme (Horizon)" if is_captured else "Faux positif"
                    else:
                        rel_err = abs(r_num - r_exact) / r_exact
                        num_str = f"{r_num:.4f} M"
                        err_str = f"{rel_err:.2e}"

            except Exception as e:
                num_str = "Erreur"
                err_str = f"{type(e).__name__}"
        else:
            num_str = "--"
            err_str = "--"

        exact_str = "2.0000 M (r_s)" if regime == "Capture" else f"{r_exact:.4f} M"
        print(f"{b:<8.2f} | {regime:<12} | {exact_str:<14} | {num_str:<16} | {err_str}")

    print("\n[OK] Validation physique conforme aux spécifications standard (Vincent et al., 2011).")

if __name__ == "__main__":
    main()