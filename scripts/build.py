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


CLANG20_ENV_PATCH = {"CC": "clang-20", "CXX": "clang++-20"}
GCC15_ENV_PATCH = {"CC": "gcc-15", "CXX": "g++-15"}


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
    misc: bool = False


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
        misc=True,
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
        misc=True,
    )
    Coverage = ModeConfig(
        coverage=True,
    )
    StaticAnalysis = ModeConfig(
        static_analysis=True,
    )
    Valgrind = ModeConfig(
        valgrind=True,
    )

    def __init__(self, config):
        self.config = config

    def __str__(self):
        if self == Mode.Clean:
            return "clean"
        if self == Mode.Coverage:
            return "coverage"
        elif self == Mode.Fast:
            return "fast"
        elif self == Mode.Format:
            return "format"
        elif self == Mode.Full:
            return "full"
        elif self == Mode.StaticAnalysis:
            return "static"
        elif self == Mode.Valgrind:
            return "valgrind"
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

    @property
    def misc(self):
        return self.config.misc


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


def run(cmd) -> typing.Tuple[bool, str | None]:
    with tempfile.TemporaryFile("r+") as f:
        try:
            result = subprocess.run(cmd, stdout=f, stderr=f)
        except FileNotFoundError:
            return False, None

        output = "\n"
        f.seek(0)
        output += f.read()
        output += "\n"

        return (result.returncode == 0), output


def is_linux():
    return platform.system() == "Linux"


def check_waypoint_hpp_has_no_includes_() -> bool:
    path = f"{PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp"
    with open(path, "r") as f:
        contents = f.read()

    return re.search(r"# *include", contents) is None


def misc_checks_fn() -> bool:
    success = check_waypoint_hpp_has_no_includes_()
    if not success:
        print("Error: waypoint.hpp must include no other headers")

        return False

    return True


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


def configure_cmake_clang_fn() -> bool:
    return configure_cmake(CMakePresets.LinuxClang, CLANG20_ENV_PATCH)


def configure_cmake_gcc_fn() -> bool:
    return configure_cmake(CMakePresets.LinuxGcc, GCC15_ENV_PATCH)


def configure_cmake(preset, env_patch) -> bool:
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        build_dir = build_dir_from_preset(preset)

        if os.path.exists(build_dir):
            return True

        os.mkdir(build_dir)

        with contextlib.chdir(CMAKE_SOURCE_DIR):
            success, output = run(["cmake", "--preset", f"{preset.configure}"])
            if not success:
                if output is not None:
                    print(output)

                return False

    return True


