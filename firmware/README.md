# ESP32 固件

主程序：[`smart_home/smart_home.ino`](smart_home/smart_home.ino)

## 依赖

- Arduino IDE 2.x 或 Arduino CLI
- ESP32 Arduino Core（建议 3.x）
- [ArduinoJson](https://arduinojson.org/) v7（`JsonDocument` API）
- ESP32 内置库：`WiFi`、`WiFiClientSecure`、`mbedtls`

## 配置

```bash
cd firmware/smart_home
cp secrets.example.h secrets.h   # Windows: copy secrets.example.h secrets.h
```

编辑 `secrets.h`：

| 宏 / 变量 | 含义 |
|-----------|------|
| `WSSID` / `WPASS` | Wi-Fi 名称和密码 |
| `DID` | 华为云设备 ID，例如 `6a2c..._0001` |
| `DSEC` | 设备密钥（在 IoTDA 控制台查看） |
| `MHOST` | 设备接入地址，例如 `xxxx.st1.iotda-device.cn-north-4.myhuaweicloud.com` |
| `MPORT` | MQTT over TLS 端口，固定 `8883` |
| `SCT_KEY` | 可选，Server酱 SendKey；保留占位符则不发送微信通知 |

`secrets.h` 已在 `.gitignore` 中，不会被提交。

## 编译与烧录

1. 开发板选择 ESP32 系列（按实际板卡选择，如 `ESP32 Dev Module` / `ESP32S3 Dev Module`）。
2. 打开 `smart_home.ino` 编译。
3. 烧录后打开串口监视器，波特率 `115200`。
4. 看到 `MQTT 接入华为云成功` 和 `系统就绪！` 即表示端云连接正常。

## 启动流程

```text
上电
  → 连接 Wi-Fi
  → NTP 对时（阿里云 / pool.ntp.org）
  → TLS 连接 IoTDA
  → 用设备密钥生成 MQTT 用户名/密码
  → MQTT CONNECT + 订阅命令 topic
  → 启动本地 HTTP 服务
  → 周期上报属性
```

## 本地 Web API

设备连上 Wi-Fi 后，串口会打印本机 IP。浏览器访问 `http://<设备IP>/`：

| 接口 | 说明 |
|------|------|
| `GET /api/data` | 返回温度、湿度、土壤、LED、喇叭、PIR、开锁次数等 JSON |
| `GET /api/cmd?type=led` | 切换 LED |
| `GET /api/cmd?type=lock` | 开锁（脉冲） |
| `GET /api/cmd?type=spk` | 切换喇叭 |
| `GET /api/cmd?type=auto` | 恢复自动模式 |

## MQTT 主题

| 方向 | Topic |
|------|-------|
| 属性上报 | `$oc/devices/{DID}/sys/properties/report` |
| 命令订阅 | `$oc/devices/{DID}/sys/commands/#` |
| 命令应答 | `$oc/devices/{DID}/sys/commands/response/request_id={id}` |

## 已知限制

- `tls.setInsecure()` 跳过了证书校验，仅适合课程演示；生产环境应改用 CA 证书。
- MQTT 报文是手写的最小实现，未处理 QoS 1/2、断线重连和遗嘱消息。
- Wi-Fi 断开后不会自动重连，需要复位设备。
- 土壤湿度阈值和 PIR 联动逻辑是演示值，实际部署需要按传感器标定。
