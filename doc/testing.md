# Testing

Zircon supports a custom testing framework that allows for flexible test cases with minimum boilerplate.
Tests are invoked by executing `./scripts/run-tests.sh`.
By default, this script will find and execute all tests in parallel under the `tests` directory.
After executing all the tests, the script will report all the tests it rain, whether they passed or failed, and a total number of passing/failing tests.

## Defining Tests

Tests are defined by one or more files in a directory with the same _basename_, the part between the last slash and the dot for the extension.
Defining only one file is sufficient, if others are needed they will be inferred.
If all the test case tested was that the simulator could load and execute a basic executable, an empty expected file would be enough to define the test.
The needed compilation and executions arguments would be inferred and a default source file would be supplied.

For a given test case, multiple sub tests can be defined.
A single source file can be paired with a execution arguments file which defines a set of arguments passed to the simulator on execution, one per line.
A two line execution argument file will result in two tests being run, one for each set of executions arguments.
They will be compared to the same expected output, unless a sub test is specified.
If an expected file for a sub test is defined, a arguments file can reference it by ending the line with `# <SUBTEST NAME>`.

Tests may require a specific toolchain to build the source file or a build of the simulator with specific arguments.
These can specified, one per line, in a `.toolchain` and `.sim` file, respectively.
Rather than specify arguments for how to build the toolchain or simulator, these list use variables defined in `tests/driver/config/TOOLCHAINS` and `tests/driver/config/COMPSIM`.
These files provide a central location to define build requirements, allowing them to function as tests in their own right.

### Default Source Files

The default C file contains `int main() {return 0;}`.
This is the default currently used if no source file is specified.
A default assembly file can be seen in `tests/driver/config/DEFAULT.s`, although it is not currently used.

## Test File Summary

| Filename             | Purpose                                     |
| -------------------- | ------------------------------------------- |
| TESTNAME.c           | C source file to be compiled and run        |
| TESTNAME.cpp         | C++ source file to be compiled and run      |
| TESTNAME.cc          | C++ source file to be compiled and run      |
| TESTNAME.s           | Assembly source file to be compiled and run |
| TESTNAME.asm         | Assembly source file to be compiled and run |
| TESTNAME.exp         | expected output of test                     |
| TESTNAME.SUBNAME.exp | expected output for a sub test of a test case             |
| TESTNAME.compopt | compilation options for the test case, each line represents a new sub test |
| TESTNAME.execopt | execution options for the test case, each line represents a new sub test |
| TESTNAME.toolchain | which toolchain configuration to use to build the test case |
| TESTNAME.sim | which simulator build configuration to use to run the test case |
| SKIP | if any directory contains a SKIP file, the driver will stop searching for tests in it |


Future files to be added
- TESTNAME.prediff
  - shell script that can modify the output of the test before it is compared against the expected output

## Arguments and Flags

Any directory or file passed as an argument to the test script will be treated as a search path for test files, replacing the default `tests`.
This allows only specific tests to be run or out-of-tree tests to be run

A number of flags can be passed to the script to customize the test run

| flag                                 | description                                                                           |
| ------------------------------------ | ------------------------------------------------------------------------------------- |
| -h, --help                           | show the help message and exit                                                        |
| --summary                            | show the final test summary report (DEFAULT)                                          |
| --no-summary                         | don't show the final test summary                                                     |
| --full-report                        | report each test cases result as it finishes                                          |
| --no-full-report                     | don't report each test cases result as it finishes (DEFAULT)                          |
| --test-build                         | build all simulator configurations, even those not needed to run tests (DEFAULT)      |
| --no-test-build                      | only build those simulator configurations needed to run the tests                     |
| --diff                               | show diff of output alongside pass/fail                                               |
| --no-diff                            | only show pass/fail for tests (DEFAULT)                                               |
| -x PATTERN, --exclude PATTERN        | exclude tests containing the pattern in their path                                    |
| -xr PATTERN, --exclude-regex PATTERN | exclude tests containing the regular expression based pattern in their path           |
| -j JOBS, --jobs JOBS                 | number of parallel tests to run, by default is number of physical CPUs                |
| -k, --keep                           | don't clean generated files after the test is run, useful for debugging failing tests |
| -v, --verbose                        | print all commands being executed                                                     |
| --color                              | override terminal detection and force color output                                    |
| --no-color                           | override terminal detection and force no color output                                 |

## Test Process

The drivers takes the following general tests

1. Identify all tests to run
   1. separates sub tests and configurations of a single test file into multiple tests
2. Identify and fulfil requirements of tests
   1. Do the tests need specific builds of the simulator
   2. Do the tests need specific toolchains (**FUTURE WORK**)
3. Run all tests, one per process, for as many processes as requested
   1. Collect the results
   2. Depending upon the flags, report test status
4. Print final test report
