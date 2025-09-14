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

PYTHON = (
    "python3" if (sys.executable is None or sys.executable == "") else sys.executable
)

THIS_SCRIPT_DIR = os.path.realpath(os.path.dirname(__file__))
PROJECT_ROOT_DIR = os.path.realpath(f"{THIS_SCRIPT_DIR}/..")

COVERAGE_DIR_LCOV = os.path.realpath(
    f"{PROJECT_ROOT_DIR}/coverage_lcov_3XxYQrYbuY5AnPYH___"
)
COVERAGE_FILE_LCOV = os.path.realpath(f"{COVERAGE_DIR_LCOV}/coverage.info")
COVERAGE_DIR_GCOVR = os.path.realpath(
    f"{PROJECT_ROOT_DIR}/coverage_gcovr_kMkR9SM1S69oCLJ5___"
)
COVERAGE_FILE_HTML_GCOVR = os.path.realpath(f"{COVERAGE_DIR_GCOVR}/index.html")
COVERAGE_FILE_JSON_GCOVR = os.path.realpath(f"{COVERAGE_DIR_GCOVR}/coverage.json")
INFRASTRUCTURE_DIR = os.path.realpath(f"{PROJECT_ROOT_DIR}/infrastructure")
CMAKE_SOURCE_DIR = INFRASTRUCTURE_DIR
CMAKE_LISTS_FILE = os.path.realpath(f"{CMAKE_SOURCE_DIR}/CMakeLists.txt")
CMAKE_PRESETS_FILE = os.path.realpath(f"{CMAKE_SOURCE_DIR}/CMakePresets.json")
assert os.path.isfile(CMAKE_LISTS_FILE) and os.path.isfile(CMAKE_PRESETS_FILE)

MAIN_HEADER_PATH = f"{PROJECT_ROOT_DIR}/src/waypoint/include/waypoint/waypoint.hpp"
assert os.path.isfile(MAIN_HEADER_PATH), "waypoint.hpp does not exist"

INSTALL_TESTS_DIR_PATH = os.path.realpath(f"{PROJECT_ROOT_DIR}/test/install_tests")
TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CLANG_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_no_version_test/waypoint_install_linux_clang_MItqq12bE9VvgzWH___"
)
TEST_INSTALL_FIND_PACKAGE_NO_VERSION_GCC_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_no_version_test/waypoint_install_linux_gcc_99V4LexZ8aO7qhLC___"
)
TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_no_version_test/infrastructure"
)
assert os.path.isfile(
    f"{TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR}/CMakeLists.txt"
) and os.path.isfile(
    f"{TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR}/CMakePresets.json"
)

TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CLANG_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_exact_version_test/waypoint_install_linux_clang_2dp6n2H9O8te806G___"
)
TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_GCC_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_exact_version_test/waypoint_install_linux_gcc_vo44y7Bxqbn3kKZA___"
)
TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/find_package_exact_version_test/infrastructure"
)
assert os.path.isfile(
    f"{TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR}/CMakeLists.txt"
) and os.path.isfile(
    f"{TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR}/CMakePresets.json"
)

TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_SOURCES_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/add_subdirectory_test/waypoint_sources_4XF31O1T1ff3B3Tq___"
)
TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_CLANG_BUILD_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/add_subdirectory_test/waypoint_build_clang_KGgicppoHf0mkVdJ___"
)
TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_GCC_BUILD_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/add_subdirectory_test/waypoint_build_gcc_ZcvFQuKcWaEwFis9___"
)
TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR = os.path.realpath(
    f"{INSTALL_TESTS_DIR_PATH}/add_subdirectory_test/infrastructure"
)
assert os.path.isfile(
    f"{TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR}/CMakeLists.txt"
) and os.path.isfile(
    f"{TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR}/CMakePresets.json"
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
    install: bool = False
    test_install: bool = False


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
        install=True,
        test_install=True,
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
        install=True,
        test_install=True,
    )
    Coverage = ModeConfig(
        gcc=True,
        coverage=True,
    )
    StaticAnalysis = ModeConfig(
        clang=True,
        static_analysis=True,
    )
    Valgrind = ModeConfig(
        clang=True,
        gcc=True,
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
    def incremental(self):
        return not self.config.clean

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

    @property
    def install(self):
        return self.config.install

    @property
    def test_install(self):
        return self.config.test_install


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
        return f"{round(nanos / 10 ** 9, 1)}s"
    elif nanos > 10**6:
        return f"{round(nanos / 10 ** 6, 1)}ms"
    elif nanos > 10**3:
        return f"{round(nanos / 10 ** 3, 1)}us"

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


def check_no_spaces_in_paths_() -> bool:
    files = find_files_by_name(PROJECT_ROOT_DIR, lambda x: True)

    for f in files:
        assert f.startswith(PROJECT_ROOT_DIR)
    files = [f[len(PROJECT_ROOT_DIR) + 1 :] for f in files]

    for f in files:
        if " " in f:
            return False

    return True


def check_main_header_has_no_includes_() -> bool:
    with open(MAIN_HEADER_PATH, "r") as f:
        contents = f.read()

    return re.search(r"# *include", contents) is None


def verify_installation_contents_(preset) -> bool:
    install_dir = install_dir_from_preset(preset)

    expected_files = [
        "cmake/waypoint-config.cmake",
        "cmake/waypoint-config-debug.cmake",
        "cmake/waypoint-config-relwithdebinfo.cmake",
        "cmake/waypoint-config-release.cmake",
        "cmake/waypoint-config-version.cmake",
        "include/waypoint/waypoint.hpp",
        "lib/Debug/libassert.a",
        "lib/Debug/libautorun.a",
        "lib/Debug/libcoverage.a",
        "lib/Debug/libprocess.a",
        "lib/Debug/libwaypoint.a",
        "lib/RelWithDebInfo/libassert.a",
        "lib/RelWithDebInfo/libautorun.a",
        "lib/RelWithDebInfo/libcoverage.a",
        "lib/RelWithDebInfo/libprocess.a",
        "lib/RelWithDebInfo/libwaypoint.a",
        "lib/Release/libassert.a",
        "lib/Release/libautorun.a",
        "lib/Release/libcoverage.a",
        "lib/Release/libprocess.a",
        "lib/Release/libwaypoint.a",
    ]

    files = find_files_by_name(install_dir, lambda x: True)
    for expected in expected_files:
        assert os.path.realpath(f"{install_dir}/{expected}") in files

    assert len(files) == len(expected_files)

    return True


def misc_checks_fn() -> bool:
    success = check_main_header_has_no_includes_()
    if not success:
        print(f"Error: Header {MAIN_HEADER_PATH} must not include other headers")

        return False

    success = check_no_spaces_in_paths_()
    if not success:
        print("Error: file paths must not contain spaces")

        return False

    success = verify_installation_contents_(CMakePresets.LinuxClang)
    if not success:
        print("Error: Invalid Clang installation contents")

        return False

    success = verify_installation_contents_(CMakePresets.LinuxGcc)
    if not success:
        print("Error: Invalid GCC installation contents")

        return False

    return True


def dir_from_preset(dir_key, preset, cmake_source_dir) -> str:
    presets_path = os.path.realpath(f"{cmake_source_dir}/CMakePresets.json")
    with open(presets_path) as f:
        data = json.load(f)
        configure_presets = [
            p for p in data["configurePresets"] if p["name"] == preset.configure
        ]
        assert len(configure_presets) == 1
        configure_presets = configure_presets[0]

        dir_path = configure_presets[dir_key]
        dir_path = dir_path.replace("${sourceDir}", f"{cmake_source_dir}")

        return os.path.realpath(dir_path)


def build_dir_from_preset(preset, cmake_source_dir) -> str:
    return dir_from_preset("binaryDir", preset, cmake_source_dir)


def install_dir_from_preset(preset) -> str:
    return dir_from_preset("installDir", preset, CMAKE_SOURCE_DIR)


def remove_dir(path):
    if os.path.exists(path) and os.path.isdir(path):
        shutil.rmtree(path)


def create_dir(path) -> bool:
    if os.path.exists(path) and not os.path.isdir(path):
        return False

    pathlib.Path(path).mkdir(parents=True, exist_ok=True)

    return True


def clean_build_dir(preset, cmake_source_dir):
    build_dir = build_dir_from_preset(preset, cmake_source_dir)
    remove_dir(build_dir)


def clean_install_dir(preset):
    install_dir = install_dir_from_preset(preset)
    remove_dir(install_dir)


def find_files_by_name(dir_path, pred) -> typing.List[str]:
    output = []
    for root, dirs, files in os.walk(dir_path):
        indices_to_remove = []
        for i, d in enumerate(dirs):
            if d.startswith("."):
                indices_to_remove.append(i)
                continue
            if "___" in d:
                indices_to_remove.append(i)
                continue
        indices_to_remove.sort()
        indices_to_remove.reverse()
        for i in indices_to_remove:
            dirs.pop(i)

        indices_to_remove = []
        for i, f in enumerate(files):
            if f.startswith("."):
                indices_to_remove.append(i)
                continue
            if "___" in f:
                indices_to_remove.append(i)
                continue
        indices_to_remove.sort()
        indices_to_remove.reverse()
        for i in indices_to_remove:
            files.pop(i)

        for f in files:
            path = os.path.realpath(os.path.join(root, f))
            if pred(path):
                output.append(path)

    output.sort()

    return output


def install_cmake(preset, config, working_dir) -> bool:
    build_dir = build_dir_from_preset(preset, CMAKE_SOURCE_DIR)

    with contextlib.chdir(working_dir):
        success, output = run(
            [
                "cmake",
                "--build",
                f"{build_dir}",
                "--target",
                "install",
                "--config",
                f"{config}",
            ]
        )
        if not success:
            if output is not None:
                print(output)

            return False

    return True


def install_gcc_debug_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxGcc, CMakeBuildConfig.Debug, PROJECT_ROOT_DIR
    )


