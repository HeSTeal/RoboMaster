/**
 * TD跟踪微分器参数说明表
 * 参数名        符号        作用                                        调节建议
 * 快速跟踪因子  r           决定跟踪速度，相当于最大加速度                r越大，跟踪越快，但可能超调。建议初始值: r = 1/T_settle，就是一秒算多少次
 * 滤波因子      N           决定平滑程度(无量纲)                         建议值1~5。N=1:滤波最弱响应最快; N=3~5:典型推荐值; N>10:强滤波但滞后明显
 * 最大速度      max_x2      限制输出速度                                 根据执行器最大速度设置，防止目标变化过快
 */

/**
 * @brief 跟踪微分器结构体
 * @note  二阶TD，用于安排过渡过程和提取微分信号
 *        采用参数固化模式，采样周期在初始化时确定
 */
typedef struct {
    float x1;       // 跟踪信号（位置）- 平滑后的目标值输出
    //TD当前认为目标应该在哪里--平滑后的目标
    float x2;       // 微分信号（速度）- 目标值的变化率
    //TD给后级控制器提供的目标速度
    float r;        // 快速跟踪因子（相当于最大加速度）
    //最大加速度量级（加速度有多猛）：越大代表跟踪更快，加减速也更猛；同时对噪声，离散误差，执行器限制更敏感
    float h;        // 积分步长（采样周期）- 【固化参数】RTOS任务周期
    //TD每隔多久更新一次
    float h0;       // 滤波因子（用于输入滤波，h0 = N * h）
    //TD在接近目标的时候，有多早开始进入“温柔模式”
    float max_x2;   // 最大速度限制（0表示不限制）
    //目标速度上限（速度最高有多快）
} td_t;


/**
 * @brief TD 初始化 - 参数固化模式
 *
 * @param td      TD结构体指针
 * @param r       快速跟踪因子，决定跟踪速度
 * @param dt      固定采样周期(秒) - RTOS任务周期，将固化到结构体中
 * @param n       滤波因子(无量纲) - 建议值1~5，值越大滤波越强但延迟越大
 * @param max_x2  最大速度限制（0表示不限制）
 *
 * @note 参数固化模式的核心逻辑：
 *       1. 固化采样周期：td->h = dt
 *       2. 自动计算内部滤波参数：td->h0 = n * dt
 *       3. 安全检查：如果 n < 1.0，强制设为 1.0，防止数学模型崩溃
 */
void td_init(td_t *td, float r, float dt, float n, float max_x2) {
    /* 安全性检查：滤波因子N不能小于1.0，否则数学模型会崩溃 */
    if (n < 1.0f) {
        n = 1.0f;
    }

    td->r = r;
    td->h = dt;         /* 固化采样周期到结构体中 */
    td->h0 = n * dt;    /* 自动计算内部滤波参数 h0 = N * dt */
    td->max_x2 = max_x2;
    td->x1 = 0.0f;
    td->x2 = 0.0f;
}


/**
 * @brief fhan 最速控制综合函数（梯形加速度曲线）
 *
 * 这是韩京清教授提出的最速控制综合函数，用于实现时间最优控制。
 * 产生的加速度曲线是梯形的（加速-匀速-减速）。
 *
 * @param x1 位置误差
 * @param x2 速度
 * @param r 快速跟踪因子（相当于最大加速度）
 * @param h 积分步长（采样周期）
 * @param h0 滤波因子（用于输入滤波，典型值为h的5-10倍）
 * @return 加速度输出
 */