def build_cmake(config, preset) -> bool:
    with contextlib.chdir(CMAKE_SOURCE_DIR):
        success, output = run(
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
        if not success:
            if output is not None:
                print(output)

            return False

        return True


def run_ctest(preset, build_config, jobs, label_include_regex) -> bool:
    with contextlib.chdir(CMAKE_SOURCE_DIR):
        success, output = run(
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
        if not success:
            if output is not None:
                print(output)

            return False

        return True


def clang_tidy_process_single_file(data) -> typing.Tuple[bool, str, float, str | None]:
    file, build_dir = data

    start_time = time.time_ns()
    success, output = run(
        [
            "clang-tidy-20",
            f"--config-file={INFRASTRUCTURE_DIR}/.clang-tidy-20",
            "-p",
            build_dir,
            file,
        ]
    )
    duration = time.time_ns() - start_time

    return (
        success,
        file,
        duration,
        None if success else (None if output is None else output.strip()),
    )


def run_clang_static_analysis_all_files_fn() -> bool:
    return run_clang_tidy(CMakePresets.LinuxClang, find_files_by_name(is_cpp_file))


def run_clang_tidy(preset, files) -> bool:
    build_dir = build_dir_from_preset(preset)

    inputs = [(f, build_dir) for f in files]
    if len(files) == 0:
        print("No files to analyze")

        return True

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

    return True


def test_gcc_debug_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(CMakePresets.LinuxGcc, CMakeBuildConfig.Debug, jobs, regex)


def test_gcc_relwithdebinfo_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(
        CMakePresets.LinuxGcc, CMakeBuildConfig.RelWithDebInfo, jobs, regex
    )


def test_gcc_release_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(CMakePresets.LinuxGcc, CMakeBuildConfig.Release, jobs, regex)


def test_clang_debug_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(CMakePresets.LinuxClang, CMakeBuildConfig.Debug, jobs, regex)


def test_clang_relwithdebinfo_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(
        CMakePresets.LinuxClang, CMakeBuildConfig.RelWithDebInfo, jobs, regex
    )


def test_clang_release_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(CMakePresets.LinuxClang, CMakeBuildConfig.Release, jobs, regex)


def test_gcc_valgrind_fn() -> bool:
    jobs = JOBS
    regex = r"^valgrind$"

    return run_ctest(CMakePresets.LinuxGcc, CMakeBuildConfig.Debug, jobs, regex)


def test_clang_valgrind_fn() -> bool:
    jobs = JOBS
    regex = r"^valgrind$"

    return run_ctest(CMakePresets.LinuxClang, CMakeBuildConfig.Debug, jobs, regex)


def run_lcov(build_dir) -> bool:
    success = create_dir(COVERAGE_DIR_LCOV)
    if not success:
        print(f"Failed to create {COVERAGE_DIR_LCOV}")
        return False

    with contextlib.chdir(PROJECT_ROOT_DIR):
        success, output = run(
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
            if output is not None:
                print(output)

            return False

        success, output = run(
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
            if output is not None:
                print(output)

            return False

    return True


def run_gcovr(build_dir) -> bool:
    success = create_dir(COVERAGE_DIR_GCOVR)
    if not success:
        print(f"Failed to create {COVERAGE_DIR_GCOVR}")
        return False

    with contextlib.chdir(PROJECT_ROOT_DIR):
        success, output = run(
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
            if output is not None:
                print(output)

            return False

    return True


def process_coverage_fn() -> bool:
    build_dir = build_dir_from_preset(CMakePresets.LinuxGccCoverage)

    success = run_lcov(build_dir)
    if not success:
        return False

    success = run_gcovr(build_dir)
    if not success:
        return False

    return True


def configure_cmake_gcc_coverage_fn() -> bool:
    return configure_cmake(CMakePresets.LinuxGccCoverage, GCC15_ENV_PATCH)


def build_gcc_coverage_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.Debug, CMakePresets.LinuxGccCoverage, GCC15_ENV_PATCH
    )


def test_gcc_coverage_fn() -> bool:
    jobs = JOBS
    regex = r"^test$"

    return run_ctest(CMakePresets.LinuxGccCoverage, CMakeBuildConfig.Debug, jobs, regex)


def analyze_gcc_coverage_fn() -> bool:
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


def _linux_compile(config, preset, env_patch):
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        success = build_cmake(config, preset)
        if not success:
            return False

    return True


def build_clang_debug_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.Debug, CMakePresets.LinuxClang, CLANG20_ENV_PATCH
    )


def build_clang_relwithdebinfo_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.RelWithDebInfo, CMakePresets.LinuxClang, CLANG20_ENV_PATCH
    )


def build_clang_release_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.Release, CMakePresets.LinuxClang, CLANG20_ENV_PATCH
    )


def build_gcc_debug_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.Debug, CMakePresets.LinuxGcc, GCC15_ENV_PATCH
    )


def build_gcc_relwithdebinfo_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.RelWithDebInfo, CMakePresets.LinuxGcc, GCC15_ENV_PATCH
    )


def build_gcc_release_fn() -> bool:
    return _linux_compile(
        CMakeBuildConfig.Release, CMakePresets.LinuxGcc, GCC15_ENV_PATCH
    )


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


def format_sources_fn() -> bool:
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
    success, output = run(["cmake-format", "-i", f])
    if not success:
        if output is not None:
            print(output)

        return False

    return True


def format_python(f) -> bool:
    success, output = run([PYTHON, "-m", "isort", "--quiet", "--line-length", "88", f])
    if not success:
        if output is not None:
            print(output)

        return False

    success, output = run(["black", "--quiet", "--line-length", "88", f])
    if not success:
        if output is not None:
            print(output)

        return False

    return True


def format_cpp(f) -> bool:
    path_to_config = os.path.realpath(f"{INFRASTRUCTURE_DIR}/.clang-format-20")

    success, output = run(
        [
            "clang-format-20",
            f"--style=file:{path_to_config}",
            "-i",
            f,
        ]
    )
    if not success:
        if output is not None:
            print(output)

        return False

    return True


class CliConfig:
    def __init__(self, mode_str):
        self.mode = None

        if mode_str == f"{Mode.Clean}":
            self.mode = Mode.Clean
        if mode_str == f"{Mode.Coverage}":
            self.mode = Mode.Coverage
        if mode_str == f"{Mode.Fast}":
            self.mode = Mode.Fast
        if mode_str == f"{Mode.Format}":
            self.mode = Mode.Format
        if mode_str == f"{Mode.Full}":
            self.mode = Mode.Full
        if mode_str == f"{Mode.StaticAnalysis}":
            self.mode = Mode.StaticAnalysis
        if mode_str == f"{Mode.Valgrind}":
            self.mode = Mode.Valgrind
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
            f"{Mode.Coverage}",
            f"{Mode.Fast}",
            f"{Mode.Format}",
            f"{Mode.Full}",
            f"{Mode.StaticAnalysis}",
            f"{Mode.Valgrind}",
            f"{Mode.Verify}",
        ],
        metavar="mode",
        help=f"""Selects build mode.
                 Mode "{Mode.Clean}" deletes the build trees.
                 Mode "{Mode.Coverage}" measures coverage.
                 Mode "{Mode.Format}" formats source files.
                 Mode "{Mode.Fast}" runs one build and the tests for quick
                 iterations.
                 Mode "{Mode.Full}" builds everything and runs all tools.
                 Mode "{Mode.StaticAnalysis}" performs static analysis.
                 Mode "{Mode.Valgrind}" runs Valgrind/memcheck.
                 Mode "{Mode.Verify}" runs "{Mode.Clean}" followed by "{Mode.Full}".""",
    )

    parsed = parser.parse_args()

    config = CliConfig(parsed.mode)

    return config, True


