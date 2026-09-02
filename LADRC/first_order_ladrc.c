/**
 * 一阶LADRC参数说明表
 * 参数名         符号           作用                     整定建议
 * ESO增益        β₁,β₂         观测器增益               基于带宽法: β₁ = 2ωₒ, β₂ = ωₒ²
 * 比例增益       kp            误差响应                 基于带宽法: kp = ωc
 * 控制增益       b             控制效率                 从阶跃响应估计，需与实际系统匹配
 * 输出限幅       max_output    执行器输出限制           根据执行器能力设置
 * 抗饱和增益     k_aw          抑制积分饱和             0表示不使用，建议值1.0‑3.0
 * TD快速跟踪因子 td_r          目标值响应速度           0表示禁用TD，建议值根据过渡时间要求设置
 * TD滤波因子     td_n          噪声过滤能力             无量纲，建议值1~5
 */

/**
 * @brief 一阶LADRC结构体
 * @note  适用于电流/简单速度控制等一阶系统
 *        采用组合模式，内嵌TD模块
 */
typedef struct {
    // ESO增益（二阶观测器）
    float beta1;        // ESO增益beta1 - 位置观测带宽
    float beta2;        // ESO增益beta2 - 扰动观测带宽

    // 控制器增益
    float kp;           // 比例增益 - 控制器刚度

    // 系统参数
    float b;            // 控制增益(b0) - 决定控制量的缩放比例
    float dt;           // 采样周期(秒) - 【固化参数】RTOS固定周期

    // 状态估计值
    float x1;           // 估计的系统输出（跟踪测量值）
    float x2;           // 估计的总扰动（包含内部动态和外部扰动）

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
} first_order_ladrc_t;

/**
 * @brief 一阶LADRC初始化 - 参数固化+TD组合模式
 *
 * @param fladrc     一阶LADRC结构体指针
 * @param max_output 控制量输出限幅（例如PWM最大值）
 * @param beta1      ESO状态观测器增益1（位置观测带宽）
 * @param beta2      ESO状态观测器增益2（扰动观测带宽）
 * @param kp         控制器比例增益（刚度）
 * @param b          系统增益估计值(b0) - 决定控制量的缩放比例
 * @param dt         RTOS固定采样周期(秒) - 必须与实际任务频率一致
 * @param k_aw       抗积分饱和增益（建议值1.0~3.0，0表示不启用）
 * @param td_r       TD快速跟踪因子(r) - 决定目标值响应速度
 *                   0表示禁用TD（直接透传目标值）
 * @param td_n       TD滤波因子(无量纲，建议值1~5) - 决定噪声过滤能力
 *                   N=1: 滤波最弱响应最快；N=3~5: 典型推荐值；N>10: 强滤波但滞后明显
 * @param td_max_x2  TD最大速度限制(0表示不限制) - 防止设定值跳变过大导致系统冲击
 */
void first_order_ladrc_init(first_order_ladrc_t *fladrc, float max_output,
                           float beta1, float beta2, float kp, float b,
                           float dt, float k_aw,
                           float td_r, float td_n, float td_max_x2) {

    /* 初始化ESO参数 */
    fladrc->beta1 = beta1;
    fladrc->beta2 = beta2;

    /* 初始化控制器参数 */
    fladrc->kp = kp;
    fladrc->b = b;

    /* 初始化状态估计值 */
    fladrc->x1 = 0.0f;                              // 输出估计清零
    fladrc->x2 = 0.0f;                              // 扰动估计清零

    /* 初始化输出限制 */
    fladrc->max_output = max_output;

    /* 初始化抗积分饱和参数 */
    fladrc->k_aw = k_aw;
    fladrc->pre_out = 0.0f;                         // 上一时刻输出清零

    /* 初始化采样周期 - 固化到结构体中 */
    fladrc->dt = dt;                                // 【关键】后续计算都使用这个固化的dt

    /* 初始化输出 */
    fladrc->out = 0.0f;

    /* 初始化TD（组合模式）- 使用参数固化模式 */
    if (td_r > 0.0f) {
        td_init(&fladrc->td, td_r, dt, td_n, td_max_x2);
                                                    // 调用TD初始化，传入滤波因子N
        fladrc->use_td = true;                      // 启用TD
    } else {
        fladrc->use_td = false;                     // 禁用TD，目标值直接透传
    }
}

/**
 * @brief 一阶LADRC参数重置 - 支持热更新
 *
 * @note 在系统运行过程中动态调整参数，同时重置ESO状态避免瞬态问题
 *       常用于自适应控制、参数调度等场景
 */
void first_order_ladrc_reset(first_order_ladrc_t *fladrc,
                             float beta1, float beta2, float kp, float b,
                             float dt, float k_aw,
                             float td_r, float td_n, float td_max_x2) {
    /* 更新控制参数 */
    fladrc->beta1 = beta1;
    fladrc->beta2 = beta2;
    fladrc->kp = kp;
    fladrc->b = b;
    fladrc->dt = dt;
    fladrc->k_aw = k_aw;

    /* 重置 ESO 状态，避免参数切换时的瞬态问题 */
    fladrc->x1 = 0.0f;
    fladrc->x2 = 0.0f;
    fladrc->pre_out = 0.0f;

    /* 重新配置TD参数 - 使用参数固化模式 */
    if (td_r > 0.0f) {
        /* 安全性检查：滤波因子N不能小于1.0 */
        float n = td_n;
        if (n < 1.0f) {
            n = 1.0f;
        }
        fladrc->td.r = td_r;
        fladrc->td.h = dt;          // 固化采样周期
        fladrc->td.h0 = n * dt;     // 计算内部滤波参数 h0 = N * dt
        fladrc->td.max_x2 = td_max_x2;
        fladrc->use_td = true;
    } else {
        fladrc->use_td = false;
    }
}

