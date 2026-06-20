#!/usr/bin/env python3
"""Paired per-image ISA speedups with image-level bootstrap intervals."""

import argparse
import csv
import math
import random
import statistics
from collections import defaultdict
from pathlib import Path


def read_rows(path: Path):
    with path.open(newline="") as handle:
        yield from csv.DictReader(handle)


def geometric_mean(values):
    return math.exp(statistics.fmean(math.log(value) for value in values))


def percentile(sorted_values, probability):
    if not sorted_values:
        return math.nan
    position = probability * (len(sorted_values) - 1)
    lower = int(position)
    upper = min(lower + 1, len(sorted_values) - 1)
    fraction = position - lower
    return sorted_values[lower] * (1.0 - fraction) + sorted_values[upper] * fraction


def bootstrap_ci(ratios, samples, seed):
    rng = random.Random(seed)
    estimates = []
    for _ in range(samples):
        draw = [ratios[rng.randrange(len(ratios))] for _ in ratios]
        estimates.append(geometric_mean(draw))
    estimates.sort()
    return percentile(estimates, 0.025), percentile(estimates, 0.975)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("--baseline", default="scalar_build")
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--bootstrap-samples", type=int, default=10000)
    parser.add_argument("--seed", type=int, default=2026)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    required = {"image", "case", "build_mode", "enc_MBps", "dec_MBps", "correct"}
    rows = list(read_rows(args.input))
    if not rows:
        raise SystemExit("input contains no data rows")
    missing = required - set(rows[0])
    if missing:
        raise SystemExit("missing columns: " + ", ".join(sorted(missing)))

    # Repetitions are averaged within each image before images become the units of analysis.
    grouped = defaultdict(list)
    for row in rows:
        if row["correct"] != "1" or row["build_mode"] not in {args.baseline, args.candidate}:
            continue
        for direction in ("enc", "dec"):
            value = float(row[f"{direction}_MBps"])
            if value > 0:
                grouped[(row["image"], row["case"], row["build_mode"], direction)].append(value)

    image_means = {key: statistics.fmean(values) for key, values in grouped.items()}
    ratios_by_case = defaultdict(list)
    for (image, case, mode, direction), baseline_value in image_means.items():
        if mode != args.baseline:
            continue
        candidate_key = (image, case, args.candidate, direction)
        if candidate_key in image_means:
            ratios_by_case[(case, direction)].append(image_means[candidate_key] / baseline_value)

    output_rows = []
    for index, ((case, direction), ratios) in enumerate(sorted(ratios_by_case.items())):
        low, high = bootstrap_ci(ratios, args.bootstrap_samples, args.seed + index)
        output_rows.append({
            "case": case,
            "direction": direction,
            "baseline": args.baseline,
            "candidate": args.candidate,
            "images": len(ratios),
            "geomean_speedup": geometric_mean(ratios),
            "median_speedup": statistics.median(ratios),
            "ci95_low": low,
            "ci95_high": high,
        })

    if not output_rows:
        raise SystemExit("no paired image/case observations found")

    fieldnames = list(output_rows[0])
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(output_rows)

    print("| case | direction | images | geometric mean speedup | median | bootstrap 95% CI |")
    print("|---|---|---:|---:|---:|---:|")
    for row in output_rows:
        print(f"| {row['case']} | {row['direction']} | {row['images']} | "
              f"{row['geomean_speedup']:.3f}x | {row['median_speedup']:.3f}x | "
              f"[{row['ci95_low']:.3f}, {row['ci95_high']:.3f}] |")


if __name__ == "__main__":
    main()
