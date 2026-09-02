/**
 * 二阶LADRC参数说明表
 * 参数名         符号            作用                                   整定建议
 * ESO增益        β₁,β₂,β₃       决定观测器跟踪速度和稳定性             基于带宽法: β₁ = 3ωₒ, β₂ = 3ωₒ², β₃ = ωₒ³
 * 比例增益       kp              位置误差响应                           基于带宽法: kp = ωc²
 * 微分增益       kd              速度阻尼                               基于带宽法: kd = 2ωc。注意：LADRC中通常设为0，由b0处理
 * 控制增益       b               控制效率                               从阶跃响应估计，需与实际系统匹配
 * 输出限幅       max_output      执行器输出限制                         根据执行器能力设置
 * 抗饱和增益     k_aw            抑制积分饱和                           0表示不使用，建议值1.0‑3.0
 * TD快速跟踪因子 td_r            目标值响应速度                         0表示禁用TD，建议值根据过渡时间要求设置
 * TD滤波因子     td_n            噪声过滤能力                           无量纲，建议值1~5
 */

/**
 * @brief 二阶LADRC结构体
 * @note  适用于位置/角度控制等二阶系统
 *        采用组合模式，内嵌TD模块
 */
typedef struct {
    // ESO增益（三阶观测器）
    float beta1;        // ESO增益beta1 - 位置观测带宽
    float beta2;        // ESO增益beta2 - 速度观测带宽
    float beta3;        // ESO增益beta3 - 扰动观测带宽

    // 控制器增益
    float kp;           // 比例增益（刚度）
    float kd;           // 微分增益（阻尼）- 注意：LADRC中通常设为0，由b0处理，除非需要额外的PD

    // 系统参数
    float b;            // 控制增益(b0) - 决定控制量的缩放比例
    float dt;           // 采样周期(秒) - 【固化参数】RTOS固定周期

    // 状态估计值
    float x1;           // 估计的位置（跟踪测量值）
    float x2;           // 估计的速度
    float x3;           // 估计的总扰动（包含内部动态和外部扰动）

    // 输出限制
    float max_output;   // 输出限幅值（如PWM最大值）

    // 抗积分饱和
    float k_aw;         // 抗积分饱和增益（0表示不使用，建议值1.0~3.0）
    float pre_out;      // 上一时刻输出（用于计算饱和程度）

    // 输出
    float out;          // 当前输出

    // TD组合模式（v3.1新增）
    td_t td;            // 内嵌TD结构体
    bool use_td;        // 是否使用TD（在初始化时根据td_r参数决定）
} ladrc_t;

/**
 * @brief 二阶LADRC初始化 - 参数固化+TD组合模式
 *
 * @param ladrc      二阶LADRC结构体指针
 * @param max_output 控制量输出限幅（例如PWM最大值）
 * @param beta1      ESO状态观测器增益1（位置观测带宽）
 * @param beta2      ESO状态观测器增益2（速度观测带宽）
 * @param beta3      ESO状态观测器增益3（扰动观测带宽）
 * @param kp         控制器比例增益（刚度）
 * @param kd         控制器微分增益（阻尼）- 注意：一阶LADRC中通常设为0，由b0处理，感谢 SYW561 勘误
 * @param b          系统增益估计值(b0) - 决定控制量的缩放比例
 * @param dt         RTOS固定采样周期(秒) - 必须与实际任务频率一致，将固化到结构体
 * @param k_aw       抗积分饱和增益（建议值1.0~3.0，0表示不启用）
 * @param td_r       TD快速跟踪因子(r) - 决定目标值响应速度
 *                   0表示禁用TD（直接透传目标值）
 * @param td_n       TD滤波因子(无量纲，建议值1~5) - 决定噪声过滤能力
 *                   N=1: 滤波最弱响应最快；N=3~5: 典型推荐值；N>10: 强滤波但滞后明显
 * @param td_max_x2  TD最大速度限制(0表示不限制) - 防止设定值跳变过大导致系统冲击
 */
