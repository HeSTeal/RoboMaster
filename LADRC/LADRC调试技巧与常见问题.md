# LADRC 调试技巧与常见问题（5.1–5.3）

> 来源：<https://jackrainman.github.io/ControlTheory/07-LADRC%E4%BB%A3%E7%A0%81%E5%AE%9E%E6%88%98/>

## 5.1 调试步骤

LADRC参数整定的一般流程：

```
步骤1: 确定系统阶数
    └─→ 观察被控对象：输入到输出需要几次积分
    └─→ 一阶系统（电流、简单速度）→ 使用first_order_ladrc
    └─→ 二阶系统（位置、角度）→ 使用ladrc

步骤2: 估计b
    ├─→ 施加阶跃输入，记录输出响应
    ├─→ 测量初始加速度：a0 = (y[2] - 2*y[1] + y[0]) / h^2
    └─→ 计算：b = a0 / U（U为阶跃幅值）

步骤3: 设置带宽参数
    ├─→ wc = 期望控制器带宽（如：希望0.1s响应，wc≈10*2π/0.1≈60）
    ├─→ wo = 3~5 * wc（初始比例，观测器带宽）
    └─→ 计算LADRC增益：
        - 一阶：β1=2*wo, β2=wo^2, kp=wc
        - 二阶：β1=3*wo, β2=3*wo^2, β3=wo^3, kp=wc^2, kd=2*wc

步骤4: 设置TD参数（可选）
    ├─→ td_r = 100（TD快速跟踪因子，初始值）
    └─→ td_n = 5（滤波因子，无量纲）

步骤5: 逐步调试wc
    ├─→ 先设较小的wc（如目标值的50%）
    ├─→ 测试响应，如过慢则增大20%
    └─→ 直到响应速度满足要求

步骤6: 调试wo
    ├─→ 固定wc，调整wo/wc比例
    ├─→ 如果估计滞后明显：增大wo
    └─→ 如果噪声敏感：减小wo

步骤7: 微调b
    ├─→ 如果系统振荡：减小b（如减10%）
    └─→ 如果响应迟缓：增大b（如增10%）

步骤8: 优化TD参数
    ├─→ 如果目标跳变时超调大：减小td_r或增大td_n
    └─→ 如果过渡过程过慢：增大td_r
```

## 5.2 常见问题排查

| 现象 | 可能原因 | 解决方案 |
| --- | --- | --- |
| **系统振荡** | wc过大、b过大、wo过大 | 逐步减小参数，每次减10-20% |
| **响应迟缓** | wc过小、b过小 | 增大wc或b |
| **估计滞后** | wo过小 | 增大wo，但不要超过5wc |
| **噪声敏感** | wo过大、td_n过小 | 减小wo，或增大td_n |
| **超调严重** | wc过大、td_r过大 | 减小wc或td_r |
| **稳态误差** | b估计不准 | 微调b，或检查观测器是否正常收敛 |
| **启动冲击** | TD参数不当 | 减小td_r或增大td_n |
| **饱和后超调** | 未启用抗积分饱和 | 增大k_aw（建议1.0-3.0） |
| **目标值抖动** | td_n过小或TD禁用 | 增大td_n（建议3~5），或启用TD |

> 注：现象与可能原因均摘自网络。

## 5.3 调试技巧

### 1. 实时监控关键变量

在调试阶段，建议实时输出以下变量：

- 目标值 `target`
- 实际输出 `measure`
- 估计的扰动 `x3`（二阶）或 `x2`（一阶）
- 控制量 `out`
- 虚拟控制量 `u0`
- TD状态（如果启用）

```
// 调试输出示例（通过串口或日志）
void LADRC_DebugOutput(ladrc_t *ladrc, float target, float measure)
{
    printf("目标=%.2f, 实际=%.2f, 估计位置=%.2f, 估计速度=%.2f, 扰动=%.2f, 输出=%.2f\n",
           target, measure, ladrc->x1, ladrc->x2, ladrc->x3, ladrc->out);
    if (ladrc->use_td) {
        printf("TD: 平滑目标=%.2f, 目标速度=%.2f\n", ladrc->td.x1, ladrc->td.x2);
    }
}
```

### 2. 参数调整口诀

```
响应慢 → 增wc
有振荡 → 减wc或减b
估计慢 → 增wo
噪声大 → 减wo或增td_n
有超调 → 减td_r（TD速度）
目标抖 → 增td_n（TD滤波）
```

### 3. 安全限幅

```
// ladrc_calc内部已集成输出限幅，只需设置max_output参数
// 如需动态调整限幅值，可直接修改结构体

void LADRC_SetOutputLimit(ladrc_t *ladrc, float max_output)
{
    ladrc->max_output = max_output;
}

// 抗积分饱和通过k_aw参数启用
void LADRC_SetAntiWindup(ladrc_t *ladrc, float k_aw)
{
    ladrc->k_aw = k_aw;  // 建议值1.0-3.0，0表示禁用
}
```

### 4. 在线参数调整

在调试阶段，可以设计通信接口实现参数在线调整：

```
// 通过串口接收命令调整参数
void LADRC_AdjustParameter(ladrc_t *ladrc, char param, float value)
{
    switch(param) {
        case 'w':  // wo - 重新计算beta增益
            ladrc->beta1 = 3.0f * value;
            ladrc->beta2 = 3.0f * value * value;
            ladrc->beta3 = value * value * value;
            break;
        case 'c':  // wc - 重新计算PD增益
            ladrc->kp = value * value;
            ladrc->kd = 2.0f * value;
            break;
        case 'b':  // b - 控制增益
            ladrc->b = value;
            break;
        case 'r':  // td_r - TD快速跟踪因子
            if (value > 0.0f && !ladrc->use_td) {
                ladrc->use_td = true;
            }
            ladrc->td.r = value;
            break;
        case 'n':  // td_n - TD滤波因子
            if (value < 1.0f) value = 1.0f;
            ladrc->td.h0 = value * ladrc->dt;
            break;
    }
}
```

---

> **总结**：
> - **TD模块**：内嵌在LADRC结构体中，关键参数td_r（快速跟踪因子）和td_n（无量纲滤波因子）
> - **一阶LADRC**：适用于电流/速度控制，包含二阶ESO和P控制器，关键参数β1/β2（观测器增益）、kp（控制器增益）、b（控制增益）
> - **二阶LADRC**：适用于位置/角度控制，包含三阶ESO和PD控制器，关键参数β1/β2/β3（观测器增益）、kp/kd（控制器增益）、b（控制增益）
> - **参数固化模式**：dt在初始化时固化到结构体，消除时间抖动影响
> - **抗积分饱和**：通过k_aw参数启用，调整u0使其退出饱和状态
>
> 建议初学者按照文档顺序先理解每个模块的作用，再逐步调试参数，最终你会体会到LADRC"以简御繁"的设计哲学。
