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

from typing import Iterable
from . import _core

class LogLevel(_core.CoreLogLevel):
    pass

class Simulator(_core.CoreSimulator):
    def __init__(self) -> None:
        super().__init__()

    def run(self) -> None:
        return super().run()

    def pause(self) -> None:
        return super().pause()

    def step(self) -> bool:
        """单步推进一个tick
        
        Returns:
            bool: 如果当前tick有执行内容，或者未来还有日程未处理，返回True
                  如果连续一整圈轮子都为空（电路彻底静止），返回False
        """
        return super().step()

    def enable_logging(self, level: LogLevel = LogLevel.INFO):
        return super().enable_logging(level)

    def add_tick_breakpoint(self, tick: int) -> None:
        if (tick < 0):
            raise RuntimeError("Cannot set a negative time value for breakpoint! ")
        return super().add_tick_breakpoint(tick)

    def remove_tick_breakpoint(self, tick: int) -> None:
        if (tick < 0):
            raise RuntimeError("Cannot remove a negative time value for breakpoint! ")
        return super().remove_tick_breakpoint(tick)

    def bind_object(self, *objects: _core.CoreConnectiveObject) -> None:
        for item in objects:
            super().bind_object(item)

    def bind_objects(self, objects_list: Iterable):
        self.bind_object(*objects_list)


