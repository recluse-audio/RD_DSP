#!/usr/bin/env python3
import numpy as np
import argparse

def find_and_print_peak(path_to_csv):
    a = np.atleast_2d(np.genfromtxt(path_to_csv, delimiter=','))
    row, col = np.unravel_index(np.nanargmax(np.abs(a)), a.shape)
    print(f"row {row}, col {col} = {a[row, col]}")

def main() -> None:
    parser = argparse.ArgumentParser(description='detect peak in a waveform or wavetable csv (or any csv)')
    parser.add_argument('--csv_path', type=str, help='path to csv to analyze')
    args = parser.parse_args()

    if not args.csv_path:
        parser.error("--csv_path is required")

    find_and_print_peak(args.csv_path)

if __name__ == "__main__":
    main()
