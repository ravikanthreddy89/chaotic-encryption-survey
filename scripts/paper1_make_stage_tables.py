#!/usr/bin/env python3
import csv
import sys
from collections import defaultdict


def read_rows(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            row["bytes"] = int(row["bytes"])
            row["ms"] = float(row["ms"])
            row["MBps"] = float(row["MBps"])
            rows.append(row)
    return rows


def speedup_rows(rows, title, stage_filter=None):
    if not rows:
        return

    filtered = [r for r in rows if stage_filter is None or r["stage"] in stage_filter]
    if not filtered:
        return

    max_bytes = max(r["bytes"] for r in filtered)
    max_bytes_by_key = defaultdict(int)
    for r in filtered:
        key = (r["case"], r["stage"])
        max_bytes_by_key[key] = max(max_bytes_by_key[key], r["bytes"])

    by_key = defaultdict(dict)
    for r in filtered:
        key = (r["case"], r["stage"])
        if r["bytes"] == max_bytes_by_key[key]:
            by_key[key][r["build_mode"]] = r

    print(f"## {title}")
    print()
    print(f"Largest observed bytes={max_bytes}; rows use each case/stage's largest available size.")
    print()
    print("| case | stage | shape | scalar MB/s | SSE2 speedup | AVX2 speedup | correct |")
    print("|---|---|---|---:|---:|---:|---:|")
    for key in sorted(by_key.keys()):
        builds = by_key[key]
        scalar = builds.get("scalar_build")
        sse2 = builds.get("sse2_build")
        avx2 = builds.get("avx2_build")
        if not scalar:
            continue
        scalar_mbps = scalar["MBps"]
        sse2_sp = sse2["MBps"] / scalar_mbps if sse2 and scalar_mbps > 0 else 0.0
        avx2_sp = avx2["MBps"] / scalar_mbps if avx2 and scalar_mbps > 0 else 0.0
        correct = "1" if all(r["correct"] == "1" for r in builds.values()) else "0"
        print(f"| {key[0]} | {key[1]} | {scalar['shape']} | {scalar_mbps:.1f} | {sse2_sp:.2f}x | {avx2_sp:.2f}x | {correct} |")
    print()


def bottleneck_rows(rows):
    if not rows:
        return

    max_bytes_by_case = defaultdict(int)
    for r in rows:
        max_bytes_by_case[r["case"]] = max(max_bytes_by_case[r["case"]], r["bytes"])

    selected = [
        r for r in rows
        if r["build_mode"] == "scalar_build" and r["bytes"] == max_bytes_by_case[r["case"]]
    ]

    totals = {}
    stages = []
    for r in selected:
        key = (r["case"], r["shape"])
        if r["stage"] in ("encrypt_total", "decrypt_total"):
            totals[(key, r["stage"])] = r["ms"]
        else:
            stages.append(r)

    print("## Scalar Stage Breakdown")
    print()
    print("| case | shape | stage | ms | share of matching total |")
    print("|---|---|---|---:|---:|")
    for r in sorted(stages, key=lambda x: (x["case"], x["shape"], x["stage"])):
        key = (r["case"], r["shape"])
        total_stage = "encrypt_total" if r["stage"].startswith("encrypt_") else "decrypt_total"
        total = totals.get((key, total_stage), 0.0)
        share = (r["ms"] / total * 100.0) if total > 0 else 0.0
        print(f"| {r['case']} | {r['shape']} | {r['stage']} | {r['ms']:.3f} | {share:.1f}% |")
    print()


def main():
    if len(sys.argv) not in (2, 3):
        print("usage: paper1_make_stage_tables.py <stages_csv> [micro_csv]")
        sys.exit(1)

    stage_rows = read_rows(sys.argv[1])
    micro_rows = read_rows(sys.argv[2]) if len(sys.argv) == 3 else []

    bad = [r for r in stage_rows + micro_rows if r["correct"] != "1"]

    print("# Paper 1 Stage Tables")
    print()
    if bad:
        print(f"WARNING: {len(bad)} stage or microbench row(s) failed correctness.")
    else:
        print("All stage and microbench rows report `correct=1`.")
    print()

    speedup_rows(
        stage_rows,
        "Pipeline Stage Speedup",
        {
            "encrypt_keystream_logistic",
            "encrypt_diffuse",
            "decrypt_keystream_logistic",
            "decrypt_undiffuse",
            "encrypt_permute_map",
            "decrypt_inverse_map",
            "encrypt_sort_scores",
            "encrypt_sort_indices",
            "decrypt_sort_scores",
            "decrypt_sort_indices",
        },
    )
    speedup_rows(micro_rows, "Isolated Kernel Speedup")
    bottleneck_rows(stage_rows)


if __name__ == "__main__":
    main()
