#!/usr/bin/env python3
"""Generate publication-quality plots from the final benchmark CSV files."""

from __future__ import annotations

import argparse
import os
from pathlib import Path

os.environ.setdefault("MPLCONFIGDIR", "/tmp/chaotic-image-encryption-matplotlib")

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


COLORS = {
    "blue": "#0072B2",
    "orange": "#E69F00",
    "green": "#009E73",
    "red": "#D55E00",
    "purple": "#CC79A7",
    "sky": "#56B4E9",
    "yellow": "#F0E442",
    "gray": "#666666",
}

SCHEME_NAMES = {
    "aes_ctr": "AES-256-CTR",
    "chacha20": "ChaCha20",
    "chaotic_seed_blake3_xor": "BLAKE3-XOR",
    "tent_block_xor": "Tent-XOR",
    "logistic_xor": "Logistic-XOR",
    "coupled_lattice_xor": "CML-XOR",
    "logistic_permute_xor": "Logistic-sort-XOR",
    "arnold_xor": "Arnold-XOR",
    "tiled_arnold_xor": "Tiled-Arnold-XOR",
    "sine_xor": "Sine-XOR",
    "hamiltonian_lattice_xor": "Hamiltonian-XOR",
}

STAGE_NAMES = {
    "logistic_double": "Logistic double",
    "sine_map": "Sine map",
    "fixed_point_tent": "Fixed-point tent",
    "cml": "CML",
    "cellular_automata": "Cellular automata",
    "hamiltonian": "Hamiltonian",
    "reaction_diffusion": "Reaction-diffusion",
    "chaotic_sort": "Chaotic sort",
    "random_walk": "Random walk",
    "arnold_cat_map": "Arnold/cat map",
    "affine_modular": "Affine modular",
    "feistel_index": "Feistel index",
    "block_permutation": "Block permutation",
    "checkerboard_swaps": "Checkerboard swaps",
    "global_chain": "Global chain",
    "block_local_chain": "Block-local chain",
    "parallel_prefix": "Parallel prefix",
    "stencil_diffusion": "Stencil",
    "arx_block_diffusion": "ARX block",
    "bitplane_diffusion": "Bit-plane",
    "prefix_xor_avx2": "Prefix XOR AVX2",
    "multilane_chain_avx2": "Multi-lane AVX2",
    "tree_xor_avx2": "Tree XOR AVX2",
    "prefix_tree_reverse": "Prefix-tree-reverse",
    "reverse_prefix_xor_avx2": "Reverse prefix AVX2",
    "arx_prefix_mod256": "ARX prefix mod 256",
}


def configure_style() -> None:
    sns.set_theme(style="whitegrid", context="paper")
    mpl.rcParams.update(
        {
            "font.family": "serif",
            "font.serif": ["DejaVu Serif"],
            "font.size": 8,
            "axes.titlesize": 9,
            "axes.labelsize": 8,
            "legend.fontsize": 7,
            "xtick.labelsize": 7,
            "ytick.labelsize": 7,
            "figure.dpi": 150,
            "savefig.dpi": 300,
            "savefig.bbox": "tight",
            "pdf.fonttype": 42,
            "ps.fonttype": 42,
        }
    )


def save_figure(fig: plt.Figure, output_dir: Path, stem: str) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    for suffix in ("pdf", "png"):
        fig.savefig(output_dir / f"{stem}.{suffix}", format=suffix)
    plt.close(fig)


def plot_full_scheme_throughput(bench: pd.DataFrame, output_dir: Path) -> None:
    selected = [
        "chacha20",
        "aes_ctr",
        "chaotic_seed_blake3_xor",
        "tent_block_xor",
        "logistic_xor",
        "arnold_xor",
        "logistic_permute_xor",
    ]
    data = bench[(bench["image_size"] == "768x512") & bench["cipher"].isin(selected)].copy()
    data["scheme"] = data["cipher"].map(SCHEME_NAMES)
    data["order"] = data["cipher"].map({name: i for i, name in enumerate(selected)})
    data = data.sort_values("order", ascending=False)

    fig, ax = plt.subplots(figsize=(7.1, 3.0))
    bars = ax.barh(
        data["scheme"],
        data["MBps_mean"],
        xerr=data["MBps_ci95"],
        color=[COLORS["blue"], COLORS["sky"], COLORS["green"], COLORS["orange"],
               COLORS["purple"], COLORS["gray"], COLORS["red"]][::-1],
        capsize=2,
        error_kw={"elinewidth": 0.8, "capthick": 0.8},
    )
    ax.bar_label(bars, fmt="%.0f", padding=3, fontsize=7)
    ax.set_xlabel("Mean throughput (MiB/s), 95% CI")
    ax.set_title("Full-scheme throughput on the 24-image Kodak dataset (768x512)")
    ax.set_xlim(0, data["MBps_mean"].max() * 1.13)
    ax.grid(axis="y", visible=False)
    fig.tight_layout()
    save_figure(fig, output_dir, "full_scheme_throughput")


