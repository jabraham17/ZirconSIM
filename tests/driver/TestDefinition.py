from __future__ import annotations
import functools
import shlex
import sys
import util.FileHelper as FileHelper
from typing import Any, Dict, Generator, List, Set, Tuple, Union
from dataclasses import dataclass, field
import os
import enum
import re
import argparse as ap
import multiprocessing as mp
import glob
import itertools
from args import arguments
import TestDefinition
import util.util as util
from util.StringMatcher import StringMatcher
import subprocess as sp


@util.static_vars(
    filename_matcher=StringMatcher(
        r"([a-zA-Z0-9_\-\./ ]+)\.((c)|(cpp)|(cc)|(s)|(asm)|(exp)|([a-zA-Z0-9_\-\.]+\.exp)|(compopt)|(execopt)|(sim)|(toolchain))$",
        StringMatcher.Flags.REGEX,
    )
)
def isTestFile(
    filepath: str, exclude_list: List[StringMatcher] = []
) -> Union[str, None]:
    """
    if this file is a valid test file
    returns the basename of the test, including the full path

    criteria to be a valid file
    - file is not in the exclude list
    - file is of the test filename format
      - /path/to/TESTNAME.EXT where EXT is one of the valid source file extensions
      - /path/to/TESTNAME.exp
      - /path/to/TESTNAME.SUBNAME.exp
      - /path/to/TESTNAME.compopt
      - /path/to/TESTNAME.execopt
      - /path/to/TESTNAME.sim
      - /path/to/TESTNAME.toolchain
    """

    # check if excluded
    for e in exclude_list:
        if e.match(filepath):
            return None

    m = isTestFile.filename_matcher.match(filepath)
    if not m:
        return None
    else:
        test_name: str = m[0]  # should be only one match
        if os.path.basename(test_name).startswith("._"):
            return None
        return test_name


class TestFile:
    """wrapper class for a path, keeps state of whether this file was generated or not"""

    # TODO: generated testfiles get added to a pool of testfiles to be later deleted

    def __init__(self, path: str, generated: bool = False, prefix=None):
        self._path = path
        self._generated = generated
        self._filemode = "r"
        self._fd = None
        if prefix:
            self._path = TestFile.prefix_temp(self._path, prefix=prefix)

    def isGenerated(self):
        return self._generated

    def getPath(self):
        return self._path

    def __call__(self, filemode: str):
        self._filemode = filemode
        return self

    def __enter__(self):
        self._fd = open(self._path, self._filemode)
        return self._fd

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self._fd.close()
        self._fd = None

    def remove(self):
        """unchecked remove of TestFile"""
        os.remove(self._path)

    def clean(self):
        """checked remove of file
        dont remove non-generated files and dont remove it if doesn't exist (obviously)"""
        if self.isGenerated() and FileHelper.fileExists(self._path):
            self.remove()

    @classmethod
    def default_prefix(cls) -> str:
        return "._"

    @classmethod
    def prefix_temp(cls, path: str, prefix=None) -> str:
        """return the path with a temp prefix of '._'"""
        if not prefix:
            prefix = cls.default_prefix()
        d, basename = os.path.split(path)
        basename = prefix + basename
        path = os.path.join(d, basename)
        return path

    @classmethod
    def create(
        cls, path: str, copy_path: str = None, content: List[str] = []
    ) -> TestFile:
        """create a generated file, filename is prefixed as a temp"""
        path = TestFile.prefix_temp(path)

        # copy content from copy to here
        if copy_path:
            if not FileHelper.fileExists(copy_path):
                raise ConfigurationError(
                    "Specified copy path '" + copy_path + "' unknown"
                )
            with open(copy_path, "r") as copy:
                with open(path, "w") as f:
                    f.writelines(copy.readlines())

        # then append the content to it
        with open(path, "a") as f:
            f.writelines(content)
        return TestFile(path, generated=True)


def yieldTestSourceFile(test_name: str) -> Generator[TestFile, None, None]:
    """given a base test_name, return all next possible source files with the base name"""
    valid_src_ext = [".c", ".cpp", ".cc", ".s", ".asm"]
    for e in valid_src_ext:
        path = test_name + e
        if FileHelper.fileExists(path):
            yield TestFile(path)


def skipDirectory(directory_path: str) -> bool:
    """
    if this directory should be skipped

    criteria to be skipped
    - directory contains a SKIP file
    """
    p = os.path.join(directory_path, "SKIP")
    return os.path.exists(p)


def find_all_unique_test_names(paths: List[str]) -> List[str]:
    """find all testcases within a given path"""
    test_names = set()
    for path in paths:
        test_names.update(_find_all_unique_test_names(path))
    return list(test_names)


