"""Preload the stdlib supported by this WASI build before enabling bot imports."""

from __future__ import annotations

import importlib
import os
import sys
import warnings

# These are programs/demos, rather than libraries: importing them prints text
# or starts a browser. Package __main__ modules are also command entry points.
_PROGRAMS = {"antigravity", "this", "__hello__", "__phello__"}
# These modules do not report their unsupported platform with ImportError.
_PLATFORM_ONLY = {
    "_ios_support": "requires iOS/ctypes",
    "multiprocessing.popen_fork": "requires os.fork/os.WNOHANG",
    "multiprocessing.popen_forkserver": "requires fork",
    "multiprocessing.popen_spawn_posix": "requires fork support",
}
PRELOADED: tuple[str, ...] = ()
UNAVAILABLE: dict[str, str] = {}


def prepare(root: str = "/lib/python3.13", bot_root: str = "/guest") -> None:
    global PRELOADED
    previous = set(sys.modules)
    names = set(sys.stdlib_module_names)
    for directory, directories, files in os.walk(root):
        directories[:] = sorted(
            name for name in directories if name not in {"__pycache__", "cambc", "site-packages"}
        )
        for filename in sorted(files):
            if not filename.endswith(".py"):
                continue
            parts = os.path.relpath(os.path.join(directory, filename), root)[:-3].split(os.sep)
            if parts[-1] == "__init__":
                parts.pop()
            if parts:
                names.add(".".join(parts))

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        for name in sorted(names):
            if name.split(".")[0] in _PROGRAMS or name.split(".")[-1] == "__main__":
                continue
            if name in _PLATFORM_ONLY:
                UNAVAILABLE[name] = _PLATFORM_ONLY[name]
                continue
            try:
                importlib.import_module(name)
            except ImportError as error:
                # Optional C extensions and libraries absent from the WASI
                # distribution remain unavailable, as with a normal import.
                UNAVAILABLE[name] = str(error)

    # Newly preloaded libraries must not hide a bot's own module/package.
    # Modules already loaded by the interpreter/SDK retain their normal priority.
    shadowed = {
        name[:-3] if name.endswith(".py") else name
        for name in os.listdir(bot_root)
        if name.endswith(".py") or os.path.isfile(os.path.join(bot_root, name, "__init__.py"))
    } - previous
    for name in tuple(sys.modules):
        if name.split(".")[0] in shadowed:
            del sys.modules[name]
    for name in tuple(UNAVAILABLE):
        if name in sys.modules:
            del UNAVAILABLE[name]
    PRELOADED = tuple(sorted(
        name for name in sys.modules if name.split(".")[0] in sys.stdlib_module_names
    ))