void ladrc_init(ladrc_t *ladrc, float max_output,
                float beta1, float beta2, float beta3,
                float kp, float kd, float b,
                float dt, float k_aw,
                float td_r, float td_n, float td_max_x2) {
    /* 1. 初始化 ESO (扩张状态观测器) 参数 */
    ladrc->beta1 = beta1;
    ladrc->beta2 = beta2;
    ladrc->beta3 = beta3;

    /* 2. 初始化 控制器 (State Error Feedback) 参数 */
    ladrc->kp = kp;
    ladrc->kd = kd;                                 // 注意：LADRC中通常设为0，由b0处理
    ladrc->b = b;

    /* 3. 清空 ESO 内部状态 */
    ladrc->x1 = 0.0f;                               // 估计位置清零
    ladrc->x2 = 0.0f;                               // 估计速度清零
    ladrc->x3 = 0.0f;                               // 估计总扰动清零

    /* 4. 输出与抗饱和设置 */
    ladrc->max_output = max_output;
    ladrc->k_aw = k_aw;
    ladrc->pre_out = 0.0f;                          // 上一时刻输出清零
    ladrc->out = 0.0f;

    /* 5. 固化采样周期 (核心设计) */
    // 这个 dt 将被 ESO 和 TD 共同使用，作为时间基准
    // 固化后，update函数不再接受外部dt，消除时间抖动影响
    ladrc->dt = dt;

    /* 6. 初始化 TD (参数固化模式) */
    if (td_r > 0.0f) {
        // 调用 td_init - 使用参数固化模式
        // 传入滤波因子 N (td_n)，内部自动计算 h0 = N * dt
        td_init(&ladrc->td, td_r, dt, td_n, td_max_x2);
        ladrc->use_td = true;                       // 启用TD
    } else {
        // 禁用 TD 模式，目标值直接透传
        ladrc->use_td = false;
        // 为了安全，将 TD 状态清零
        td_reset(&ladrc->td, 0.0f);
    }
}

/**
 * @brief 二阶LADRC参数重置 - 支持热更新
 *
 * @note 在系统运行过程中动态调整参数，同时重置ESO状态避免瞬态问题
 *       常用于自适应控制、参数调度等场景
 */
void ladrc_reset(ladrc_t *ladrc, float beta1, float beta2, float beta3,
                 float kp, float kd, float b,
                 float dt, float k_aw,
                 float td_r, float td_n, float td_max_x2) {
    /* 更新控制参数 */
    ladrc->beta1 = beta1;
    ladrc->beta2 = beta2;
    ladrc->beta3 = beta3;
    ladrc->kp = kp;
    ladrc->kd = kd;
    ladrc->b = b;
    ladrc->dt = dt;
    ladrc->k_aw = k_aw;

    /* 重置 ESO 状态，避免参数切换时的瞬态问题 */
    ladrc->x1 = 0.0f;
    ladrc->x2 = 0.0f;
    ladrc->x3 = 0.0f;
    ladrc->pre_out = 0.0f;

    /* 重新配置TD参数 - 使用参数固化模式 */
    if (td_r > 0.0f) {
        /* 安全性检查：滤波因子N不能小于1.0 */
        float n = td_n;
        if (n < 1.0f) {
            n = 1.0f;
        }
        ladrc->td.r = td_r;
        ladrc->td.h = dt;           // 固化采样周期
        ladrc->td.h0 = n * dt;      // 计算内部滤波参数 h0 = N * dt
        ladrc->td.max_x2 = td_max_x2;
        ladrc->use_td = true;
    } else {
        ladrc->use_td = false;
    }
}

/**
 * @brief 二阶LADRC计算 - 核心控制算法
 *
 * @param ladrc    二阶LADRC结构体指针
 * @param target   目标值
 * @param measure  系统实际测量值（如编码器读数）
 * @return         控制量输出
 *
 * @note 控制流程：
 *       1. TD平滑目标值（如果启用）
 *       2. 三阶ESO估计系统状态（位置、速度）和总扰动
 *       3. PD控制器计算虚拟控制量u0
 *       4. 扰动补偿得到实际控制量
 *       5. 输出限幅和抗积分饱和处理
 */
