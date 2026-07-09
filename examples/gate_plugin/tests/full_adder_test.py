import gate_plugin as gp
import redstonex as rsx

class FullAdder:
    def __init__(self):
        self.xor1 = gp.XORGate()
        self.and1 = gp.ANDGate()
        self.xor2 = gp.XORGate()
        self.and2 = gp.ANDGate()
        self.or1  = gp.ORGate()
        
        self.A = gp.SLine()
        self.B = gp.SLine()
        self.Cin = gp.SLine()
        self.S = gp.SLine()
        self.Cout = gp.SLine()
        
        self.net_xor1 = gp.SLine()
        self.net_and1 = gp.SLine()
        self.net_and2 = gp.SLine()
        
        self.A.connect(self.xor1)
        self.A.connect(self.and1)
        self.B.connect(self.xor1)
        self.B.connect(self.and1)
        
        self.xor1.output_slot.connect(self.net_xor1)
        self.net_xor1.connect(self.xor2)
        self.net_xor1.connect(self.and2)
        
        self.Cin.connect(self.xor2)
        self.Cin.connect(self.and2)
        
        self.and1.output_slot.connect(self.net_and1)
        self.and2.output_slot.connect(self.net_and2)
        self.net_and1.connect(self.or1)
        self.net_and2.connect(self.or1)
        
        self.xor2.output_slot.connect(self.S)
        self.or1.output_slot.connect(self.Cout)

    def get_all_objects(self):
        return [
            self.xor1, self.and1, self.xor2, self.and2, self.or1,
            self.A, self.B, self.Cin, self.S, self.Cout,
            self.net_xor1, self.net_and1, self.net_and2
        ]

def test_full_adder_case(a_val: int, b_val: int, cin_val: int) -> tuple[int, int]:
    """
    运行单次全加器仿真并返回 (S_power, Cout_power)
    """
    sim = rsx.Simulator()
    adder = FullAdder()
    
    sim.bind_object(*adder.get_all_objects())
    
    vcc = gp.VCC()
    gnd = gp.GND()
    sim.bind_object(vcc, gnd)
    
    if a_val == 1:
        adder.A.connect(vcc)
    else:
        adder.A.connect(gnd)
        
    if b_val == 1:
        adder.B.connect(vcc)
    else:
        adder.B.connect(gnd)
        
    if cin_val == 1:
        adder.Cin.connect(vcc)
    else:
        adder.Cin.connect(gnd)
            
    sim.run()
    
    return adder.S.power, adder.Cout.power

if __name__ == "__main__":
    truth_table_cases = [
        (0, 0, 0),
        (0, 0, 1),
        (0, 1, 0),
        (0, 1, 1),
        (1, 0, 0),
        (1, 0, 1),
        (1, 1, 0),
        (1, 1, 1)
    ]
    
    print("======================================================")
    print("   开始全面跑测【全加器】完整的真值表 (Truth Table) ")
    print("======================================================")
    print("  输入组合  │   预期输出   │   实际仿真结果   │ 状态")
    print("  A  B Cin  │   S   Cout   │   S.p   Cout.p   │")
    print("────────────┼──────────────┼──────────────────┼──────")
    
    passed_count = 0
    
    for a, b, cin in truth_table_cases:
        total_sum = a + b + cin
        expected_s = 1 if (total_sum % 2 == 1) else 0
        expected_cout = 1 if (total_sum >= 2) else 0
        
        actual_s_power, actual_cout_power = test_full_adder_case(a, b, cin)
        
        sim_s = 1 if actual_s_power > 0 else 0
        sim_cout = 1 if actual_cout_power > 0 else 0
        
        is_ok = (sim_s == expected_s) and (sim_cout == expected_cout)
        status_str = " ✅ OK " if is_ok else " ❌ ERR"
        if is_ok:
            passed_count += 1
            
        print(f"  {a}  {b}  {cin}   │   {expected_s}    {expected_cout}     │    {actual_s_power:<2}     {actual_cout_power:<2}     │{status_str}")
        
    print("────────────┴──────────────┴──────────────────┴──────")
    print(f" 测试完成！通过率: {passed_count}/{len(truth_table_cases)}")
    print("======================================================")
