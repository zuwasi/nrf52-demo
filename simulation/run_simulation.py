#!/usr/bin/env python3
"""Build and exercise the nRF52 BLE range detector in host simulation.

The script validates the same C scanner core used by the ARM application. If an
ARM GCC 13 toolchain is installed, pass --arm to also compile the Cortex-M4 ELF.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


def run(command: list[str], cwd: Path) -> str:
    completed = subprocess.run(
        command,
        cwd=str(cwd),
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return completed.stdout


def configure_and_test_host(root: Path) -> None:
    build_dir = root / "build-host"
    generator = "Ninja" if shutil.which("ninja") is not None else None
    configure = ["cmake", "-S", str(root), "-B", str(build_dir), "-DNRF52_BUILD_ARM_APP=OFF"]

    if generator is not None:
        configure.extend(["-G", generator])

    print(run(configure, root), end="")
    print(run(["cmake", "--build", str(build_dir)], root), end="")
    print(run(["ctest", "--test-dir", str(build_dir), "--output-on-failure"], root), end="")

    simulation_output = run([str(build_dir / "scanner_sim.exe")], root)
    print(simulation_output, end="")

    if "NEAR" not in simulation_output:
        raise RuntimeError("simulation did not report a near BLE device")

    if "tracked=2" not in simulation_output:
        raise RuntimeError("simulation did not track the expected devices")


def configure_arm(root: Path) -> None:
    if shutil.which("arm-none-eabi-gcc") is None:
        print("arm-none-eabi-gcc not found; skipping ARM build")
        return

    version_text = run(["arm-none-eabi-gcc", "-dumpfullversion"], root).strip()
    major_text = version_text.split(".", maxsplit=1)[0]
    major = int(major_text)

    if major < 13:
        raise RuntimeError(f"ARM GCC 13+ required, found {version_text}")

    build_dir = root / "build-arm"
    generator = "Ninja" if shutil.which("ninja") is not None else None
    configure = [
        "cmake",
        "-S",
        str(root),
        "-B",
        str(build_dir),
        f"-DCMAKE_TOOLCHAIN_FILE={root / 'cmake' / 'arm-gcc13-toolchain.cmake'}",
        "-DNRF52_BUILD_TESTS=OFF",
    ]

    if generator is not None:
        configure.extend(["-G", generator])

    print(run(configure, root), end="")
    print(run(["cmake", "--build", str(build_dir)], root), end="")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--arm", action="store_true", help="also compile ARM Cortex-M4 target when GCC 13 is installed")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]

    try:
        configure_and_test_host(root)
        if args.arm:
            configure_arm(root)
    except (RuntimeError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
