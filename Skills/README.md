Skills/README.md 文件中记录了对 BuildReplyPacket 的修改：

变更文件：
- UserApp/Source/SendDataProcess.c

变更要点：
1. 允许 dataLen 为 0 时构建应答包（即无数据字段的包）。
2. 增加参数校验：当 dataLen>0 时必须确保 data 指针不为 NULL，若为 NULL 返回错误码 3。
3. 保持原有包格式：头(0xAA)、功能码、执行索引(2字节)、可选数据、校验码。
4. 所有新增或修改的代码行均添加了简短注释，遵循 Skills.md 的要求。

路径说明：
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\SendDataProcess.c
- 头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\SendDataProcess.h
- README：c:\Users\Administrator\Desktop\FreeRTOS\Skills\README.md

构建与测试：
1. 在项目根目录运行 `make` 来编译工程，确保修改不会引入编译错误。
2. 通过单元或集成测试验证在 dataLen 为 0、dataLen>0 且 data 为 NULL、dataLen>0 且 data 有效等场景下的行为。

注意：
- 请不要新建其他说明文件，所有变更说明都写在本文件中。
- 如需我自动运行 make，请回复确认，我将执行构建命令并返回输出。

Skills/README.md 文件新增说明：

新增功能：ReplyPacket 封装函数

变更文件：
- UserApp/Source/SendDataProcess.c  （新增 ReplyPacket 实现）

功能说明：
- 封装函数 ReplyPacket(ReplyCode_t reply) 用于构建并发送应答包给上位机。
- 生成包格式：0xAA | FUNCTION_CODE_Reply(0x01) | execIndexHigh | execIndexLow | CRC8
  - 当传入 REPLY_OK（值 0）时，示例输出为：0xAA 0x01 0x00 0x00 <CRC8>
- outBuf 在函数内部使用固定大小缓冲区（PACKET_MAX_SIZE），确保不会产生外部内存分配。

路径说明：
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\SendDataProcess.c
- 头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\SendDataProcess.h
- 枚举定义：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\TaskList.h

测试建议：
- 调用 ReplyPacket(REPLY_OK) 并在上位机使用串口/USB 监听，验证接收到的数据序列为：0xAA 0x01 0x00 0x00 CRC8

注意：
- 所有新增代码已添加简短注释，符合 Skills 要求。
- 若需将 execIndex 用其他字段表示（例如单字节 data），可进一步修改 BuildReplyPacket 与 ReplyPacket。

新增功能：ADC 独立任务

变更文件：
- UserApp/Include/ADCAPP.h （新增任务声明）
- UserApp/Source/ADCAPP.c  （新增任务实现）

功能说明：
- 创建了名为 `vADCProcessTask` 的 FreeRTOS 任务。
- `vADCProcessTask` 循环调用 `print_voltage()` 函数。
- 任务栈大小设为 512 字，优先级为 `tskIDLE_PRIORITY + 1`。
- 默认采样间隔为 1000ms（可调整 vTaskDelay 参数）。
- 需在主程序（如 `main.c` 或系统初始化处）调用 `vCreateADCTask()` 启动该任务。

路径说明：
- ADC 头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\ADCAPP.h
- ADC 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\ADCAPP.c

测试建议：
- 在 main.c（或其他初始化位置）调用 `vCreateADCTask()`。
- 编译并在设备上运行，通过串口查看每秒一次的电压打印日志。

变更文件：
- UserApp/Source/ADCAPP.c

变更要点：
1. 重构 adc_to_voltage 函数：精简变量，直接返回计算公式结果，保持原有计算逻辑。
2. 重构 print_voltage 函数：
   - 将原有 printf 替换为系统统一的 `QueueSendfmt`（配合 xReceiveLogQueue），实现线程安全的日志输出。
   - 增加 DMA 等待超时机制（timeout=100ms），防止硬件异常导致的死循环。
   - 将忙等待 `while` 循环中的空转改为 `vTaskDelay(1)`，避免 DMA 转换期间独占 CPU 资源。
   - 包含 `SendDataProcess.h` 以使用日志队列接口。

路径说明：
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\ADCAPP.c

测试建议：
- 确保系统已启动 SendLogTask (RecieveLogQueue)。
- 运行 vCreateADCTask，观察 USB/串口 日志输出，验证电压数据刷新且无 "ADC DMA Timeout" 错误。

变更文件：
- UserApp/Source/ADCAPP.c

变更要点：
1. 修正电压计算公式：基于实测数据（3.5V读成2.92V，0V读成-0.60V），增加了 +0.60V 的线性补偿 `ADC_VOLTAGE_OFFSET`。
   - 这表明此时硬件上可能有约 0.6V 的负偏置或者是参考电压/分压系数的系统性误差。
2. 保持 print_voltage 使用 QueueSendfmt 打印，并使用 vTaskDelay 做非阻塞等待。

路径说明：
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\ADCAPP.c

测试建议：
- 重新烧录并观察日志。
- 预期：VDD 应接近 3.52V (2.92+0.6)，GND 相关通道应接近 0.00V (-0.60+0.6)。

变更文件：
- UserApp/Include/DPPTM.h （新增函数声明）
- UserApp/Source/DPPTM.c  （新增函数实现）

新增功能：VDD电压转抽头值函数

功能说明：
- 函数名：`VoltageToTapValue(float voltage)`
- 功能：将上位机请求的 VDD 电压值（单位V）转换为数字电位器抽头位置（0~15）
- 映射关系：
  - 2.0V → 抽头值 0
  - 3.6V → 抽头值 15
  - 中间值使用线性插值计算
