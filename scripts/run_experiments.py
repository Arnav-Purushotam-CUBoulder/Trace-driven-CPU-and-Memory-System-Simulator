#!/usr/bin/env python3
"""Run a small parameter sweep and export simulator results."""

from __future__ import annotations

import argparse
import csv
import json
import statistics
import subprocess
import sys
from collections import defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--binary",
        type=Path,
        default=ROOT / "build" / "td_sim",
        help="Path to the compiled simulator binary",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=ROOT / "results",
        help="Directory for CSV summaries and optional figures",
    )
    parser.add_argument(
        "--plot",
        action="store_true",
        help="Generate plots when matplotlib is available",
    )
    parser.add_argument(
        "--traces",
        nargs="*",
        type=Path,
        help="Explicit trace paths. Defaults to every *.trace in traces/",
    )
    return parser.parse_args()


def run_sim(binary: Path, trace: Path, config: dict[str, int | str]) -> dict[str, object]:
    command = [
        str(binary),
        "--trace",
        str(trace),
        "--width",
        str(config["width"]),
        "--rob",
        str(config["rob_size"]),
        "--cache-size",
        str(config["cache_size_bytes"]),
        "--cache-line-size",
        str(config["cache_line_size"]),
        "--cache-assoc",
        str(config["cache_associativity"]),
        "--cache-hit-latency",
        str(config["cache_hit_latency"]),
        "--cache-miss-penalty",
        str(config["cache_miss_penalty"]),
        "--predictor",
        str(config["predictor"]),
        "--bp-entries",
        str(config["branch_predictor_entries"]),
        "--mispredict-penalty",
        str(config["branch_mispredict_penalty"]),
        "--json",
    ]
    completed = subprocess.run(
        command,
        check=True,
        capture_output=True,
        text=True,
    )
    return json.loads(completed.stdout)


def write_csv(output_path: Path, rows: list[dict[str, object]]) -> None:
    if not rows:
        return
    with output_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def write_summary(output_path: Path, rows: list[dict[str, object]]) -> None:
    grouped: dict[str, list[dict[str, object]]] = defaultdict(list)
    for row in rows:
        grouped[str(row["trace"])].append(row)

    lines = ["# Experiment Summary", ""]
    for trace_name, trace_rows in sorted(grouped.items()):
        best_ipc = max(trace_rows, key=lambda row: float(row["ipc"]))
        best_accuracy = max(trace_rows, key=lambda row: float(row["branch_accuracy"]))
        best_miss = min(trace_rows, key=lambda row: float(row["cache_miss_rate"]))
        lines.append(f"## {trace_name}")
        lines.append(
            f"- Best IPC: {float(best_ipc['ipc']):.3f} "
            f"(width={best_ipc['width']}, rob={best_ipc['rob_size']}, "
            f"assoc={best_ipc['cache_associativity']}, predictor={best_ipc['predictor']})"
        )
        lines.append(
            f"- Best Branch Accuracy: {float(best_accuracy['branch_accuracy']) * 100.0:.2f}% "
            f"(predictor={best_accuracy['predictor']})"
        )
        lines.append(
            f"- Lowest Miss Rate: {float(best_miss['cache_miss_rate']) * 100.0:.2f}% "
            f"(assoc={best_miss['cache_associativity']}, width={best_miss['width']})"
        )
        lines.append("")

    averages = defaultdict(list)
    for row in rows:
        averages["ipc"].append(float(row["ipc"]))
        averages["cache_miss_rate"].append(float(row["cache_miss_rate"]))
        averages["branch_accuracy"].append(float(row["branch_accuracy"]))

    lines.extend(
        [
            "## Overall Means",
            "",
            f"- Mean IPC: {statistics.mean(averages['ipc']):.3f}",
            f"- Mean Cache Miss Rate: {statistics.mean(averages['cache_miss_rate']) * 100.0:.2f}%",
            f"- Mean Branch Accuracy: {statistics.mean(averages['branch_accuracy']) * 100.0:.2f}%",
            "",
        ]
    )

    output_path.write_text("\n".join(lines), encoding="utf-8")


def maybe_plot(output_dir: Path, rows: list[dict[str, object]]) -> None:
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed; skipping plots", file=sys.stderr)
        return

    filtered = [
        row
        for row in rows
        if int(row["rob_size"]) == 64
        and int(row["cache_associativity"]) == 4
        and str(row["predictor"]) == "two_bit"
    ]

    traces = sorted({str(row["trace"]) for row in filtered})
    widths = sorted({int(row["width"]) for row in filtered})

    plt.figure(figsize=(8, 5))
    for trace_name in traces:
        ipc_values = [
            float(
                next(
                    row["ipc"]
                    for row in filtered
                    if str(row["trace"]) == trace_name and int(row["width"]) == width
                )
            )
            for width in widths
        ]
        plt.plot(widths, ipc_values, marker="o", label=trace_name.replace(".trace", ""))
    plt.xlabel("Pipeline Width")
    plt.ylabel("IPC")
    plt.title("IPC Scaling with Width (ROB=64, Assoc=4, Predictor=two_bit)")
    plt.xticks(widths)
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig(output_dir / "ipc_by_width.png", dpi=150)
    plt.close()

    plt.figure(figsize=(8, 5))
    predictors = ["always_not_taken", "one_bit", "two_bit"]
    accuracy_rows = [
        row
        for row in rows
        if int(row["width"]) == 4 and int(row["rob_size"]) == 64 and int(row["cache_associativity"]) == 4
    ]
    means = []
    for predictor in predictors:
        scores = [
            float(row["branch_accuracy"])
            for row in accuracy_rows
            if str(row["predictor"]) == predictor
        ]
        means.append(statistics.mean(scores) * 100.0)
    plt.bar(predictors, means, color=["#c8553d", "#588157", "#3a86ff"])
    plt.ylabel("Branch Accuracy (%)")
    plt.title("Branch Predictor Comparison")
    plt.tight_layout()
    plt.savefig(output_dir / "branch_accuracy_by_predictor.png", dpi=150)
    plt.close()


def main() -> int:
    args = parse_args()
    binary = args.binary.resolve()
    if not binary.exists():
        raise SystemExit(f"Simulator binary not found: {binary}")

    traces = args.traces or sorted((ROOT / "traces").glob("*.trace"))
    if not traces:
        raise SystemExit("No traces found. Run scripts/generate_sample_traces.py first.")

    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    sweep_rows: list[dict[str, object]] = []
    widths = [1, 2, 4]
    rob_sizes = [16, 32, 64]
    associativities = [1, 4]
    predictors = ["always_not_taken", "one_bit", "two_bit"]

    for trace in traces:
        for width in widths:
            for rob_size in rob_sizes:
                for associativity in associativities:
                    for predictor in predictors:
                        config = {
                            "width": width,
                            "rob_size": rob_size,
                            "cache_size_bytes": 16 * 1024,
                            "cache_line_size": 64,
                            "cache_associativity": associativity,
                            "cache_hit_latency": 1,
                            "cache_miss_penalty": 40,
                            "predictor": predictor,
                            "branch_predictor_entries": 1024,
                            "branch_mispredict_penalty": 6,
                        }
                        sweep_rows.append(run_sim(binary, trace, config))

    write_csv(output_dir / "experiments.csv", sweep_rows)
    write_summary(output_dir / "summary.md", sweep_rows)

    if args.plot:
        maybe_plot(output_dir, sweep_rows)

    print(f"Wrote {len(sweep_rows)} experiment rows to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