float td_fhan(float x1, float x2, float r, float h, float h0) {
    float d = r * h;                                // 单步速度变化量 (delta v)
                                                    // d = r·h 是离散系统能感知的最小速度单位
    //一个采样周期内，最大允许改变多少加速度
    float d0 = h0 * d;                              // 线性区宽度
                                                    // 当误差|y| <= d0时，系统进入线性区平滑处理

    float y = x1 + h * x2;                          /* 预测：当前位置 + h×速度 = 不加控制时的未来位置
                                                     * 产生"超前意识"，抵消离散系统的相位滞后，是不超调的第一道防线 */

    /* 安全检查：限制 y 的范围防止 sqrtf 溢出 */
    const float MAX_Y = 1e15f;
    if (fabsf(y) > MAX_Y) {
        y = (y > 0.0f) ? MAX_Y : -MAX_Y;
    }

    float sqrt_arg = d * d + 8.0f * r * fabsf(y);   // 离散刹车曲线方程的核心
                                                    // 源于等差数列求和公式，8 = 4×2 是离散求和系数

    /* 安全检查：确保 sqrtf 参数非负且有限 */
    if (sqrt_arg < 0.0f || !isfinite(sqrt_arg)) {
        sqrt_arg = 0.0f;
    }

    float a0 = sqrtf(sqrt_arg);                     /* 状态解算指标
                                                     * 代表在当前位置误差y下，系统若想最快停下，
                                                     * 理想中应该具备的"速度量级" */

    float a;                                        // 综合切换指标
    if (fabsf(y) <= d0) {                           // 线性区：系统非常靠近目标
        a = x2 + y / h;                             /* 预测下一时刻刚好归零的逻辑
                                                     * 此时fhan退化为PD控制器结构
                                                     * P项: -y/h², D项: -x2/h */
    } else {                                        // 非线性区：系统离目标较远（赶路）
        a = x2 + 0.5f * (a0 - d) * ((y > 0) ? 1.0f : -1.0f);
                                                    /* 0.5*(a0-d) 是基于当前剩余距离y，
                                                     * 计算出的"当前时刻应该具有的临界速度"
                                                     * 系统始终保持在最速控制切换曲线上 */
    }

    float fhan_out;                                 // 加速度输出
    if (fabsf(a) <= d) {                            // 线性插值区（接近停稳）
        fhan_out = -r * a / d;                      /* 比例缩放：加速度随靠近目标而逐渐减小
                                                     * 如果不做这个线性处理，最后一步可能输出过大加速度导致跨过原点
                                                     * 通过a/d比例缩放，最终在目标点刚好减为0 */
    } else {                                        // 饱和区（满速加速/刹车）- Bang-Bang控制
        fhan_out = -r * ((a > 0) ? 1.0f : -1.0f);   /* 只有+r或-r两种状态
                                                     * 保证最快响应，油门踩到底或刹车踩到底 */
    }

    return fhan_out;
}


/**
 * @brief TD 更新计算 - 参数固化模式
 *
 * @param td      TD结构体指针
 * @param target  目标值
 * @return        跟踪输出x1（平滑后的目标值）
 *
 * @note 参数固化模式：此函数使用结构体中固化的采样周期td->h，
 *       不再接受外部传入的dt。这适用于RTOS固定频率调用的场景，
 *       可以消除时间抖动对积分的影响。
 */
float td_update(td_t *td, float target) {
    float x1_error = td->x1 - target;               // 计算当前跟踪误差

    /* 使用固化的采样周期td->h进行计算，不再依赖外部传入的dt */
    float fh = td_fhan(x1_error, td->x2, td->r, td->h, td->h0);
                                                    // 调用fhan计算最优加速度

    /* 使用固化的td->h进行积分，确保时间一致性 */
    float new_x2 = td->x2 + td->h * fh;             // 速度积分：v = v0 + a·dt

    /* 速度限制检查 */
    if (td->max_x2 > 0.0f) {
        if (new_x2 > td->max_x2) {
            new_x2 = td->max_x2;
        } else if (new_x2 < -td->max_x2) {
            new_x2 = -td->max_x2;
        }
    }

    td->x2 = new_x2;                                // 更新速度状态
    /* 使用固化的td->h进行位置积分 */
    td->x1 = td->x1 + td->h * td->x2;               // 位置积分：x = x0 + v·dt

    return td->x1;                                  // 返回平滑后的目标值
}


/**
 * @brief TD 重置状态
 *
 * @param td         TD结构体指针
 * @param init_value 初始值
 *
 * @note 在系统复位、模式切换或故障恢复时调用，
 *       将TD状态重置为指定值，避免历史状态影响新的控制过程
 */
void td_reset(td_t *td, float init_value) {
    td->x1 = init_value;                            // 位置重置为初始值
    td->x2 = 0.0f;                                  // 速度重置为0
}


// 定义TD结构体
td_t my_td;

// 初始化：快速跟踪因子r=100，采样周期dt=0.001s，滤波因子N=5，最大速度限制50
// N = 5 是典型设置，h0 = 5 * dt 会在内部自动计算
td_init(&my_td, 100.0f, 0.001f, 5.0f, 50.0f);

// 主循环（每1ms执行一次）
while(1) {
    float target = Get_Target();                    // 获取目标值
    float smooth_target = td_update(&my_td, target);  // 更新TD（无需传入dt）
    float target_speed = my_td.x2;                  // 获取目标值的变化率

    // 将smooth_target和target_speed用于后续控制...

    Delay_ms(1);                                    // 等待下一个周期
}

// 如果需要重置TD状态（如系统复位时）
td_reset(&my_td, 0.0f);  // 将位置重置为0，速度重置为0