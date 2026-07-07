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

class ConnectiveError(Exception):
    def __init__(self, message: str, source = None, target = None) -> None:
        full_message = f"[{message}] " if message else ""
        if source and target:
            full_message += f"Source: {source.__class__.__name__} -> Target: {target.__class__.__name__} | Connective Error: Type not Matched!"
        else:
            full_message += "Connective Error: Type not Matched!"
        super().__init__(full_message)

        self.source = source
        self.target = target

class ConnectionLimitError(Exception):
    def __init__(self, message: str, limit: int = 1) -> None:
        full_message = f"[{message}] " if message else ""
        full_message += f"Connection Limit Error: connection is limited to {limit}"

        super().__init__(full_message)

def _safe_connect(source_py_obj, target_py_obj):
    success = super(source_py_obj.__class__, source_py_obj).connect(target_py_obj)

    if not success:
        if hasattr(source_py_obj, "limit") and hasattr(source_py_obj, "connect_count"):
            if source_py_obj.connect_count >= source_py_obj.limit:
                raise ConnectionLimitError("Connection failed", source_py_obj.limit)
        
        raise ConnectiveError("Connection refused by RedStoneX", source=source_py_obj, target=target_py_obj)

def _safe_disconnect(source_py_obj, target_py_obj):
    success = super(source_py_obj.__class__, source_py_obj).connect(target_py_obj)

    if not success:
        raise ConnectiveError("Disconnection refused by RedStoneX", source=source_py_obj, target=target_py_obj)

class Line(_core.CoreLineObject):
    def __init__(self, limit: int = 4) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

class Source(_core.CoreSourceObject):
    """代表所有能发出红石源头的信号源

    Attributes:
        limit (str): 能连接的最多的物件
        power (int): 红石源的信号强度
    """

    def __init__(self, limit: int = 4, power: int = 15) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit, power)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

class Slot(_core.CoreSlotObject):
    def __init__(self, parent: _core.ConnectiveObject, power_type: PowerType = PowerType.NONE) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, parent, power_type)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

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

    def set_delay():
        pass

    @property
    def input_slot(self):
        return _core.CoreConnectiveObject.create_view(self._input_slot_ptr)

    @property
    def output_slot(self):
        return _core.CoreConnectiveObject.create_view(self._output_slot_ptr)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

class ComparatorSource(_core.CoreComparatorSource):
    def __init__(self, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, delay)

    @property
    def input_slot(self):
        return _core.CoreConnectiveObject.create_view(self._input_slot_ptr)

    @property
    def calculate_slot_a(self):
        return _core.CoreConnectiveObject.create_view(self._caculate_slot_a_ptr)
    
    @property
    def calculate_slot_b(self):
        return _core.CoreConnectiveObject.create_view(self._caculate_slot_b_ptr)

    @property
    def output_slot(self):
        return _core.CoreConnectiveObject.create_view(self._output_slot_ptr)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

class TorchSource(_core.CoreTorchSource):
    def __init__(self, power: int = 15, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, power, delay)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

    @property
    def bottom_slot(self):
        return _core.CoreConnectiveObject.create_view(self._bottom_slot_ptr)

    @property
    def power_slot(self):
        return _core.CoreConnectiveObject.create_view(self._power_slot_ptr)

class Block(_core.CoreBlock):
    def __init__(self, limit: int = 6) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)

class Custom(_core.CoreCustomObject):
    def __init__(self, plugin_name: str, **kwargs) -> None:
        auto_id = _get_next_id()
        kwargs["id"] = auto_id
        
        super().__init__(plugin_name, **kwargs)

    def connect(self, target) -> None:
        _safe_connect(self, target)

    def disconnect(self, target) -> None:
        _safe_disconnect(self, target)
