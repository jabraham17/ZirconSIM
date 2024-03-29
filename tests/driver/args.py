from __future__ import annotations
import sys
import util.FileHelper as FileHelper
from typing import Dict, Generator, List, Set, Union
from dataclasses import dataclass
import os
import enum
import re
import argparse as ap
import multiprocessing as mp
import glob
import itertools


class FilesAction(ap.Action):
    def __call__(self, parser, ns, values, option):
        files = []
        for v in values:
            if not os.path.exists(v):
                parser.error(f"'{v}' is not a valid path")
            files.append(FileHelper.makeabs(v))
        setattr(ns, self.dest, files)


class BooleanOptionalActionOverrideDefault(ap.BooleanOptionalAction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.help = self.help.replace(" (default: %(default)s)", "")


def arguments(args: List[str], env: Dict[str, str]) -> ap.Namespace:
    a = ap.ArgumentParser(formatter_class=ap.HelpFormatter)

    a.add_argument(
        "paths",
        nargs="*",
        default=[env["PRIMARY_TEST_DIR"]],
        action=FilesAction,
        help="paths to search for test cases. Can be any valid path",
    )

    a.add_argument(
        "--summary",
        default=True,
        action=ap.BooleanOptionalAction,
        help="print a final summary report",
    )

    a.add_argument(
        "--full-report",
        default=False,
        action=ap.BooleanOptionalAction,
        help="print test results as they finish",
    )

    a.add_argument(
        "--test-build",
        default=True,
        action=ap.BooleanOptionalAction,
        help="test all simulator build settings, not just the needed ones",
    )

    a.add_argument(
        "--diff",
        default=False,
        action=ap.BooleanOptionalAction,
        help="show diff of failing tests",
    )

    a.add_argument(
        "--dry-run",
        default=False,
        action="store_true",
        help="don't run the tests, just print which would have been run",
    )

    a.add_argument(
        "-x",
        "--exclude",
        default=[],
        action="append",
        metavar="PATTERN",
        help="exclude tests matching PATTERN",
    )

    a.add_argument(
        "-xr",
        "--exclude-regex",
        default=[],
        action="append",
        metavar="PATTERN",
        help="exclude tests matching regex PATTERN",
    )

    # default is number of cores
    a.add_argument(
        "-j",
        "--jobs",
        default=mp.cpu_count(),
        help="how many jobs (tests) to run at once",
    )

    a.add_argument(
        "-k",
        "--keep",
        default=False,
        action="store_true",
        help="keep files generated by running tests",
    )

    # default is if stdout is atty
    a.add_argument(
        "--color",
        default=sys.stdout.isatty(),
        action=BooleanOptionalActionOverrideDefault,
        help="override the default automatic color output",
    )

    a.add_argument(
        "-v",
        "--verbose",
        default=False,
        action="store_true",
        help="print the commands being executed",
    )

    # BooleanOptionalAction

    return a.parse_args(args)
