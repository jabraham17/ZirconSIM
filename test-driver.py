#!/usr/bin/env python3
from enum import Enum, IntFlag, auto
import glob
import shlex
import tempfile
from typing import Callable, Dict, List, Set, Tuple
import os
import subprocess as sp
import sys
import multiprocessing as mp


TEST_ENV = {

}


# def list_files(pattern: str) -> List[str]:
#     return glob.glob(pattern)




def execute(args: List[str], stdin: str = None) -> Tuple[int, str]:
    cp = sp.run(args, stdout=sp.PIPE, stderr=sp.STDOUT, input=stdin, text=True, errors="ignore")
    return (
        cp.returncode,
        cp.stdout
    )


def build_asm(inpath: str, outpath: str, tools: Dict = {}) -> bool:
    if "assembler" not in tools or "linker" not in tools:
        return False

    assembler_path = tools["assembler"]["path"]
    assembler_args = tools["assembler"].get("args", [])
    fd, tmp_file = tempfile.mkstemp()
    os.close(fd)
    cmd = [assembler_path] + assembler_args + [inpath, "-o", tmp_file]
    ret, _ = execute(cmd)
    if ret != 0:
        return False

    linker_path = tools["linker"]["path"]
    linker_args = tools["linker"].get("args", [])
    cmd = [linker_path] + linker_args + [tmp_file, "-o", outpath]
    ret, _ = execute(cmd)
    os.remove(tmp_file)

    return ret == 0

def build_c(
    inpath: str, outpath: str, tools: Dict = {}
) -> bool:
    if "compiler" not in tools:
        return False

    compiler_path = tools["compiler"]["path"]
    compiler_args = tools["compiler"].get("args", [])

    cmd = [compiler_path] + compiler_args + [inpath, "-o", outpath]

    ret, _ = execute(cmd)
    return ret == 0


extensions = {
    "c": {
        "language": "C",
        "build_func": build_c,
    },
    "s": {
        "language": "Assembly",
        "build_func": build_asm
    }
}

def get_file_ext(path: str) -> str:
    return os.path.splitext(path)[1].strip(".")


def is_testcase(path: str) -> bool:
    return get_file_ext(path) in extensions.keys()

def make_absolute(path: str) -> str:
    path = os.path.expanduser(path)
    path = os.path.expandvars(path)
    path = os.path.realpath(path)
    path = os.path.abspath(path)
    path = os.path.normpath(path)
    return path


def check_file_exists(path: str) -> bool:
    return os.path.exists(path) and os.path.isfile(path)

def check_directory_exists(path: str) -> bool:
    return os.path.exists(path) and os.path.isdir(path)



# class SourceType(Enum):
#     # ASM = auto(), "s", build_asm
#     C = auto(), "c", build_c
#     # CPP = auto(), "cpp", build_cpp

#     def __new__(cls, *args, **kwds):
#         assert(len(args) == 2 and len(kwds)==0)
#         obj = object.__new__(cls, args[0])
#         return obj

#     def __init__(self, _, extension: str, build_func: Callable):
#         self._extension = extension
#         self._build_func = build_func

#     def __str__(self) -> str:
#         return self.value

#     @property
#     def extension(self) -> str:
#         return self._extension

#     @property
#     def build_func(self) -> Callable:
#         return self._build_func

# class ExpectedType(IntFlag):
#     STDOUT = auto()
#     STATE = auto()


