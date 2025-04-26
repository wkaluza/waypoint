import argparse
import contextlib
import enum
import json
import os
import platform
import re
import shutil
import subprocess
import sys

THIS_SCRIPT_DIR = os.path.realpath(os.path.dirname(__file__))
PROJECT_ROOT_DIR = os.path.realpath(f"{THIS_SCRIPT_DIR}/..")


@enum.unique
class Mode(enum.Enum):
    Fast = (False, False, True, False, True, False, False, True, False)
    Full = (False, True, True, True, True, True, True, True, True)
    Clean = (True, False, False, False, False, False, False, False, False)
    Verify = (True, True, True, True, True, True, True, True, True)
    # Tool-specific modes
    StaticAnalysis = (False, False, True, False, True, True, True, False, False)
    Valgrind = (False, False, True, True, True, False, False, False, True)

    def __init__(
        self,
        clean_,
        format_,
        clang_,
        gcc_,
        debug_,
        release_,
        static_analysis_,
        test_,
        valgrind_,
    ):
        self._clean = clean_
        self._format = format_
        self._clang = clang_
        self._gcc = gcc_
        self._debug = debug_
        self._release = release_
        self._static_analysis = static_analysis_
        self._test = test_
        self._valgrind = valgrind_

    def __str__(self):
        if self == Mode.Clean:
            return "clean"
        elif self == Mode.Full:
            return "full"
        elif self == Mode.Fast:
            return "fast"
        elif self == Mode.Verify:
            return "verify"

        return "verify"

    @property
    def clean(self):
        return self._clean

    @property
    def format(self):
        return self._format

    @property
    def clang(self):
        return self._clang

    @property
    def gcc(self):
        return self._gcc

    @property
    def debug(self):
        return self._debug

    @property
    def release(self):
        return self._release

    @property
    def static_analysis(self):
        return self._static_analysis

    @property
    def test(self):
        return self._test

    @property
    def valgrind(self):
        return self._valgrind


@enum.unique
class CMakePresets(enum.Enum):
    LinuxClang = ("configure_linux_clang", "build_linux_clang", "test_linux_clang")
    LinuxGcc = ("configure_linux_gcc", "build_linux_gcc", "test_linux_gcc")

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

    def __init__(self, config_name):
        self.config_name = config_name

    def __str__(self):
        return self.config_name


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


def is_linux():
    return platform.system() == "Linux"


def build_dir_from_config_preset(preset):
    with open(f"{PROJECT_ROOT_DIR}/infrastructure/CMakePresets.json") as f:
        data = json.load(f)
        preset = [p for p in data["configurePresets"] if p["name"] == preset]
        assert len(preset) == 1
        preset = preset[0]

        binary_dir = preset["binaryDir"]
        binary_dir.replace("${sourceDir}", f"{PROJECT_ROOT_DIR}/infrastructure")

        return os.path.realpath(binary_dir)


def remove_dir(path):
    if os.path.exists(path) and os.path.isdir(path):
        shutil.rmtree(path)


def clean_build_dir(preset):
    build_dir = build_dir_from_config_preset(preset.configure)
    remove_dir(build_dir)


def find_files_by_name(regex):
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
            if re.search(regex, f) is not None:
                output.append(os.path.realpath(os.path.join(root, f)))

    output.sort()

    return output


def configure_cmake(preset) -> bool:
    build_dir = build_dir_from_config_preset(preset.configure)

    needs_configure = False
    if not os.path.exists(build_dir):
        needs_configure = True
        os.mkdir(build_dir)

    if not needs_configure:
        return True

    with contextlib.chdir(f"{PROJECT_ROOT_DIR}/infrastructure"):
        config_cmd = ["cmake", "--preset", f"{preset.configure}"]

        result = subprocess.run(config_cmd)

        return result.returncode == 0


def build_cmake(config, preset) -> bool:
    with contextlib.chdir(f"{PROJECT_ROOT_DIR}/infrastructure"):
        build_cmd = [
            "cmake",
            "--build",
            "--preset",
            f"{preset.build}",
            "--config",
            f"{config}",
            "--parallel",
            f"{os.process_cpu_count()}",
        ]

        result = subprocess.run(build_cmd)

        return result.returncode == 0


