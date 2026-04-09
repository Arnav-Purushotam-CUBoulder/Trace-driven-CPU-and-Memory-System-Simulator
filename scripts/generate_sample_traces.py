#!/usr/bin/env python3
"""Generate deterministic sample traces for the simulator."""

from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
TRACES_DIR = ROOT / "traces"


def write_trace(path: Path, lines: list[str]) -> None:
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def streaming_trace() -> list[str]:
    lines: list[str] = [
        "# Streaming workload with strong spatial locality and a predictable loop branch."
    ]
    base_pc = 0x400000
    base_addr = 0x100000
    body = [
        ("ALU", base_pc + 0x00, None),
        ("LOAD", base_pc + 0x04, 0),
        ("LOAD", base_pc + 0x08, 64),
        ("ALU", base_pc + 0x0C, None),
        ("STORE", base_pc + 0x10, 128),
        ("LOAD", base_pc + 0x14, 192),
        ("ALU", base_pc + 0x18, None),
        ("BRANCH", base_pc + 0x1C, None),
    ]

    for iteration in range(96):
        iteration_base = base_addr + iteration * 256
        for opcode, pc, offset in body:
            if opcode == "ALU":
                lines.append(f"ALU 0x{pc:x}")
            elif opcode == "LOAD":
                lines.append(f"LOAD 0x{pc:x} 0x{iteration_base + offset:x}")
            elif opcode == "STORE":
                lines.append(f"STORE 0x{pc:x} 0x{iteration_base + offset:x}")
            else:
                taken = "T" if iteration < 95 else "N"
                lines.append(f"BRANCH 0x{pc:x} {taken}")
    return lines


def conflict_trace() -> list[str]:
    lines: list[str] = [
        "# Cache-conflict workload that repeatedly targets the same default L1 set."
    ]
    base_pc = 0x500000
    conflict_addresses = [
        0x200000,
        0x204000,
        0x208000,
        0x20C000,
        0x210000,
        0x214000,
    ]
    body_pcs = [
        base_pc + 0x00,
        base_pc + 0x04,
        base_pc + 0x08,
        base_pc + 0x0C,
        base_pc + 0x10,
        base_pc + 0x14,
    ]

    for iteration in range(80):
        for pc, address in zip(body_pcs, conflict_addresses, strict=True):
            lines.append(f"LOAD 0x{pc:x} 0x{address + ((iteration % 4) * 64):x}")
        lines.append(f"ALU 0x{base_pc + 0x18:x}")
        taken = "T" if iteration < 79 else "N"
        lines.append(f"BRANCH 0x{base_pc + 0x1C:x} {taken}")
    return lines


def branchy_trace() -> list[str]:
    lines: list[str] = [
        "# Branch-heavy workload with alternating and periodic control-flow patterns."
    ]
    base_pc = 0x600000
    base_addr = 0x300000

    for iteration in range(160):
        lines.append(f"ALU 0x{base_pc + 0x00:x}")
        first_outcome = "T" if iteration % 2 == 0 else "N"
        second_outcome = "T" if iteration % 4 != 3 else "N"
        lines.append(f"BRANCH 0x{base_pc + 0x04:x} {first_outcome}")
        lines.append(f"LOAD 0x{base_pc + 0x08:x} 0x{base_addr + ((iteration * 32) % 2048):x}")
        lines.append(f"ALU 0x{base_pc + 0x0C:x}")
        lines.append(f"BRANCH 0x{base_pc + 0x10:x} {second_outcome}")
        lines.append(f"STORE 0x{base_pc + 0x14:x} 0x{base_addr + 4096 + ((iteration * 64) % 2048):x}")
    return lines


def mixed_trace() -> list[str]:
    lines = ["# Mixed workload with locality, conflict misses, and branch pressure."]
    lines.extend(streaming_trace()[1:257])
    lines.extend(conflict_trace()[1:257])
    lines.extend(branchy_trace()[1:257])
    return lines


def main() -> None:
    TRACES_DIR.mkdir(parents=True, exist_ok=True)
    write_trace(TRACES_DIR / "streaming.trace", streaming_trace())
    write_trace(TRACES_DIR / "conflict.trace", conflict_trace())
    write_trace(TRACES_DIR / "branchy.trace", branchy_trace())
    write_trace(TRACES_DIR / "mixed.trace", mixed_trace())


if __name__ == "__main__":
    main()