class Test:
    def __init__(self, path: str):
        """path MUST be an absolute path"""
        assert(os.path.isabs(path))
        self.path = path
        self.dirpath, self.filename = os.path.split(self.path)
        self.basename, self.ext = os.path.splitext(self.filename)
        self.ext = self.ext.strip(".")
        self._name = os.path.join(self.dirpath, self.basename)

        self.source_file = self.path
        self.build_func = extensions[self.ext]["build_func"]

        self.stdout_file = os.path.join(self.dirpath, self.basename + ".stdout")
        if not check_file_exists(self.stdout_file):
            self.stdout_file = None
        self.args_file = os.path.join(self.dirpath, self.basename + ".args")
        if not check_file_exists(self.args_file):
            self.args_file = None
        self.state_file = os.path.join(self.dirpath, self.basename + ".state")
        if not check_file_exists(self.state_file):
            self.state_file = None

        self.executable_file = os.path.join(self.dirpath, self.basename + ".out")

    def valid(self) -> bool:
        return True

    def build(self, compiler: str) -> bool:
        success = self.build_func(
            self.source_file,
            self.executable_file,
            tools={
                "compiler": {"path": compiler, "args": ["-static"]},
                "assembler": {"path": TEST_ENV["ASSEMBLER"], "args": []},
                "linker": {"path": TEST_ENV["LINKER"], "args": []},
                },
        )
        return success

    def prerun(self):
        pass

    def run(self, simulator: str) -> Tuple[int, str]:
        if self.args_file:
            simulator_args = open(self.args_file, "r").read()
            simulator_args = shlex.split(simulator_args)
            simulator_args = [a.strip() for a in simulator_args]
        else:
            simulator_args = []
        cmd = [simulator, self.executable_file] + simulator_args
        # print(cmd)
        ret, output = execute(cmd)
        # print(output)
        return ret, output

    def check_output(self, output: str) -> bool:
        if self.stdout_file:
            expected = open(self.stdout_file, "r").read()
            return expected == output
        else:
            # no stdout specified, we will not check the output
            return True
    
    def run_and_check(self, simulator: str) -> bool:
        ret, output = self.run(simulator)
        return ret == 0 and self.check_output(output)

    def clean(self):
        pass

    def name(self) -> str:
        name = os.path.relpath(self._name)
        return name


def identify_tests(root_dirs: List[str]) -> Set[str]:
    tests = set()
    for root_dir in root_dirs:
        for dirpath, dirs, files in os.walk(root_dir):
            # get all files from this dir
            files_in_walk = [os.path.join(dirpath, f) for f in files]
            # get just the source files, these define a single test
            tests_in_walk = [f for f in files_in_walk if is_testcase(f)]
            tests.update(tests_in_walk)

            # walk the dirs to get more tests
            dirs_in_walk = [os.path.join(dirpath, d) for d in dirs]
            tests_from_dirs = identify_tests(dirs_in_walk)
            tests.update(tests_from_dirs)
    return list(tests)


def process_testcase(t: Test) -> Tuple[Test, str]:
    if not t.valid():
        return t, "Invalid Test Case"
    if not t.build(TEST_ENV["C_COMPILER"]):
        return t, "Failed to Build"
    if not t.run_and_check(TEST_ENV["SIMULATOR"]):
        return t, "Failed Check"
    return t, None

def main(args: List[str]) -> int:
    # TODO: for now use all args as root dirs for tests
    paths = args

    # if no paths specified, use default tests directory
    if len(paths) == 0:
        paths.append(make_absolute(os.path.join(TEST_ENV["SCRIPT_DIR"], "tests")))

    tests: Set[str] = set()
    root_dirs: Set[str] = set()
    for p in paths:
        fullpath = make_absolute(p)
        # if file and testcase, add to list of tests
        # if directory, check it later for more tests
        if check_file_exists(fullpath) and is_testcase(fullpath):
            tests.add(fullpath)
        elif check_directory_exists(fullpath):
            root_dirs.add(fullpath)
    tests.update(identify_tests(list(root_dirs)))

    tests = [Test(t) for t in tests]


    results = []
    with mp.Pool(4) as pool:
        results = pool.map(process_testcase, tests)

    for t, r in results:
        if not r:
            print(t.name(), ": Passed")
        else:
            print(t.name(), ": Failed -", r)

    return 0


if __name__ == "__main__":
    TEST_ENV["SCRIPT_PATH"] = make_absolute(sys.argv[0])
    TEST_ENV["SCRIPT_DIR"] = os.path.dirname(TEST_ENV["SCRIPT_PATH"])
    # TODO: have this detected and configurasble per test
    TEST_ENV["C_COMPILER"] = make_absolute(os.path.join(TEST_ENV["SCRIPT_DIR"], "toolchains/rv64ima/bin/riscv64-unknown-elf-gcc"))
    TEST_ENV["ASSEMBLER"] = make_absolute(os.path.join(TEST_ENV["SCRIPT_DIR"], "toolchains/rv64ima/bin/riscv64-unknown-elf-as"))
    TEST_ENV["LINKER"] = make_absolute(os.path.join(TEST_ENV["SCRIPT_DIR"], "toolchains/rv64ima/bin/riscv64-unknown-elf-ld"))
    TEST_ENV["SIMULATOR"] = make_absolute(os.path.join(TEST_ENV["SCRIPT_DIR"], "build/bin/zircon"))
    exit(main(sys.argv[1:]))
