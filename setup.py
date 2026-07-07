# 以下全部来自Gemini
import os
import subprocess
import sys
import pybind11
from setuptools import Extension, setup, find_packages
from setuptools.command.build_ext import build_ext

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        setup_py_dir = os.path.abspath(os.path.dirname(__file__))
        self.sourcedir = os.path.abspath(os.path.join(setup_py_dir, sourcedir))

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            "-DCMAKE_BUILD_TYPE=Release",
            f"-Dpybind11_DIR={pybind11.get_cmake_dir()}",
        ]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        # 1. Configure CMake
        subprocess.check_call(["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp)
        # 2. Build C++ code
        subprocess.check_call(["cmake", "--build", "."], cwd=self.build_temp)

setup(
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    ext_modules=[CMakeExtension("redstonex._core", sourcedir=".")],
    cmdclass={"build_ext": CMakeBuild},
    package_data={
        "redstonex": [
            "*.pyi", 
            "include/*.h"
        ]
    },
    include_package_data=True,
)
