#!/usr/bin/env python3
"""Aggregate final benchmark runs into paper-ready CSVs, tables, plots, and summaries."""

from __future__ import annotations

import argparse
import csv
import math
import platform
import statistics
import subprocess
import struct
import zlib
from collections import defaultdict
from pathlib import Path


def read_csv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="") as f:
        return list(csv.DictReader(f))


def write_csv(path: Path, rows: list[dict[str, object]], fields: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for row in rows:
            w.writerow({k: row.get(k, "") for k in fields})


def parse_image_meta(image: str) -> tuple[str, str]:
    stem = Path(image).stem
    parts = stem.rsplit("_", 1)
    if len(parts) == 2 and "x" in parts[1]:
        return parts[0], parts[1]
    return stem, ""


def load_kind(root: Path, file_name: str) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for path in sorted((root / "raw").glob(f"*/{file_name}")):
        run_id = path.parent.name
        parts = run_id.split("__")
        mode = parts[0] if len(parts) > 0 else ""
        repeat = parts[-1].replace("rep", "") if parts[-1].startswith("rep") else ""
        for row in read_csv(path):
            kind, size = parse_image_meta(row.get("image", ""))
            row["run_id"] = run_id
            row["mode"] = mode
            row["repeat"] = repeat
            row["image_kind"] = kind
            row["image_size"] = size
            rows.append(row)
    return rows


def fnum(v: object) -> float:
    try:
        return float(v)
    except (TypeError, ValueError):
        return math.nan


def aggregate(rows: list[dict[str, object]], group_fields: list[str], metric: str) -> list[dict[str, object]]:
    groups: dict[tuple[object, ...], list[float]] = defaultdict(list)
    for row in rows:
        value = fnum(row.get(metric))
        if not math.isnan(value):
            groups[tuple(row.get(k, "") for k in group_fields)].append(value)
    out: list[dict[str, object]] = []
    for key, values in sorted(groups.items()):
        item = {k: v for k, v in zip(group_fields, key)}
        item.update(
            {
                "n": len(values),
                f"{metric}_mean": statistics.fmean(values),
                f"{metric}_median": statistics.median(values),
                f"{metric}_min": min(values),
                f"{metric}_max": max(values),
                f"{metric}_stddev": statistics.stdev(values) if len(values) > 1 else 0.0,
            }
        )
        out.append(item)
    return out


def best_by(rows: list[dict[str, object]], key_name: str, metric: str, limit: int = 8) -> list[dict[str, object]]:
    best: dict[str, dict[str, object]] = {}
    for row in rows:
        key = str(row.get(key_name, ""))
        value = fnum(row.get(metric))
        if math.isnan(value):
            continue
        if key not in best or value > fnum(best[key].get(metric)):
            best[key] = row
    return sorted(best.values(), key=lambda r: fnum(r.get(metric)), reverse=True)[:limit]


def markdown_table(rows: list[dict[str, object]], columns: list[str]) -> str:
    lines = ["| " + " | ".join(columns) + " |", "|" + "|".join("---" for _ in columns) + "|"]
    for row in rows:
        vals = []
        for col in columns:
            v = row.get(col, "")
            vals.append(f"{v:.3f}" if isinstance(v, float) else str(v))
        lines.append("| " + " | ".join(vals) + " |")
    return "\n".join(lines)


def command_output(cmd: list[str]) -> str:
    try:
        return subprocess.check_output(cmd, text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return "unavailable"


def write_png(path: Path, width: int, height: int, rgb: bytearray) -> None:
    def chunk(kind: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + kind
            + data
            + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
        )

    rows = bytearray()
    stride = width * 3
    for y in range(height):
        rows.append(0)
        rows.extend(rgb[y * stride : (y + 1) * stride])
    payload = b"".join(
        [
            b"\x89PNG\r\n\x1a\n",
            chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)),
            chunk(b"IDAT", zlib.compress(bytes(rows), 9)),
            chunk(b"IEND", b""),
        ]
    )
    path.write_bytes(payload)