/**
 * @brief 一阶LADRC计算函数 - 核心控制算法
 *
 * @param fladrc  一阶LADRC结构体指针
 * @param target  目标值
 * @param measure 系统实际测量值
 * @return        控制量输出
 *
 * @note 控制流程：
 *       1. TD平滑目标值（如果启用）
 *       2. 二阶ESO估计系统状态和总扰动
 *       3. P控制器计算虚拟控制量u0
 *       4. 扰动补偿得到实际控制量
 *       5. 输出限幅和抗积分饱和处理
 */
float first_order_ladrc_calc(first_order_ladrc_t *fladrc,
                              float target, float measure) {
    /*
     * 一阶LADRC原理：
     * 被控对象：ẋ = f(x, w, t) + b*u  (一阶系统)
     * 其中f(x,w,t)为总扰动，包含模型不确定性和外部扰动
     */

    /* 步骤0: TD（跟踪微分器）处理目标值 */
    float td_target = target;
    if (fladrc->use_td) {
        /* 参数固化模式：不再传入dt，使用结构体中固化的td->h */
        td_target = td_update(&fladrc->td, target);
                                                    // 获取平滑后的目标值
                                                    // 同时fladrc->td.x2为目标变化率
    }

    /* 步骤1: 执行二阶扩张状态观测器(ESO) */
    /*
     * 一阶系统ESO公式 (二阶观测器):
     * dx1 = x2 + b*u + beta1 * (measure - x1)   <- ẋ1 = x2 + b*u + 修正项
     * dx2 = beta2 * (measure - x1)              <- ẋ2 = 扰动变化率
     *
     * 状态含义:
     * x1 - 系统输出估计（跟踪测量值measure）
     * x2 - 总扰动估计（包含内部动态f(x)和外部扰动w）
     *
     * 关键设计：使用上一时刻的实际输出(限幅后的pre_out)进行ESO更新，
     *          防止积分饱和导致观测器发散
     */

    /* 计算ESO微分方程 - 优化：只计算一次误差 */
    float error = measure - fladrc->x1;             // 观测误差 = 测量值 - 估计值
    float dx1 = fladrc->x2 + fladrc->b * fladrc->pre_out + fladrc->beta1 * error;
                                                    // 输出估计的微分
                                                    // = 扰动估计 + b*控制量 + 修正项
    float dx2 = fladrc->beta2 * error;              // 扰动估计的微分（假设扰动变化缓慢）

    /* 更新状态估计值(欧拉积分，乘以dt) */
    fladrc->x1 += dx1 * fladrc->dt;                 // 离散积分更新输出估计
    fladrc->x2 += dx2 * fladrc->dt;                 // 离散积分更新扰动估计

    /* 步骤2: 计算控制量 */
    /*
     * 一阶LADRC控制律:
     * u0 = kp * (target - x1)        // 名义控制：P控制器
     * u = (u0 - x2) / b              // 扰动补偿：用估计的扰动x2进行前馈补偿
     *
     * 物理意义：通过ESO估计出总扰动x2，在控制量中将其抵消，
     *          使系统变为纯粹的积分器 ẋ = b*u0
     */

    /* 计算名义控制量u0 */
    float u0 = fladrc->kp * (td_target - fladrc->x1);
                                                    // P控制器：u0 = kp * 误差

    /* 抗积分饱和处理 */
    /*
     * 当控制量达到限幅时，ESO中的扰动估计可能会持续累积（积分饱和），
     * 导致系统退出饱和时出现大的超调。
     * 抗积分饱和通过检测饱和误差，调整u0使其退出饱和状态。
     */
    if (fladrc->k_aw > 0.0f && fabsf(fladrc->pre_out) >= fladrc->max_output * 0.99f) {
        float u_ideal = (u0 - fladrc->x2) / fladrc->b;
                                                    // 理论上的理想控制量（无限幅时）
        float saturation_error = fladrc->pre_out - u_ideal;
                                                    // 饱和误差 = 实际输出 - 理想输出
        u0 -= fladrc->k_aw * fladrc->b * saturation_error;
                                                    /* 调整名义控制量u0，使其趋向于退出饱和
                                                     * k_aw越大，退出饱和越快，但可能影响稳态精度 */
    }

    /* 计算理论控制输出u */
    float out_temp;
    if (fabsf(fladrc->b) < 0.0001f) {               // 安全检查：防止除零
        out_temp = 0.0f;
    } else {
        out_temp = (u0 - fladrc->x2) / fladrc->b;   // 扰动补偿：u = (u0 - x2) / b
                                                    // 将估计的扰动x2从控制量中抵消
    }

    /* 输出限幅 */
    if (out_temp > fladrc->max_output) {
        out_temp = fladrc->max_output;
    } else if (out_temp < -fladrc->max_output) {
        out_temp = -fladrc->max_output;
    }

    /* 保存输出用于下一次ESO计算 */
    fladrc->out = out_temp;
    fladrc->pre_out = out_temp;                     // 保存限幅后的输出，用于下一轮ESO更新
                                                    // 【关键】这防止了ESO看到"想要的"控制量，
                                                    // 而是看到"实际的"控制量，避免积分饱和

    return fladrc->out;
}