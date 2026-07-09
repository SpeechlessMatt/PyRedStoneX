# PyRedStoneX

PyRedStoneX 是一个面向 Python 的红石电路模拟库。它把 RedStoneX 的底层模拟能力封装成一个直观、可扩展的 Python API，让你可以轻松搭建红石电路、测试逻辑门、编写自定义组件，并在 Python 中进行可视化和自动化实验。

它的目标很简单：让“红石电路模拟”不再只是底层 C/C++ 的世界，而是成为一个可被 Python 轻松驱动的工程能力。

## 主要特性

- 支持基础红石元件建模：源信号、导线、方块、继电器、比较器、火把等
- 提供基于 tick 的模拟执行模型，可逐步推进或连续运行
- 支持插件系统，允许通过 C/C++ 动态库扩展新的元件类型
- 适合做电路逻辑实验、自动化测试、教学演示和自定义模拟器扩展
- 兼容 Python 3.10+，并且使用现代构建方式打包

## 安装

### 从源码安装

```bash
git clone https://github.com/SpeechlessMatt/PyRedStoneX.git
cd PyRedStoneX
python -m pip install -e .
```

如果你的环境还没有编译工具链，确保已经安装了 C/C++ 编译器以及 Python 的开发头文件。

## 快速开始

下面是一个最小示例：创建一个信号源与导线，然后运行模拟器。

```python
from redstonex import Simulator, Source, Line

sim = Simulator()
source = Source(power=15)
line = Line(limit=4)

source.connect(line)
sim.bind_object(source, line)

sim.add_tick_breakpoint(1)
sim.run()

print("source power:", source.power)
print("line power:", line.power)
```

## 内置组件

PyRedStoneX 提供了几个常用的基础对象：

- Source：信号源
- Line：导线
- Block：方块
- RelaySource：中继器
- ComparatorSource：比较器
- TorchSource：火把
- Slot：插槽，用于连接和传输信号
- Custom：用于承载插件组件

## 进阶用法：模拟器控制

你可以按 tick 逐步推进，也可以暂停、恢复、设置断点：

```python
from redstonex import Simulator, LogLevel

sim = Simulator()
sim.enable_logging(LogLevel.INFO)

sim.step()
sim.pause()
sim.run()
```

## 插件系统

PyRedStoneX 的一大亮点是它支持插件扩展。你可以把新的红石元件实现为共享库，然后由 Python 侧注册并使用。

### 插件注册示例

```python
from redstonex.registry import PluginRegistry

with PluginRegistry("/path/to/gate_core.so") as reg:
    reg.add("ANDGate", "create_and_gate", "destroy_logic_gate")
```

### 插件开发要求

- 插件库需要暴露创建和销毁函数
- 对于 C++ 插件，建议使用 `extern "C"` 导出符号
- 可选地提供属性查询函数，用于让 Python 侧读取插件组件属性

这个仓库自带了一个完整的示例插件工程，位于 [examples/gate_plugin](examples/gate_plugin)。

## 示例与测试

仓库中包含一组示例与测试脚本，帮助你快速理解如何搭建复杂电路和插件：

- [examples/gate_plugin](examples/gate_plugin)
- [tests/test.py](tests/test.py)

## 项目结构

```text
.
├── src/redstonex        # Python 包与绑定层
├── examples            # 示例插件与使用案例
├── tests               # 测试脚本
└── extern/RedStoneX    # 底层 RedStoneX 仿真引擎
```

## 许可证

本项目基于 GPL-3.0-or-later 许可证开源。

## 贡献

欢迎提交 issue、PR 或分享你基于 PyRedStoneX 做出的电路与插件实现。如果你有想扩展的元件类型，也非常欢迎一起完善这个生态。

