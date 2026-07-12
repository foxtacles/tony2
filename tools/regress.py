#!/usr/bin/env python3
"""Per-function regression gate for the rename/refactor effort.

Usage: python3 tools/regress.py build/rename_baseline.json build/current.json
Fails (exit 1) if any function's match ratio dropped below the baseline,
or if any baseline function disappeared from the current report.
"""
import json
import sys


def load(path):
    with open(path) as f:
        data = json.load(f)["data"]
    return {e["address"]: e for e in data if e.get("matching") is not None}


def main():
    base = load(sys.argv[1])
    cur = load(sys.argv[2])
    bad = []
    for addr, e in base.items():
        c = cur.get(addr)
        if c is None:
            bad.append((addr, e["matching"], None, e.get("name", "")))
        elif c["matching"] < e["matching"] - 1e-9:
            bad.append((addr, e["matching"], c["matching"], c.get("name", "")))
    if bad:
        print(f"REGRESSION: {len(bad)} function(s) dropped:")
        for addr, was, now, name in bad:
            nows = "MISSING" if now is None else f"{now:.4f}"
            print(f"  {addr}  {was:.4f} -> {nows}  {name}")
        sys.exit(1)
    n100b = sum(1 for e in base.values() if e["matching"] >= 1.0)
    n100c = sum(1 for e in cur.values() if e["matching"] >= 1.0)
    print(f"OK: no regressions ({len(base)} baseline funcs; 100% count {n100b} -> {n100c})")


if __name__ == "__main__":
    main()
