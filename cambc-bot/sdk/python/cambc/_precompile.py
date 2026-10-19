"""Compile fresh bot sources and initialize modules, saving main.py for the match."""

from __future__ import annotations

import importlib
import os
from importlib.machinery import SourceFileLoader
from types import CodeType

_original_get_code = SourceFileLoader.get_code


class PreloadError(Exception):
    """A submission failed while its modules were being initialized."""


def prepare(root: str = "/guest") -> CodeType:
    """Freeze bot modules into the snapshot; return the unexecuted entry point."""
    root = os.path.abspath(root)
    compiled: dict[str, CodeType] = {}
    modules: set[str] = set()
    for directory, directories, files in os.walk(root):
        directories[:] = sorted(name for name in directories if name != "__pycache__")
        for name in sorted(files):
            if not name.endswith(".py"):
                continue
            filename = os.path.join(directory, name)
            # Always read source, never an existing .pyc (even unchecked-hash
            # bytecode). Compile bytes to honor source encoding declarations.
            with open(filename, "rb") as source:
                compiled[filename] = compile(source.read(), filename, "exec", dont_inherit=True)
            parts = os.path.relpath(filename, root)[:-3].split(os.sep)
            if parts[-1] == "__init__":
                parts.pop()
            # Entry points and files that are not importable module names are
            # compiled, but not executed. Package/namespace parents are handled
            # by Python's regular importer, including relative/circular imports.
            if parts and parts != ["main"] and "__main__" not in parts:
                if all(part.isidentifier() for part in parts):
                    modules.add(".".join(parts))

    main_path = os.path.join(root, "main.py")
    main = compiled[main_path]
    preparing = True

    def get_code(loader: SourceFileLoader, fullname: str) -> CodeType | None:
        path = os.path.abspath(loader.path)
        if preparing and path == main_path:
            raise PreloadError("main.py runs when the game starts; move shared definitions to another module")
        cached = compiled.get(path)
        if cached is not None:
            return cached
        return _original_get_code(loader, fullname)

    # Keep standard module discovery, packages, relative imports and tracebacks.
    SourceFileLoader.get_code = get_code  # type: ignore[method-assign, assignment]
    for name in sorted(modules):
        try:
            importlib.import_module(name)
        except MemoryError:
            raise
        except BaseException as error:
            raise PreloadError(f"Could not preload bot module {name!r}") from error
    preparing = False
    return main
