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
from redstonex.objects import Block, Line, TorchSource
from redstonex.simulator import Simulator

CLOCK_COUNT = 2500 
TICK_COUNT = 1000
MAX_POWER = 15
LIMIT = 4

def main():
    print("[Benchmark Oscillators] Initializing RedstoneX Topological Engine...")
    
    sim = Simulator()

    print(f"[Benchmark Oscillators] Creating {CLOCK_COUNT} independent clock loops...")
    
    setup_start_ns = time.monotonic_ns()

    torch_1_arr = []
    torch_2_arr = []
    line_1_arr = []
    line_2_arr = []
    block_1_arr = []
    block_2_arr = []

    for _ in range(CLOCK_COUNT):
        t1 = TorchSource(power=MAX_POWER, delay=1)
        t2 = TorchSource(power=MAX_POWER, delay=1)
        l1 = Line(limit=LIMIT)
        l2 = Line(limit=LIMIT)
        b1 = Block(limit=LIMIT)
        b2 = Block(limit=LIMIT)

        torch_1_arr.append(t1)
        torch_2_arr.append(t2)
        line_1_arr.append(l1)
        line_2_arr.append(l2)
        block_1_arr.append(b1)
        block_2_arr.append(b2)

        sim.bind_object(t1, t2, l1, l2, b1, b2)

        t1.power_slot.connect(l1)
        l1.connect(b2)
        b2.connect(t2.bottom_slot)

        t2.power_slot.connect(l2)
        l2.connect(b1)
        b1.connect(t1.bottom_slot)

    setup_end_ns = time.monotonic_ns()
    setup_ms = (setup_end_ns - setup_start_ns) / 1_000_000.0
    print(f"[Benchmark Oscillators] Setup took {setup_ms:.2f} ms")

    sim.add_tick_breakpoint(1)
    sim.run()

    total_nodes = CLOCK_COUNT * 6
    print(f"[Benchmark Oscillators] Running on {total_nodes} nodes (100% active) for {TICK_COUNT} ticks...")

    # 在执行核心仿真大循环前，强制清除 Python 构建期产生的内存碎片，让测试更纯粹
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
    flips_per_sec = (total_nodes * TICK_COUNT) / elapsed_sec

    print("\n=================== Oscillator Results ===================")
    print(f"  Topology        : {CLOCK_COUNT} Active Clock Loops ({total_nodes} Nodes)")
    print(f"  Test Duration   : {TICK_COUNT} Ticks")
    print(f"  Total Time      : {elapsed_ms:.3f} ms")
    print(f"  Avg per Tick    : {avg_per_tick_us:.3f} us")
    print(f"  Throughput      : {ticks_per_sec:.2f} Ticks/sec")
    print(f"  Node Flips/sec  : {flips_per_sec:.2f} Flips/sec")
    print("==========================================================")

if __name__ == "__main__":
    main()
