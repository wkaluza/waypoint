import argparse
import contextlib
import dataclasses
import enum
import json
import multiprocessing
import os
import pathlib
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time
import typing

TIME_START = time.time_ns()

PYTHON = (
    "python3" if (sys.executable is None or sys.executable == "") else sys.executable
)

THIS_SCRIPT_DIR = os.path.realpath(os.path.dirname(__file__))
PROJECT_ROOT_DIR = os.path.realpath(f"{THIS_SCRIPT_DIR}/..")

COVERAGE_DIR_LCOV = os.path.realpath(f"{PROJECT_ROOT_DIR}/coverage_lcov___")
COVERAGE_FILE_LCOV = os.path.realpath(f"{COVERAGE_DIR_LCOV}/coverage.info")
COVERAGE_DIR_GCOVR = os.path.realpath(f"{PROJECT_ROOT_DIR}/coverage_gcovr___")
COVERAGE_FILE_HTML_GCOVR = os.path.realpath(f"{COVERAGE_DIR_GCOVR}/index.html")
COVERAGE_FILE_JSON_GCOVR = os.path.realpath(f"{COVERAGE_DIR_GCOVR}/coverage.json")
INFRASTRUCTURE_DIR = os.path.realpath(f"{PROJECT_ROOT_DIR}/infrastructure")
CMAKE_SOURCE_DIR = INFRASTRUCTURE_DIR
assert os.path.isfile(f"{INFRASTRUCTURE_DIR}/CMakeLists.txt") and os.path.isfile(
    f"{INFRASTRUCTURE_DIR}/CMakePresets.json"
)

JOBS = os.process_cpu_count()


@dataclasses.dataclass(frozen=True)
class ModeConfig:
    clean: bool = False
    format: bool = False
    clang: bool = False
    gcc: bool = False
    debug: bool = False
    release: bool = False
    static_analysis: bool = False
    test: bool = False
    valgrind: bool = False
    coverage: bool = False


@enum.unique
class Mode(enum.Enum):
    Fast = ModeConfig(
        clang=True,
        debug=True,
        test=True,
    )
    Format = ModeConfig(
        format=True,
    )
    Full = ModeConfig(
        format=True,
        clang=True,
        gcc=True,
        debug=True,
        release=True,
        static_analysis=True,
        test=True,
        valgrind=True,
        coverage=True,
    )
    Clean = ModeConfig(
        clean=True,
    )
    Verify = ModeConfig(
        clean=True,
        format=True,
        clang=True,
        gcc=True,
        debug=True,
        release=True,
        static_analysis=True,
        test=True,
        valgrind=True,
        coverage=True,
    )
    # Tool-specific modes
    Coverage = ModeConfig(
        gcc=True,
        debug=True,
        test=True,
        coverage=True,
    )
    StaticAnalysis = ModeConfig(
        clang=True,
        debug=True,
        release=True,
        static_analysis=True,
    )
    Valgrind = ModeConfig(
        clang=True,
        gcc=True,
        debug=True,
        valgrind=True,
    )

    def __init__(self, config):
        self.config = config

    def __str__(self):
        if self == Mode.Clean:
            return "clean"
        elif self == Mode.Fast:
            return "fast"
        elif self == Mode.Format:
            return "format"
        elif self == Mode.Full:
            return "full"
        elif self == Mode.Verify:
            return "verify"

        assert False, "This should not happen"

    @property
    def clean(self):
        return self.config.clean

    @property
    def format(self):
        return self.config.format

    @property
    def clang(self):
        return self.config.clang

    @property
    def gcc(self):
        return self.config.gcc

    @property
    def debug(self):
        return self.config.debug

    @property
    def release(self):
        return self.config.release

    @property
    def static_analysis(self):
        return self.config.static_analysis

    @property
    def test(self):
        return self.config.test

    @property
    def valgrind(self):
        return self.config.valgrind

    @property
    def coverage(self):
        return self.config.coverage


@enum.unique
class CMakePresets(enum.Enum):
    LinuxClang = ("configure_linux_clang", "build_linux_clang", "test_linux_clang")
    LinuxGcc = ("configure_linux_gcc", "build_linux_gcc", "test_linux_gcc")
    LinuxGccCoverage = (
        "configure_linux_gcc_coverage",
        "build_linux_gcc_coverage",
        "test_linux_gcc_coverage",
    )

    def __init__(self, configure_preset, build_preset, test_preset):
        self._configure_preset = configure_preset
        self._build_preset = build_preset
        self._test_preset = test_preset

    @property
    def configure(self):
        return self._configure_preset

    @property
    def build(self):
        return self._build_preset

    @property
    def test(self):
        return self._test_preset


