from redstonex import PluginRegistry
import redstonex as rsx

# 这个与门写的一坨大便 我脑子不太清醒了 只是为了验证插件可用
with PluginRegistry("./gate.so") as reg:
    reg.add("ANDGate", "create_and_gate", "destroy_and_gate", "ANDGate_get_property")
    reg.add("ORGate", "create_or_gate", "destroy_or_gate", "ORGate_get_property")

sim = rsx.Simulator()
and_1 = rsx.Custom("ANDGate")

source = rsx.Source()
line_1 = rsx.Line()
line_2 = rsx.Line()

and_1.connect(line_1)
and_1.connect(line_2)

sim.bind_object(source)
sim.bind_object(line_1)
sim.bind_object(line_2)

sim.run()

print(source.power)
print(line_1.power)
print(line_2.power)
print(and_1.power)
