/*
无需单独声明 TD 结构体：TD 内嵌在 LADRC 结构体中，一体化调用
参数固化模式：dt 在初始化时固化，调用 ladrc_calc 时无需再传 dt
TD 通过参数启用：td_r > 0 启用 TD，td_r = 0 禁用 TD（直接透传目标值）
滤波因子改为无量纲 N：td_n 是 1~5 的无量纲参数，内部自动计算 h0 = N * dt
*/
#include <stdio.h>
#include <math.h>
#include "ladrc.h"

// 模拟被控对象（二阶系统）
float Plant_Update(float u, float h) {
    static float x1 = 0, x2 = 0;    // 位置和速度状态
    static float d = 0;              // 外部扰动

    // 系统动力学：ddot(y) = -a1*dot(y) - a0*y + b*u + d
    float a1 = 2.0f, a0 = 1.0f, b = 10.0f;
    float dx2 = -a1 * x2 - a0 * x1 + b * u + d;

    // 离散积分（欧拉法）
    x2 += h * dx2;      // 更新速度
    x1 += h * x2;       // 更新位置

    return x1;          // 返回输出（位置）
}

int main(void)
{
    // 定义LADRC控制器（v3.1版本，内嵌TD，无需单独声明td_t）
    ladrc_t controller;

    // 系统参数
    float dt = 0.001f;              // 1ms采样周期
    float wo = 100.0f;              // 观测器带宽
    float wc = 25.0f;               // 控制器带宽
    float b = 10.0f;                // 控制增益

    // 计算LADRC参数（带宽法）
    float beta1 = 3.0f * wo;                // 3*wo
    float beta2 = 3.0f * wo * wo;           // 3*wo^2
    float beta3 = wo * wo * wo;             // wo^3
    float kp = wc * wc;                     // wc^2
    float kd = 2.0f * wc;                   // 2*wc

    // 初始化LADRC：max_output=50, k_aw=2.0（启用抗积分饱和）
    // td_r=100启用TD，td_n=5设置滤波因子，td_max_x2=0不限制速度
    ladrc_init(&controller, 50.0f, beta1, beta2, beta3,
               kp, kd, b, dt, 2.0f,          // 基本参数
               100.0f, 5.0f, 0.0f);          // TD参数：r=100, N=5, max_x2=0

    printf("LADRC控制仿真开始...\n");
    printf("时间\t目标\t输出\t控制量\t扰动估计\n");

    // 仿真循环
    float y = 0.0f;                  // 系统输出
    for (int k = 0; k < 2000; k++) {
        float t = k * dt;            // 当前时间

        // 目标值：0-0.5s为0，0.5s后跳变到10
        float ref = (t > 0.5f) ? 10.0f : 0.0f;

        // LADRC控制计算（TD处理已在内部完成）
        float u = ladrc_calc(&controller, ref, y);

        // 更新被控对象（实际系统中，这是物理过程）
        y = Plant_Update(u, dt);

        // 每100ms打印一次
        if (k % 100 == 0) {
            printf("%.3f\t%.2f\t%.2f\t%.2f\t%.2f\n",
                   t, ref, y, u, controller.x3);
        }
    }

    printf("仿真结束\n");
    return 0;
}