@enum.unique
class CMakeBuildConfig(enum.Enum):
    Debug = ("Debug",)
    RelWithDebInfo = ("RelWithDebInfo",)
    Release = ("Release",)

    def __init__(self, config_name_):
        self._config_name = config_name_

    def __str__(self):
        return self._config_name


class NewEnv:
    def __init__(self, env):
        self.backup = os.environ.copy()
        self.env = env

    def __enter__(self):
        os.environ.clear()
        os.environ.update(self.env)

        return None

    def __exit__(self, exc_type, exc_val, exc_tb):
        os.environ.clear()
        os.environ.update(self.backup)

        return False


def ns_to_string(nanos) -> str:
    if nanos > 10**9:
        return f"{round(nanos / 10**9, 1)}s"
    elif nanos > 10**6:
        return f"{round(nanos / 10**6, 1)}ms"
    elif nanos > 10**3:
        return f"{round(nanos / 10**3, 1)}us"

    return f"{nanos}ns"


def run(cmd) -> bool:
    with tempfile.TemporaryFile("r+") as f:
        result = subprocess.run(cmd, stdout=f, stderr=f)

        if result.returncode == 0:
            return True

        print("")
        f.seek(0)
        print(f.read())

        return False


def is_linux():
    return platform.system() == "Linux"


def build_dir_from_preset(preset) -> str:
    presets_file = os.path.realpath(f"{CMAKE_SOURCE_DIR}/CMakePresets.json")

    with open(presets_file) as f:
        data = json.load(f)
        configure_presets = [
            p for p in data["configurePresets"] if p["name"] == preset.configure
        ]
        assert len(configure_presets) == 1
        configure_presets = configure_presets[0]

        binary_dir = configure_presets["binaryDir"]
        binary_dir.replace("${sourceDir}", f"{CMAKE_SOURCE_DIR}")

        return os.path.realpath(binary_dir)


def remove_dir(path):
    if os.path.exists(path) and os.path.isdir(path):
        shutil.rmtree(path)


def create_dir(path) -> bool:
    if os.path.exists(path) and not os.path.isdir(path):
        return False

    pathlib.Path(path).mkdir(parents=True, exist_ok=True)

    return True


def clean_build_dir(preset):
    build_dir = build_dir_from_preset(preset)
    remove_dir(build_dir)


def find_files_by_name(pred):
    output = []
    for root, dirs, files in os.walk(PROJECT_ROOT_DIR):
        dir_indices_to_remove = []
        for i, d in enumerate(dirs):
            if d.startswith("."):
                dir_indices_to_remove.append(i)
                continue
            if "___" in d:
                dir_indices_to_remove.append(i)
                continue
        dir_indices_to_remove.sort()
        dir_indices_to_remove.reverse()
        for i in dir_indices_to_remove:
            dirs.pop(i)

        for f in files:
            path = os.path.realpath(os.path.join(root, f))
            if pred(path):
                output.append(path)

    output.sort()

    return output


def configure_cmake(preset) -> bool:
    build_dir = build_dir_from_preset(preset)

    if os.path.exists(build_dir):
        return True

    os.mkdir(build_dir)

    with contextlib.chdir(CMAKE_SOURCE_DIR):
        return run(["cmake", "--preset", f"{preset.configure}"])


def build_cmake(config, preset) -> bool:
    with contextlib.chdir(CMAKE_SOURCE_DIR):
        print(f" {config}", end="")
        sys.stdout.flush()

        return run(
            [
                "cmake",
                "--build",
                "--preset",
                f"{preset.build}",
                "--config",
                f"{config}",
                "--parallel",
                f"{JOBS}",
            ]
        )


def run_ctest(preset, build_config, jobs, label_include_regex) -> bool:
    with contextlib.chdir(CMAKE_SOURCE_DIR):
        print(f" {build_config}", end="")
        sys.stdout.flush()

        return run(
            [
                "ctest",
                "--preset",
                preset.test,
                "--build-config",
                f"{build_config}",
                "--parallel",
                f"{jobs}",
                "--label-regex",
                label_include_regex,
            ]
        )


def clang_tidy_process_single_file(data) -> typing.Tuple[bool, str, float, str | None]:
    file, build_dir = data

    with tempfile.TemporaryFile("r+") as f:
        start_time = time.time_ns()
        result = subprocess.run(
            [
                "clang-tidy-20",
                f"--config-file={INFRASTRUCTURE_DIR}/.clang-tidy-20",
                "-p",
                build_dir,
                file,
            ],
            stdout=f,
            stderr=f,
        )
        duration = time.time_ns() - start_time

        if result.returncode == 0:
            return True, file, duration, None

        f.seek(0)
        output = f.read()

        return (
            False,
            file,
            duration,
            output,
        )


