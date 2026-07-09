# PyRedStoneX

[![License](https://img.shields.io/badge/License-GPL--3.0--or--later-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![Python Version](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![C Standard](https://img.shields.io/badge/C-11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))

**PyRedStoneX** 是一个面向 Python 生态的高性能红石电路逻辑模拟库。它将纯 C 编写的高效率底层红石模拟引擎 **RedStoneX** 通过 C++ (pybind11) 胶水层进行极致封装，不仅提供了面向对象且极其直观的 Python 接口，还开创性地设计了**一套基于二进制兼容性（ABI）的插件开发模式**。

无论是进行红石电路的大规模逻辑实验、自动化电路验证、教学演示、还是开发自定义组件，PyRedStoneX 都能在保障极速仿真的同时，提供极佳的扩展性与开发友好度。

---

## 🗺️ 系统架构

PyRedStoneX 的架构旨在隔离底层红石仿真的高频开销与上层 Python 的业务逻辑，并提供了稳定的 ABI 插件扩展接口：

```text
 ┌────────────────────────────────────────────────────────┐
 │                      Python 业务层                      │
 │     - 编写电路拓扑、连接导线/器件、脚本自动化/逻辑测试      │
 │     - 使用 Simulator, Source, Line, custom.ANDGate 等   │
 └──────────────────────────┬─────────────────────────────┘
                            │ (通过 pybind11 高效绑定)
 ┌──────────────────────────▼─────────────────────────────┐
 │                      C++ 胶水桥接层                     │
 │     - 统一管理 C++ 对象与底层 C 结构体的生命周期 (_core)    │
 │     - 线程安全的 C 语言日志回调映射至 Python 打印 (GIL控制)  │
 │     - 存储动态插件注册表 (ComponentOps Map)                  │
 └──────────────────────────┬─────────────────────────────┘
                            │ (直接指针传递 / C API 调用)
 ┌──────────────────────────▼─────────────────────────────┐
 │                RedStoneX 底层 C 仿真引擎                 │
 │     - 基于时间轮(Tick Wheel)与双端队列(Deque)的事件驱动系统│
 │     - 极致紧凑的内存布局 (RSXConnectiveObject 结构体)     │
 │     - 毫秒级支持数十万红石元件高频振荡仿真                    │
 └──────────────────────────▲─────────────────────────────┘
                            │ (稳定 ABI 接口注册 / ctypes 动态加载)
 ┌──────────────────────────┴─────────────────────────────┐
 │                  C/C++ 动态链接插件库                   │
 │     - 符合 ComponentOps 接口规范 (.so / .dll)          │
 │     - 插件创建(create)、销毁(destroy)、属性查询(get_prop) │
 │     - 例如: AND/OR/XOR/NOT 逻辑门, VCC/GND, 信号线插件     │
 └────────────────────────────────────────────────────────┘
```

---

## 🛠️ 环境依赖与安装

### 1. 系统依赖要求
在从源码编译安装 PyRedStoneX 之前，请确保您的系统中已经安装了以下工具链：
* **C++ 编译器**：支持 C++17 或更高标准。
  * **Linux**: GCC 8+ 或 Clang 7+
  * **macOS**: Xcode Command Line Tools
  * **Windows**: MSVC 2019 或更高版本（安装 C++ 桌面开发工作负载）
* **C 编译器**：支持 C11 标准（用于编译底层 RedStoneX 引擎和 C 插件）。
* **CMake**：版本在 **3.15** 或以上。
* **Python**：版本在 **3.10** 或以上，并安装有 Python 头文件（如 Ubuntu 上的 `python3-dev`）。

### 2. 从源码安装

PyRedStoneX 使用现代的 `scikit-build-core` 和 `pybind11` 进行构建管理。你可以直接通过 `pip` 自定义编译并安装：

```bash
# 克隆主项目及其子模块 (RedStoneX 引擎)
git clone --recursive https://github.com/SpeechlessMatt/PyRedStoneX.git
cd PyRedStoneX

# 以可编辑(Editable)开发模式安装（推荐）
python -m pip install -e .

# 或者是标准安装模式
python -m pip install .
```

> **提示**: `--recursive` 参数非常重要，用于拉取 `extern/RedStoneX` 底层 C 语言模拟器依赖。如果已经克隆了未包含子模块的项目，请执行 `git submodule update --init --recursive`。

---

## 🚀 快速开始

下面是一个最简示例：创建一个红石信号源、一条导线、一个红石块，然后绑定并运行模拟。

```python
import redstonex as rsx

# 1. 创建仿真器实例
sim = rsx.Simulator()

# 2. 实例化红石组件
source = rsx.Source(limit=4, power=15)  # 信号源：可连接4个邻居，初始强度15
line = rsx.Line(limit=4)               # 红石线：无损/有损充能
block = rsx.Block(limit=6)             # 实体方块：弱/强充能接收与转发

# 3. 连接组件（建立电路拓扑）
source.connect(line)
line.connect(block)

# 4. 将对象绑定到仿真器中
sim.bind_object(source, line, block)

# 5. 设置仿真断点：在第 1 个 Tick 暂停
sim.add_tick_breakpoint(1)

# 6. 开启控制台错误/信息日志（可选）
sim.enable_logging(rsx.simulator.LogLevel.INFO)

# 7. 开始运行仿真
sim.run()

# 8. 查看第 1 个 Tick 后的信号状态
print(f"=== Tick {sim.current_tick} 仿真状态 ===")
print(f"信号源强度: {source.power}")  # 应该为 15
print(f"红石线强度: {line.power}")    # 应该为 15
print(f"实体块强度: {block.power}")   # 应该为 15（无损线下）
```

---

## 📖 核心 API 及内置组件说明

PyRedStoneX 贴心地封装了红石模拟器中不可或缺的底层逻辑。以下是各个常用内置组件及接口的说明。

### 1. 基础物理组件

所有仿真对象都继承自底层的 `CoreConnectiveObject`，它们具有公共属性如 `id`, `uri`, `power`, `limit`, `connect_count` 等。

| 组件类名 | 说明 | 常见构造参数及默认值 | 典型连接模式 |
| :--- | :--- | :--- | :--- |
| `Line` | **红石线**：用于传输能量。无损传输，传递的信号强度与距离无关。 | `limit=4` (最多邻居连接数) | 连接 Source / Block / Relay 等 |
| `Source` | **信号源**：无条件发出红石能量的起始源。 | `limit=4`, `power=15` | 连接 Line / Block |
| `Block` | **实体方块**：接收并传递强/弱充能，充当强阻断或大容量转发媒介。 | `limit=6` | 连接 Line / Torch |
| `Slot` | **插槽/槽位**：特殊的虚拟中转槽，一般用来实现复杂器件的方向性接口。 | `parent`, `power_type=PowerType.NONE` | 内部封装使用 |
| `RelaySource` | **红石中继器**：单向能量中转。具有 1-Tick 到多个 Tick 的信号延时。 | `power=15`, `max_delay=4` | 通过 `input_slot` 和 `output_slot` 连接 |
| `ComparatorSource`| **红石比较器**：用于比较（Comparison）或做减法（Subtraction）。 | `delay=1` | 通过输入、左右侧和输出 Slot 连接 |
| `TorchSource` | **红石火把**：提供源源不断的红石信号，若底部被充能，则熄灭。 | `power=15`, `delay=1` | 通过 `bottom_slot` 充能，由 `power_slot` 输出 |
| `Custom` | **自定义器件**：用于加载和承载来自外部插件生成的 ABI 元件。 | `plugin_name, **kwargs` | 视插件具体实现而定 |

### 2. 仿真器控制 (Simulator)

`Simulator` 对象是控制仿真时序、时间轮推进的核心。
* `run()`: 持续运行仿真，直到系统内没有任何待处理的事件或遇到时间断点。
* `step() -> bool`: 仿真时间往前推进 1 个 tick。如果发生了事件活动或未来有待处理日程则返回 `True`，若整个电路完全静止返回 `False`。
* `pause()`: 暂停当前正在运行的仿真。
* `resume()`: 恢复已暂停的仿真。
* `add_tick_breakpoint(tick: int)`: 在指定的 tick 刻度增加一个硬件级断点（仿真运行到该 tick 时会立刻自动挂起并返回）。
* `remove_tick_breakpoint(tick: int)`: 移除指定 tick 上的仿真断点。
* `bind_object(*objects)`: 向仿真器绑定一个或多个红石元件，使其加入到当前仿真时空中。
* `bind_objects(objects_list)`: 绑定一组可迭代的元件。
* `enable_logging(level)`: 开启日志桥接。支持调试等级：`DEBUG`, `INFO`, `WARN`, `ERROR`。

### 3. 连接性限制 (Connection Restrictions)
由于红石拓扑本身的严格性，底层对连接做了防重、防爆上限控制：
* 任何器件不能与其自身连接。
* 两个器件之间只能存在单条逻辑连接，不能重复建立连接。
* 每个组件的连接数不能超出初始化时指定的 `limit`。如果强行连接会抛出 `ConnectionLimitError` 异常，不兼容类型的强行连接则会引发 `ConnectiveError`。

---

## 🔌 插件扩展系统 (ABI Plugin Mode)

PyRedStoneX 设计了一套极致轻量、彻底解耦、无需二次编译主框架的**共享库插件机制**。

您可以通过纯 C 语言编写极速逻辑门或特定功能电路器件，编译为 `.so` 或 `.dll` 后在 Python 侧直接动态注册：

```python
import os
import redstonex as rsx
from redstonex.registry import PluginRegistry

# 1. 加载动态库插件
with PluginRegistry("./plugins/libgate_core.so") as reg:
    # 2. 注册 C 回调函数到指定组件名
    reg.add(
        component_name="ANDGate",
        create="create_and_gate",
        destroy="destroy_logic_gate",
        get_property="get_logic_gate_property"
    )

# 3. 实例化自定义组件并使用
and_gate = rsx.Custom("ANDGate", limit=3, power=15)
sim = rsx.Simulator()
sim.bind_object(and_gate)
```

> 💡 欲了解保姆级的插件开发流程、C 语言 API 结构、内存对齐与强制转换规范、完整 C/C++ 示例以及 Python 的面向对象二次优雅包装，请阅读专门编写的指南：**[docs/plugin_development.md](docs/plugin_development.md)**。

---

## 📂 项目目录结构说明

```text
PyRedStoneX/
├── pyproject.toml              # Python 现代打包配置 (scikit-build-core 构建系统后端)
├── MANIFEST.in                 # 打包源码分发时的文件清单规则
├── CMakeLists.txt              # 顶层 C/C++ 编译配置文件（链接 pybind11 并编译 _core 库）
├── README.md                   # 本说明文件
├── LICENSE                     # 项目许可证 (GPL-3.0-or-later)
├── CMake/                      # CMake 配置辅助模板
│   └── PyRedStoneXConfig.cmake.in
├── docs/                       # 官方详尽技术文档
│   └── plugin_development.md   # ABI 插件开发保姆级教程 (包含 API 示例及原理规范)
├── src/                        # Python 包源码主目录
│   └── redstonex/
│       ├── __init__.py         # 暴露 Simulator, Built-in Objects 和 PowerType 
│       ├── simulator.py        # Python 封装的仿真器控制器
│       ├── objects.py          # Python 对各原生红石器件类的优雅封装与自动 ID 计数器
│       ├── registry.py         # 插件注册中心：使用 ctypes 动态装载并解析共享库符号指针
│       ├── types.py            # 红石相关的枚举定义（如充能类型 WEAK / STRONG）
│       ├── utils.py            # 实用工具函数
│       ├── _core.pyi           # 对 pybind11 导出的编译模块提供 PEP 561 类型注解（IDE友好）
│       └── csrc/
│           └── core.cpp        # 核心 C++ pybind11 胶水代码（对接 C 语言引擎与 Python 类）
│           └── include/
│               └── redstonex/
│                   └── plugin.h # 专供插件开发者引入的 C 语言统一 ABI 核心头文件
├── extern/                     # 外部依赖子模块目录
│   └── RedStoneX/              # 纯 C 语言编写的高性能底层红石仿真引擎
│       ├── CMakeLists.txt      # 独立编译红石 C 引擎的配置文件
│       ├── include/            # 仿真器底层 API
│       │   ├── redstonex_components.h  # 底层中继器、比较器、火把等算法实现接口
│       │   ├── redstonex_obj.h         # 红石元器件基类结构体和基础连接、广播行为
│       │   ├── redstonex_sim.h         # 仿真器时钟轮、事件队列和主执行循环定义
│       │   └── redstonex_types.h       # 底层公共基本枚举
│       └── src/                # 仿真引擎纯 C 源码实现 (redstonex_sim.c 等)
├── examples/                   # 经典开发示例
│   └── gate_plugin/            # 完整的 ABI 逻辑门器件插件工程
│       ├── CMakeLists.txt      # 独立编译逻辑门插件共享库的 CMake 脚本
│       ├── pyproject.toml      # 插件本地打包设置
│       ├── src/gate_plugin/
│       │   ├── __init__.py     # 插件注册和 AND/OR/XOR... 优雅 Python OOP 类定义
│       │   ├── csrc/
│       │   │   └── gate_plugin.c  # 逻辑门 C 语言具体逻辑与 ComponentOps 回调实现
│       │   └── include/
│       │       └── gate_plugin.h  # 逻辑门结构体与函数声明
│       └── tests/              # 插件逻辑门应用测试（全加器、波纹进位加法器、ALU等）
│           ├── alu_test.py
│           ├── full_adder_test.py
│           └── ripple_carry_adder_test.py
└── tests/                      # 核心功能测试脚本目录
    ├── test.py                 # 原生红石大回路逻辑仿真测试
    ├── high_fan_out_benchmark.py # 高扇出极端仿真压力跑测
    ├── long_chain_benchmark.py   # 长红石导线链延时与能量衰减测试
    └── high_churn_torch_oscillators_benchmark.py # 高频火把振荡时钟源极端基准测试
```

---

## 📜 开源许可证

本项目采用 **GPL-3.0-or-later** 许可证开源。底层 RedStoneX 仿真引擎也包含在相应的开源许可中。

在基于本项目进行二次修改、商业化再发布或嵌入到其他软件中时，请务必遵守 GPL-3.0 协议中关于源码公开的相关约束。

---

## 🤝 贡献与讨论

1. **提交 Issue**：如果您在安装、开发插件或构建复杂电路时发现任何 bug 或未定义的异常行为，请随时在 GitHub 提交 issue。
2. **贡献代码 (Pull Request)**：非常欢迎提交 PR！请遵循以下流程：
   *  fork 本仓库。
   * 新建特性分支（例如 `feature/logic-custom-delay`）。
   * **请确保添加配套的单元测试**（可以参考 `tests/test.py`）。
   * 提交 PR 并简要描述改动的物理机制和性能表现。

再次感谢对 PyRedStoneX 开源社区的支持！让我们用 Python 点亮属于我们的红石科技树！ 💡🔴
