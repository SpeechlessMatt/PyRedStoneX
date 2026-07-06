# Copyright (C) 2026 Czy_4201b
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import ctypes
import os
from . import _core

_loaded_libs = {}

class PluginRegistry:
    def __init__(self, plugin_path: str):
        self.abs_path = os.path.abspath(plugin_path)
        
        if self.abs_path not in _loaded_libs:
            _loaded_libs[self.abs_path] = ctypes.CDLL(self.abs_path)
            
        self.plugin_lib = _loaded_libs[self.abs_path]

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        # 过掉可恶的pyright检查
        f"{exc_type} {exc_val} {exc_tb}"
        return False

    def add(self, component_name: str, create: str, destroy: str):
        try:
            create_func = getattr(self.plugin_lib, create)
            destroy_func = getattr(self.plugin_lib, destroy)
            
            create_ptr = ctypes.cast(create_func, ctypes.c_void_p).value
            destroy_ptr = ctypes.cast(destroy_func, ctypes.c_void_p).value
            
        except AttributeError:
            lib_name = os.path.basename(self.abs_path)
            raise AttributeError(f"在动态库 {lib_name} 中未找到指定的 C 函数对: {create} 或 {destroy}！如果是cpp开发的话，记得写extern 'C'哦")

        _core.register_plugin_from_ptr(component_name, create_ptr, destroy_ptr)

        return self

