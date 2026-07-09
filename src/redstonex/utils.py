from pathlib import Path

_pkg_dir = Path(__file__).resolve().parent

def get_include_path() -> str:
    return str(_pkg_dir / "include")

def get_library_path() -> str:
    return str(_pkg_dir)

def get_cmake_path() -> str:
    return str(_pkg_dir / "cmake")
