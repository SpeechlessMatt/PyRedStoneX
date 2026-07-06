import redstonex as rsx

if __name__ == "__main__":
    sim = rsx.Simulator()
    s = rsx.Source()
    l1 = rsx.Line()
    l2 = rsx.Line()
    s.connect(l1)
    l1.connect(l2)
    
    sim.bind_object(s)
    sim.bind_object(l1)
    sim.bind_object(l2)

    sim.run()

    print(s.power)
    print(l1.power)
    print(l2.power)
