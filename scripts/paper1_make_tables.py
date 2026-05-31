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

max_bytes = max(r["bytes"] for r in rows)
max_bytes_by_case = defaultdict(int)
for r in rows:
    max_bytes_by_case[r["case"]] = max(max_bytes_by_case[r["case"]], r["bytes"])

# speedup table by case, using each case's largest available size
by_case = defaultdict(dict)
for r in rows:
    if r["bytes"] == max_bytes_by_case[r["case"]]:
        by_case[r["case"]][r["build_mode"]] = r

bad_rows = [r for r in rows if r["correct"] != "1"]

print("# Paper 1 Tables")
print()
print(f"Largest-image subset bytes={max_bytes}")
print()
print("## Correctness Summary")
print()
if bad_rows:
    print(f"WARNING: {len(bad_rows)} experiment(s) failed correctness.")
else:
    print("All experiments report `correct=1`.")
print()
print("## ISA Build Speedup (largest available image per case)")
print()
print("| case | shape | scalar enc MB/s | SSE2 enc speedup | AVX2 enc speedup | scalar dec MB/s | SSE2 dec speedup | AVX2 dec speedup |")
print("|---|---|---:|---:|---:|---:|---:|---:|")
for case in sorted(by_case.keys()):
    c = by_case[case]
    s = c.get("scalar_build")
    sse2 = c.get("sse2_build")
    avx2 = c.get("avx2_build") or c.get("simd_build")
    if not s:
        continue
    sse2_enc_sp = sse2["enc_MBps"] / s["enc_MBps"] if sse2 and s["enc_MBps"] > 0 else 0.0
    avx2_enc_sp = avx2["enc_MBps"] / s["enc_MBps"] if avx2 and s["enc_MBps"] > 0 else 0.0
    sse2_dec_sp = sse2["dec_MBps"] / s["dec_MBps"] if sse2 and s["dec_MBps"] > 0 else 0.0
    avx2_dec_sp = avx2["dec_MBps"] / s["dec_MBps"] if avx2 and s["dec_MBps"] > 0 else 0.0
    print(
        f"| {case} | {s['shape']} | {s['enc_MBps']:.1f} | {sse2_enc_sp:.2f}x | {avx2_enc_sp:.2f}x | "
        f"{s['dec_MBps']:.1f} | {sse2_dec_sp:.2f}x | {avx2_dec_sp:.2f}x |"
    )

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
