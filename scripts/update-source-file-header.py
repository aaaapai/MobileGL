#!/usr/bin/env python3
import os
import sys
import pathlib
from glob import glob

try:
    import colorama
    colorama.init()
    CYAN = colorama.Fore.CYAN + colorama.Style.BRIGHT
    GREEN = colorama.Fore.GREEN
    YELLOW = colorama.Fore.YELLOW
    RED = colorama.Fore.RED
    RESET = colorama.Style.RESET_ALL
except ImportError:
    CYAN = GREEN = YELLOW = RED = RESET = ''

OK = "✔"
FAIL = "✖"
ARROW = "╰─>"
SIMPLE_ARROW = "─>"

HEADER_TEMPLATE = """// MobileGL - {file_path}
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header
"""

HEADER_MARKER = """End of Source File Header
"""

SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()
TARGET_DIR = SCRIPT_DIR / "../MobileGL"
FILE_PATTERNS = ["*.h", "*.hpp", "*.cpp", "*.cc", "*.c"]

if not TARGET_DIR.exists():
    print(f"{YELLOW}Target directory '{TARGET_DIR}' does not exist.{RESET}")
    sys.exit(1)

def find_source_files():
    candidates = []
    for root, dirs, files in os.walk(TARGET_DIR):
        dirs[:] = [d for d in dirs if d not in ("build", "out")]
        for pattern in FILE_PATTERNS:
            for f in glob(os.path.join(root, pattern)):
                candidates.append(f)
    return candidates

candidates = find_source_files()
total = len(candidates)
print(f"{YELLOW}Found {total} candidate(s).{RESET}")

if total == 0:
    sys.exit(0)

def draw_progress(current, total, width=36):
    percent = int(current * 100 / total) if total else 0
    filled = int(current * width / total) if total else 0
    empty = width - filled
    bar = "█" * filled + "░" * empty
    print(f"\r{CYAN}Progress{RESET} [{current}/{total}] {percent:3d}% |{bar}|", end='')

processed = 0
skipped = 0
for idx, file_path in enumerate(candidates, start=1):
    draw_progress(idx, total)
    try:
        with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()

        rel_path = os.path.relpath(file_path, SCRIPT_DIR.parent)
        new_header = HEADER_TEMPLATE.format(file_path=rel_path.replace("\\", "/"))

        if HEADER_MARKER in content:
            rest = content.split(HEADER_MARKER, 1)[1]
            new_content = new_header + rest
        else:
            new_content = new_header + "\n" + content

        if new_content != content:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(new_content)
            processed += 1
            print(f"\n{CYAN}Processing{RESET} {SIMPLE_ARROW} {file_path}\n   {ARROW} {GREEN}{OK}{RESET} Updated")
        else:
            skipped += 1
            print(f"\n{CYAN}Processing{RESET} {SIMPLE_ARROW} {file_path}\n   {ARROW} {GREEN}{OK}{RESET} {YELLOW}No changes{RESET}")

    except Exception as e:
        print(f"\n{CYAN}Processing{RESET} {SIMPLE_ARROW} {file_path}\n   {ARROW} {RED}{FAIL}{RESET} {e}")

print(f"\n{GREEN}All done!{RESET} Processed: {processed}, Skipped: {skipped}, Total: {total}")

