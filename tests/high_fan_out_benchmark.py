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
import gc
from redstonex.objects import Line, Source
from redstonex.simulator import Simulator

WIRE_COUNT = 10000
TICK_COUNT = 2000
MAX_POWER = 15
LIMIT = 10005

def main():
    print("[Benchmark Fan-out] Initializing RedstoneX Topological Engine...")
    
    sim = Simulator()

    source = Source(limit=LIMIT, power=MAX_POWER)
    
    print(f"[Benchmark Fan-out] Creating 1 Source and {WIRE_COUNT} Wires...")
    
    wires = [Line(limit=4) for _ in range(WIRE_COUNT)]

    print("[Benchmark Fan-out] Binding objects to simulator...")
    
    sim.bind_object(source)
    sim.bind_objects(wires)

    print("[Benchmark Fan-out] Building high fan-out network graph...")
    for wire in wires:
        source.connect(wire)

    sim.add_tick_breakpoint(1)
    sim.run()

    print("[Benchmark Fan-out] Warming up (10 ticks)...")
    for _ in range(10):
        sim.step()

    total_nodes = 1 + WIRE_COUNT
    print(f"[Benchmark Fan-out] Running on {total_nodes} nodes for {TICK_COUNT} ticks...")
    
    gc.collect()

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

    print("\n=================== Fan-out Results ===================")
    print(f"  Topology        : 1 Source -> {WIRE_COUNT} Wires (Star Network)")
    print(f"  Test Duration   : {TICK_COUNT} Ticks")
    print(f"  Total Time      : {elapsed_ms:.3f} ms")
    print(f"  Avg per Tick    : {avg_per_tick_us:.3f} us")
    print(f"  Throughput      : {ticks_per_sec:.2f} Ticks/sec")
    print(f"  Node Processing : {nodes_per_sec:.2f} Nodes/sec")
    print("=======================================================")

    last_wire_power = wires[-1].power
    print(f"[Verification] Wire[{WIRE_COUNT - 1}] Power = {last_wire_power}")

if __name__ == "__main__":
    main()