def install_gcc_relwithdebinfo_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxGcc, CMakeBuildConfig.RelWithDebInfo, PROJECT_ROOT_DIR
    )


def install_gcc_release_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxGcc, CMakeBuildConfig.Release, PROJECT_ROOT_DIR
    )


def install_clang_debug_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxClang, CMakeBuildConfig.Debug, PROJECT_ROOT_DIR
    )


def install_clang_relwithdebinfo_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxClang, CMakeBuildConfig.RelWithDebInfo, PROJECT_ROOT_DIR
    )


def install_clang_release_fn() -> bool:
    return install_cmake(
        CMakePresets.LinuxClang, CMakeBuildConfig.Release, PROJECT_ROOT_DIR
    )


def configure_cmake_clang_fn() -> bool:
    return configure_cmake(CMakePresets.LinuxClang, CLANG20_ENV_PATCH, CMAKE_SOURCE_DIR)


def configure_cmake_gcc_fn() -> bool:
    return configure_cmake(CMakePresets.LinuxGcc, GCC15_ENV_PATCH, CMAKE_SOURCE_DIR)


def configure_cmake(preset, env_patch, cmake_source_dir) -> bool:
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        build_dir = build_dir_from_preset(preset, cmake_source_dir)

        if os.path.exists(build_dir):
            return True

        os.mkdir(build_dir)

        with contextlib.chdir(cmake_source_dir):
            success, output = run(["cmake", "--preset", f"{preset.configure}"])
            if not success:
                if output is not None:
                    print(output)

                return False

    return True


def build_cmake(config, preset, env_patch, cmake_source_dir, target):
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        with contextlib.chdir(cmake_source_dir):
            success, output = run(
                [
                    "cmake",
                    "--build",
                    "--preset",
                    f"{preset.build}",
                    "--config",
                    f"{config}",
                    "--target",
                    f"{target}",
                    "--parallel",
                    f"{JOBS}",
                ]
            )
            if not success:
                if output is not None:
                    print(output)

                return False

    return True


def run_ctest(
    preset, build_config, jobs, label_include_regex, cmake_source_dir
) -> bool:
    with contextlib.chdir(cmake_source_dir):
        success, output = run(
            [
                "ctest",
                "--preset",
                preset.test,
                "--build-config",
                f"{build_config}",
                "--timeout",
                "60",
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
    files = find_files_by_name(PROJECT_ROOT_DIR, is_cpp_file)
    files = [f for f in files if not f.startswith(INSTALL_TESTS_DIR_PATH)]

    return run_clang_tidy(CMakePresets.LinuxClang, files)


def run_clang_static_analysis_changed_files_fn() -> bool:
    files = changed_cpp_files_and_dependents(CMakePresets.LinuxClang)
    files = [f for f in files if not f.startswith(INSTALL_TESTS_DIR_PATH)]

    return run_clang_tidy(CMakePresets.LinuxClang, files)


def run_clang_tidy(preset, files) -> bool:
    if len(files) == 0:
        return True

    build_dir = build_dir_from_preset(preset, CMAKE_SOURCE_DIR)

    inputs = [(f, build_dir) for f in files]
    with multiprocessing.Pool(JOBS) as pool:
        results = pool.map(clang_tidy_process_single_file, inputs)

        errors = [
            (file, stdout) for success, file, duration, stdout in results if not success
        ]
        if len(errors) > 0:
            for f, stdout in errors:
                print(f"Error running clang-tidy on {f}")
                if stdout is not None:
                    print(stdout)

            return False

    return True


def test_gcc_debug_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc, CMakeBuildConfig.Debug, JOBS, r"^test$", CMAKE_SOURCE_DIR
    )


def test_gcc_relwithdebinfo_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


def test_gcc_release_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


def test_clang_debug_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


def test_clang_relwithdebinfo_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


def test_clang_release_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


def test_gcc_valgrind_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^valgrind$",
        CMAKE_SOURCE_DIR,
    )


def test_clang_valgrind_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^valgrind$",
        CMAKE_SOURCE_DIR,
    )


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
                "--exclude-pattern-prefix",
                "GCOV_COVERAGE_58QuSuUgMN8onvKx_*",
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
    build_dir = build_dir_from_preset(CMakePresets.LinuxGccCoverage, CMAKE_SOURCE_DIR)

    success = run_lcov(build_dir)
    if not success:
        return False

    success = run_gcovr(build_dir)
    if not success:
        return False

    return True


def configure_cmake_gcc_coverage_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxGccCoverage, GCC15_ENV_PATCH, CMAKE_SOURCE_DIR
    )