def clean_fn() -> bool:
    clean_build_dir(CMakePresets.LinuxClang)
    clean_build_dir(CMakePresets.LinuxGcc)
    clean_build_dir(CMakePresets.LinuxGccCoverage)
    remove_dir(COVERAGE_DIR_GCOVR)
    remove_dir(COVERAGE_DIR_LCOV)

    return True


def noop_fn() -> bool:
    return True


class Task:
    def __init__(self, name: str = None, fn: typing.Callable[[], bool] = noop_fn):
        self.name_ = name
        self.fn_ = fn
        self.task_attempted_ = False
        self.deps_success_ = False
        self.success_ = False
        self.dependencies_ = []

    def depends_on(self, deps: typing.List["Task"]):
        for d in deps:
            self.dependencies_.append(d)

    def run(self) -> bool:
        if self.task_attempted_ and not (self.deps_success_ and self.success_):
            return False

        if self.task_attempted_ and self.deps_success_ and self.success_:
            return True

        self.task_attempted_ = True

        start_deps = time.time_ns()

        if len(self.dependencies_) > 0 and self.name_ is not None:
            print(f"Preparing task: {self.name_}")
        for d in self.dependencies_:
            success = d.run()
            if not success:
                return False

        self.deps_success_ = True

        if self.name_ is not None:
            print(f"Running task: {self.name_}")
        start = time.time_ns()
        success = self.fn_()
        if self.name_ is not None:
            if len(self.dependencies_) > 0:
                print(
                    "Finished task:",
                    f"{self.name_} ({ns_to_string(time.time_ns() - start)},",
                    f"total: {ns_to_string(time.time_ns() - start_deps)})",
                )
            else:
                print(
                    "Finished task:",
                    f"{self.name_} ({ns_to_string(time.time_ns() - start)})",
                )
        if not success:
            print("Error running task", f"{self.name_}")

            return False

        self.success_ = True

        return True