- 边界处理：
  - 电压 ≤ 2.0V 时返回 0
  - 电压 ≥ 3.6V 时返回 15
  - 中间值四舍五入取整
- 每行代码均添加简短注释，符合 Skills 要求

路径说明：
- 头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\DPPTM.h
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\DPPTM.c

使用示例：
```c
uint8_t tapValue = VoltageToTapValue(3.0f); // 计算3.0V对应的抽头值
ControlDPPTM(tapValue); // 设置数字电位器
```

测试建议：
- 测试边界值：VoltageToTapValue(2.0f) 应返回 0
- 测试边界值：VoltageToTapValue(3.6f) 应返回 15
- 测试中间值：VoltageToTapValue(2.8f) 应返回 7 或 8（取决于四舍五入）

新增功能：VDD电压闭环控制系统

变更文件：
- UserApp/Include/ADCAPP.h （新增 get_current_vdd_voltage 函数声明）
- UserApp/Source/ADCAPP.c  （新增 get_current_vdd_voltage 函数实现）
- UserApp/Include/DPPTM.h  （新增 #include "ADCAPP.h"）
- UserApp/Source/DPPTM.c   （重写 SetESLVDDVoltage 函数，实现闭环控制）

功能说明：
1. **get_current_vdd_voltage()** - 从ADC获取当前VDD电压值
   - 返回值：当前VDD电压（单位V），失败返回 -1.0f
   - 功能：启动ADC DMA采样，读取VDD通道（adcbuf[0]）的电压值

2. **SetESLVDDVoltage(uint8_t VDDIndx)** - 闭环控制VDD电压
   - 输入：VDDIndx（0-16），对应2.0V-3.6V，步进0.1V
   - 控制逻辑：
     * 使用比例控制算法
     * 读取当前电压 → 计算误差 → 调整DPPTM值 → 等待稳定 → 循环
     * data越小电压越大，data越大电压越小（范围0-254）
   - 参数：
     * 电压容差：±0.05V
     * 最大迭代次数：50次
     * 稳定等待时间：100ms/次
     * 比例系数：30（可调整）
   - 输出：通过日志队列实时显示调整过程

路径说明：
- ADC头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\ADCAPP.h
- ADC源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\ADCAPP.c
- DPPTM头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\DPPTM.h
- DPPTM源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\DPPTM.c

使用示例：
```c
SetESLVDDVoltage(10); // 设置VDD电压为3.0V（索引10）
// 函数会自动进行闭环调整，直到达到目标值或超时
```

测试建议：
- 调用 SetESLVDDVoltage(10) 设置3.0V，观察日志输出的迭代过程
- 验证最终电压是否在 2.95V-3.05V 范围内（±0.05V容差）
- 若收敛速度慢或振荡，可调整比例系数（当前为30）

注意事项：
- 每次调整后等待100ms，确保电压稳定
- 比例系数可能需要根据实际硬件特性微调
- 所有新增代码均添加简短注释，符合Skills要求

新增功能：ADC电压数据包发送

变更文件：
- UserApp/Include/ADCAPP.h （新增 send_voltage_packet 函数声明）
- UserApp/Source/ADCAPP.c  （新增 send_voltage_packet 函数实现，新增头文件引用）

功能说明：
- 函数名：`send_voltage_packet(void)`
- 功能：将11个ADC通道的原始值转换为实际电压值，通过BuildReplyPacket构建数据包发送给上位机
- 数据格式：
  * 每个电压值用2字节表示（int16_t，单位0.01V）
  * 11个通道共22字节数据
  * 例如：3.50V 表示为 350 (0x015E)，-0.60V 表示为 -60 (0xFFC4)
- 数据包格式：
  * 功能码：FUNCTION_CODE_DataPacket (0x02)
  * 执行索引：0（表示电压数据）
  * 数据长度：22字节
  * 通道顺序：adcbuf[0]~adcbuf[10]（VDD, VSPL, VDD15V等）

工作流程：
1. 启动ADC DMA采样11个通道
2. 等待DMA完成（超时10ms）
3. 将每个ADC原始值通过 adc_to_voltage() 转换为实际电压（float）
4. 将浮点电压值乘以100转为整数（int16_t），分高低字节存入数组
5. 调用 BuildReplyPacket 构建数据包
6. 通过 CDC_Transmit_FS 发送给上位机（最多重试3次）

路径说明：
- 头文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Include\ADCAPP.h
- 源文件：c:\Users\Administrator\Desktop\FreeRTOS\UserApp\Source\ADCAPP.c

使用示例：
```c
send_voltage_packet(); // 采样并发送电压数据包
```

上位机解析示例（Python）：
```python
# 假设接收到的数据包为 packet
# 跳过包头(1) + 功能码(1) + 执行索引(2) = 4字节
data_start = 4
voltages = []
for i in range(11):
    high = packet[data_start + i*2]
    low = packet[data_start + i*2 + 1]
    voltage_int = (high << 8) | low
    if voltage_int > 32767:  # 处理负数
        voltage_int -= 65536
    voltage = voltage_int / 100.0  # 转为实际电压
    voltages.append(voltage)
```

测试建议：
- 调用 send_voltage_packet() 并在上位机监听USB CDC
- 验证接收到的数据包格式：0xAA 0x02 0x00 0x00 [22字节数据] [CRC]
- 解析数据并与实际电压值对比验证