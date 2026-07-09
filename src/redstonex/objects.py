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

from . import _core
from .types import PowerType

_id_counter = 0

# 自动计数器，写什么global_id++的重复劳动太讨厌了
def _get_next_id() -> int:
    global _id_counter
    _id_counter += 1
    return _id_counter

class Line(_core.CoreLineObject):
    def __init__(self, limit: int = 4) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit)

class Source(_core.CoreSourceObject):
    """代表所有能发出红石源头的信号源

    Attributes:
        limit (str): 能连接的最多的物件
        power (int): 红石源的信号强度
    """

    def __init__(self, limit: int = 4, power: int = 15) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit, power)

class Slot(_core.CoreSlotObject):
    def __init__(self, parent: _core.ConnectiveObject, power_type: PowerType = PowerType.NONE) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, parent, power_type)

class RelaySource(_core.CoreRelaySource):
    """红石中继器，连接的时候不能直接连接他的本体哦，只能连接他的input_slot和output_slot
    而且两个接口都是只能连接一个元件的哦

    Attributes:
        power (int): 红石源的信号强度
        max_delay (int): 设置最大延迟时间，游戏里面中继器最大延时4个Tick哦，但是毕竟RedStoneX是红石的超集，你可以调整到更大
    """

    def __init__(self, power: int = 15, max_delay: int = 4) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, power, max_delay)

    def set_delay(self, delay: int):
        super().set_delay(delay)

    @property
    def input_slot(self):
        return self._input_slot

    @property
    def output_slot(self):
        return self._output_slot

class ComparatorSourceMode(_core.CoreComparatorSourceMode):
    pass

class ComparatorSource(_core.CoreComparatorSource):
    def __init__(self, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, delay)

    def set_mode(self, mode: ComparatorSourceMode):
        super().set_mode(mode)

    @property
    def input_slot(self):
        return self._input_slot

    @property
    def calculate_slot_a(self):
        return self._calculate_slot_a
    
    @property
    def calculate_slot_b(self):
        return self._calculate_slot_b

    @property
    def output_slot(self):
        return self._output_slot

class TorchSource(_core.CoreTorchSource):
    def __init__(self, power: int = 15, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, power, delay)

    @property
    def bottom_slot(self):
        return self._bottom_slot

    @property
    def power_slot(self):
        return self._power_slot

class Block(_core.CoreBlock):
    def __init__(self, limit: int = 6) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit)

class Custom(_core.CoreCustomObject):
    def __init__(self, plugin_name: str, **kwargs) -> None:
        auto_id = _get_next_id()
        kwargs["id"] = auto_id
        
        super().__init__(plugin_name, **kwargs)
