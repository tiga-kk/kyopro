#!/usr/bin/env python3
"""Compare a candidate program with a reference on generated inputs."""

from __future__ import annotations

import argparse
import importlib.util
import random
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path


def load_generator(path: Path):
    spec = importlib.util.spec_from_file_location("stress_generator", path)
    if spec is None or spec.loader is None:
        raise ValueError(f"cannot load generator: {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    generate_case = getattr(module, "generate_case", None)
    if not callable(generate_case):
        raise ValueError("generator must define generate_case(rng)")
    return generate_case


def command_args(command: str, base_dir: Path) -> list[str]:
    args = shlex.split(command)
    if not args:
        raise ValueError("program command cannot be empty")
    resolved = []
    for arg in args:
        path = Path(arg)
        local_path = base_dir / path
        resolved.append(
            str(local_path.resolve()) if not path.is_absolute() and local_path.exists() else arg
        )
    return resolved


def run(command: list[str], input_data: bytes, cwd: Path, timeout: float):
    (cwd / "input.txt").write_bytes(input_data)
    try:
        result = subprocess.run(
            command,
            input=input_data,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=cwd,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as error:
        return 124, error.stdout or b"", (error.stderr or b"") + b"\nTimed out\n"
    return result.returncode, result.stdout, result.stderr


def save_failure(directory: Path, seed: int, case_no: int, input_data: bytes, main, fool):
    directory.mkdir(parents=True, exist_ok=True)
    stem = f"seed-{seed}-case-{case_no}"
    path = directory / f"{stem}.in"
    suffix = 1
    while path.exists():
        path = directory / f"{stem}-{suffix}.in"
        suffix += 1

    path.write_bytes(input_data)
    path.with_suffix(".main.out").write_bytes(main[1])
    path.with_suffix(".main.err").write_bytes(main[2])
    path.with_suffix(".fool.out").write_bytes(fool[1])
    path.with_suffix(".fool.err").write_bytes(fool[2])
    return path


def show_output(output: bytes):
    text = output.decode(errors="replace")
    if not text:
        print("<empty>")
        return
    sys.stdout.write(text)
    if not text.endswith("\n"):
        sys.stdout.write("\n")


def show_result(name: str, result):
    print(f"=== {name} (exit code: {result[0]}) ===")
    for stream_name, output in (("stdout", result[1]), ("stderr", result[2])):
        print(f"{stream_name}:")
        show_output(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--generator", type=Path, required=True, help="Python file defining generate_case(rng)"
    )
    parser.add_argument(
        "--candidate", required=True, help="candidate command, e.g. '.build/main-fast'"
    )
    parser.add_argument(
        "--reference", required=True, help="reference command, e.g. 'python fool.py'"
    )
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--cases", type=int, default=1000)
    parser.add_argument("--timeout", type=float, default=2.0, help="per-program timeout in seconds")
    parser.add_argument("--failure-dir", type=Path, default=Path("stress_failures"))
    args = parser.parse_args()

    if args.cases < 1 or args.timeout <= 0:
        parser.error("--cases and --timeout must be positive")

    base_dir = Path.cwd()
    generator_path = (base_dir / args.generator).resolve()
    failure_dir = (base_dir / args.failure_dir).resolve()
    try:
        generate_case = load_generator(generator_path)
        candidate_command = command_args(args.candidate, base_dir)
        reference_command = command_args(args.reference, base_dir)
    except (OSError, ValueError) as error:
        parser.error(str(error))

    rng = random.Random(args.seed)
    interactive_progress = sys.stderr.isatty()
    progress_step = max(1, args.cases // 10)
    for case_no in range(1, args.cases + 1):
        if interactive_progress:
            sys.stderr.write(f"\r{case_no}/{args.cases}")
            sys.stderr.flush()
        elif case_no == 1 or case_no % progress_step == 0 or case_no == args.cases:
            print(f"{case_no}/{args.cases}", file=sys.stderr, flush=True)
        generated = generate_case(rng)
        if not isinstance(generated, (str, bytes)):
            parser.error("generate_case(rng) must return str or bytes")
        input_data = generated.encode() if isinstance(generated, str) else generated

        with tempfile.TemporaryDirectory(prefix="stress-") as temporary:
            work_dir = Path(temporary)
            candidate_dir = work_dir / "candidate"
            reference_dir = work_dir / "reference"
            candidate_dir.mkdir()
            reference_dir.mkdir()
            candidate = run(candidate_command, input_data, candidate_dir, args.timeout)
            reference = run(reference_command, input_data, reference_dir, args.timeout)

        matches = (
            candidate[0] == 0 and reference[0] == 0 and candidate[1].split() == reference[1].split()
        )
        if not matches:
            if interactive_progress:
                print(file=sys.stderr)
            failure_path = save_failure(
                failure_dir, args.seed, case_no, input_data, candidate, reference
            )
            print("=== input ===")
            show_output(input_data)
            show_result("main", candidate)
            show_result("fool", reference)
            print(f"Mismatch at {case_no}/{args.cases}; seed={args.seed}")
            print(f"Saved input and outputs under {failure_path}")
            return 1

    if interactive_progress:
        print(file=sys.stderr)
    print(f"OK: {args.cases} cases, seed={args.seed}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