def main() -> int:
    config, success = preamble()
    if not success:
        return 1

    mode = config.mode

    test_clang_debug = Task("Test Clang Debug", test_clang_debug_fn)
    test_clang_relwithdebinfo = Task(
        "Test Clang RelWithDebInfo", test_clang_relwithdebinfo_fn
    )
    test_clang_release = Task("Test Clang Release", test_clang_release_fn)
    build_clang_debug = Task("Build Clang Debug", build_clang_debug_fn)
    build_clang_relwithdebinfo = Task(
        "Build Clang RelWithDebInfo", build_clang_relwithdebinfo_fn
    )
    build_clang_release = Task("Build Clang Release", build_clang_release_fn)

    configure_cmake_clang = Task("Configure CMake for Clang", configure_cmake_clang_fn)

    build_clang_debug.depends_on([configure_cmake_clang])
    build_clang_relwithdebinfo.depends_on([configure_cmake_clang])
    build_clang_release.depends_on([configure_cmake_clang])
    test_clang_debug.depends_on([build_clang_debug])
    test_clang_relwithdebinfo.depends_on([build_clang_relwithdebinfo])
    test_clang_release.depends_on([build_clang_release])

    test_gcc_debug = Task("Test GCC Debug", test_gcc_debug_fn)
    test_gcc_relwithdebinfo = Task(
        "Test GCC RelWithDebInfo", test_gcc_relwithdebinfo_fn
    )
    test_gcc_release = Task("Test GCC Release", test_gcc_release_fn)
    build_gcc_debug = Task("Build GCC Debug", build_gcc_debug_fn)
    build_gcc_relwithdebinfo = Task(
        "Build GCC RelWithDebInfo", build_gcc_relwithdebinfo_fn
    )
    build_gcc_release = Task("Build GCC Release", build_gcc_release_fn)

    configure_cmake_gcc = Task("Configure CMake for GCC", configure_cmake_gcc_fn)

    build_gcc_debug.depends_on([configure_cmake_gcc])
    build_gcc_relwithdebinfo.depends_on([configure_cmake_gcc])
    build_gcc_release.depends_on([configure_cmake_gcc])
    test_gcc_debug.depends_on([build_gcc_debug])
    test_gcc_relwithdebinfo.depends_on([build_gcc_relwithdebinfo])
    test_gcc_release.depends_on([build_gcc_release])

    analyze_gcc_coverage = Task("Analyze GCC coverage data", analyze_gcc_coverage_fn)
    process_gcc_coverage = Task("Process GCC coverage data", process_coverage_fn)
    test_gcc_coverage = Task("Test GCC with coverage", test_gcc_coverage_fn)
    build_gcc_coverage = Task("Build GCC with coverage", build_gcc_coverage_fn)
    configure_cmake_gcc_coverage = Task(
        "Configure CMake for GCC with coverage", configure_cmake_gcc_coverage_fn
    )

    analyze_gcc_coverage.depends_on([process_gcc_coverage])
    process_gcc_coverage.depends_on([test_gcc_coverage])
    test_gcc_coverage.depends_on([build_gcc_coverage])
    build_gcc_coverage.depends_on([configure_cmake_gcc_coverage])

    configure_cmake_gcc_valgrind = Task("Configure CMake for GCC with Valgrind")
    configure_cmake_gcc_valgrind.depends_on([configure_cmake_gcc])
    build_gcc_valgrind = Task("Build GCC for Valgrind")
    build_gcc_valgrind.depends_on([build_gcc_debug])
    test_gcc_valgrind = Task("Test GCC build with Valgrind", test_gcc_valgrind_fn)
    test_gcc_valgrind.depends_on([build_gcc_valgrind])
    build_gcc_valgrind.depends_on([configure_cmake_gcc_valgrind])

    configure_cmake_clang_valgrind = Task("Configure CMake for Clang with Valgrind")
    configure_cmake_clang_valgrind.depends_on([configure_cmake_clang])
    build_clang_valgrind = Task("Build Clang for Valgrind")
    build_clang_valgrind.depends_on([build_clang_debug])
    test_clang_valgrind = Task("Test Clang build with Valgrind", test_clang_valgrind_fn)
    test_clang_valgrind.depends_on([build_clang_valgrind])
    build_clang_valgrind.depends_on([configure_cmake_clang_valgrind])

    configure_cmake_clang_static_analysis = Task("Configure CMake for clang-tidy")
    configure_cmake_clang_static_analysis.depends_on([configure_cmake_clang])
    build_clang_static_analysis = Task("Build Clang for clang-tidy")
    build_clang_static_analysis.depends_on(
        [
            configure_cmake_clang_static_analysis,
            build_clang_debug,
            build_clang_relwithdebinfo,
            build_clang_release,
        ]
    )
    run_clang_static_analysis = Task(
        "Run clang-tidy", run_clang_static_analysis_all_files_fn
    )
    run_clang_static_analysis.depends_on([build_clang_static_analysis])

    misc_checks = Task("Miscellaneous checks", misc_checks_fn)

    format_sources = Task("Format code", format_sources_fn)

    clean = Task("Clean build files", clean_fn)

    root_dependencies = []

    if mode.clean:
        root_dependencies.append(clean)

    if mode.format:
        root_dependencies.append(format_sources)

    if mode.gcc:
        if mode.debug:
            root_dependencies.append(build_gcc_debug)

            if mode.test:
                root_dependencies.append(test_gcc_debug)

        if mode.release:
            root_dependencies.append(build_gcc_relwithdebinfo)
            if mode.test:
                root_dependencies.append(test_gcc_relwithdebinfo)

            root_dependencies.append(build_gcc_release)
            if mode.test:
                root_dependencies.append(test_gcc_release)

    if mode.clang:
        if mode.debug:
            root_dependencies.append(build_clang_debug)

            if mode.test:
                root_dependencies.append(test_clang_debug)

        if mode.release:
            root_dependencies.append(build_clang_relwithdebinfo)
            if mode.test:
                root_dependencies.append(test_clang_relwithdebinfo)

            root_dependencies.append(build_clang_release)
            if mode.test:
                root_dependencies.append(test_clang_release)

    if mode.coverage:
        root_dependencies.append(analyze_gcc_coverage)

    if mode.valgrind:
        root_dependencies.append(test_gcc_valgrind)
        root_dependencies.append(test_clang_valgrind)

    if mode.static_analysis:
        root_dependencies.append(run_clang_static_analysis)

    if mode.misc:
        root_dependencies.append(misc_checks)

    root = Task()
    root.depends_on(root_dependencies)
    root.run()

    print(f"Build duration: {ns_to_string(time.time_ns() - TIME_START)}")
    print(f"Success: {os.path.basename(sys.argv[0])}")

    return 0


if __name__ == "__main__":
    exit(main())