float ladrc_calc(ladrc_t *ladrc, float target, float measure) {
    /* 步骤0: TD（跟踪微分器）处理目标值 */
    float td_target = target;
    if (ladrc->use_td) {
        /* 参数固化模式：不再传入dt，使用结构体中固化的td->h */
        td_target = td_update(&ladrc->td, target);
                                                    // 获取平滑后的目标值
                                                    // ladrc->td.x1: 平滑目标值
                                                    // ladrc->td.x2: 目标变化率（微分信号）
    }

    /* 步骤1: 执行三阶扩张状态观测器(ESO) */
    /*
     * 二阶LADRC被控对象: ẍ = f + b*u
     * 三阶ESO公式:
     * dx1 = x2 + β1*(y - x1)         <- ẋ1 = x2 (速度)
     * dx2 = x3 + b*u + β2*(y - x1)   <- ẋ2 = x3 + b*u (加速度=扰动+控制)
     * dx3 = β3*(y - x1)              <- ẋ3 = df/dt (扰动变化率，假设变化缓慢)
     *
     * 状态含义:
     * x1 = y (位置估计)
     * x2 = ẏ = v (速度估计)
     * x3 = f(x,ẋ,d) (总扰动估计，包含模型不确定性和外部扰动)
     *
     * 关键设计：使用上一时刻的实际输出(限幅后的pre_out)进行ESO更新，
     *          防止积分饱和导致观测器发散
     */

    /* 计算ESO微分方程 - 优化：只计算一次误差 */
    float error = measure - ladrc->x1;              // 观测误差 = 测量值 - 估计值
    float dx1 = ladrc->x2 + ladrc->beta1 * error;   // 位置估计的微分 = 速度估计 + 修正项
    float dx2 = ladrc->x3 + ladrc->b * ladrc->pre_out + ladrc->beta2 * error;
                                                    // 速度估计的微分 = 扰动估计 + b*控制量 + 修正项
    float dx3 = ladrc->beta3 * error;               // 扰动估计的微分（假设扰动变化缓慢）

    /* 更新状态估计值(欧拉积分，乘以dt) */
    ladrc->x1 += dx1 * ladrc->dt;                   // 离散积分更新位置估计
    ladrc->x2 += dx2 * ladrc->dt;                   // 离散积分更新速度估计
    ladrc->x3 += dx3 * ladrc->dt;                   // 离散积分更新扰动估计

    /* 步骤2: 计算控制量u0 */
    /*
     * 控制律公式:
     * u0 = kp * (r - x1) - kd * x2   // 名义控制：PD控制器
     * u = (u0 - x3) / b              // 扰动补偿：用估计的扰动x3进行前馈补偿
     *
     * 物理意义：通过ESO估计出总扰动x3，在控制量中将其抵消，
     *          使系统变为纯粹的二重积分器 ẍ = b*u0
     */

    /* 计算名义控制量u0 */
    float u0 = ladrc->kp * (td_target - ladrc->x1) - ladrc->kd * ladrc->x2;
                                                    // PD控制器：u0 = kp*误差 - kd*速度
                                                    // 注意：LADRC中kd通常设为0，由b0处理阻尼

    /* 抗积分饱和处理 */
    /*
     * 当控制量达到限幅时，通过调整名义控制量u0使其退出饱和状态
     */
    if (ladrc->k_aw > 0.0f && fabsf(ladrc->pre_out) >= ladrc->max_output * 0.99f) {
        float u_ideal = (u0 - ladrc->x3) / ladrc->b;
                                                    // 理论上的理想控制量（无限幅时）
        float saturation_error = ladrc->pre_out - u_ideal;
                                                    // 饱和误差 = 实际输出 - 理想输出
        u0 -= ladrc->k_aw * ladrc->b * saturation_error;
                                                    // 调整u0使其趋向于退出饱和
    }

    /* 计算理论控制输出u */
    float out_temp;
    if (fabsf(ladrc->b) < 0.0001f) {                // 安全检查：防止除零
        out_temp = 0.0f;
    } else {
        out_temp = (u0 - ladrc->x3) / ladrc->b;     // 扰动补偿：u = (u0 - x3) / b
                                                    // 将估计的扰动x3从控制量中抵消
    }

    /* 输出限幅 */
    if (out_temp > ladrc->max_output) {
        out_temp = ladrc->max_output;
    } else if (out_temp < -ladrc->max_output) {
        out_temp = -ladrc->max_output;
    }

    /* 保存输出用于下一次ESO计算 */
    ladrc->out = out_temp;
    ladrc->pre_out = out_temp;                      // 保存限幅后的输出，用于下一轮ESO更新
                                                    // 【关键】ESO看到"实际的"控制量，避免积分饱和

    return ladrc->out;
}