def run_clang_tidy(preset) -> bool:
    build_dir = build_dir_from_preset(preset)

    files = find_files_by_name(is_cpp_file)

    inputs = [(f, build_dir) for f in files]
    start_time = time.time_ns()
    with multiprocessing.Pool(JOBS) as pool:
        results = pool.map(clang_tidy_process_single_file, inputs)

        errors = [
            (file, stdout) for success, file, duration, stdout in results if not success
        ]
        if len(errors) > 0:
            for f, stdout in errors:
                print("----------")
                print(f"Error running clang-tidy on {f}")
                print(stdout)
                print("----------")

            return False

        durations = [duration for success, file, duration, stdout in results]
        avg_duration = sum(durations) / len(durations)

        # Select files taking more than X seconds
        threshold_duration_ns = 10**9
        results = [
            (file, duration)
            for success, file, duration, stdout in results
            if duration > threshold_duration_ns
        ]
        # Sort by descending duration
        results = sorted(results, reverse=True, key=lambda x: x[1])

        # Only print X worst offenders
        max_results = min(5, len(results))
        for file, duration in reversed(results[0:max_results]):
            print(f"Info: {file} took", ns_to_string(duration), "to analyse")

    print("Static analysis duration:", ns_to_string(time.time_ns() - start_time))
    print(f"Average duration per file: {ns_to_string(avg_duration)}")

    return True


def run_tests(mode, preset) -> bool:
    jobs = JOBS
    regex = r"^test$"

    if mode.debug:
        success = run_ctest(preset, CMakeBuildConfig.Debug, jobs, regex)
        if not success:
            return False

    if mode.release:
        success = run_ctest(preset, CMakeBuildConfig.RelWithDebInfo, jobs, regex)
        if not success:
            return False
        success = run_ctest(preset, CMakeBuildConfig.Release, jobs, regex)
        if not success:
            return False

    return True


def run_valgrind(mode, preset) -> bool:
    regex = r"^valgrind$"

    if mode.debug:
        success = run_ctest(preset, CMakeBuildConfig.Debug, JOBS, regex)
        if not success:
            return False

    return True


def run_lcov(build_dir) -> bool:
    success = create_dir(COVERAGE_DIR_LCOV)
    if not success:
        print(f"Failed to create {COVERAGE_DIR_LCOV}")
        return False

    with contextlib.chdir(PROJECT_ROOT_DIR):
        success = run(
            [
                "lcov",
                "--branch-coverage",
                "--capture",
                "--directory",
                build_dir,
                "--exclude",
                f"{PROJECT_ROOT_DIR}/test/",
                "--function-coverage",
                "--ignore-errors",
                "inconsistent",
                "--include",
                PROJECT_ROOT_DIR,
                "--output-file",
                COVERAGE_FILE_LCOV,
            ]
        )
        if not success:
            print("Error running lcov")
            return False

        success = run(
            [
                "genhtml",
                "--branch-coverage",
                "--dark-mode",
                "--flat",
                "--function-coverage",
                "--ignore-errors",
                "inconsistent",
                "--legend",
                "--output-directory",
                COVERAGE_DIR_LCOV,
                "--show-zero-columns",
                "--sort",
                COVERAGE_FILE_LCOV,
            ]
        )
        if not success:
            print("Error running genhtml")
            return False

    return True


def run_gcovr(build_dir) -> bool:
    success = create_dir(COVERAGE_DIR_GCOVR)
    if not success:
        print(f"Failed to create {COVERAGE_DIR_GCOVR}")
        return False

    with contextlib.chdir(PROJECT_ROOT_DIR):
        success = run(
            [
                "gcovr",
                "--decisions",
                "--exclude",
                f"{PROJECT_ROOT_DIR}/test/",
                "--filter",
                PROJECT_ROOT_DIR,
                "--gcov-object-directory",
                build_dir,
                "--html-details",
                COVERAGE_FILE_HTML_GCOVR,
                "--html-theme",
                "github.dark-green",
                "--json-summary",
                COVERAGE_FILE_JSON_GCOVR,
                "--json-summary-pretty",
                "--root",
                PROJECT_ROOT_DIR,
                "--sort",
                "uncovered-percent",
                "--verbose",
            ]
        )
        if not success:
            print("Error running gcovr")
            return False

    return True


