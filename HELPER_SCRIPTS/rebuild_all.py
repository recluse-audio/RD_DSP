#!/usr/bin/env python3
"""Clean BUILD/ then rebuild Tests + Standalone."""
from __future__ import annotations
import argparse
import shutil
import subprocess
import sys
from pathlib import Path
from build_complete import find_cmake, beep, repo_root


def run(cmd: list[str], cwd: Path) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), check=True)


def regen() -> None:
    script = Path(__file__).parent / "regenSource.py"
    if script.exists():
        subprocess.run([sys.executable, str(script)], check=True)


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", choices=["Debug", "Release"], default="Debug")
    ap.add_argument("--no-clean", action="store_true",
                    help="Skip wiping BUILD/ before configuring")
    return ap.parse_args()


def main() -> int:
    args = parse_args()
    regen()

    root = repo_root()
    build_dir = root / "BUILD"
    if build_dir.exists() and not args.no_clean:
        print(f"Removing {build_dir}")
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    cmake = find_cmake()

    try:
        run([cmake, "-S", str(root), "-B", str(build_dir),
             f"-DCMAKE_BUILD_TYPE={args.config}",
             "-DBUILD_TESTS=ON", "-DBUILD_STANDALONE=ON"], cwd=root)

        for target in ("Tests", "RD_DSP_Standalone"):
            cmd = [cmake, "--build", str(build_dir), "--target", target]
            if sys.platform.startswith("win"):
                cmd += ["--config", args.config]
            run(cmd, cwd=root)
    except subprocess.CalledProcessError:
        beep(success=False)
        return 1

    beep(success=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
