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

from redstonex.objects import ComparatorSource, Line, RelaySource, Source
from redstonex.simulator import LogLevel, Simulator

if __name__ == "__main__":
    sim = Simulator()
    sim.enable_logging(LogLevel.ERROR)

    source = Source()
    relay = RelaySource()
    cp = ComparatorSource()

    line_L1 = Line()
    line_L2 = Line()
    line_L3 = Line()
    line_L4 = Line()

    source.connect(line_L1)
    line_L1.connect(line_L2)
    line_L2.connect(relay.input_slot)
    relay.output_slot.connect(line_L3)
    line_L3.connect(cp.input_slot)
    cp.output_slot.connect(line_L4)

    sim.bind_object(source, line_L1, line_L2, line_L3, line_L4, relay, cp)

    sim.run()

    print(f"source: {source.power}")
    print(f"line_L1: {line_L1.power}")
    print(f"line_L2: {line_L2.power}")
    print(f"relay: {relay.power}")
    print(f"lineL3: {line_L3.power}")
    print(f"comparator: {cp.power}")
    print(f"line_L4: {line_L4.power}")

