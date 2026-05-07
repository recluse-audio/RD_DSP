#!/usr/bin/env python3
"""Configure and build the RD_DSP_Standalone CLI demo."""
from __future__ import annotations
import argparse
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
    ap.add_argument("--clean", action="store_true")
    return ap.parse_args()


def main() -> int:
    args = parse_args()
    regen()

    root = repo_root()
    build_dir = root / "BUILD"
    build_dir.mkdir(parents=True, exist_ok=True)

    cmake = find_cmake()
    print(f"cmake: {cmake} | config: {args.config}")

    try:
        run([cmake, "-S", str(root), "-B", str(build_dir),
             f"-DCMAKE_BUILD_TYPE={args.config}",
             "-DBUILD_TESTS=OFF", "-DBUILD_STANDALONE=ON"], cwd=root)

        build_cmd = [cmake, "--build", str(build_dir), "--target", "RD_DSP_Standalone"]
        if sys.platform.startswith("win"):
            build_cmd += ["--config", args.config]
        if args.clean:
            build_cmd.append("--clean-first")
        run(build_cmd, cwd=root)
    except subprocess.CalledProcessError:
        beep(success=False)
        return 1

    beep(success=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