def run_coverage(preset) -> bool:
    build_dir = build_dir_from_preset(preset)

    success = run_lcov(build_dir)
    if not success:
        return False

    success = run_gcovr(build_dir)
    if not success:
        return False

    return True


def check_coverage() -> bool:
    with open(COVERAGE_FILE_JSON_GCOVR, "r") as f:
        data = json.load(f)

    all_branches_covered = data["branch_covered"] == data["branch_total"]
    all_decisions_covered = data["decision_covered"] == data["decision_total"]
    all_functions_covered = data["function_covered"] == data["function_total"]
    all_lines_covered = data["line_covered"] == data["line_total"]

    if (
        not all_branches_covered
        or not all_decisions_covered
        or not all_functions_covered
        or not all_lines_covered
    ):
        print("Incomplete coverage")
        return False

    return True


def _linux_compile(mode, preset, env_patch):
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        success = configure_cmake(preset)
        if not success:
            return False

        if mode.debug:
            success = build_cmake(CMakeBuildConfig.Debug, preset)
            if not success:
                return False

        if mode.release:
            success = build_cmake(CMakeBuildConfig.RelWithDebInfo, preset)
            if not success:
                return False
            success = build_cmake(CMakeBuildConfig.Release, preset)
            if not success:
                return False

    return True


def build_gcc_linux(mode, preset) -> bool:
    return _linux_compile(mode, preset, {"CC": "gcc-15", "CXX": "g++-15"})


def build_clang_linux(mode, preset) -> bool:
    return _linux_compile(mode, preset, {"CC": "clang-20", "CXX": "clang++-20"})


def is_json_file(f) -> bool:
    return re.search(r"\.json$", f) is not None


def is_python_file(f) -> bool:
    return re.search(r"\.py$", f) is not None


def is_cmake_file(f) -> bool:
    return (
        re.search(r"CMakeLists\.txt$", f) is not None
        or re.search(r"\.cmake$", f) is not None
    )


def is_cpp_file(f) -> bool:
    return re.search(r"\.cpp$", f) is not None or re.search(r"\.hpp$", f) is not None


def format_single_file(f) -> typing.Tuple[bool, str]:
    if is_python_file(f):
        return format_python(f), f
    elif is_cmake_file(f):
        return format_cmake(f), f
    elif is_json_file(f):
        return format_json(f), f
    elif is_cpp_file(f):
        return format_cpp(f), f

    return False, f


def format_files() -> bool:
    files = find_files_by_name(is_json_file)
    files += find_files_by_name(is_cmake_file)
    files += find_files_by_name(is_python_file)
    files += find_files_by_name(is_cpp_file)
    files.sort()

    with multiprocessing.Pool(JOBS) as pool:
        results = pool.map(format_single_file, files)
        errors = [file for success, file in results if not success]
        if len(errors) > 0:
            for f in errors:
                print(f"Error formatting file {f}")

            return False

    return True


def format_json(f) -> bool:
    with open(f, "r") as handle:
        original = handle.read()
    with open(f, "r") as handle:
        data = json.load(handle)

    data_str = json.dumps(data, indent=2, sort_keys=True)
    data_str += "\n"
    if data_str != original:
        with open(f, "w") as handle:
            handle.write(data_str)

    return True


def format_cmake(f) -> bool:
    return run(["cmake-format", "-i", f])


def format_python(f) -> bool:
    success = run([PYTHON, "-m", "isort", "--quiet", "--line-length", "88", f])
    if not success:
        return False

    return run(["black", "--quiet", "--line-length", "88", f])


def format_cpp(f) -> bool:
    path_to_config = os.path.realpath(f"{INFRASTRUCTURE_DIR}/.clang-format-20")

    return run(
        [
            "clang-format-20",
            f"--style=file:{path_to_config}",
            "-i",
            f,
        ]
    )


class CliConfig:
    def __init__(self, mode_str):
        self.mode = None

        if mode_str == f"{Mode.Clean}":
            self.mode = Mode.Clean
        if mode_str == f"{Mode.Fast}":
            self.mode = Mode.Fast
        if mode_str == f"{Mode.Format}":
            self.mode = Mode.Format
        if mode_str == f"{Mode.Full}":
            self.mode = Mode.Full
        if mode_str == f"{Mode.Verify}":
            self.mode = Mode.Verify

        assert self.mode is not None