def plot_stage_throughput(stages: pd.DataFrame, output_dir: Path) -> None:
    selected = {
        "keystream": [
            "sine_map", "reaction_diffusion", "cml", "fixed_point_tent",
            "logistic_double", "cellular_automata",
        ],
        "permutation": [
            "chaotic_sort", "random_walk", "feistel_index", "arnold_cat_map",
            "affine_modular", "checkerboard_swaps", "block_permutation",
        ],
        "diffusion": [
            "global_chain", "parallel_prefix", "prefix_xor_avx2",
            "arx_block_diffusion", "tree_xor_avx2", "multilane_chain_avx2",
        ],
    }
    palette = [COLORS["red"], COLORS["orange"], COLORS["gray"], COLORS["purple"],
               COLORS["sky"], COLORS["green"], COLORS["blue"]]
    fig, axes = plt.subplots(1, 3, figsize=(7.2, 3.55))
    for ax, (category, names) in zip(axes, selected.items()):
        data = stages[
            (stages["image_size"] == "1024x1024")
            & (stages["category"] == category)
            & stages["stage"].isin(names)
        ].copy()
        data["label"] = data["stage"].map(STAGE_NAMES)
        data = data.sort_values("MBps_mean")
        bars = ax.barh(
            data["label"],
            data["MBps_mean"],
            xerr=data["MBps_ci95"],
            color=palette[-len(data):],
            capsize=1.5,
            error_kw={"elinewidth": 0.7, "capthick": 0.7},
        )
        ax.bar_label(bars, fmt="%.0f", padding=2, fontsize=6)
        ax.set_title(category.capitalize())
        ax.set_xlabel("Mean MiB/s")
        ax.set_xlim(0, data["MBps_mean"].max() * 1.18)
        ax.grid(axis="y", visible=False)
    fig.suptitle("Replaceable-stage throughput at 1024x1024 (95% CI)", y=1.01)
    fig.tight_layout()
    save_figure(fig, output_dir, "stage_throughput_1024")


def plot_diffusion_speedup(stages: pd.DataFrame, output_dir: Path) -> None:
    selected = [
        "global_chain",
        "arx_block_diffusion",
        "prefix_xor_avx2",
        "tree_xor_avx2",
        "multilane_chain_avx2",
    ]
    square_sizes = ["512x512", "1024x1024", "2048x2048", "4096x4096"]
    data = stages[
        (stages["category"] == "diffusion")
        & stages["stage"].isin(selected)
        & stages["image_size"].isin(square_sizes)
    ].copy()
    data["pixels"] = data["image_size"].str.split("x").str[0].astype(int)

    fig, ax = plt.subplots(figsize=(7.1, 3.2))
    colors = [COLORS["gray"], COLORS["orange"], COLORS["purple"], COLORS["sky"], COLORS["blue"]]
    markers = ["o", "s", "^", "D", "P"]
    for stage, color, marker in zip(selected, colors, markers):
        part = data[data["stage"] == stage].sort_values("pixels")
        ax.errorbar(
            part["pixels"],
            part["MBps_mean_speedup_vs_baseline"],
            marker=marker,
            linewidth=1.4,
            markersize=4,
            color=color,
            label=STAGE_NAMES[stage],
        )
    ax.axhline(1.0, color="#333333", linewidth=0.8, linestyle="--")
    ax.set_xscale("log", base=2)
    ax.set_xticks([512, 1024, 2048, 4096], ["512", "1024", "2048", "4096"])
    ax.set_xlabel("Square image dimension")
    ax.set_ylabel("Speedup over global chain")
    ax.set_title("Diffusion speedup decreases at 4K but remains positive")
    ax.legend(ncol=3, frameon=False, loc="upper center")
    fig.tight_layout()
    save_figure(fig, output_dir, "diffusion_speedup")


def plot_candidate_scaling(candidates: pd.DataFrame, output_dir: Path) -> None:
    square_sizes = ["512x512", "1024x1024", "2048x2048", "4096x4096"]
    data = candidates[candidates["image_size"].isin(square_sizes)].copy()
    data["pixels"] = data["image_size"].str.split("x").str[0].astype(int)

    fig, ax = plt.subplots(figsize=(7.1, 3.6))
    palette = sns.color_palette("colorblind", n_colors=data["scheme"].nunique())
    markers = ["o", "s", "^", "D", "P", "X", "v"]
    for (scheme, part), color, marker in zip(data.groupby("scheme"), palette, markers):
        part = part.sort_values("pixels")
        ax.errorbar(
            part["pixels"],
            part["MBps_mean"],
            yerr=part["MBps_ci95"],
            marker=marker,
            markersize=3.8,
            linewidth=1.2,
            capsize=2,
            color=color,
            label=scheme,
        )
    ax.set_xscale("log", base=2)
    ax.set_xticks([512, 1024, 2048, 4096], ["512", "1024", "2048", "4096"])
    ax.set_xlabel("Square image dimension")
    ax.set_ylabel("Mean throughput (MiB/s)")
    ax.set_title("Candidate-pipeline scaling (95% CI)", pad=42)
    ax.legend(
        ncol=2,
        frameon=False,
        loc="upper center",
        bbox_to_anchor=(0.5, 1.13),
    )
    fig.tight_layout()
    save_figure(fig, output_dir, "candidate_scaling")


