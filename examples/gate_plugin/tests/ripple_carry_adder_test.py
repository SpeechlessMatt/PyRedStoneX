import redstonex as rsx
import gate_plugin as gp
from full_adder_test import FullAdder

class Adder4:
    def __init__(self):
        self.fa0 = FullAdder()
        self.fa1 = FullAdder()
        self.fa2 = FullAdder()
        self.fa3 = FullAdder()

        # Carry
        self.c1 = gp.SLine()
        self.c2 = gp.SLine()
        self.c3 = gp.SLine()

        # Ripple Carry
        self.fa0.Cout.connect(self.c1)
        self.c1.connect(self.fa1.Cin)

        self.fa1.Cout.connect(self.c2)
        self.c2.connect(self.fa2.Cin)

        self.fa2.Cout.connect(self.c3)
        self.c3.connect(self.fa3.Cin)

        # 对外接口（直接引用）
        self.A = [
            self.fa0.A,
            self.fa1.A,
            self.fa2.A,
            self.fa3.A,
        ]

        self.B = [
            self.fa0.B,
            self.fa1.B,
            self.fa2.B,
            self.fa3.B,
        ]

        self.S = [
            self.fa0.S,
            self.fa1.S,
            self.fa2.S,
            self.fa3.S,
        ]

        self.Cin = self.fa0.Cin
        self.Cout = self.fa3.Cout

    def get_all_objects(self):
        objs = []

        for fa in (self.fa0, self.fa1, self.fa2, self.fa3):
            objs.extend(fa.get_all_objects())

        objs.extend([
            self.c1,
            self.c2,
            self.c3,
        ])

        return objs

def test_adder4_case(a, b):
    sim = rsx.Simulator()

    adder = Adder4()

    # 电源，注意调大 limit
    vcc = gp.VCC(limit=16)
    gnd = gp.GND(limit=16)

    sim.bind_object(*adder.get_all_objects(), vcc, gnd)

    # A
    for i in range(4):
        if (a >> i) & 1:
            adder.A[i].connect(vcc)
        else:
            adder.A[i].connect(gnd)

    # B
    for i in range(4):
        if (b >> i) & 1:
            adder.B[i].connect(vcc)
        else:
            adder.B[i].connect(gnd)

    # Cin = 0
    adder.Cin.connect(gnd)

    sim.run()

    result = 0
    for i in range(4):
        if adder.S[i].power > 0:
            result |= (1 << i)

    carry = 1 if adder.Cout.power > 0 else 0

    return result, carry

if __name__ == "__main__":
    print(" A   B | Sum Cout | Expect")
    print("--------------------------")

    passed = 0

    for a in range(16):
        for b in range(16):
            s, c = test_adder4_case(a, b)

            total = a + b
            expect_s = total & 0xF
            expect_c = (total >> 4) & 1

            ok = (s == expect_s and c == expect_c)

            if ok:
                passed += 1
                flag = "OK"
            else:
                flag = "ERR"

            print(
                f"{a:2d} {b:2d} | "
                f"{s:2d}   {c} | "
                f"{expect_s:2d}   {expect_c} | {flag}"
            )

    print("--------------------------")
    print(f"Passed: {passed}/256")
