"""Shared utilities for RD_DSP build scripts."""
from __future__ import annotations
import shutil
import sys
from pathlib import Path


def find_cmake() -> str:
    """Return cmake executable path. Checks PATH, then common install locations."""
    if found := shutil.which("cmake"):
        return found

    if sys.platform == "darwin":
        candidates = [
            "/opt/homebrew/bin/cmake",
            "/usr/local/bin/cmake",
            "/Applications/CMake.app/Contents/bin/cmake",
        ]
        for path in candidates:
            if Path(path).exists():
                return path

    if sys.platform.startswith("win"):
        candidates = [
            r"C:\Program Files\CMake\bin\cmake.exe",
            r"C:\Program Files (x86)\CMake\bin\cmake.exe",
        ]
        for path in candidates:
            if Path(path).exists():
                return path

    raise FileNotFoundError(
        "cmake not found. Install via package manager or https://cmake.org/download/"
    )


def beep(*, success: bool = True) -> None:
    """Cross-platform audible notification. Best-effort."""
    try:
        if sys.platform.startswith("win"):
            import winsound
            winsound.Beep(880 if success else 220, 120 if success else 400)
        else:
            sys.stdout.write("\a")
            sys.stdout.flush()
    except Exception as e:
        print(f"beep error: {e}")


def repo_root() -> Path:
    """Resolve the repo root from this script's location."""
    return Path(__file__).resolve().parents[1]