def fallback_bar_png(path: Path, rows: list[dict[str, object]], value_key: str) -> None:
    width, height = 1000, 520
    margin_l, margin_r, margin_t, margin_b = 60, 30, 30, 80
    rgb = bytearray([255, 255, 255] * width * height)

    def px(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < width and 0 <= y < height:
            i = (y * width + x) * 3
            rgb[i : i + 3] = bytes(color)

    def rect(x0: int, y0: int, x1: int, y1: int, color: tuple[int, int, int]) -> None:
        for yy in range(max(0, y0), min(height, y1)):
            row = yy * width * 3
            for xx in range(max(0, x0), min(width, x1)):
                i = row + xx * 3
                rgb[i : i + 3] = bytes(color)

    for x in range(margin_l, width - margin_r):
        px(x, height - margin_b, (40, 40, 40))
    for y in range(margin_t, height - margin_b):
        px(margin_l, y, (40, 40, 40))

    rows = rows[:18]
    if not rows:
        write_png(path, width, height, rgb)
        return
    max_v = max(fnum(r.get(value_key)) for r in rows) or 1.0
    plot_w = width - margin_l - margin_r
    plot_h = height - margin_t - margin_b
    slot = max(1, plot_w // len(rows))
    palette = [(35, 118, 183), (236, 133, 46), (57, 158, 89), (190, 66, 66), (112, 81, 160)]
    for i, row in enumerate(rows):
        value = fnum(row.get(value_key))
        bar_h = int((value / max_v) * (plot_h - 8))
        x0 = margin_l + i * slot + 4
        x1 = margin_l + (i + 1) * slot - 4
        y1 = height - margin_b
        y0 = y1 - bar_h
        rect(x0, y0, x1, y1, palette[i % len(palette)])
    write_png(path, width, height, rgb)


def write_metadata(root: Path) -> None:
    text = [
        "# Experiment Metadata",
        "",
        f"platform: {platform.platform()}",
        f"python: {platform.python_version()}",
        f"processor: {platform.processor() or 'unavailable'}",
        f"machine: {platform.machine()}",
        f"compiler: {command_output(['c++', '--version']).splitlines()[0]}",
        f"cmake: {command_output(['cmake', '--version']).splitlines()[0]}",
        f"git_head: {command_output(['git', 'rev-parse', 'HEAD'])}",
        "",
    ]
    (root / "metadata.md").write_text("\n".join(text), encoding="utf-8")


def make_plots(root: Path, candidates: list[dict[str, object]], stages: list[dict[str, object]]) -> str:
    fig_dir = root / "figures"
    fig_dir.mkdir(parents=True, exist_ok=True)
    try:
        import matplotlib.pyplot as plt
    except Exception as exc:
        cand_agg = aggregate(candidates, ["scheme", "image_size"], "MBps")
        stage_agg = aggregate(stages, ["category", "stage"], "MBps")
        fallback_bar_png(fig_dir / "candidate_throughput.png",
                         sorted(cand_agg, key=lambda r: fnum(r["MBps_mean"]), reverse=True),
                         "MBps_mean")
        fallback_bar_png(fig_dir / "stage_throughput.png",
                         sorted(stage_agg, key=lambda r: fnum(r["MBps_mean"]), reverse=True),
                         "MBps_mean")
        return f"Matplotlib unavailable; generated dependency-free PNG bar charts instead ({exc})."

    cand_agg = aggregate(candidates, ["scheme", "image_size"], "MBps")
    if cand_agg:
        top = sorted(cand_agg, key=lambda r: fnum(r["MBps_mean"]), reverse=True)[:16]
        plt.figure(figsize=(11, 5))
        plt.bar([f"{r['scheme']}\n{r['image_size']}" for r in top], [fnum(r["MBps_mean"]) for r in top])
        plt.xticks(rotation=45, ha="right", fontsize=8)
        plt.ylabel("MB/s")
        plt.title("Candidate Pipeline Throughput")
        plt.tight_layout()
        plt.savefig(fig_dir / "candidate_throughput.png", dpi=160)
        plt.close()

    stage_agg = aggregate(stages, ["category", "stage"], "MBps")
    if stage_agg:
        top = sorted(stage_agg, key=lambda r: fnum(r["MBps_mean"]), reverse=True)[:20]
        plt.figure(figsize=(11, 5))
        plt.bar([f"{r['category']}:{r['stage']}" for r in top], [fnum(r["MBps_mean"]) for r in top])
        plt.xticks(rotation=45, ha="right", fontsize=8)
        plt.ylabel("MB/s")
        plt.title("Replaceable Stage Throughput")
        plt.tight_layout()
        plt.savefig(fig_dir / "stage_throughput.png", dpi=160)
        plt.close()
    return "Generated paper plots in results/final/figures."


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--root", default="results/final")
    args = p.parse_args()
    root = Path(args.root)

    bench = load_kind(root, "bench.csv")
    analysis = load_kind(root, "analysis.csv")
    stages = load_kind(root, "stage_bench.csv")
    candidates = load_kind(root, "candidate_schemes.csv")

    write_csv(root / "bench.csv", bench, sorted({k for r in bench for k in r}))
    write_csv(root / "analysis.csv", analysis, sorted({k for r in analysis for k in r}))
    write_csv(root / "stage_bench.csv", stages, sorted({k for r in stages for k in r}))
    write_csv(root / "candidate_schemes.csv", candidates, sorted({k for r in candidates for k in r}))

    bench_agg = aggregate(bench, ["cipher", "variant", "image_size"], "MBps")
    stage_agg = aggregate(stages, ["category", "stage", "image_size"], "MBps")
    cand_agg = aggregate(candidates, ["scheme", "keystream", "permutation", "diffusion", "image_size"], "MBps")
    write_csv(root / "bench_stats.csv", bench_agg, sorted({k for r in bench_agg for k in r}))
    write_csv(root / "stage_stats.csv", stage_agg, sorted({k for r in stage_agg for k in r}))
    write_csv(root / "candidate_stats.csv", cand_agg, sorted({k for r in cand_agg for k in r}))

    plot_note = make_plots(root, candidates, stages)
    write_metadata(root)

    top_candidates = best_by(candidates, "scheme", "MBps")
    top_stages = best_by(stages, "stage", "MBps", 12)
    top_full = best_by(bench, "cipher", "MBps", 12)

    tables = [
        "# Paper Tables",
        "",
        "## Fastest Full Schemes",
        markdown_table(top_full, ["cipher", "image_size", "MBps", "keygen_ms", "permutation_ms", "diffusion_ms", "total_ms"]),
        "",
        "## Fastest Replaceable Stages",
        markdown_table(top_stages, ["category", "stage", "image_size", "MBps", "ms"]),
        "",
        "## Fastest Candidate Pipelines",
        markdown_table(top_candidates, ["scheme", "image_size", "MBps", "keygen_ms", "permutation_ms", "diffusion_ms", "total_ms"]),
        "",
    ]
    (root / "tables.md").write_text("\n".join(tables), encoding="utf-8")

    summary = [
        "# Q2/Q3 Applied Performance Summary",
        "",
        "This experiment package is positioned as an applied performance study, not as a claim that chaotic schemes beat AES or ChaCha.",
        "",
        "## Main Empirical Story",
        "",
        "- Traditional chaotic schemes are dominated by implementation-hostile stages: floating-point keystreams, sort-based permutation, and serial diffusion chains.",
        "- SIMD-friendly redesigns move the useful contribution to replaceable stages: cellular automata keystreams, checkerboard/block/Feistel-style permutations, and lane-local/tree/prefix diffusion.",
        "- AES-CTR and ChaCha20 remain the correct cryptographic baselines; the redesigned chaotic candidates are research prototypes for image-domain transformation tradeoffs.",
        "",
        "## Best Full Schemes",
        markdown_table(top_full[:8], ["cipher", "image_size", "MBps", "total_ms"]),
        "",
        "## Best Stage Primitives",
        markdown_table(top_stages[:10], ["category", "stage", "image_size", "MBps", "ms"]),
        "",
        "## Best Redesigned Candidates",
        markdown_table(top_candidates[:8], ["scheme", "image_size", "MBps", "keygen_ms", "permutation_ms", "diffusion_ms"]),
        "",
        "## Security Caveats",
        "",
        "- Entropy, histogram, NPCR, UACI, and correlation are image-statistical diagnostics, not cryptographic proofs.",
        "- Deterministic stream-XOR variants remain weak under key/nonce reuse and should be reported as negative controls.",
        "- The current `chaotic_seed_mix_xor` label intentionally avoids claiming official BLAKE3.",
        "- A Q2/Q3-ready manuscript should present the proposed candidates as SIMD-native redesigns with measured tradeoffs, not as standardized secure ciphers.",
        "",
        f"## Plot Status\n\n{plot_note}",
        "",
    ]
    (root / "performance_summary.md").write_text("\n".join(summary), encoding="utf-8")
    Path("summary.out").write_text("\n".join(summary), encoding="utf-8")

    paper = [
        "# Stage-Level SIMD Redesign and Benchmarking of Chaotic Image Encryption Pipelines",
        "",
        "## Abstract",
        "",
        "Chaotic image encryption papers frequently report image-domain statistical metrics while giving limited attention to implementation bottlenecks. This work decomposes representative chaotic image encryption pipelines into keystream generation, permutation, and diffusion stages, then benchmarks both traditional and SIMD-native alternatives. The results show that sort-based permutation, floating-point scalar maps, and global feedback diffusion are the main performance barriers. Cellular automata, coupled-map lattices, affine/checkerboard/block permutations, and prefix/tree/lane-local diffusion reduce this gap while retaining useful image-domain statistical behavior. AES-CTR and ChaCha20 are included as cryptographic baselines and remain the appropriate reference for general-purpose confidentiality.",
        "",
        "## Contribution",
        "",
        "1. A C++17/OpenCV/OpenSSL benchmark suite with common cipher interfaces and stage-level timing.",
        "2. A reproducible matrix over image types, image sizes, repetitions, legacy chaotic schemes, redesigned candidates, and AES/ChaCha baselines.",
        "3. An applied analysis showing which chaotic-image stages are implementation-hostile and which replacements are SIMD-friendly.",
        "4. An explicit security discussion separating image-statistical metrics from cryptographic security claims.",
        "",
        "## Results",
        "",
        "See `results/final/tables.md`, `results/final/performance_summary.md`, and `results/final/figures/`.",
        "",
        "## Limitations",
        "",
        "The proposed candidates are research prototypes. They require deeper cryptanalysis before any security deployment. The strongest defensible claim is performance-oriented stage redesign, not superiority to AES or ChaCha.",
        "",
    ]
    Path("paper_draft.md").write_text("\n".join(paper), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