def plot_candidate_stage_shares(candidate_raw: pd.DataFrame, output_dir: Path) -> None:
    columns = ["keygen_share_pct", "permutation_share_pct", "diffusion_share_pct"]
    data = candidate_raw.groupby("scheme", as_index=True)[columns].mean()
    data = data.sort_values("keygen_share_pct")

    fig, ax = plt.subplots(figsize=(7.1, 3.4))
    left = np.zeros(len(data))
    labels = ["Keystream", "Permutation", "Diffusion"]
    colors = [COLORS["blue"], COLORS["orange"], COLORS["green"]]
    for column, label, color in zip(columns, labels, colors):
        ax.barh(data.index, data[column], left=left, label=label, color=color)
        left += data[column].to_numpy()
    ax.set_xlim(0, 100)
    ax.set_xlabel("Mean share of candidate runtime (%)")
    ax.legend(ncol=3, frameon=False, loc="lower center", bbox_to_anchor=(0.5, 1.0))
    ax.grid(axis="y", visible=False)
    fig.tight_layout()
    save_figure(fig, output_dir, "candidate_stage_shares")


def plot_security_diagnostics(analysis: pd.DataFrame, output_dir: Path) -> None:
    data = analysis.copy()
    data["abs_corr"] = data[["corr_h", "corr_v", "corr_d"]].abs().mean(axis=1)
    means = data.groupby("cipher", as_index=False).agg(
        entropy=("entropy", "mean"),
        abs_corr=("abs_corr", "mean"),
        key_sensitivity=("key_sensitivity", "mean"),
        kpa_score=("kpa_score", "mean"),
    )
    selected = [
        "chacha20",
        "aes_ctr",
        "chaotic_seed_blake3_xor",
        "tent_block_xor",
        "logistic_xor",
        "coupled_lattice_xor",
    ]
    means = means[means["cipher"].isin(selected)].copy()
    means["scheme"] = means["cipher"].map(SCHEME_NAMES)
    means["order"] = means["cipher"].map({name: i for i, name in enumerate(selected)})
    means = means.sort_values("order")

    fig, axes = plt.subplots(2, 2, figsize=(7.1, 5.0))
    metrics = [
        ("entropy", "Entropy (bits)", (7.95, 8.005)),
        ("abs_corr", "Mean absolute adjacent correlation", None),
        ("key_sensitivity", "Key sensitivity (%)", (98.8, 100.0)),
        ("kpa_score", "Reused-keystream KPA recovery score", (-0.02, 1.05)),
    ]
    colors = [COLORS["blue"], COLORS["sky"], COLORS["green"], COLORS["orange"],
              COLORS["purple"], COLORS["red"]]
    for ax, (column, title, limits) in zip(axes.flat, metrics):
        ax.bar(means["scheme"], means[column], color=colors)
        ax.set_title(title)
        if limits:
            ax.set_ylim(*limits)
        ax.tick_params(axis="x", labelrotation=35)
        ax.grid(axis="x", visible=False)
    fig.suptitle("Image statistics do not imply resistance to reused-keystream attacks", y=1.01)
    fig.tight_layout()
    save_figure(fig, output_dir, "security_diagnostics")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--results", type=Path, default=Path("results/final"))
    parser.add_argument("--output", type=Path, default=Path("paper/access/figures"))
    args = parser.parse_args()

    configure_style()
    bench = pd.read_csv(args.results / "bench_stats.csv")
    stages = pd.read_csv(args.results / "stage_stats.csv")
    candidates = pd.read_csv(args.results / "candidate_stats.csv")
    candidate_raw = pd.read_csv(args.results / "candidate_schemes.csv")
    analysis = pd.read_csv(args.results / "analysis.csv")

    plot_full_scheme_throughput(bench, args.output)
    plot_stage_throughput(stages, args.output)
    plot_diffusion_speedup(stages, args.output)
    plot_candidate_scaling(candidates, args.output)
    plot_candidate_stage_shares(candidate_raw, args.output)
    plot_security_diagnostics(analysis, args.output)
    print(f"Generated 6 PDF and 6 PNG figures in {args.output}")


if __name__ == "__main__":
    main()
