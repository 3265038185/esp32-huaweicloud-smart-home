# Django 后台

华为云 IoTDA 数据展示与命令下发的 Django 后台，同时包含课程项目中的登录、个人中心、论坛等页面。

## 技术栈

- Django 1.8.2（课程项目原始版本）
- MySQL
- 华为云 `huaweicloudsdkcore` / `huaweicloudsdkiotda`
- Bootstrap 3 前端模板

> 这是一个 **2016 年前后的 Django 1.8 项目**，使用 `MIDDLEWARE_CLASSES`、`url()` 等旧 API。`requirements.txt` 保持原始版本以便复现；在现代 Python 上运行需要先升级 Django 和相应写法。

## 配置

复制 `.env.example` 中的变量到你的 shell 环境（项目不自动读取 `.env` 文件）：

```bash
export DJANGO_SECRET_KEY='change-me'
export MYSQL_DATABASE='school'
export MYSQL_USER='root'
export MYSQL_PASSWORD='your-password'
export MYSQL_HOST='127.0.0.1'
export MYSQL_PORT='3306'
export HUAWEI_CLOUD_AK='your-ak'
export HUAWEI_CLOUD_SK='your-sk'
export HUAWEI_PROJECT_ID='your-project-id'
export HUAWEI_DEVICE_ID='your-device-id'
export HUAWEI_IOTDA_ENDPOINT='https://iotda.cn-north-4.myhuaweicloud.com'
```

Windows PowerShell：

```powershell
$env:DJANGO_SECRET_KEY='change-me'
$env:MYSQL_DATABASE='school'
# ...其余变量同理
```

## 初始化数据库

```sql
CREATE DATABASE school CHARACTER SET utf8mb4;
```

```bash
pip install -r requirements.txt
python manage.py migrate
python manage.py createsuperuser
python manage.py runserver
```

访问 `http://127.0.0.1:8000/`。

## 主要模块

| 模块 | 作用 |
|------|------|
| `school/` | 项目配置、URL 路由 |
| `login/` | 注册、登录、邮箱激活 |
| `home/` | 首页 |
| `fourm/` | 论坛页面 + IoTDA 设备影子/命令接口 |
| `person/` | 个人资料 |
| `presonal_center/` | 个人中心 |

`fourm/views.py` 中的华为云凭据全部来自环境变量：

```python
HUAWEI_AK = os.environ.get("HUAWEI_CLOUD_AK", "")
HUAWEI_SK = os.environ.get("HUAWEI_CLOUD_SK", "")
HUAWEI_PROJECT_ID = os.environ.get("HUAWEI_PROJECT_ID", "")
HUAWEI_DEVICE_ID = os.environ.get("HUAWEI_DEVICE_ID", "")
```

## 安全说明

- 已移除硬编码的 `SECRET_KEY`、MySQL 密码、邮箱授权码和华为云 AK/SK。
- `login/views.py` 仍是课程项目的明文密码比较逻辑，**不要用于生产**。
- 生产部署应关闭 `DEBUG`、限制 `ALLOWED_HOSTS`、启用 CSRF、使用 HTTPS，并把凭据放入密钥管理服务。
