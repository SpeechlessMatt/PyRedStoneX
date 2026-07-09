import redstonex as rsx
import gate_plugin as gp

sim = rsx.Simulator()

s = rsx.Source()
l1 = rsx.Line()
l2 = rsx.Line()
l3 = rsx.Line()

and_1 = gp.ANDGate()
s.connect(l2)
l2.connect(and_1)
l1.connect(and_1)
# TODO
l3.connect(and_1.output_slot)

sim.bind_object(s, l1, l2, l3, and_1)

sim.run()
