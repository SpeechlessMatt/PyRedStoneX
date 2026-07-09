import os
from redstonex.objects import Custom
from redstonex.registry import PluginRegistry

package_dir = os.path.dirname(os.path.abspath(__file__))
so_path = os.path.join(package_dir, "gate_core.so")

with PluginRegistry(so_path) as reg:
    reg.add("ANDGate", "create_and_gate", "destroy_logic_gate", "get_logic_gate_property")
    reg.add("ORGate", "create_or_gate", "destroy_logic_gate", "get_logic_gate_property")
    reg.add("NANDGate", "create_nand_gate", "destroy_logic_gate", "get_logic_gate_property")
    reg.add("XORGate", "create_xor_gate", "destroy_logic_gate", "get_logic_gate_property")
    reg.add("NORGate", "create_nor_gate", "destroy_logic_gate", "get_logic_gate_property")
    reg.add("XNORGate", "create_xnor_gate", "destroy_logic_gate", "get_logic_gate_property")

class ANDGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("ANDGate", **kwargs)

class ORGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("ORGate", **kwargs)

class NANDGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("NANDGate", **kwargs)

class XORGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("XORGate", **kwargs)

class NORGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("NORGate", **kwargs)

class XNORGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("XNORGate", **kwargs)