def run_ctest(preset, build_config, jobs, label_include_regex) -> bool:
    with contextlib.chdir(f"{PROJECT_ROOT_DIR}/infrastructure"):
        result = subprocess.run(
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

    return result.returncode == 0


def run_clang_tidy(preset) -> bool:
    build_dir = build_dir_from_config_preset(preset.configure)

    files = find_files_by_name(r"\.cpp$")
    files += find_files_by_name(r"\.hpp$")

    for f in files:
        print(f"Analyzing {f}...")
        result = subprocess.run(
            [
                "clang-tidy-20",
                f"--config-file={PROJECT_ROOT_DIR}/infrastructure/.clang-tidy-20",
                "-p",
                build_dir,
                f,
            ]
        )
        if result.returncode != 0:
            print(f"Error running clang-tidy on {f}")

            return False

    return True


def run_tests(mode, preset) -> bool:
    jobs = os.process_cpu_count()
    regex = r"^test$"

    if mode.debug:
        result = run_ctest(preset, CMakeBuildConfig.Debug, jobs, regex)
        if not result:
            return False

    if mode.release:
        result = run_ctest(preset, CMakeBuildConfig.RelWithDebInfo, jobs, regex)
        if not result:
            return False
        result = run_ctest(preset, CMakeBuildConfig.Release, jobs, regex)
        if not result:
            return False

    return True


def run_valgrind(mode, preset) -> bool:
    jobs = 1
    regex = r"^valgrind$"

    if mode.debug:
        result = run_ctest(preset, CMakeBuildConfig.Debug, jobs, regex)
        if not result:
            return False

    return True


def _linux_compile(mode, preset, env_patch):
    env = os.environ.copy()
    env.update(env_patch)
    with NewEnv(env):
        result = configure_cmake(preset)
        if not result:
            return False

        if mode.debug:
            result = build_cmake(CMakeBuildConfig.Debug, preset)
            if not result:
                return False

        if mode.release:
            result = build_cmake(CMakeBuildConfig.RelWithDebInfo, preset)
            if not result:
                return False
            result = build_cmake(CMakeBuildConfig.Release, preset)
            if not result:
                return False

    return True


def build_gcc_linux(mode, preset) -> bool:
    return _linux_compile(mode, preset, {"CC": "gcc-15", "CXX": "g++-15"})


def build_clang_linux(mode, preset) -> bool:
    return _linux_compile(mode, preset, {"CC": "clang-20", "CXX": "clang++-20"})


def format_json() -> bool:
    files = find_files_by_name(r"\.json$")

    for f in files:
        with open(f, "r") as handle:
            data = json.load(handle)

        data_str = json.dumps(data, indent=2, sort_keys=True)
        data_str += "\n"
        with open(f, "w") as handle:
            handle.write(data_str)

    return True


def format_cmake() -> bool:
    files = find_files_by_name(r"^CMakeLists\.txt$")
    files += find_files_by_name(r"\.cmake$")

    for f in files:
        result = subprocess.run(["cmake-format", "-i", f])

        if result.returncode != 0:
            print(f"Error running cmake-format on {f}")

            return False

    return True


def format_python() -> bool:
    files = find_files_by_name(r"\.py$")

    for f in files:
        result = subprocess.run(["black", "-q", f])

        if result.returncode != 0:
            print(f"Error running black on {f}")

            return False

    return True


def format_cpp() -> bool:
    files = find_files_by_name(r"\.cpp$")
    files += find_files_by_name(r"\.hpp$")

    for f in files:
        result = subprocess.run(
            [
                "clang-format-20",
                f"--style=file:{PROJECT_ROOT_DIR}/infrastructure/.clang-format-20",
                "-i",
                f,
            ]
        )
        if result.returncode != 0:
            print(f"Error running clang-format on {f}")

            return False

    return True


class CliConfig:
    def __init__(self, mode_str):
        self.mode = None
        if mode_str == f"{Mode.Clean}":
            self.mode = Mode.Clean
        if mode_str == f"{Mode.Fast}":
            self.mode = Mode.Fast
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
        choices=[f"{Mode.Clean}", f"{Mode.Fast}", f"{Mode.Full}", f"{Mode.Verify}"],
        metavar="mode",
        help=f"""Selects build mode.
                 Mode "{Mode.Clean}" deletes the build trees.
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
    config, result = preamble()
    if not result:
        return 1

    mode = config.mode

    if mode.clean:
        clean_build_dir(CMakePresets.LinuxGcc)
        clean_build_dir(CMakePresets.LinuxClang)

    if mode.format:
        print("Formatting JSON files...")
        result = format_json()
        if not result:
            return 1
        print("Formatting C++ files...")
        result = format_cpp()
        if not result:
            return 1
        print("Formatting Python files...")
        result = format_python()
        if not result:
            return 1
        print("Formatting CMake files...")
        result = format_cmake()
        if not result:
            return 1

    if mode.gcc:
        print("Running GCC build...")
        result = build_gcc_linux(mode, CMakePresets.LinuxGcc)
        if not result:
            return 1

        if mode.test:
            print("Testing GCC build...")
            result = run_tests(mode, CMakePresets.LinuxGcc)
            if not result:
                return 1

    if mode.clang:
        print("Running Clang build...")
        result = build_clang_linux(mode, CMakePresets.LinuxClang)
        if not result:
            return 1

        if mode.test:
            print("Testing Clang build...")
            result = run_tests(mode, CMakePresets.LinuxClang)
            if not result:
                return 1

    if mode.valgrind:
        # Test debug with Valgrind, release may yield false positives
        mode_valgrind = Mode.Valgrind
        assert mode_valgrind.debug
        assert not mode_valgrind.release

        result = build_gcc_linux(mode_valgrind, CMakePresets.LinuxGcc)
        if not result:
            return 1
        result = build_clang_linux(mode_valgrind, CMakePresets.LinuxClang)
        if not result:
            return 1

        print("Testing GCC build with Valgrind...")
        result = run_valgrind(mode_valgrind, CMakePresets.LinuxGcc)
        if not result:
            return 1

        print("Testing Clang build with Valgrind...")
        result = run_valgrind(mode_valgrind, CMakePresets.LinuxClang)
        if not result:
            return 1

    if mode.static_analysis:
        # Need to build all configs for static analysis
        mode_static_analysis = Mode.StaticAnalysis
        assert mode_static_analysis.debug
        assert mode_static_analysis.release

        result = build_clang_linux(mode_static_analysis, CMakePresets.LinuxClang)
        if not result:
            return 1

        print("Running clang-tidy analysis...")
        result = run_clang_tidy(CMakePresets.LinuxClang)
        if not result:
            return 1

    print(f"Success: {os.path.basename(sys.argv[0])}")

    return 0


if __name__ == "__main__":
    exit(main())
