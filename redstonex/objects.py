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
    def __init__(self, limit: int = 4, power: int = 15) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit, power)

class Slot(_core.CoreSlotObject):
    def __init__(self, parent: _core.ConnectiveObject, power_type: PowerType = PowerType.NONE) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, parent, power_type)

class RelaySource(_core.CoreRelaySource):
    def __init__(self, power: int = 15, max_delay: int = 4) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, power, max_delay)

class ComparatorSource(_core.CoreComparatorSource):
    def __init__(self, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, delay)

class TorchSource(_core.CoreTorchSource):
    def __init__(self, power: int = 15, delay: int = 1) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, power, delay)

class Block(_core.CoreBlock):
    def __init__(self, limit: int = 6) -> None:
        auto_id = _get_next_id()
        super().__init__(auto_id, limit)