# TODO: refactor this method name
# TODO: maybe make this a generator for perf?
def _find_all_unique_test_names(path: str) -> Set[str]:
    # check a single file and return its match immediately
    # since its a file dont need to walk its dir, there is none :)
    if FileHelper.fileExists(path) and (test_name := isTestFile(path)):
        return {test_name}

    test_names = set()
    for p in os.listdir(path):
        full_p = os.path.join(path, p)

        # skip directories not included
        if skipDirectory(full_p):
            continue

        if os.path.isfile(full_p) and (test_name := isTestFile(full_p)):
            test_names.add(test_name)

        if os.path.isdir(full_p):
            test_names.update(_find_all_unique_test_names(full_p))

    return test_names


def get_all_required_simulators(tests: List[TestConfiguration]) -> List[str]:
    required = set()
    for t in tests:
        required.update(t.get_required_simulators())
    return list(required)


def _build_one_zircon(build_dir: str, name: str, cmd: str):
    if cmd is None:
        print("Unknown configuration for '" + name + "'")
        return False
    cmd = "make " + cmd + f" BUILD='{os.path.join(build_dir, name)}'"
    cmd = shlex.split(cmd)
    print(cmd)
    ret = sp.call(cmd, stdout=sp.DEVNULL, stderr=sp.DEVNULL)
    if ret != 0:
        # TODO: add more info here, should return an object and print later
        print("Failed to build '" + name + "'")
        return False
    return True


# TODO: this seriously needs a revamp, but itll work
def build_zircons(
    variables: Dict[str, str],
    zircon_home: str,
    build_dir: str,
    config_names: List[str] = None,
) -> bool:
    """build the zircon configurations in
    if config_names is none, build them all"""
    cwd = os.getcwd()
    os.chdir(zircon_home)
    res = True

    if config_names is None:
        config_names = list(variables.keys())

    with mp.Pool() as pool:
        pool_res = pool.starmap(
            _build_one_zircon,
            zip(
                itertools.repeat(build_dir),
                config_names,
                [variables.get(name, None) for name in config_names],
            ),
        )
        # if any are false, res is False
        if any([r == False for r in pool_res]):
            res = False

    os.chdir(cwd)
    return res


def parse_variable_file(config_file: str, env: Dict[str, Any]) -> Dict[str, str]:
    variables = dict()

    class ContinueOuterLoop(Exception):
        pass

    with open(config_file, "r") as f:
        for idx, l in enumerate(f.readlines()):
            try:
                l = l.strip()
                # skip if no test defined
                if not l or len(l) == 0 or l.startswith("#"):
                    raise ContinueOuterLoop
                elms = l.split("=", maxsplit=1)
                if len(elms) != 1 and len(elms) != 2:
                    print(
                        "Invalid line '"
                        + l
                        + "' in '"
                        + config_file
                        + "' on line '"
                        + (idx + 1)
                        + "'"
                    )
                    raise ContinueOuterLoop

                name = elms[0].strip()
                cmd = elms[1].strip() if len(elms) == 2 else ""

                def do_var_replace(match: re.Match):
                    env_name = match.group(1)
                    if env_name in env:
                        return env[env_name]
                    else:
                        print(
                            "Unknown variable name '"
                            + env_name
                            + "' in '"
                            + config_file
                            + "' on line '"
                            + (idx + 1)
                            + "'"
                        )
                        raise ContinueOuterLoop

                cmd = re.sub(r"\$\((.+)\)", do_var_replace, cmd)

                variables[name] = cmd

            except ContinueOuterLoop:
                pass
    return variables


class ConfigurationError(Exception):
    """raised when a configuration is invalid"""

    def __init__(self, msg: str):
        self.msg = msg

    def __str__(self):
        return str(__class__) + ": " + self.msg


@dataclass
class Configuration:
    """represents a set of compile/exec options"""

    options: str = ""
    subtest_name: Union[str, None] = None

    def __post_init__(self):
        # if there is a subtest name, make sure it is valid
        self.subtest_name = (
            self.subtest_name
            if self.subtest_name and len(self.subtest_name) != 0
            else None
        )

    @classmethod
    def _parse_config_line(cls, line: str) -> Configuration:
        elms = [e.strip() for e in line.split("#", maxsplit=1)]
        if len(elms) == 0:
            return None
        if len(elms) == 1:
            c = Configuration(elms[0])
        elif len(elms) == 2:
            c = Configuration(elms[0], elms[1])
        else:
            raise ConfigurationError("Invalid configuration for line - '" + line + "'")
        return c

    @classmethod
    def build(cls, path: TestFile) -> List[Configuration]:
        """build a series of configurations based on a path"""
        with path("r") as fd:
            configs = [
                c for l in fd.readlines() if (c := Configuration._parse_config_line(l))
            ]
            return configs

    @classmethod
    def build_or_default(cls, path: str, default: str = "") -> List[Configuration]:
        """build a series of configurations based on a path, if path is invalid return an empty config with default args"""
        configs: List[Configuration] = []
        if FileHelper.fileExists(path):
            configs = Configuration.build(TestFile(path))
        else:
            configs = [Configuration(options=default)]
        return configs

    def __hash__(self):
        return hash(self.options + self.subtest_name)


