#!/usr/bin/env python3
import csv
import sys
from collections import defaultdict

if len(sys.argv) != 2:
    print("usage: paper1_make_tables.py <combined_csv>")
    sys.exit(1)

path = sys.argv[1]
rows = []
with open(path, newline="") as f:
    r = csv.DictReader(f)
    for row in r:
        rows.append(row)

if not rows:
    print("no rows")
    sys.exit(1)

# normalize numeric fields
for row in rows:
    row["enc_MBps"] = float(row["enc_MBps"])
    row["dec_MBps"] = float(row["dec_MBps"])
    row["bytes"] = int(row["bytes"])

# pick largest image shape by bytes
max_bytes = max(r["bytes"] for r in rows)
largest = [r for r in rows if r["bytes"] == max_bytes]

# speedup table by case
by_case = defaultdict(dict)
for r in largest:
    by_case[r["case"]][r["build_mode"]] = r

print("# Paper 1 Tables")
print()
print(f"Largest-image subset bytes={max_bytes}")
print()
print("## SIMD Build Speedup (largest image)")
print()
print("| case | scalar enc MB/s | simd enc MB/s | enc speedup | scalar dec MB/s | simd dec MB/s | dec speedup |")
print("|---|---:|---:|---:|---:|---:|---:|")
for case in sorted(by_case.keys()):
    c = by_case[case]
    s = c.get("scalar") or c.get("scalar_build")
    v = c.get("simd") or c.get("simd_build")
    if not s or not v:
        continue
    enc_sp = v["enc_MBps"] / s["enc_MBps"] if s["enc_MBps"] > 0 else 0.0
    dec_sp = v["dec_MBps"] / s["dec_MBps"] if s["dec_MBps"] > 0 else 0.0
    print(f"| {case} | {s['enc_MBps']:.1f} | {v['enc_MBps']:.1f} | {enc_sp:.2f}x | {s['dec_MBps']:.1f} | {v['dec_MBps']:.1f} | {dec_sp:.2f}x |")

print()
print("## Full Results")
print()
print("| build_mode | case | simd_backend | shape | enc MB/s | dec MB/s | correct |")
print("|---|---|---|---|---:|---:|---:|")
for r in sorted(rows, key=lambda x: (x["bytes"], x["build_mode"], x["case"])):
    print(
        f"| {r['build_mode']} | {r['case']} | {r['simd_backend']} | {r['shape']} | "
        f"{r['enc_MBps']:.1f} | {r['dec_MBps']:.1f} | {r['correct']} |"
    )
