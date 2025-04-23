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
    Fast = (False, False, True, False, True, False, False, True)
    Full = (False, True, True, True, True, True, True, True)
    StaticAnalysis = (False, False, True, False, True, True, True, False)
    Clean = (True, True, True, True, True, True, True, True)

    def __init__(
        self, clean_, format_, clang_, gcc_, debug_, release_, static_analysis_, test_
    ):
        self._clean = clean_
        self._format = format_
        self._clang = clang_
        self._gcc = gcc_
        self._debug = debug_
        self._release = release_
        self._static_analysis = static_analysis_
        self._test = test_

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


def run_ctest(preset, build_config) -> bool:
    with contextlib.chdir(f"{PROJECT_ROOT_DIR}/infrastructure"):
        result = subprocess.run(
            [
                "ctest",
                "--preset",
                preset.test,
                "--build-config",
                f"{build_config}",
                "--parallel",
                f"{os.process_cpu_count()}",
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
    if mode.debug:
        result = run_ctest(preset, CMakeBuildConfig.Debug)
        if not result:
            return False

    if mode.release:
        result = run_ctest(preset, CMakeBuildConfig.RelWithDebInfo)
        if not result:
            return False
        result = run_ctest(preset, CMakeBuildConfig.Release)
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


def preamble() -> tuple[Mode, bool]:
    if not is_linux():
        print(f"Unknown OS: {platform.system()}")
        return Mode.Clean, False

    if len(sys.argv) > 1:
        if len(sys.argv) == 2 and sys.argv[1] == "fast":
            return Mode.Fast, True
        elif len(sys.argv) == 2 and sys.argv[1] == "full":
            return Mode.Full, True
        elif len(sys.argv) == 2 and sys.argv[1] == "clean":
            return Mode.Clean, True

        print(
            "Invalid argument(s): expected 'clean' (default), 'full', 'fast' or nothing"
        )
        return Mode.Clean, False

    return Mode.Clean, False


def main() -> int:
    mode, result = preamble()
    if not result:
        return 1

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
        if mode.clean:
            clean_build_dir(CMakePresets.LinuxGcc)

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
        if mode.clean:
            clean_build_dir(CMakePresets.LinuxClang)

        print("Running Clang build...")
        result = build_clang_linux(mode, CMakePresets.LinuxClang)
        if not result:
            return 1

        if mode.test:
            print("Testing Clang build...")
            result = run_tests(mode, CMakePresets.LinuxClang)
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
