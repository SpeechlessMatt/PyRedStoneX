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

import time
from typing import List
from redstonex.objects import Line, RelaySource, Source
from redstonex.simulator import Simulator

RELAY_COUNT = 5000
WIRE_PER_SECTION = 14 
TOTAL_WIRES = RELAY_COUNT * WIRE_PER_SECTION
TICK_COUNT = 10000

def connect_line_section(line_list: List[Line], start_idx: int, count: int):
    """辅助函数：将指定范围内的红石线首尾相连"""
    for i in range(start_idx, start_idx + count - 1):
        line_list[i].connect(line_list[i + 1])

def main():
    print("[Benchmark Chain] Initializing RedstoneX Topological Engine...")
    
    sim = Simulator()

    limit = 4
    max_power = 15
    relay_delay = 1

    source = Source(limit=limit, power=max_power)
    
    print(f"[Benchmark Chain] Creating {RELAY_COUNT} Relays and {TOTAL_WIRES} Wires...")
    
    wires = [Line(limit=limit) for _ in range(TOTAL_WIRES)]
    relays = [RelaySource(power=max_power, max_delay=4) for _ in range(RELAY_COUNT)]
    
    for relay in relays:
        relay.set_delay(relay_delay)

    print("[Benchmark Chain] Building realistic redstone network graph...")
    
    source.connect(wires[0])
    
    for i in range(RELAY_COUNT):
        start_wire_idx = i * WIRE_PER_SECTION

        connect_line_section(wires, start_wire_idx, WIRE_PER_SECTION)

        last_wire_idx = start_wire_idx + WIRE_PER_SECTION - 1
        wires[last_wire_idx].connect(relays[i].input_slot)

        if i < RELAY_COUNT - 1:
            next_section_first_wire = wires[start_wire_idx + WIRE_PER_SECTION]
            relays[i].output_slot.connect(next_section_first_wire)

    print("[Benchmark Chain] Binding objects to simulator...")
    
    sim.bind_object(source)
    sim.bind_objects(wires)
    sim.bind_objects(relays)

    sim.add_tick_breakpoint(1)
    sim.run()

    print("[Benchmark Chain] Warming up (10 ticks)...")
    for _ in range(10):
        sim.step()

    total_nodes = 1 + RELAY_COUNT + TOTAL_WIRES
    print(f"[Benchmark Chain] Running on {total_nodes} nodes for {TICK_COUNT} ticks...")
    
    start_ns = time.monotonic_ns()

    # 这里受限于python循环的时间导致性能下降，如果run的话性能是不会影响的
    for _ in range(TICK_COUNT):
        sim.step()

    end_ns = time.monotonic_ns()

    elapsed_ns = end_ns - start_ns
    elapsed_ms = elapsed_ns / 1_000_000.0
    elapsed_sec = elapsed_ns / 1_000_000_000.0 if elapsed_ns > 0 else 1e-9
    
    avg_per_tick_us = (elapsed_ns / TICK_COUNT) / 1000.0
    ticks_per_sec = TICK_COUNT / elapsed_sec
    nodes_per_sec = (total_nodes * TICK_COUNT) / elapsed_sec
    
    print("\n==================== Chain Results ====================")
    print(f"  Topology        : {total_nodes} Nodes (Realistic Linear Relay-Chain)")
    print(f"  Test Duration   : {TICK_COUNT} Ticks")
    print(f"  Total Time      : {elapsed_ms:.3f} ms")
    print(f"  Avg per Tick    : {avg_per_tick_us:.3f} us")
    print(f"  Throughput      : {ticks_per_sec:.2f} Ticks/sec")
    print(f"  Node Processing : {nodes_per_sec:.2f} Nodes/sec")
    print("=======================================================")

    # 验证末端中继器的能量状态
    last_relay_power = relays[-1].power
    print(f"[Verification] Last Relay Power = {last_relay_power}")

if __name__ == "__main__":
    main()