def preamble() -> tuple[CliConfig | None, bool]:
    if not is_linux():
        print(f"Unknown OS: {platform.system()}")
        return None, False

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "mode",
        choices=[
            f"{Mode.Clean}",
            f"{Mode.Fast}",
            f"{Mode.Format}",
            f"{Mode.Full}",
            f"{Mode.Verify}",
        ],
        metavar="mode",
        help=f"""Selects build mode.
                 Mode "{Mode.Clean}" deletes the build trees.
                 Mode "{Mode.Format}" formats source files.
                 Mode "{Mode.Fast}" runs one build and the tests for quick
                 iterations.
                 Mode "{Mode.Full}" builds everything and runs all tools.
                 Mode "{Mode.Verify}" runs "{Mode.Clean}" followed by
                 "{Mode.Full}".""",
    )

    parsed = parser.parse_args()

    config = CliConfig(parsed.mode)

    return config, True


def main() -> int:
    config, success = preamble()
    if not success:
        return 1

    mode = config.mode

    if mode.clean:
        print("Deleting build files...")
        clean_build_dir(CMakePresets.LinuxClang)
        clean_build_dir(CMakePresets.LinuxGcc)
        clean_build_dir(CMakePresets.LinuxGccCoverage)
        remove_dir(COVERAGE_DIR_GCOVR)
        remove_dir(COVERAGE_DIR_LCOV)

    if mode.format:
        print("Formatting files...")
        success = format_files()
        if not success:
            return 1

    if mode.gcc:
        print("Running GCC build...", end="")
        sys.stdout.flush()
        success = build_gcc_linux(mode, CMakePresets.LinuxGcc)
        if not success:
            print("")
            return 1
        print("")

        if mode.test:
            print("Testing GCC build...", end="")
            sys.stdout.flush()
            success = run_tests(mode, CMakePresets.LinuxGcc)
            if not success:
                print("")
                return 1
            print("")

    if mode.clang:
        print("Running Clang build...", end="")
        sys.stdout.flush()
        success = build_clang_linux(mode, CMakePresets.LinuxClang)
        if not success:
            print("")
            return 1
        print("")

        if mode.test:
            print("Testing Clang build...", end="")
            sys.stdout.flush()
            success = run_tests(mode, CMakePresets.LinuxClang)
            if not success:
                print("")
                return 1
            print("")

    if mode.coverage:
        mode_coverage = Mode.Coverage
        assert mode_coverage.debug
        assert not mode_coverage.release

        print("Building coverage...", end="")
        sys.stdout.flush()
        success = build_gcc_linux(mode_coverage, CMakePresets.LinuxGccCoverage)
        if not success:
            print("")
            return 1
        print("")

        print("Running coverage tests...", end="")
        sys.stdout.flush()
        success = run_tests(mode_coverage, CMakePresets.LinuxGccCoverage)
        if not success:
            print("")
            return 1
        print("")

        print("Processing coverage data...")
        success = run_coverage(CMakePresets.LinuxGccCoverage)
        if not success:
            return 1
        success = check_coverage()
        if not success:
            return 1

    if mode.valgrind:
        # Test debug with Valgrind, release may yield false positives
        mode_valgrind = Mode.Valgrind
        assert mode_valgrind.debug
        assert not mode_valgrind.release

        print("Building GCC for Valgrind...", end="")
        sys.stdout.flush()
        success = build_gcc_linux(mode_valgrind, CMakePresets.LinuxGcc)
        if not success:
            print("")
            return 1
        print("")

        print("Testing GCC build with Valgrind...", end="")
        sys.stdout.flush()
        success = run_valgrind(mode_valgrind, CMakePresets.LinuxGcc)
        if not success:
            print("")
            return 1
        print("")

        print("Building Clang for Valgrind...", end="")
        sys.stdout.flush()
        success = build_clang_linux(mode_valgrind, CMakePresets.LinuxClang)
        if not success:
            print("")
            return 1
        print("")

        print("Testing Clang build with Valgrind...", end="")
        sys.stdout.flush()
        success = run_valgrind(mode_valgrind, CMakePresets.LinuxClang)
        if not success:
            print("")
            return 1
        print("")

    if mode.static_analysis:
        # Need to build all configs for static analysis
        mode_static_analysis = Mode.StaticAnalysis
        assert mode_static_analysis.debug
        assert mode_static_analysis.release

        print("Building Clang for static analysis...", end="")
        sys.stdout.flush()
        success = build_clang_linux(mode_static_analysis, CMakePresets.LinuxClang)
        if not success:
            print("")
            return 1
        print("")

        print("Running clang-tidy analysis...")
        success = run_clang_tidy(CMakePresets.LinuxClang)
        if not success:
            return 1

    print("Build duration: ", ns_to_string(time.time_ns() - TIME_START))
    print(f"Success: {os.path.basename(sys.argv[0])}")

    return 0


if __name__ == "__main__":
    exit(main())