def build_gcc_coverage_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGccCoverage,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_gcc_coverage_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGccCoverage,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_gcc_coverage_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGccCoverage,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        CMAKE_SOURCE_DIR,
    )


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


def build_clang_debug_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_clang_debug_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def build_clang_relwithdebinfo_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_clang_relwithdebinfo_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def build_clang_release_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_clang_release_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def build_gcc_debug_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_gcc_debug_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def build_gcc_relwithdebinfo_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_gcc_relwithdebinfo_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
    )


def build_gcc_release_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all",
    )


def build_gcc_release_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        CMAKE_SOURCE_DIR,
        "all_tests",
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
    files = find_files_by_name(PROJECT_ROOT_DIR, is_json_file)
    files += find_files_by_name(PROJECT_ROOT_DIR, is_cmake_file)
    files += find_files_by_name(PROJECT_ROOT_DIR, is_python_file)
    files += find_files_by_name(PROJECT_ROOT_DIR, is_cpp_file)
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


def invert_index(index) -> typing.Dict[str, typing.Set[str]]:
    output = {}
    for file in index.keys():
        deps = index[file]
        for d in deps:
            if d not in output.keys():
                output[d] = set()
            output[d].add(file)
            output[d].add(d)

    return output


def process_depfiles(depfile_paths) -> typing.Dict[str, typing.Set[str]]:
    index = {}
    for path in depfile_paths:
        with open(path, "r") as f:
            lines = f.readlines()
        lines = lines[1:]
        lines = [l.lstrip(" ").rstrip(" \\\n") for l in lines]
        paths = []
        for l in lines:
            if " " in l:
                for x in l.split(" "):
                    paths.append(x)
                continue
            paths.append(l)

        paths = [os.path.realpath(p) for p in paths if os.path.isfile(p)]
        paths = [p for p in paths if p.startswith(f"{PROJECT_ROOT_DIR}/")]

        cpp_file = paths[0]

        if cpp_file not in index.keys():
            index[cpp_file] = set()

        index[cpp_file].update(paths)

    return index


def get_changed_files(predicate) -> typing.List[str]:
    with contextlib.chdir(PROJECT_ROOT_DIR):
        success, output = run(["git", "status", "--porcelain"])
        # Fall back to all files if git is not available
        if not success:
            return find_files_by_name(PROJECT_ROOT_DIR, predicate)

        files = output.split("\n")
        files = [f"{PROJECT_ROOT_DIR}/{f[3:]}" for f in files if len(f) > 0]
        files = [
            os.path.realpath(f) for f in files if os.path.isfile(f) and predicate(f)
        ]

        files.sort()

        return files


def collect_depfiles(preset):
    build_dir = build_dir_from_preset(preset, CMAKE_SOURCE_DIR)
    depfiles = []
    for root, dirs, files in os.walk(build_dir):
        for f in files:
            depfiles.append(f"{root}/{f}")
    depfiles = [
        os.path.realpath(f)
        for f in depfiles
        if os.path.isfile(f) and re.search(r"\.o\.d$", f) is not None
    ]
    depfiles.sort()

    return depfiles


def changed_cpp_files_and_dependents(preset) -> typing.List[str]:
    changed_cpp_files = get_changed_files(is_cpp_file)
    if len(changed_cpp_files) == 0:
        return []

    depfiles = collect_depfiles(preset)
    index = process_depfiles(depfiles)
    reverse_index = invert_index(index)
    files_for_analysis = set()
    for changed in changed_cpp_files:
        if changed not in reverse_index.keys():
            continue

        files_for_analysis.update(reverse_index[changed])

    output = list(files_for_analysis)
    output.sort()

    return output


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
    clean_build_dir(CMakePresets.LinuxClang, CMAKE_SOURCE_DIR)
    clean_build_dir(CMakePresets.LinuxGcc, CMAKE_SOURCE_DIR)
    clean_build_dir(
        CMakePresets.LinuxClang, TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR
    )
    clean_build_dir(
        CMakePresets.LinuxGcc, TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR
    )
    clean_build_dir(
        CMakePresets.LinuxClang,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )
    clean_build_dir(
        CMakePresets.LinuxGcc, TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR
    )
    clean_build_dir(
        CMakePresets.LinuxClang, TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR
    )
    clean_build_dir(
        CMakePresets.LinuxGcc, TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR
    )
    clean_build_dir(CMakePresets.LinuxGccCoverage, CMAKE_SOURCE_DIR)
    clean_install_dir(CMakePresets.LinuxClang)
    clean_install_dir(CMakePresets.LinuxGcc)
    clean_install_dir(CMakePresets.LinuxGccCoverage)
    remove_dir(COVERAGE_DIR_GCOVR)
    remove_dir(COVERAGE_DIR_LCOV)
    remove_dir(TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CLANG_DIR)
    remove_dir(TEST_INSTALL_FIND_PACKAGE_NO_VERSION_GCC_DIR)
    remove_dir(TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CLANG_DIR)
    remove_dir(TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_GCC_DIR)
    remove_dir(TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_SOURCES_DIR)
    remove_dir(TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_CLANG_BUILD_DIR)
    remove_dir(TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_GCC_BUILD_DIR)

    return True


class Task:
    def __init__(self, name: str, fn: typing.Callable[[], bool] | None = None):
        assert name is not None
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

        if len(self.dependencies_) > 0:
            print(f"Preparing task: {self.name_}")
        for d in self.dependencies_:
            success = d.run()
            if not success:
                return False

        self.deps_success_ = True

        print(f"Running task: {self.name_}")
        start = time.time_ns()
        success = True if self.fn_ is None else self.fn_()
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
            if len(self.dependencies_) > 0:
                print(
                    "Task failed:",
                    f"{self.name_} ({ns_to_string(time.time_ns() - start)},",
                    f"total: {ns_to_string(time.time_ns() - start_deps)})",
                )
            else:
                print(
                    "Task failed:",
                    f"{self.name_} ({ns_to_string(time.time_ns() - start)})",
                )

            return False

        self.success_ = True

        return True


def recursively_copy_dir(source, destination):
    shutil.copytree(source, destination, dirs_exist_ok=True)


def test_install_find_package_no_version_gcc_copy_artifacts_fn() -> bool:
    install_dir = install_dir_from_preset(CMakePresets.LinuxGcc)
    recursively_copy_dir(install_dir, TEST_INSTALL_FIND_PACKAGE_NO_VERSION_GCC_DIR)

    return True


def test_install_find_package_no_version_clang_copy_artifacts_fn() -> bool:
    install_dir = install_dir_from_preset(CMakePresets.LinuxClang)
    recursively_copy_dir(install_dir, TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CLANG_DIR)

    return True


def test_install_find_package_no_version_gcc_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_clang_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_gcc_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_gcc_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_gcc_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_gcc_relwithdebinfo_build_all_tests_fn() -> (
    bool
):
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_gcc_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_gcc_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_clang_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_clang_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_clang_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_clang_relwithdebinfo_build_all_tests_fn() -> (
    bool
):
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_clang_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_no_version_clang_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_no_version_gcc_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_gcc_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_gcc_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_clang_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_clang_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_no_version_clang_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_NO_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_gcc_copy_artifacts_fn() -> bool:
    install_dir = install_dir_from_preset(CMakePresets.LinuxGcc)
    recursively_copy_dir(install_dir, TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_GCC_DIR)

    return True


