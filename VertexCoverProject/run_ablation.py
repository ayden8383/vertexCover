#!/usr/bin/env python3
"""Run one-factor-at-a-time ablations against a CLI adapter for the EXISTING exact solver.

Expected CLI, with one JSON object on final stdout line:
  ablation_cli --n 40 --p 0.2 --seed 0 [--disable-reductions | --disable-bounds | --disable-greedy]
  {"n":40,"m":142,"cover_size":...,"optimal":true,"nodes_explored":...,"runtime_ms":...}

Use the same C++ random graph generator as the original benchmark. Results cannot
be interpreted until the teammate's completed source is wired into the adapter.
"""
import argparse
import csv
import json
import math
import statistics
import subprocess
import sys
import time
from pathlib import Path

MODES = {
    "full": [],
    "no_reductions": ["--disable-reductions"],
    "no_bounds": ["--disable-bounds"],
    "no_greedy": ["--disable-greedy"],
}
FIELDS = ("family", "instance", "n", "m", "p", "seed", "mode", "trial",
          "status", "cover_size", "expected_optimum", "matches_optimum",
          "nodes_explored", "runtime_ms", "wall_ms", "error")


def benchmark_case(solver, base, mode, trial, timeout):
    cmd = [str(solver), "--n", base["n"], "--p", base["p"], "--seed", base["seed"]] + MODES[mode]
    row = {col: base[col] for col in ("family", "instance", "n", "m", "p", "seed")}
    row.update(mode=mode, trial=trial, status="error", cover_size="",
               expected_optimum=base["cover_size"], matches_optimum="",
               nodes_explored="", runtime_ms="", wall_ms="", error="")
    start = time.perf_counter()
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        row["wall_ms"] = round(1000*(time.perf_counter()-start), 3)
        if proc.returncode != 0:
            raise RuntimeError(f"exit {proc.returncode}: {proc.stderr.strip()[-350:]}")
        lines = [s.strip() for s in proc.stdout.splitlines() if s.strip()]
        if not lines:
            raise ValueError("no JSON output")
        data = json.loads(lines[-1])
        if int(data["m"]) != int(base["m"]):
            raise ValueError(f"graph edges differ: solver m={data['m']}, benchmark m={base['m']}; check generator")
        if int(data.get("n", base["n"])) != int(base["n"]):
            raise ValueError("vertex count differs")
        row["cover_size"] = int(data["cover_size"])
        row["nodes_explored"] = int(data["nodes_explored"])
        row["runtime_ms"] = float(data["runtime_ms"])
        row["matches_optimum"] = (row["cover_size"] == int(base["cover_size"]))
        if data.get("optimal") is not True:
            row["status"] = "unproven"
            row["error"] = "solver did not claim optimality"
        elif not row["matches_optimum"]:
            row["status"] = "mismatch"
            row["error"] = "cover size disagrees with the original exact benchmark"
        else:
            row["status"] = "ok"
    except subprocess.TimeoutExpired:
        row["status"] = "timeout"
        row["wall_ms"] = round(1000*(time.perf_counter()-start), 3)
        row["error"] = f"exceeded {timeout}s per execution"
    except (OSError, ValueError, RuntimeError, KeyError, TypeError) as exc:
        row["error"] = str(exc)
    return row


def main():
    ap = argparse.ArgumentParser(description="Compare full Branch-and-Bound against three ablations")
    ap.add_argument("--solver", required=True, help="compiled CLI adapter (e.g. build/ablation_cli.exe)")
    ap.add_argument("--baseline", default="exact_benchmark.csv")
    ap.add_argument("--out", default="ablation_results.csv")
    ap.add_argument("--repeats", type=int, default=1, help="one deterministic node-count run is enough initially")
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--seeds", default="0,1,2")
    ap.add_argument("--sizes", default="40,60,80")
    ap.add_argument("--probabilities", default="0.2,0.5")
    args = ap.parse_args()
    seeds = {int(x) for x in args.seeds.split(",")}
    sizes = {int(x) for x in args.sizes.split(",")}
    ps = {round(float(x),6) for x in args.probabilities.split(",")}
    if args.repeats < 1 or args.timeout <= 0:
        ap.error("repeats and timeout must be positive")
    with open(args.baseline, newline="", encoding="utf-8-sig") as f:
        baseline = list(csv.DictReader(f))
    selected = [r for r in baseline if r["family"].lower()=="random" and
                int(r["n"]) in sizes and round(float(r["p"]),6) in ps and
                int(r["seed"]) in seeds]
    selected.sort(key=lambda r:(int(r["n"]),float(r["p"]),int(r["seed"])))
    if not selected:
        ap.error("No matching baseline graph instances")
    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True,exist_ok=True)
    rows = []
    with out_path.open("w",newline="",encoding="utf-8") as f:
        writer=csv.DictWriter(f,fieldnames=FIELDS)
        writer.writeheader()
        for base in selected:
            for mode in MODES:
                for trial in range(1,args.repeats+1):
                    row=benchmark_case(args.solver,base,mode,trial,args.timeout)
                    writer.writerow(row)
                    f.flush()
                    rows.append(row)
                    print(f"n={base['n']:>3} p={base['p']} seed={base['seed']:>2} {mode:15} "
                          f"status={row['status']:8} nodes={row['nodes_explored']}")
    summary_path = out_path.with_name("ablation_summary.csv")
    groups = {}
    for row in rows:
        k = (row["n"],row["p"],row["mode"])
        groups.setdefault(k,[]).append(row)
    with summary_path.open("w",newline="",encoding="utf-8") as f:
        writer=csv.DictWriter(f,fieldnames=("n","p","mode","completed","timeouts","errors",
                                            "median_nodes_explored","median_runtime_ms"))
        writer.writeheader()
        for (n,p,mode), rs in sorted(groups.items(),key=lambda z:(int(z[0][0]),float(z[0][1]),z[0][2])):
            good = [r for r in rs if r["status"]=="ok"]
            writer.writerow({"n":n,"p":p,"mode":mode,"completed":len(good),
                             "timeouts":sum(r["status"]=="timeout" for r in rs),
                             "errors":sum(r["status"] not in ("ok","timeout") for r in rs),
                             "median_nodes_explored":statistics.median(int(r["nodes_explored"]) for r in good) if good else "",
                             "median_runtime_ms":statistics.median(float(r["runtime_ms"]) for r in good) if good else ""})
    print(f"\nWrote {out_path} and {summary_path}")
    if any(r["status"] not in ("ok","timeout") for r in rows):
        print("WARNING: check errors and mismatched optimal solutions before using results in report.",file=sys.stderr)
        return 2
    return 0

if __name__=="__main__":
    sys.exit(main())
