import redstonex as rsx
import gate_plugin as gp
from ripple_carry_adder_test import Adder4

class MUX2to1:
    """
    安全版 2选1数据选择器
    """
    def __init__(self):
        self.not_s = gp.NOTGate() 
        self.and0 = gp.ANDGate()
        self.and1 = gp.ANDGate()
        self.or_gate = gp.ORGate()

        self.I0 = gp.SLine()
        self.I1 = gp.SLine()
        self.S = gp.SLine()
        self.Out = gp.SLine()

        # 内部中继线，严格单对单连接
        self.net_not_s = gp.SLine()
        self.net_and0 = gp.SLine()
        self.net_and1 = gp.SLine()
        
        # 解决 S 既要连 not_s 又要连 and1 的问题
        # 如果引擎不允许 self.S.connect 两次，我们就用两条独立的分支线
        self.S_to_not = gp.SLine()
        self.S_to_and1 = gp.SLine()

        # 连线逻辑 (严格模仿 FullAdder 跑通的 A.connect(gate) 模式)
        self.S_to_not.connect(self.not_s)
        self.S_to_and1.connect(self.and1)
        
        self.not_s.output_slot.connect(self.net_not_s)
        self.net_not_s.connect(self.and0)

        self.I0.connect(self.and0)
        self.I1.connect(self.and1)

        self.and0.output_slot.connect(self.net_and0)
        self.and1.output_slot.connect(self.net_and1)

        self.net_and0.connect(self.or_gate)
        self.net_and1.connect(self.or_gate)
        self.or_gate.output_slot.connect(self.Out)

    def get_all_objects(self):
        return [
            self.not_s, self.and0, self.and1, self.or_gate,
            self.I0, self.I1, self.S, self.Out,
            self.net_not_s, self.net_and0, self.net_and1,
            self.S_to_not, self.S_to_and1
        ]


class SimpleALU4:
    """
    安全版 4位简易 ALU
    """
    def __init__(self):
        self.adder = Adder4()
        self.and_gates = [gp.ANDGate() for _ in range(4)]
        self.muxes = [MUX2to1() for _ in range(4)]

        self.A = [gp.SLine() for _ in range(4)]
        self.B = [gp.SLine() for _ in range(4)]
        self.S = [gp.SLine() for _ in range(4)]
        self.Op = gp.SLine() 
        self.Cout = gp.SLine()

        self.net_and_out = [gp.SLine() for _ in range(4)]
        
        # 为每个 MUX 的 S 端创建独立的驱动线，规避重复连接问题
        self.op_drivers_not = [gp.SLine() for _ in range(4)]
        self.op_drivers_and1 = [gp.SLine() for _ in range(4)]

        for i in range(4):
            # 这里的 A[i] 和 B[i] 会在 test 里面 connect 到 VCC/GND
            # 为了防止底层报错，内部全部通过专属中继线分流
            a_to_adder = gp.SLine()
            a_to_and = gp.SLine()
            b_to_adder = gp.SLine()
            b_to_and = gp.SLine()
            
            a_to_adder.connect(self.adder.A[i])
            a_to_and.connect(self.and_gates[i])
            b_to_adder.connect(self.adder.B[i])
            b_to_and.connect(self.and_gates[i])
            
            # 把外部的 A[i], B[i] 暂存，在 test 中我们会特殊处理

            self.and_gates[i].output_slot.connect(self.net_and_out[i])

            self.net_and_out[i].connect(self.muxes[i].I0)
            self.adder.S[i].connect(self.muxes[i].I1)

            # 连接操作码驱动线
            self.op_drivers_not[i].connect(self.muxes[i].S_to_not)
            self.op_drivers_and1[i].connect(self.muxes[i].S_to_and1)

            self.muxes[i].Out.connect(self.S[i])
            
            # 挂载到实例上方便外部连线
            self.__dict__[f"a_to_adder_{i}"] = a_to_adder
            self.__dict__[f"a_to_and_{i}"] = a_to_and
            self.__dict__[f"b_to_adder_{i}"] = b_to_adder
            self.__dict__[f"b_to_and_{i}"] = b_to_and

        self.adder.Cout.connect(self.Cout)

    def get_all_objects(self):
        objs = []
        objs.extend(self.adder.get_all_objects())
        for i in range(4):
            objs.append(self.and_gates[i])
            objs.extend(self.muxes[i].get_all_objects())
            objs.extend([self.A[i], self.B[i], self.S[i], self.net_and_out[i]])
            objs.extend([self.op_drivers_not[i], self.op_drivers_and1[i]])
            objs.extend([
                self.__dict__[f"a_to_adder_{i}"], self.__dict__[f"a_to_and_{i}"],
                self.__dict__[f"b_to_adder_{i}"], self.__dict__[f"b_to_and_{i}"]
            ])
        objs.append(self.Op)
        objs.append(self.Cout)
        return objs


def test_alu_case(a: int, b: int, op: int):
    sim = rsx.Simulator()
    alu = SimpleALU4()

    vcc = gp.VCC(limit=128)
    gnd = gp.GND(limit=128)

    sim.bind_object(*alu.get_all_objects(), vcc, gnd)

    # 外部输入 VCC/GND 时，直接并联连入各个分流线
    for i in range(4):
        p_a = vcc if ((a >> i) & 1) else gnd
        p_b = vcc if ((b >> i) & 1) else gnd
        
        p_a.connect(alu.__dict__[f"a_to_adder_{i}"])
        p_a.connect(alu.__dict__[f"a_to_and_{i}"])
        
        p_b.connect(alu.__dict__[f"b_to_adder_{i}"])
        p_b.connect(alu.__dict__[f"b_to_and_{i}"])

    # 操作码分流连接
    p_op = vcc if op == 1 else gnd
    for i in range(4):
        p_op.connect(alu.op_drivers_not[i])
        p_op.connect(alu.op_drivers_and1[i])

    sim.run()

    result = 0
    for i in range(4):
        if alu.S[i].power > 0:
            result |= (1 << i)

    carry = 1 if alu.Cout.power > 0 else 0
    return result, carry


if __name__ == "__main__":
    print("=========================================")
    print("    开始测试【防御性重构】4位简易 ALU      ")
    print("=========================================")
    
    test_cases = [
        (5, 3, 0),   # 0101 & 0011 = 0001 (1)
        (5, 3, 1),   # 5 + 3 = 8
        (12, 4, 0),  # 1100 & 0100 = 0100 (4)
        (12, 5, 1),  # 12 + 5 = 17 -> 1, Cout=1
        (15, 15, 0), # 1111 & 1111 = 1111 (15)
        (15, 1, 1),  # 15 + 1 = 16 -> 0, Cout=1
    ]

    print(" 模式  |  A   B  | 仿真结果 (Cout) | 预期结果")
    print("-----------------------------------------")
    for a, b, op in test_cases:
        res, c = test_alu_case(a, b, op)
        
        if op == 0:
            mode_str = "AND "
            expect_res = a & b
            out_str = f"{res:2d}"
            exp_str = f"{expect_res:2d}"
        else:
            mode_str = "ADD "
            total = a + b
            expect_res = total & 0xF
            expect_c = (total >> 4) & 1
            out_str = f"{res:2d} (Cout={c})"
            exp_str = f"{expect_res:2d} (Cout={expect_c})"

        ok = (res == expect_res) and (op == 0 or c == expect_c)
        flag = "✅ OK" if ok else "❌ ERR"

        print(f" {mode_str} | {a:2d}  {b:2d}  | {out_str:<13} | {exp_str:<13} {flag}")
