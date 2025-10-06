# Copyright (c) 2025 Wojciech Kałuża
# SPDX-License-Identifier: MIT
# For license details, see LICENSE file

import datetime
import hashlib
import os
import re
import subprocess
import sys
import tempfile
import typing

THIS_SCRIPT_DIR = os.path.realpath(os.path.dirname(__file__))
PROJECT_ROOT_DIR = os.path.realpath(f"{THIS_SCRIPT_DIR}/..")

COPYRIGHT_HOLDER_NAME = "Wojciech Kałuża"


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


def get_changed_files() -> typing.List[str]:
    assert os.getcwd() == PROJECT_ROOT_DIR
    success, output = run(["git", "diff", "--cached", "--name-only"])
    if not success:
        return find_files_by_name(PROJECT_ROOT_DIR, lambda x: True)

    files = output.strip().split("\n")
    out = []
    for f in files:
        path = os.path.realpath(f"{PROJECT_ROOT_DIR}/{f.strip()}")
        if os.path.isfile(path):
            out.append(path)

    out.sort()

    return out


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


def is_bash_file(f) -> bool:
    return re.search(r"\.bash$", f) is not None


def is_cmake_file(f) -> bool:
    return (
        re.search(r"CMakeLists\.txt$", f) is not None
        or re.search(r"\.cmake$", f) is not None
    )


def is_cpp_file(f) -> bool:
    return re.search(r"\.cpp$", f) is not None or re.search(r"\.hpp$", f) is not None


def is_docker_file(f) -> bool:
    return re.search(r"\.dockerfile$", f) is not None


def is_python_file(f) -> bool:
    return re.search(r"\.py$", f) is not None


def is_file_with_licensing_comment(f) -> bool:
    return (
        is_bash_file(f)
        or is_cmake_file(f)
        or is_cpp_file(f)
        or is_docker_file(f)
        or is_python_file(f)
    )


def validate_notice_of_copyright(file: str, copyright_notice: str) -> bool:
    pattern_single_year = r"^Copyright \(c\) ([0-9]{4}) (.+)$"
    pattern_year_range = r"^Copyright \(c\) ([0-9]{4})-([0-9]{4}) (.+)$"

    single_year = re.match(pattern_single_year, copyright_notice)
    year_range = re.match(pattern_year_range, copyright_notice)

    if single_year is None and year_range is None:
        print(f"Error ({file}):\n" "Notice of copyright not found or is malformed")

        return False

    current_year = datetime.datetime.now().year

    result = re.match(pattern_single_year, copyright_notice)
    if result is not None:
        name = result.group(2)
        if name != COPYRIGHT_HOLDER_NAME:
            print(f"Error ({file}):\n" "Unexpected copyright holder name")

            return False

        start_year = int(result.group(1))
        if current_year < start_year:
            print(
                f"Error ({file}):\n"
                "Year in notice of copyright appears to be in the future "
                f"({start_year}; current year is {current_year})"
            )

            return False

        if start_year == current_year:
            return True

        print(
            f"Error ({file}):\n"
            f'Notice of copyright begins with "Copyright (c) {start_year}", '
            f'but it should begin with "Copyright (c) {start_year}-{current_year}"'
        )

        return False

    result = re.match(pattern_year_range, copyright_notice)
    if result is not None:
        name = result.group(3)
        if name != COPYRIGHT_HOLDER_NAME:
            print(f"Error ({file}):\n" "Unexpected copyright holder name")

            return False

        start_year = int(result.group(1))
        end_year = int(result.group(2))
        if end_year <= start_year:
            print(
                f"Error ({file}):\n"
                f"Malformed year range in notice of copyright ({start_year}-{end_year})"
            )

            return False

        if current_year < end_year:
            print(
                f"Error ({file}):\n"
                "Year in notice of copyright appears to be in the future "
                f"({start_year}-{end_year}; current year is {current_year})"
            )

            return False

        if end_year == current_year:
            return True

        print(
            f"Error ({file}):\n"
            f'Notice of copyright begins with "Copyright (c) {start_year}-{end_year}", '
            f'but it should begin with "Copyright (c) {start_year}-{current_year}"'
        )

        return False

    return True


def check_license_file() -> bool:
    license_file_path = os.path.realpath(f"{PROJECT_ROOT_DIR}/LICENSE")
    if not os.path.isfile(license_file_path):
        print(f"Error: file {license_file_path} does not exist")

        return False

    with open(license_file_path, "br") as f:
        data = f.read()
    sha3_256 = hashlib.sha3_256()
    sha3_256.update(data)
    sha3_256_digest = sha3_256.hexdigest()
    expected_sha3_256_digest = (
        "4885e645412f0073f7c5211e3dd1c581e87e67e6ec1ac33dac1221d3fe66101c"
    )

    if sha3_256_digest != expected_sha3_256_digest:
        print(
            f"Unexpected LICENSE file digest: {sha3_256_digest}\n"
            "Verify the LICENSE file is correct and update the variable "
            f"expected_sha3_256_digest in {os.path.basename(sys.argv[0])}"
        )

        return False

    with open(license_file_path, "r") as f:
        lines = f.readlines()
    lines = [line.strip() for line in lines]
    copyright_lines = [
        line
        for line in lines
        if re.match(r"^Copyright \(c\) [0-9]{4}[\- ].+$", line) is not None
    ]
    if len(copyright_lines) != 1:
        print(
            f"Error ({license_file_path}):\n"
            "Notice of copyright not found or multiple lines matched in error"
        )

        return False

    copyright_notice = copyright_lines[0]
    success = validate_notice_of_copyright(license_file_path, copyright_notice)
    if not success:
        return False

    return True


def match_copyright_notice_pattern(text: str):
    return re.match(r"^(?://|#) (Copyright \(c\) [0-9]{4}[\- ].+)$", text)


def check_license_comments_in_changed_files() -> bool:
    files = get_changed_files()
    files = [f for f in files if is_file_with_licensing_comment(f)]
    for file in files:
        with open(file, "r") as f:
            lines = f.readlines()
        lines = lines[0:3]
        lines = [line.strip() for line in lines]
        copyright_lines = [
            line for line in lines if match_copyright_notice_pattern(line) is not None
        ]
        if len(copyright_lines) != 1:
            print(
                f"Error ({file}):\n"
                "Notice of copyright not found or multiple lines matched in error"
            )

            return False

        copyright_notice = match_copyright_notice_pattern(copyright_lines[0]).group(1)
        success = validate_notice_of_copyright(file, copyright_notice)
        if not success:
            return False

        spdx_license_id_lines = [
            line
            for line in lines
            if re.match(r"^(?://|#) SPDX-License-Identifier: .+$", line) is not None
        ]
        if len(spdx_license_id_lines) != 1:
            print(
                f"Error ({file}):\n"
                "SPDX-License-Identifier not found or multiple lines matched in error"
            )

            return False

        license_file_ref_lines = [
            line
            for line in lines
            if re.match(r"^(?://|#) For license details, see LICENSE file$", line)
            is not None
        ]
        if len(license_file_ref_lines) != 1:
            print(
                f"Error ({file}):\n"
                "Reference to LICENSE file not found or multiple lines matched in error"
            )

            return False

    return True


def main() -> int:
    success = check_license_file()
    if not success:
        return 1

    success = check_license_comments_in_changed_files()
    if not success:
        return 1

    return 0


if __name__ == "__main__":
    assert os.getcwd() == PROJECT_ROOT_DIR
    exit(main())