# FIXME: problem, I need simulator configs to be global
# i dont want to rebuild the simualtor for every test
# solution 1: allocate out simulator configs from a pool
# this is implemented as a subclass of Configuration, we have many duplicate copies but can sort them later
# solution 2: make it so you can only reference predefined sim build configurations, similar to the future tools proposal?
# this makes it harder to specify large amounts of combinations, but centralizes for the programmer where they are defiened and we dont need to specify functional tests whos sole purpose is testing the diffeent combinations of compilers? but it does require special machinery? which is probably ok


@dataclass
class TestConfiguration:
    """represents a combination of compile options, exec options, tool options"""

    test_name: str
    source_file: TestFile
    expected_file: TestFile
    compile_options: Configuration
    exec_options: Configuration
    sim_options: Configuration
    toolchain_options: Configuration
    sub_name: Union[str, None] = None

    def get_required_simulators(self) -> List(str):
        """return which compiled simulators are required to run these tests"""
        return [self.sim_options.options]

    # TODO: add required toolchain so these can be checked for automatically

    def name(self):
        if self.sub_name:
            return f"{self.test_name}.{self.sub_name}"
        else:
            return self.test_name

    @classmethod
    def build(cls, test_name: str, env: Dict[str, Any]) -> List[TestConfiguration]:
        """build a series of test configurations, given a basename"""

        # get all possible source files for the base name, if there is more than one its an error
        src_files: Generator[TestFile, None, None] = yieldTestSourceFile(test_name)
        src_file: TestFile = None
        try:
            src_file = next(src_files)
        except StopIteration:
            # No src file, use the default
            src_file = TestFile.create(
                test_name + "." + FileHelper.getext(env["DEFAULT_SOURCE_FILE"]),
                copy_path=env["DEFAULT_SOURCE_FILE"],
            )
        # try and next() the generator one more time, if it yields something we have a conflict, config error
        try:
            next(src_files)
            raise ConfigurationError("Too many source files for '" + test_name + "'")
        except StopIteration:
            pass

        # get all possible exp files
        # if default doesnt exist, use an empty file
        exp_files: Dict[str, TestFile] = dict()
        default_exp = test_name + ".exp"
        if FileHelper.fileExists(default_exp):
            exp_files["DEFAULT"] = TestFile(default_exp)
        else:
            exp_files["DEFAULT"] = TestFile.create(default_exp)

        for f in glob.glob(test_name + ".*.exp"):
            # get just the star part
            sub_name = os.path.basename(f).split(".")[1]
            exp_files[sub_name] = TestFile(f)

        # load the exec opt and comp opt if they exists, if they dont exist use empty configs
        comp_configs = Configuration.build_or_default(test_name + ".compopt")
        exec_configs = Configuration.build_or_default(test_name + ".execopt")
        sim_configs = Configuration.build_or_default(test_name + ".sim", "default")
        # TODO: evalulate if this is a good default. maybe we should use all configurations
        # but that might be ALOT and cause tests to run forever
        toolchain_configs = Configuration.build_or_default(
            test_name + ".toolchain", "rv64ima_elf_gcc"
        )

        test_configurations = []
        # need all possible combinations of the three configs
        # however not all combinations, if more than one file specifies an sub_name
        # then that possible combination of configurations is invalid
        for configs in itertools.product(
            comp_configs, exec_configs, sim_configs, toolchain_configs
        ):
            # if there are more than 1 subtest_names, skip
            # if there is exactly 1, use it
            n_subtest_names = sum(1 for c in configs if c.subtest_name)
            sub_name = None
            if n_subtest_names > 1:
                continue
            elif n_subtest_names == 1:
                sub_name = next(c.subtest_name for c in configs if c.subtest_name)

            exp_file = exp_files["DEFAULT"] if not sub_name else exp_files[sub_name]

            (comp_config, exec_config, sim_config, toolchain_config) = configs

            t = TestConfiguration(
                test_name=test_name,
                source_file=src_file,
                expected_file=exp_file,
                compile_options=comp_config,
                exec_options=exec_config,
                sim_options=sim_config,
                toolchain_options=toolchain_config,
                sub_name=sub_name,
            )
            test_configurations.append(t)
        return test_configurations


TestBuildSuccess = Tuple[TestFile, TestFile]
TestBuildFailure = Tuple[TestFile, str]


