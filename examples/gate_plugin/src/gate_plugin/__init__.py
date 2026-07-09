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

    reg.add("NOTGate", "create_not_gate", "destroy_not_gate", "get_not_gate_property")
    reg.add("TRIGate", "create_tri_gate", "destroy_tri_gate", "get_tri_gate_property")
    
    reg.add("VCC", "create_vcc", "destroy_source_gate", "get_vcc_property")
    reg.add("GND", "create_gnd", "destroy_source_gate", "get_gnd_property")
    reg.add("SignalLine", "create_signal_line", "destroy_signal_line", "get_signal_line_property")

class ANDGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("ANDGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class ORGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("ORGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class NANDGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("NANDGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class XORGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("XORGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class NORGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("NORGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class XNORGate(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("XNORGate", limit=limit, power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

class NOTGate(Custom):
    def __init__(self, power: int = 15, **kwargs) -> None:
        super().__init__("NOTGate", power=power, **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

    @property
    def input_slot(self):
        return self._input_slot

class TRIGate(Custom):
    def __init__(self, **kwargs) -> None:
        super().__init__("TRIGate", **kwargs)

    @property
    def output_slot(self):
        return self._output_slot

    @property
    def input_slot(self):
        return self._input_slot

    @property
    def en_slot(self):
        return self._en_slot

class VCC(Custom):
    def __init__(self, limit: int = 3, power: int = 15, **kwargs) -> None:
        super().__init__("VCC", limit=limit, power=power, **kwargs)

class GND(Custom):
    def __init__(self, limit: int = 3, **kwargs) -> None:
        super().__init__("GND", limit=limit, **kwargs)

class SLine(Custom):
    def __init__(self, limit: int = 3, **kwargs) -> None:
        super().__init__("SignalLine", limit=limit, **kwargs)
