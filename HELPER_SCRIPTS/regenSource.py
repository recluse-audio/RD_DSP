#!/usr/bin/env python3
"""
Regenerate CMAKE/SOURCES.cmake and CMAKE/TESTS.cmake by scanning directories.
Run after adding or removing .h/.cpp files. Build scripts call this automatically.
"""
from __future__ import annotations
import os
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]


def generate_files_list(root_folders: list[str], output_file: Path, variable_name: str) -> None:
    files_list: list[str] = []
    for root_folder in root_folders:
        if not os.path.exists(root_folder):
            continue
        for folder, _subfolders, files in os.walk(root_folder):
            for filename in files:
                if filename.endswith(".cpp") or filename.endswith(".h"):
                    file_path = os.path.join(folder, filename).replace("\\", "/")
                    files_list.append(file_path)
    files_list.sort()
    if files_list:
        joined = "\n    ".join(files_list)
        output = f"set({variable_name}\n    {joined}\n)"
    else:
        output = f"set({variable_name}\n    # No files found\n)"
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, "w") as f:
        f.write(output)
    print(f"Generated {output_file} with {len(files_list)} file(s)")


def main() -> None:
    os.chdir(REPO_ROOT)

    src_folders = ["SOURCE"] if os.path.exists("SOURCE") else []
    print(f"Scanning source folders: {src_folders or '(none)'}")
    generate_files_list(src_folders, REPO_ROOT / "CMAKE" / "SOURCES.cmake", "SOURCES")

    test_folders = ["TESTS"] if os.path.exists("TESTS") else []
    print(f"Scanning test folders: {test_folders or '(none)'}")
    generate_files_list(test_folders, REPO_ROOT / "CMAKE" / "TESTS.cmake", "TEST_SOURCES")


if __name__ == "__main__":
    main()