def test_install_find_package_exact_version_clang_copy_artifacts_fn() -> bool:
    install_dir = install_dir_from_preset(CMakePresets.LinuxClang)
    recursively_copy_dir(install_dir, TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CLANG_DIR)

    return True


def test_install_find_package_exact_version_gcc_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_clang_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_gcc_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_gcc_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_tests_fn() -> (
    bool
):
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_gcc_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_gcc_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_clang_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_clang_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_clang_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_clang_relwithdebinfo_build_all_tests_fn() -> (
    bool
):
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_clang_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_find_package_exact_version_clang_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_find_package_exact_version_gcc_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_gcc_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_gcc_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_clang_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_clang_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_find_package_exact_version_clang_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_FIND_PACKAGE_EXACT_VERSION_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_copy_sources_fn() -> bool:
    recursively_copy_dir(
        INFRASTRUCTURE_DIR,
        f"{TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_SOURCES_DIR}/infrastructure",
    )
    recursively_copy_dir(
        f"{PROJECT_ROOT_DIR}/src",
        f"{TEST_INSTALL_ADD_SUBDIRECTORY_WAYPOINT_SOURCES_DIR}/src",
    )

    return True


def test_install_add_subdirectory_gcc_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_clang_configure_fn() -> bool:
    return configure_cmake(
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_gcc_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_gcc_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_gcc_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_gcc_relwithdebinfo_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_gcc_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_gcc_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxGcc,
        GCC15_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_clang_debug_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_clang_debug_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Debug,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_clang_relwithdebinfo_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_clang_relwithdebinfo_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.RelWithDebInfo,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_clang_release_build_all_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all",
    )


def test_install_add_subdirectory_clang_release_build_all_tests_fn() -> bool:
    return build_cmake(
        CMakeBuildConfig.Release,
        CMakePresets.LinuxClang,
        CLANG20_ENV_PATCH,
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
        "all_tests",
    )


def test_install_add_subdirectory_gcc_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_gcc_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_gcc_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxGcc,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_clang_debug_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Debug,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_clang_relwithdebinfo_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.RelWithDebInfo,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


