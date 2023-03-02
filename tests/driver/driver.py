#!/usr/bin/env python3

from __future__ import annotations
import functools
import sys
import util.FileHelper as FileHelper
from typing import Any, Dict, Generator, List, Set, Union
from dataclasses import dataclass
import os
import enum
import re
import argparse as ap
import multiprocessing as mp
import glob
import itertools
from args import arguments
import TestDefinition
import shutil


def get_env(script: str) -> Dict[str, Any]:
    """set env vars based on this files path, relative to the current working directory
    this is usually sys.argv[0]"""
    # should be /path/to/ZirconSim/tests/driver/test-driver.py
    env = dict()
    env["SCRIPT_DIR"] = os.path.dirname(FileHelper.makeabs(script))
    env["PRIMARY_TEST_DIR"] = os.path.dirname(env["SCRIPT_DIR"])
    env["TEST_CONFIG"] = os.path.join(env["SCRIPT_DIR"], "config")
    env["ZIRCON_HOME"] = os.path.dirname(env["PRIMARY_TEST_DIR"])

    env["ZIRCON_BUILD_DIR_BASENAME"] = "._build-zircon"
    env["ZIRCON_BUILD_DIR"] = os.path.join(
        env["ZIRCON_HOME"], env["ZIRCON_BUILD_DIR_BASENAME"]
    )
    os.makedirs(env["ZIRCON_BUILD_DIR"], exist_ok=True)

    env["DEFAULT_SOURCE_FILE"] = os.path.join(env["TEST_CONFIG"], "DEFAULT.c")
    env["SIMULATOR_CONFIG_FILE"] = os.path.join(env["TEST_CONFIG"], "COMPSIM")
    env["SIMULATOR_DEFS"] = TestDefinition.parse_variable_file(
        env["SIMULATOR_CONFIG_FILE"], env
    )
    env["SIMULATOR_PATHS"] = {
        k: os.path.join(env["ZIRCON_BUILD_DIR"], k)
        for k, v in env["SIMULATOR_DEFS"].items()
    }

    env["TOOLCHAIN_CONFIG_FILE"] = os.path.join(env["TEST_CONFIG"], "TOOLCHAINS")
    env["TOOLCHAIN_PATHS"] = TestDefinition.parse_variable_file(
        env["TOOLCHAIN_CONFIG_FILE"], env
    )
    return env


def main(args: List[str], env: Dict[str, str]) -> int:

    a = arguments(args, env)

    test_names = TestDefinition.find_all_unique_test_names(a.paths)
    print()
    print(*test_names, sep="\n")
    print()

    tests: List[TestDefinition.TestConfiguration] = []
    for test_name in test_names:
        tests.extend(TestDefinition.TestConfiguration.build(test_name, env))

    # build all required zircon configurations
    # TODO: needs a serious revamp
    success = TestDefinition.build_zircons(
        env["SIMULATOR_DEFS"],
        env["ZIRCON_HOME"],
        env["ZIRCON_BUILD_DIR_BASENAME"],
        None if a.test_build else TestDefinition.get_all_required_simulators(tests),
    )
    if success:
        print("Successfully built all Zircons")
    else:
        print("Failed to build Zircons")

    test_instances = [TestDefinition.TestInstance(t) for t in tests]
    print(*test_instances, sep="\n")
    print()
    print(*list(env.items()), sep="\n")
    print()

    env_run_func = functools.partial(TestDefinition.TestInstance.run, env=env)
    test_results: List[TestDefinition.TestResult]
    with mp.Pool() as pool:
        test_results = pool.map(env_run_func, test_instances)

    if not a.keep:
        shutil.rmtree(env["ZIRCON_BUILD_DIR_BASENAME"])

    passed = 0
    for t in sorted(test_results, key=lambda t: t.name()):
        if t.isSuccess():
            passed += 1
        print(t.name(), ("Passed" if t.isSuccess() else "Failed"))
        if not a.keep:
            t.clean()
    print()
    print(
        f"Passed {passed}/{len(test_results)} ({100.0*(float(passed)/len(test_results)):4.2f})"
    )

    return 0


if __name__ == "__main__":
    env = get_env(sys.argv[0])
    exit(main(sys.argv[1:], env))
