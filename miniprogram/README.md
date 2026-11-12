# 微信小程序控制端

## 目录

```text
miniprogram/
├── app.js / app.json / app.wxss
├── config.js              # 需要手动填写的配置
├── project.config.json    # 开发者工具项目配置
├── pages/Huawei_IOT.*     # 设备影子 + 命令下发页面
└── utils/util.js
```

## 配置

编辑 [`config.js`](config.js)：

```js
module.exports = {
  IAM_USERNAME: 'YOUR_IAM_USERNAME',
  IAM_PASSWORD: 'YOUR_IAM_PASSWORD',
  IAM_DOMAIN: 'YOUR_IAM_DOMAIN',
  IAM_PROJECT: 'cn-north-4',
  IOTDA_ENDPOINT: 'https://YOUR_ENDPOINT.st1.iotda-app.cn-north-4.myhuaweicloud.com',
  PROJECT_ID: 'YOUR_PROJECT_ID',
  DEVICE_ID: 'YOUR_DEVICE_ID'
};
```

同时把 `project.config.json` 中的 `appid` 改为你自己的小程序 AppID（当前是占位符）。

## 运行

1. 用微信开发者工具导入本目录。
2. 编译后点击“获取 token”。
3. 点击“获取设备影子”查看最新属性。
4. 调整三个滑块，点击“命令下发”向设备发送 `cmd1`。

开发阶段如果请求域名未备案，可在开发者工具中勾选 **不校验合法域名、web-view（业务域名）、TLS 版本以及 HTTPS 证书**。

## 实现说明

- `gettoken()` 调用华为云 IAM `POST /v3/auth/tokens`，从响应头读取 `X-Subject-Token`，缓存到 `wx.setStorageSync('token', ...)`。
- `getshadow()` 调用 IoTDA 应用侧 `GET /v5/iot/{project_id}/devices/{device_id}/shadow`。
- `setCommand()` 调用 `POST /v5/iot/{project_id}/devices/{device_id}/commands`，发送 `service_id=fuwu`、`command_name=cmd1`。

## 安全提醒

这是课程演示用的客户端直连方案：IAM 用户名和密码打包在小程序里，可以被逆向获取。生产环境应改为：

```text
小程序 → 自己的后端 → 华为云 IAM / IoTDA
```

后端保存 AK/SK，向前端只发放短期令牌。
