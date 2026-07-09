import gate_plugin as gp
import redstonex as rsx

class FullAdder:
    def __init__(self):
        """
        创建一个全加器模块
        """
        # 声明对外暴露的 输入 信号线
        self.A = gp.SLine()
        self.B = gp.SLine()
        self.Cin = gp.SLine()
        
        # 声明对外暴露的 输出 信号线
        self.S = gp.SLine()
        self.Cout = gp.SLine()
        
        # 实例化内部所需的逻辑门
        self.xor1 = gp.XORGate()
        self.xor2 = gp.XORGate()
        self.and1 = gp.ANDGate()
        self.and2 = gp.ANDGate()
        self.or1  = gp.ORGate()
        
        # 内部中间连接线（线网）
        self.net_xor1_out = gp.SLine()
        self.net_and1_out = gp.SLine()
        self.net_and2_out = gp.SLine()
        
        # 开始布线 (Connecting)
        # --- 计算求和电路 (Sum) ---
        self.A.connect(self.xor1)
        self.B.connect(self.xor1)             # XOR1 输出了 A ⊕ B
        self.xor1.output_slot.connect(self.net_xor1_out)  # 将结果引出到中间线
        
        self.net_xor1_out.connect(self.xor2)
        self.Cin.connect(self.xor2)           # XOR2 算 (A ⊕ B) ⊕ Cin
        self.xor2.output_slot.connect(self.S)             # 最终求和输出 S
        
        # --- 计算进位电路 (Carry Out) ---
        self.A.connect(self.and1)
        self.B.connect(self.and1)             # AND1 算 A · B
        self.and1.output_slot.connect(self.net_and1_out)  # 结果引出
        
        self.net_xor1_out.connect(self.and2)  # AND2 的一个输入是 A ⊕ B
        self.Cin.connect(self.and2)           # AND2 的另一个输入是 Cin
        self.and2.output_slot.connect(self.net_and2_out)  # 结果引出
        
        self.net_and1_out.connect(self.or1)
        self.net_and2_out.connect(self.or1)   # OR1 将两路进位条件或起来
        self.or1.output_slot.connect(self.Cout)           # 最终进位输出 Cout

    def get_all_objects(self):
        """返回全加器内部所有的仿真对象，方便绑定到 Simulator"""
        return [
            self.A, self.B, self.Cin, self.S, self.Cout,
            self.xor1, self.xor2, self.and1, self.and2, self.or1,
            self.net_xor1_out, self.net_and1_out, self.net_and2_out
        ]

def test_full_adder_case(a_val: int, b_val: int, cin_val: int) -> tuple[int, int]:
    """
    运行单次全加器仿真并返回 (S_power, Cout_power)
    """
    # 在函数内部创建独立的模拟器和全加器对象
    sim = rsx.Simulator()
    adder = FullAdder()
    
    # 绑定所有的电路组件
    sim.bind_object(*adder.get_all_objects())
    
    # 创建 VCC 电源（用于提供高电平 15）
    vcc = gp.VCC()
    gnd = gp.GND()
    sim.bind_object(vcc, gnd)
    
    # 根据真值表输入组合，决定哪些引脚需要连接 VCC
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
            
    # 启动仿真，让信号级联传递到底
    sim.run()
    
    # 返回最终的输出电平
    return adder.S.power, adder.Cout.power

if __name__ == "__main__":
    # 定义 8 种标准的全加器输入组合 (A, B, Cin)
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
        # 计算理论上的预期结果（标准二进制加法）
        total_sum = a + b + cin
        expected_s = 1 if (total_sum % 2 == 1) else 0
        expected_cout = 1 if (total_sum >= 2) else 0
        
        # 运行你手写组件拼出的仿真器
        actual_s_power, actual_cout_power = test_full_adder_case(a, b, cin)
        
        # 将实际的 15/0 电平转换为逻辑 1/0 方便人类对账
        sim_s = 1 if actual_s_power > 0 else 0
        sim_cout = 1 if actual_cout_power > 0 else 0
        
        # 校验结果是否完全匹配
        is_ok = (sim_s == expected_s) and (sim_cout == expected_cout)
        status_str = " ✅ OK " if is_ok else " ❌ ERR"
        if is_ok:
            passed_count += 1
            
        print(f"  {a}  {b}  {cin}   │   {expected_s}    {expected_cout}     │    {actual_s_power:<2}     {actual_cout_power:<2}     │{status_str}")
        
    print("────────────┴──────────────┴──────────────────┴──────")
    print(f" 测试完成！通过率: {passed_count}/{len(truth_table_cases)}")
    print("======================================================")