def test_install_add_subdirectory_clang_release_test_fn() -> bool:
    return run_ctest(
        CMakePresets.LinuxClang,
        CMakeBuildConfig.Release,
        JOBS,
        r"^test$",
        TEST_INSTALL_ADD_SUBDIRECTORY_CMAKE_SOURCE_DIR,
    )


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
    build_clang_debug_all = Task("Build Clang Debug (all)", build_clang_debug_all_fn)
    build_clang_debug_all_tests = Task(
        "Build Clang Debug (all_tests)", build_clang_debug_all_tests_fn
    )
    build_clang_relwithdebinfo_all = Task(
        "Build Clang RelWithDebInfo (all)", build_clang_relwithdebinfo_all_fn
    )
    build_clang_relwithdebinfo_all_tests = Task(
        "Build Clang RelWithDebInfo (all_tests)",
        build_clang_relwithdebinfo_all_tests_fn,
    )
    build_clang_release_all = Task(
        "Build Clang Release (all)", build_clang_release_all_fn
    )
    build_clang_release_all_tests = Task(
        "Build Clang Release (all_tests)", build_clang_release_all_tests_fn
    )

    configure_cmake_clang = Task("Configure CMake for Clang", configure_cmake_clang_fn)

    build_clang_debug_all.depends_on([configure_cmake_clang])
    build_clang_debug_all_tests.depends_on([build_clang_debug_all])
    test_clang_debug.depends_on([build_clang_debug_all_tests])
    build_clang_relwithdebinfo_all.depends_on([configure_cmake_clang])
    build_clang_relwithdebinfo_all_tests.depends_on([build_clang_relwithdebinfo_all])
    test_clang_relwithdebinfo.depends_on([build_clang_relwithdebinfo_all_tests])
    build_clang_release_all.depends_on([configure_cmake_clang])
    build_clang_release_all_tests.depends_on([build_clang_release_all])
    test_clang_release.depends_on([build_clang_release_all_tests])

    test_gcc_debug = Task("Test GCC Debug", test_gcc_debug_fn)
    test_gcc_relwithdebinfo = Task(
        "Test GCC RelWithDebInfo", test_gcc_relwithdebinfo_fn
    )
    test_gcc_release = Task("Test GCC Release", test_gcc_release_fn)
    build_gcc_debug_all = Task("Build GCC Debug (all)", build_gcc_debug_all_fn)
    build_gcc_debug_all_tests = Task(
        "Build GCC Debug (all_tests)", build_gcc_debug_all_tests_fn
    )
    build_gcc_relwithdebinfo_all = Task(
        "Build GCC RelWithDebInfo (all)", build_gcc_relwithdebinfo_all_fn
    )
    build_gcc_relwithdebinfo_all_tests = Task(
        "Build GCC RelWithDebInfo (all_tests)", build_gcc_relwithdebinfo_all_tests_fn
    )
    build_gcc_release_all = Task("Build GCC Release (all)", build_gcc_release_all_fn)
    build_gcc_release_all_tests = Task(
        "Build GCC Release (all_tests)", build_gcc_release_all_tests_fn
    )

    configure_cmake_gcc = Task("Configure CMake for GCC", configure_cmake_gcc_fn)

    install_gcc_debug = Task("Install GCC Debug", install_gcc_debug_fn)
    install_gcc_relwithdebinfo = Task(
        "Install GCC RelWithDebInfo", install_gcc_relwithdebinfo_fn
    )
    install_gcc_release = Task("Install GCC Release", install_gcc_release_fn)
    install_clang_debug = Task("Install Clang Debug", install_clang_debug_fn)
    install_clang_relwithdebinfo = Task(
        "Install Clang RelWithDebInfo", install_clang_relwithdebinfo_fn
    )
    install_clang_release = Task("Install Clang Release", install_clang_release_fn)

    install_gcc_debug.depends_on([build_gcc_debug_all])
    install_gcc_relwithdebinfo.depends_on([build_gcc_relwithdebinfo_all])
    install_gcc_release.depends_on([build_gcc_release_all])
    install_clang_debug.depends_on([build_clang_debug_all])
    install_clang_relwithdebinfo.depends_on([build_clang_relwithdebinfo_all])
    install_clang_release.depends_on([build_clang_release_all])

    build_gcc_debug_all.depends_on([configure_cmake_gcc])
    build_gcc_debug_all_tests.depends_on([build_gcc_debug_all])
    test_gcc_debug.depends_on([build_gcc_debug_all_tests])
    build_gcc_relwithdebinfo_all.depends_on([configure_cmake_gcc])
    build_gcc_relwithdebinfo_all_tests.depends_on([build_gcc_relwithdebinfo_all])
    test_gcc_relwithdebinfo.depends_on([build_gcc_relwithdebinfo_all_tests])
    build_gcc_release_all.depends_on([configure_cmake_gcc])
    build_gcc_release_all_tests.depends_on([build_gcc_release_all])
    test_gcc_release.depends_on([build_gcc_release_all_tests])

    analyze_gcc_coverage = Task("Analyze GCC coverage data", analyze_gcc_coverage_fn)
    process_gcc_coverage = Task("Process GCC coverage data", process_coverage_fn)
    test_gcc_coverage = Task("Test GCC with coverage", test_gcc_coverage_fn)
    build_gcc_coverage_all = Task(
        "Build GCC with coverage (all)", build_gcc_coverage_all_fn
    )
    build_gcc_coverage_all_tests = Task(
        "Build GCC with coverage (all_tests)", build_gcc_coverage_all_tests_fn
    )
    configure_cmake_gcc_coverage = Task(
        "Configure CMake for GCC with coverage", configure_cmake_gcc_coverage_fn
    )

    analyze_gcc_coverage.depends_on([process_gcc_coverage])
    process_gcc_coverage.depends_on([test_gcc_coverage])
    test_gcc_coverage.depends_on([build_gcc_coverage_all_tests])
    build_gcc_coverage_all_tests.depends_on([build_gcc_coverage_all])
    build_gcc_coverage_all.depends_on([configure_cmake_gcc_coverage])

    configure_cmake_gcc_valgrind = Task("Configure CMake for GCC with Valgrind")
    configure_cmake_gcc_valgrind.depends_on([configure_cmake_gcc])
    build_gcc_valgrind = Task("Build GCC for Valgrind")
    build_gcc_valgrind.depends_on([build_gcc_debug_all, build_gcc_debug_all_tests])
    test_gcc_valgrind = Task("Test GCC build with Valgrind", test_gcc_valgrind_fn)
    test_gcc_valgrind.depends_on([build_gcc_valgrind])
    build_gcc_valgrind.depends_on([configure_cmake_gcc_valgrind])

    configure_cmake_clang_valgrind = Task("Configure CMake for Clang with Valgrind")
    configure_cmake_clang_valgrind.depends_on([configure_cmake_clang])
    build_clang_valgrind = Task("Build Clang for Valgrind")
    build_clang_valgrind.depends_on(
        [build_clang_debug_all, build_clang_debug_all_tests]
    )
    test_clang_valgrind = Task("Test Clang build with Valgrind", test_clang_valgrind_fn)
    test_clang_valgrind.depends_on([build_clang_valgrind])
    build_clang_valgrind.depends_on([configure_cmake_clang_valgrind])

    configure_cmake_clang_static_analysis = Task("Configure CMake for clang-tidy")
    configure_cmake_clang_static_analysis.depends_on([configure_cmake_clang])
    build_clang_static_analysis = Task("Build Clang for clang-tidy")
    build_clang_static_analysis.depends_on(
        [
            configure_cmake_clang_static_analysis,
            build_clang_debug_all,
            build_clang_debug_all_tests,
            build_clang_relwithdebinfo_all,
            build_clang_relwithdebinfo_all_tests,
            build_clang_release_all,
            build_clang_release_all_tests,
        ]
    )
    run_clang_static_analysis_all_files = Task(
        "Run clang-tidy", run_clang_static_analysis_all_files_fn
    )
    run_clang_static_analysis_all_files.depends_on([build_clang_static_analysis])

    run_clang_static_analysis_changed_files = Task(
        "Run clang-tidy (incremental)", run_clang_static_analysis_changed_files_fn
    )
    run_clang_static_analysis_changed_files.depends_on([build_clang_static_analysis])

    misc_checks = Task("Miscellaneous checks", misc_checks_fn)

    format_sources = Task("Format code", format_sources_fn)

    clean = Task("Clean build files", clean_fn)

    test_install_find_package_no_version_gcc_copy_artifacts = Task(
        "Copy GCC artifacts for test install (find_package, no version)",
        test_install_find_package_no_version_gcc_copy_artifacts_fn,
    )
    test_install_find_package_no_version_clang_copy_artifacts = Task(
        "Copy Clang artifacts for test install (find_package, no version)",
        test_install_find_package_no_version_clang_copy_artifacts_fn,
    )

    test_install_find_package_no_version_gcc_copy_artifacts.depends_on(
        [install_gcc_debug, install_gcc_relwithdebinfo, install_gcc_release]
    )
    test_install_find_package_no_version_clang_copy_artifacts.depends_on(
        [install_clang_debug, install_clang_relwithdebinfo, install_clang_release]
    )

    test_install_find_package_no_version_gcc_configure = Task(
        "Configure CMake for GCC test install (find_package, no version)",
        test_install_find_package_no_version_gcc_configure_fn,
    )
    test_install_find_package_no_version_clang_configure = Task(
        "Configure CMake for Clang test install (find_package, no version)",
        test_install_find_package_no_version_clang_configure_fn,
    )

    test_install_find_package_no_version_gcc_configure.depends_on(
        [test_install_find_package_no_version_gcc_copy_artifacts]
    )
    test_install_find_package_no_version_clang_configure.depends_on(
        [test_install_find_package_no_version_clang_copy_artifacts]
    )

    test_install_find_package_no_version_gcc_debug_build_all = Task(
        "Build GCC Debug test install (all; find_package, no version)",
        test_install_find_package_no_version_gcc_debug_build_all_fn,
    )
    test_install_find_package_no_version_gcc_debug_build_all_tests = Task(
        "Build GCC Debug test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_gcc_debug_build_all_tests_fn,
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_build_all = Task(
        "Build GCC RelWithDebInfo test install (all; find_package, no version)",
        test_install_find_package_no_version_gcc_relwithdebinfo_build_all_fn,
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_build_all_tests = Task(
        "Build GCC RelWithDebInfo test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_gcc_relwithdebinfo_build_all_tests_fn,
    )
    test_install_find_package_no_version_gcc_release_build_all = Task(
        "Build GCC Release test install (all; find_package, no version)",
        test_install_find_package_no_version_gcc_release_build_all_fn,
    )
    test_install_find_package_no_version_gcc_release_build_all_tests = Task(
        "Build GCC Release test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_gcc_release_build_all_tests_fn,
    )
    test_install_find_package_no_version_clang_debug_build_all = Task(
        "Build Clang Debug test install (all; find_package, no version)",
        test_install_find_package_no_version_clang_debug_build_all_fn,
    )
    test_install_find_package_no_version_clang_debug_build_all_tests = Task(
        "Build Clang Debug test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_clang_debug_build_all_tests_fn,
    )
    test_install_find_package_no_version_clang_relwithdebinfo_build_all = Task(
        "Build Clang RelWithDebInfo test install (all; find_package, no version)",
        test_install_find_package_no_version_clang_relwithdebinfo_build_all_fn,
    )
    test_install_find_package_no_version_clang_relwithdebinfo_build_all_tests = Task(
        "Build Clang RelWithDebInfo test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_clang_relwithdebinfo_build_all_tests_fn,
    )
    test_install_find_package_no_version_clang_release_build_all = Task(
        "Build Clang Release test install (all; find_package, no version)",
        test_install_find_package_no_version_clang_release_build_all_fn,
    )
    test_install_find_package_no_version_clang_release_build_all_tests = Task(
        "Build Clang Release test install (all_tests; find_package, no version)",
        test_install_find_package_no_version_clang_release_build_all_tests_fn,
    )

    test_install_find_package_no_version_gcc_debug_test = Task(
        "Test GCC Debug test install (find_package, no version)",
        test_install_find_package_no_version_gcc_debug_test_fn,
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_test = Task(
        "Test GCC RelWithDebInfo test install (find_package, no version)",
        test_install_find_package_no_version_gcc_relwithdebinfo_test_fn,
    )
    test_install_find_package_no_version_gcc_release_test = Task(
        "Test GCC Release test install (find_package, no version)",
        test_install_find_package_no_version_gcc_release_test_fn,
    )
    test_install_find_package_no_version_clang_debug_test = Task(
        "Test Clang Debug test install (find_package, no version)",
        test_install_find_package_no_version_clang_debug_test_fn,
    )
    test_install_find_package_no_version_clang_relwithdebinfo_test = Task(
        "Test Clang RelWithDebInfo test install (find_package, no version)",
        test_install_find_package_no_version_clang_relwithdebinfo_test_fn,
    )
    test_install_find_package_no_version_clang_release_test = Task(
        "Test Clang Release test install (find_package, no version)",
        test_install_find_package_no_version_clang_release_test_fn,
    )

    test_install_find_package_no_version_gcc_debug_build_all.depends_on(
        [test_install_find_package_no_version_gcc_configure]
    )
    test_install_find_package_no_version_gcc_debug_build_all_tests.depends_on(
        [test_install_find_package_no_version_gcc_debug_build_all]
    )
    test_install_find_package_no_version_gcc_debug_test.depends_on(
        [test_install_find_package_no_version_gcc_debug_build_all_tests]
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_build_all.depends_on(
        [test_install_find_package_no_version_gcc_configure]
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_build_all_tests.depends_on(
        [test_install_find_package_no_version_gcc_relwithdebinfo_build_all]
    )
    test_install_find_package_no_version_gcc_relwithdebinfo_test.depends_on(
        [test_install_find_package_no_version_gcc_relwithdebinfo_build_all_tests]
    )
    test_install_find_package_no_version_gcc_release_build_all.depends_on(
        [test_install_find_package_no_version_gcc_configure]
    )
    test_install_find_package_no_version_gcc_release_build_all_tests.depends_on(
        [test_install_find_package_no_version_gcc_release_build_all]
    )
    test_install_find_package_no_version_gcc_release_test.depends_on(
        [test_install_find_package_no_version_gcc_release_build_all_tests]
    )
    test_install_find_package_no_version_clang_debug_build_all.depends_on(
        [test_install_find_package_no_version_clang_configure]
    )
    test_install_find_package_no_version_clang_debug_build_all_tests.depends_on(
        [test_install_find_package_no_version_clang_debug_build_all]
    )
    test_install_find_package_no_version_clang_debug_test.depends_on(
        [test_install_find_package_no_version_clang_debug_build_all_tests]
    )
    test_install_find_package_no_version_clang_relwithdebinfo_build_all.depends_on(
        [test_install_find_package_no_version_clang_configure]
    )
    test_install_find_package_no_version_clang_relwithdebinfo_build_all_tests.depends_on(
        [test_install_find_package_no_version_clang_relwithdebinfo_build_all]
    )
    test_install_find_package_no_version_clang_relwithdebinfo_test.depends_on(
        [test_install_find_package_no_version_clang_relwithdebinfo_build_all_tests]
    )
    test_install_find_package_no_version_clang_release_build_all.depends_on(
        [test_install_find_package_no_version_clang_configure]
    )
    test_install_find_package_no_version_clang_release_build_all_tests.depends_on(
        [test_install_find_package_no_version_clang_release_build_all]
    )
    test_install_find_package_no_version_clang_release_test.depends_on(
        [test_install_find_package_no_version_clang_release_build_all_tests]
    )

    test_install_find_package_exact_version_gcc_copy_artifacts = Task(
        "Copy GCC artifacts for test install (find_package, exact version)",
        test_install_find_package_exact_version_gcc_copy_artifacts_fn,
    )
    test_install_find_package_exact_version_clang_copy_artifacts = Task(
        "Copy Clang artifacts for test install (find_package, exact version)",
        test_install_find_package_exact_version_clang_copy_artifacts_fn,
    )

    test_install_find_package_exact_version_gcc_copy_artifacts.depends_on(
        [install_gcc_debug, install_gcc_relwithdebinfo, install_gcc_release]
    )
    test_install_find_package_exact_version_clang_copy_artifacts.depends_on(
        [install_clang_debug, install_clang_relwithdebinfo, install_clang_release]
    )

    test_install_find_package_exact_version_gcc_configure = Task(
        "Configure CMake for GCC test install (find_package, exact version)",
        test_install_find_package_exact_version_gcc_configure_fn,
    )
    test_install_find_package_exact_version_clang_configure = Task(
        "Configure CMake for Clang test install (find_package, exact version)",
        test_install_find_package_exact_version_clang_configure_fn,
    )

    test_install_find_package_exact_version_gcc_configure.depends_on(
        [test_install_find_package_exact_version_gcc_copy_artifacts]
    )
    test_install_find_package_exact_version_clang_configure.depends_on(
        [test_install_find_package_exact_version_clang_copy_artifacts]
    )

    test_install_find_package_exact_version_gcc_debug_build_all = Task(
        "Build GCC Debug test install (all; find_package, exact version)",
        test_install_find_package_exact_version_gcc_debug_build_all_fn,
    )
    test_install_find_package_exact_version_gcc_debug_build_all_tests = Task(
        "Build GCC Debug test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_gcc_debug_build_all_tests_fn,
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_build_all = Task(
        "Build GCC RelWithDebInfo test install (all; find_package, exact version)",
        test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_fn,
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_tests = Task(
        "Build GCC RelWithDebInfo test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_tests_fn,
    )
    test_install_find_package_exact_version_gcc_release_build_all = Task(
        "Build GCC Release test install (all; find_package, exact version)",
        test_install_find_package_exact_version_gcc_release_build_all_fn,
    )
    test_install_find_package_exact_version_gcc_release_build_all_tests = Task(
        "Build GCC Release test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_gcc_release_build_all_tests_fn,
    )
    test_install_find_package_exact_version_clang_debug_build_all = Task(
        "Build Clang Debug test install (all; find_package, exact version)",
        test_install_find_package_exact_version_clang_debug_build_all_fn,
    )
    test_install_find_package_exact_version_clang_debug_build_all_tests = Task(
        "Build Clang Debug test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_clang_debug_build_all_tests_fn,
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_build_all = Task(
        "Build Clang RelWithDebInfo test install (all; find_package, exact version)",
        test_install_find_package_exact_version_clang_relwithdebinfo_build_all_fn,
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_build_all_tests = Task(
        "Build Clang RelWithDebInfo test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_clang_relwithdebinfo_build_all_tests_fn,
    )
    test_install_find_package_exact_version_clang_release_build_all = Task(
        "Build Clang Release test install (all; find_package, exact version)",
        test_install_find_package_exact_version_clang_release_build_all_fn,
    )
    test_install_find_package_exact_version_clang_release_build_all_tests = Task(
        "Build Clang Release test install (all_tests; find_package, exact version)",
        test_install_find_package_exact_version_clang_release_build_all_tests_fn,
    )

    test_install_find_package_exact_version_gcc_debug_test = Task(
        "Test GCC Debug test install (find_package, exact version)",
        test_install_find_package_exact_version_gcc_debug_test_fn,
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_test = Task(
        "Test GCC RelWithDebInfo test install (find_package, exact version)",
        test_install_find_package_exact_version_gcc_relwithdebinfo_test_fn,
    )
    test_install_find_package_exact_version_gcc_release_test = Task(
        "Test GCC Release test install (find_package, exact version)",
        test_install_find_package_exact_version_gcc_release_test_fn,
    )
    test_install_find_package_exact_version_clang_debug_test = Task(
        "Test Clang Debug test install (find_package, exact version)",
        test_install_find_package_exact_version_clang_debug_test_fn,
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_test = Task(
        "Test Clang RelWithDebInfo test install (find_package, exact version)",
        test_install_find_package_exact_version_clang_relwithdebinfo_test_fn,
    )
    test_install_find_package_exact_version_clang_release_test = Task(
        "Test Clang Release test install (find_package, exact version)",
        test_install_find_package_exact_version_clang_release_test_fn,
    )

    test_install_find_package_exact_version_gcc_debug_build_all.depends_on(
        [test_install_find_package_exact_version_gcc_configure]
    )
    test_install_find_package_exact_version_gcc_debug_build_all_tests.depends_on(
        [test_install_find_package_exact_version_gcc_debug_build_all]
    )
    test_install_find_package_exact_version_gcc_debug_test.depends_on(
        [test_install_find_package_exact_version_gcc_debug_build_all_tests]
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_build_all.depends_on(
        [test_install_find_package_exact_version_gcc_configure]
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_tests.depends_on(
        [test_install_find_package_exact_version_gcc_relwithdebinfo_build_all]
    )
    test_install_find_package_exact_version_gcc_relwithdebinfo_test.depends_on(
        [test_install_find_package_exact_version_gcc_relwithdebinfo_build_all_tests]
    )
    test_install_find_package_exact_version_gcc_release_build_all.depends_on(
        [test_install_find_package_exact_version_gcc_configure]
    )
    test_install_find_package_exact_version_gcc_release_build_all_tests.depends_on(
        [test_install_find_package_exact_version_gcc_release_build_all]
    )
    test_install_find_package_exact_version_gcc_release_test.depends_on(
        [test_install_find_package_exact_version_gcc_release_build_all_tests]
    )
    test_install_find_package_exact_version_clang_debug_build_all.depends_on(
        [test_install_find_package_exact_version_clang_configure]
    )
    test_install_find_package_exact_version_clang_debug_build_all_tests.depends_on(
        [test_install_find_package_exact_version_clang_debug_build_all]
    )
    test_install_find_package_exact_version_clang_debug_test.depends_on(
        [test_install_find_package_exact_version_clang_debug_build_all_tests]
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_build_all.depends_on(
        [test_install_find_package_exact_version_clang_configure]
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_build_all_tests.depends_on(
        [test_install_find_package_exact_version_clang_relwithdebinfo_build_all]
    )
    test_install_find_package_exact_version_clang_relwithdebinfo_test.depends_on(
        [test_install_find_package_exact_version_clang_relwithdebinfo_build_all_tests]
    )
    test_install_find_package_exact_version_clang_release_build_all.depends_on(
        [test_install_find_package_exact_version_clang_configure]
    )
    test_install_find_package_exact_version_clang_release_build_all_tests.depends_on(
        [test_install_find_package_exact_version_clang_release_build_all]
    )
    test_install_find_package_exact_version_clang_release_test.depends_on(
        [test_install_find_package_exact_version_clang_release_build_all_tests]
    )

    test_install_test_install_add_subdirectory_copy_sources = Task(
        "Copy sources for test install (add_subdirectory)",
        test_install_add_subdirectory_copy_sources_fn,
    )

    test_install_add_subdirectory_gcc_configure = Task(
        "Configure CMake for GCC test install (add_subdirectory)",
        test_install_add_subdirectory_gcc_configure_fn,
    )
    test_install_add_subdirectory_clang_configure = Task(
        "Configure CMake for Clang test install (add_subdirectory)",
        test_install_add_subdirectory_clang_configure_fn,
    )

    test_install_add_subdirectory_gcc_configure.depends_on(
        [test_install_test_install_add_subdirectory_copy_sources]
    )
    test_install_add_subdirectory_clang_configure.depends_on(
        [test_install_test_install_add_subdirectory_copy_sources]
    )

    test_install_add_subdirectory_gcc_debug_build_all = Task(
        "Build GCC Debug test install (all; add_subdirectory)",
        test_install_add_subdirectory_gcc_debug_build_all_fn,
    )
    test_install_add_subdirectory_gcc_debug_build_all_tests = Task(
        "Build GCC Debug test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_gcc_debug_build_all_tests_fn,
    )
    test_install_add_subdirectory_gcc_relwithdebinfo_build_all = Task(
        "Build GCC RelWithDebInfo test install (all; add_subdirectory)",
        test_install_add_subdirectory_gcc_relwithdebinfo_build_all_fn,
    )
    test_install_add_subdirectory_gcc_relwithdebinfo_build_all_tests = Task(
        "Build GCC RelWithDebInfo test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_gcc_relwithdebinfo_build_all_tests_fn,
    )
    test_install_add_subdirectory_gcc_release_build_all = Task(
        "Build GCC Release test install (all; add_subdirectory)",
        test_install_add_subdirectory_gcc_release_build_all_fn,
    )
    test_install_add_subdirectory_gcc_release_build_all_tests = Task(
        "Build GCC Release test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_gcc_release_build_all_tests_fn,
    )
    test_install_add_subdirectory_clang_debug_build_all = Task(
        "Build Clang Debug test install (all; add_subdirectory)",
        test_install_add_subdirectory_clang_debug_build_all_fn,
    )
    test_install_add_subdirectory_clang_debug_build_all_tests = Task(
        "Build Clang Debug test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_clang_debug_build_all_tests_fn,
    )
    test_install_add_subdirectory_clang_relwithdebinfo_build_all = Task(
        "Build Clang RelWithDebInfo test install (all; add_subdirectory)",
        test_install_add_subdirectory_clang_relwithdebinfo_build_all_fn,
    )
    test_install_add_subdirectory_clang_relwithdebinfo_build_all_tests = Task(
        "Build Clang RelWithDebInfo test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_clang_relwithdebinfo_build_all_tests_fn,
    )
    test_install_add_subdirectory_clang_release_build_all = Task(
        "Build Clang Release test install (all; add_subdirectory)",
        test_install_add_subdirectory_clang_release_build_all_fn,
    )
    test_install_add_subdirectory_clang_release_build_all_tests = Task(
        "Build Clang Release test install (all_tests; add_subdirectory)",
        test_install_add_subdirectory_clang_release_build_all_tests_fn,
    )

    test_install_add_subdirectory_gcc_debug_test = Task(
        "Test GCC Debug test install (add_subdirectory)",
        test_install_add_subdirectory_gcc_debug_test_fn,
    )
    test_install_add_subdirectory_gcc_relwithdebinfo_test = Task(
        "Test GCC RelWithDebInfo test install (add_subdirectory)",
        test_install_add_subdirectory_gcc_relwithdebinfo_test_fn,
    )
    test_install_add_subdirectory_gcc_release_test = Task(
        "Test GCC Release test install (add_subdirectory)",
        test_install_add_subdirectory_gcc_release_test_fn,
    )
    test_install_add_subdirectory_clang_debug_test = Task(
        "Test Clang Debug test install (add_subdirectory)",
        test_install_add_subdirectory_clang_debug_test_fn,
    )
    test_install_add_subdirectory_clang_relwithdebinfo_test = Task(
        "Test Clang RelWithDebInfo test install (add_subdirectory)",
        test_install_add_subdirectory_clang_relwithdebinfo_test_fn,
    )
    test_install_add_subdirectory_clang_release_test = Task(
        "Test Clang Release test install (add_subdirectory)",
        test_install_add_subdirectory_clang_release_test_fn,
    )

    test_install_add_subdirectory_gcc_debug_build_all.depends_on(
        [test_install_add_subdirectory_gcc_configure]
    )
    test_install_add_subdirectory_gcc_debug_build_all_tests.depends_on(
        [test_install_add_subdirectory_gcc_debug_build_all]
    )
    test_install_add_subdirectory_gcc_debug_test.depends_on(
        [test_install_add_subdirectory_gcc_debug_build_all_tests]
    )

    test_install_add_subdirectory_gcc_relwithdebinfo_build_all.depends_on(
        [test_install_add_subdirectory_gcc_configure]
    )
    test_install_add_subdirectory_gcc_relwithdebinfo_build_all_tests.depends_on(
        [test_install_add_subdirectory_gcc_relwithdebinfo_build_all]
    )
    test_install_add_subdirectory_gcc_relwithdebinfo_test.depends_on(
        [test_install_add_subdirectory_gcc_relwithdebinfo_build_all_tests]
    )
    test_install_add_subdirectory_gcc_release_build_all.depends_on(
        [test_install_add_subdirectory_gcc_configure]
    )
    test_install_add_subdirectory_gcc_release_build_all_tests.depends_on(
        [test_install_add_subdirectory_gcc_release_build_all]
    )
    test_install_add_subdirectory_gcc_release_test.depends_on(
        [test_install_add_subdirectory_gcc_release_build_all_tests]
    )
    test_install_add_subdirectory_clang_debug_build_all.depends_on(
        [test_install_add_subdirectory_clang_configure]
    )
    test_install_add_subdirectory_clang_debug_build_all_tests.depends_on(
        [test_install_add_subdirectory_clang_debug_build_all]
    )
    test_install_add_subdirectory_clang_debug_test.depends_on(
        [test_install_add_subdirectory_clang_debug_build_all_tests]
    )
    test_install_add_subdirectory_clang_relwithdebinfo_build_all.depends_on(
        [test_install_add_subdirectory_clang_configure]
    )
    test_install_add_subdirectory_clang_relwithdebinfo_build_all_tests.depends_on(
        [test_install_add_subdirectory_clang_relwithdebinfo_build_all]
    )
    test_install_add_subdirectory_clang_relwithdebinfo_test.depends_on(
        [test_install_add_subdirectory_clang_relwithdebinfo_build_all_tests]
    )
    test_install_add_subdirectory_clang_release_build_all.depends_on(
        [test_install_add_subdirectory_clang_configure]
    )
    test_install_add_subdirectory_clang_release_build_all_tests.depends_on(
        [test_install_add_subdirectory_clang_release_build_all]
    )
    test_install_add_subdirectory_clang_release_test.depends_on(
        [test_install_add_subdirectory_clang_release_build_all_tests]
    )

    root_dependencies = []

    if mode.clean:
        root_dependencies.append(clean)

    if mode.format:
        root_dependencies.append(format_sources)

    if mode.gcc:
        if mode.debug:
            root_dependencies.append(build_gcc_debug_all)
            if mode.test:
                root_dependencies.append(test_gcc_debug)
            if mode.install:
                root_dependencies.append(install_gcc_debug)

        if mode.release:
            root_dependencies.append(build_gcc_relwithdebinfo_all)
            if mode.test:
                root_dependencies.append(test_gcc_relwithdebinfo)
            if mode.install:
                root_dependencies.append(install_gcc_relwithdebinfo)

            root_dependencies.append(build_gcc_release_all)
            if mode.test:
                root_dependencies.append(test_gcc_release)
            if mode.install:
                root_dependencies.append(install_gcc_release)

    if mode.clang:
        if mode.debug:
            root_dependencies.append(build_clang_debug_all)
            if mode.test:
                root_dependencies.append(test_clang_debug)
            if mode.install:
                root_dependencies.append(install_clang_debug)

        if mode.release:
            root_dependencies.append(build_clang_relwithdebinfo_all)
            if mode.test:
                root_dependencies.append(test_clang_relwithdebinfo)
            if mode.install:
                root_dependencies.append(install_clang_relwithdebinfo)

            root_dependencies.append(build_clang_release_all)
            if mode.test:
                root_dependencies.append(test_clang_release)
            if mode.install:
                root_dependencies.append(install_clang_release)

    if mode.coverage:
        if mode.gcc:
            root_dependencies.append(analyze_gcc_coverage)

    if mode.valgrind:
        if mode.gcc:
            root_dependencies.append(test_gcc_valgrind)
        if mode.clang:
            root_dependencies.append(test_clang_valgrind)

    if mode.test_install:
        if mode.gcc:
            if mode.debug:
                root_dependencies.append(
                    test_install_find_package_no_version_gcc_debug_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_gcc_debug_test
                )
                root_dependencies.append(test_install_add_subdirectory_clang_debug_test)
            if mode.release:
                root_dependencies.append(
                    test_install_find_package_no_version_gcc_relwithdebinfo_test
                )
                root_dependencies.append(
                    test_install_find_package_no_version_gcc_release_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_gcc_relwithdebinfo_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_gcc_release_test
                )
                root_dependencies.append(
                    test_install_add_subdirectory_gcc_relwithdebinfo_test
                )
                root_dependencies.append(test_install_add_subdirectory_gcc_release_test)

        if mode.clang:
            if mode.debug:
                root_dependencies.append(
                    test_install_find_package_no_version_clang_debug_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_clang_debug_test
                )
                root_dependencies.append(test_install_add_subdirectory_clang_debug_test)
            if mode.release:
                root_dependencies.append(
                    test_install_find_package_no_version_clang_relwithdebinfo_test
                )
                root_dependencies.append(
                    test_install_find_package_no_version_clang_release_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_clang_relwithdebinfo_test
                )
                root_dependencies.append(
                    test_install_find_package_exact_version_clang_release_test
                )
                root_dependencies.append(
                    test_install_add_subdirectory_clang_relwithdebinfo_test
                )
                root_dependencies.append(
                    test_install_add_subdirectory_clang_release_test
                )

    if mode.misc:
        root_dependencies.append(misc_checks)

    if mode.static_analysis:
        if mode.clang:
            if mode.incremental:
                root_dependencies.append(run_clang_static_analysis_changed_files)
            else:
                root_dependencies.append(run_clang_static_analysis_all_files)

    root = Task("Build")
    root.depends_on(root_dependencies)

    success = root.run()
    if not success:
        return 1

    print(f"Success: {os.path.basename(sys.argv[0])}")

    return 0


if __name__ == "__main__":
    exit(main())
