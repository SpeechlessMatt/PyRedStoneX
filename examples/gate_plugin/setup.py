from setuptools import setup, Extension
import os

ext_modules = [
    Extension(
        name="plugin._core", 
        sources=[
            os.path.join("src", "gate_plugin", "csrc", "my_c_code.c")
        ],
        include_dirs=[
            os.path.join("src", "plugin", "include")
        ],
    )
]

setup(ext_modules=ext_modules)
