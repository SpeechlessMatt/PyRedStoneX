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

from redstonex.objects import Block, ComparatorSource, Line, RelaySource, Source
from redstonex.simulator import LogLevel, Simulator

def connect_line_chain(line_list):
    for i in range(len(line_list) - 1):
        line_list[i].connect(line_list[i + 1])

if __name__ == "__main__":
    sim = Simulator()
    sim.enable_logging(LogLevel.ERROR)

    limit = 4
    source_power = 15
    delay = 1

    source = Source(limit=limit, power=source_power)

    comparator_1 = ComparatorSource(delay=delay)
    comparator_2 = ComparatorSource(delay=delay)
    comparator_3 = ComparatorSource(delay=delay)

    relay_1 = RelaySource(power=source_power, max_delay=4)
    relay_2 = RelaySource(power=source_power, max_delay=4)
    relay_2.set_delay(2)

    block = Block(limit=limit)

    line_L1 = Line(limit=limit)
    line_L2 = Line(limit=limit)
    line_L3 = Line(limit=limit)
    line_L4 = Line(limit=limit)
    line_L5 = Line(limit=limit)

    line1 = [Line(limit=limit) for _ in range(9)]
    line2 = [Line(limit=limit) for _ in range(4)]
    line3 = [Line(limit=limit) for _ in range(5)]

    connect_line_chain(line1)
    connect_line_chain(line2)
    connect_line_chain(line3)

    source.connect(line1[0])
    line1[-1].connect(comparator_1.input_slot)

    source.connect(line2[0])
    line2[-1].connect(comparator_2.input_slot)

    source.connect(line3[0])
    line3[-1].connect(relay_1.input_slot)

    comparator_3.input_slot.connect(block)
    relay_1.output_slot.connect(line_L5)
    line_L5.connect(relay_2.input_slot)
    relay_2.output_slot.connect(line_L4)
    line_L4.connect(comparator_2.calculate_slot_a)

    comparator_1.output_slot.connect(block)
    comparator_2.output_slot.connect(line_L1)
    comparator_3.output_slot.connect(line_L2)

    line_L1.connect(block)
    line_L3.connect(block)

    sim.bind_objects(line1)
    sim.bind_objects(line2)
    sim.bind_objects(line3)

    sim.bind_object(line_L1, line_L2, line_L3, line_L4, line_L5)
    sim.bind_object(source, relay_1, relay_2, comparator_1, comparator_2, comparator_3, block)

    sim.add_tick_breakpoint(1)
    sim.run()

    expected_source_power = 15
    expected_line_start_power = expected_source_power
    expected_line_1_end_power = 7
    expected_line_2_end_power = 12
    expected_line_3_end_power = 11
    expected_block_power = 0

    expected_L1_power = [0, 12, 12, 12, 7, 7]
    expected_L2_power = [0, 0, 12, 12, 12, 7]
    expected_L3_power = [0, 7, 7, 7, 7, 7]
    expected_L4_power = [0, 0, 0, 15, 15, 15]
    expected_L5_power = [0, 15, 15, 15, 15, 15]

    expected_relay_1 = [15, 15, 15, 15, 15, 15]
    expected_relay_2 = [0, 15, 15, 15, 15, 15]

    expected_comparator_1 = [7, 7, 7, 7, 7, 7]
    expected_comparator_2 = [12, 12, 12, 0, 0, 0]
    expected_comparator_3 = [0, 12, 12, 12, 7, 7]

    for i in range(6):
        assert source.power == expected_source_power
        assert line1[0].power == expected_line_start_power
        assert line2[0].power == expected_line_start_power
        assert line3[0].power == expected_line_start_power
        assert block.power == expected_block_power

        assert line1[-1].power == expected_line_1_end_power
        assert line2[-1].power == expected_line_2_end_power
        assert line3[-1].power == expected_line_3_end_power

        assert line_L1.power == expected_L1_power[i]
        assert line_L2.power == expected_L2_power[i]
        assert line_L3.power == expected_L3_power[i]
        assert line_L4.power == expected_L4_power[i]
        assert line_L5.power == expected_L5_power[i]

        assert relay_1.power == expected_relay_1[i]
        assert relay_2.power == expected_relay_2[i]

        assert comparator_1.power == expected_comparator_1[i]
        assert comparator_2.power == expected_comparator_2[i]
        assert comparator_3.power == expected_comparator_3[i]

        sim.step()

    sim.resume()

    assert source.power == expected_source_power
    assert block.power == expected_block_power
    assert line_L1.power == expected_L1_power[5]
    assert line_L2.power == expected_L2_power[5]
    assert line_L3.power == expected_L3_power[5]
    assert line_L4.power == expected_L4_power[5]
    assert line_L5.power == expected_L5_power[5]