@dataclass
class TestInstance:
    """instance of test, continues information to run and check the output of a test"""

    config: TestConfiguration

    @classmethod
    def _execute(cls, cmd: List[str], output_file: TestFile = None) -> int:
        if output_file:
            with output_file("w") as output_fd:
                p = sp.Popen(cmd, stdout=output_fd, stderr=output_fd)
                p.communicate()
                return p.returncode
        else:
            p = sp.Popen(cmd, stdout=sp.DEVNULL, stderr=sp.DEVNULL)
            p.communicate()
            return p.returncode

    def _build_testcase(
        self, env: Dict[str, Any]
    ) -> Tuple[bool, Union[TestBuildSuccess, TestBuildFailure]]:
        """if build success, returns (success, object)
        object on success is (build_output, executable)
        object on failure is (build_output, reason)"""
        test_name = self.config.test_name
        compile_options = self.config.compile_options.options
        source_file = self.config.source_file
        executable_file = TestFile(
            test_name + ".out", generated=True, prefix=TestFile.default_prefix()
        )
        build_output = TestFile(
            test_name + ".build",
            generated=True,
            prefix=TestFile.default_prefix(),
        )

        toolchain_name = self.config.toolchain_options.options
        if toolchain_name in env["TOOLCHAIN_PATHS"]:
            toolchain_path = env["TOOLCHAIN_PATHS"][toolchain_name]
        else:
            return (
                False,
                (build_output, "Invalid toolchain '" + toolchain_name + "'"),
            )

        # find first compiler like tool in toolchain
        # TODO: for now use g++ for everything
        tool = glob.glob(os.path.join(toolchain_path, "bin", "*-g++"))[0]

        cmd = (
            [tool]
            + shlex.split(compile_options)
            + [source_file.getPath(), "-o", executable_file.getPath()]
        )
        ret = TestInstance._execute(cmd, build_output)

        if ret == 0:
            return (True, (build_output, executable_file))
        else:
            return (False, (build_output, "Failed to build"))

    def _execute_testcase(
        self, executable: TestFile, env: Dict[str, Any]
    ) -> Union[Tuple[bool, TestFile], Tuple[bool, str]]:
        """runs executable and returns on success (True, output_file)
        on an error returns (False, msg)
        Note this function does NOT check return code of the testcase"""
        test_name = self.config.test_name
        exec_options = self.config.exec_options.options
        exec_output = TestFile(
            test_name + ".output",
            generated=True,
            prefix=TestFile.default_prefix(),
        )

        simulator_name = self.config.sim_options.options
        if simulator_name in env["SIMULATOR_PATHS"]:
            simulator_path = env["SIMULATOR_PATHS"][simulator_name]
        else:
            return (False, "Invalid simulator '" + simulator_name + "'")

        sim = os.path.join(simulator_path, "bin", "zircon")

        print(exec_options)
        cmd = [sim] + [executable.getPath()] + shlex.split(exec_options)

        # intentionally not checking return code, this has no bearing on test case result
        TestInstance._execute(cmd, exec_output)

        return (True, (exec_output))

    # TODO: instead of execing `diff` in quiet, this should capture the differences and log them so we can show a report to the user
    def _check_testcase(self, output: TestFile) -> bool:
        exp = self.config.expected_file
        ret = TestInstance._execute(["diff", "-q", exp.getPath(), output.getPath()])
        return ret == 0

    # TODO: needs a serious refactor
    def run(self, env: Dict[str, Any]) -> TestResult:
        """run a test case and generate the result"""

        (build_success, build_object) = self._build_testcase(env)
        if not build_success:
            return TestResult(self.config, build_object[0], None, [build_object[1]])
        build_output = build_object[0]
        executable = build_object[1]

        (execute_success, execute_object) = self._execute_testcase(executable, env)
        if not execute_success:
            return TestResult(self.config, build_output, None, [execute_object])

        execute_output = execute_object

        success = self._check_testcase(execute_output)
        if not success:
            return TestResult(
                self.config,
                build_output,
                execute_output,
                ["Output did not match"],
            )

        return TestResult(self.config, build_output, execute_output, [])


@dataclass
class TestResult:
    test: TestConfiguration

    compilation_output: Union[TestFile, None]
    execution_output: Union[TestFile, None]

    msgs: List[str] = field(default_factory=list)

    def name(self):
        return self.test.name()

    def isSuccess(self):
        return (
            self.compilation_output is not None
            and self.execution_output is not None
            and len(self.msgs) == 0
        )

    def clean(self):
        if self.compilation_output:
            self.compilation_output.clean()
        if self.execution_output:
            self.execution_output.clean()
        if self.test.expected_file:
            self.test.expected_file.clean()
        if self.test.source_file:
            self.test.source_file.clean()
