from django.shortcuts import render
from django.core.paginator import Paginator
from login.models import person
from django.shortcuts import render
from django.shortcuts import redirect
# from django.shortcuts import reverse
from django.conf import settings
from . import models
from django.http import HttpResponse ,HttpResponseRedirect
from django.shortcuts import redirect



from django.template import Template, Context, RequestContext

# Create your views here.

import os
import re
from huaweicloudsdkcore.auth.credentials import BasicCredentials
from huaweicloudsdkcore.exceptions import exceptions
from huaweicloudsdkcore.http.http_config import HttpConfig
from huaweicloudsdkiotda.v5 import *

# Huawei Cloud credentials are loaded from environment variables.
# See ../.env.example for the full list.
HUAWEI_AK = os.environ.get("HUAWEI_CLOUD_AK", "")
HUAWEI_SK = os.environ.get("HUAWEI_CLOUD_SK", "")
HUAWEI_PROJECT_ID = os.environ.get("HUAWEI_PROJECT_ID", "")
HUAWEI_DEVICE_ID = os.environ.get("HUAWEI_DEVICE_ID", "")
HUAWEI_IOTDA_ENDPOINT = os.environ.get(
    "HUAWEI_IOTDA_ENDPOINT", "https://iotda.cn-north-4.myhuaweicloud.com"
)


def single(request):
    return render(request,'single.html')



#参数pIndex表示：当前要显示的页码
def page_test(request,):

    message = find_device_message()
    message_out = dis_str(message)
    humidity = message_out[2]
    temperature = message_out[6]
    battery = message_out[4]




    #将当前页码、当前页的数据、页码信息传递到模板中
    #return render(request, 'fourm.html', {'list': list2, 'plist': plist, 'pIndex': pIndex,'n_pIndex':n_pIndex,'nn_pIndex':nn_pIndex,'l_pIndex':l_pIndex,'ll_pIndex':ll_pIndex,})

    return render(request, 'fourm.html',{ 'humidity': humidity ,'temperature': temperature,'battery':  battery, })
def post_meaage(request):
    command = request.GET.get('command',None)
    arg = request.GET.get('arg',None)
    print(command,arg)
    post_huawei_message(command,arg)


    message = find_device_message()
    message_out = dis_str(message)
    humidity = message_out[2]
    temperature = message_out[6]
    battery = message_out[4]

    return render(request, 'fourm.html', {'humidity': humidity, 'temperature': temperature, 'battery': battery, })




###  huawei


### 查询所有信息
def find_device_message():
    ak = HUAWEI_AK
    sk = HUAWEI_SK
    endpoint = HUAWEI_IOTDA_ENDPOINT
    project_id = HUAWEI_PROJECT_ID

    config = HttpConfig.get_default_config()
    config.timeout = 3

    credentials = BasicCredentials(ak, sk, project_id)

    client = IoTDAClient.new_builder(IoTDAClient) \
        .with_http_config(config) \
        .with_credentials(credentials) \
        .with_endpoint(endpoint) \
        .build()
    print("start build ")

    try:
        request = ShowDeviceShadowRequest()
        request.device_id = HUAWEI_DEVICE_ID
        response = client.show_device_shadow(request)
        print("ok\n")
        print(response)

        return(str(response))

    except exceptions.ClientRequestException as e:
        print(e.status_code)
        print("************")
        print(e.request_id)
        print("************")
        print(e.error_code)
        print("************")
        print(e.error_msg)

### 如果都有引号,就都能返回
def dis_str(content):
    str = re.findall(r"(\n[\s\S]*\n)", content, re.DOTALL)
    # str1=re.findall(r"\n([\s\S]*)\n",str[0])
    str1 = str[0].replace('\n', '')
    str2 = str1.replace(' ', '')
    str3 = re.findall(r"'properties':{(.+?)}},", str2)

    str4 = re.findall(r"'(.+?)'", str3[0])
    str5 = re.findall(r"'(.+?)'", str3[1])
    str6 = re.findall(r"[0-9][0-9]", str3[1])
    str4.append(str5[0])
    str4.append(str6[0])
    print(str4[3])
    return str4



### 下发参数 可同时传递两个变量
def post_huawei_message(message,name):
    ak = HUAWEI_AK
    sk = HUAWEI_SK
    endpoint = HUAWEI_IOTDA_ENDPOINT
    project_id = HUAWEI_PROJECT_ID

    config = HttpConfig.get_default_config()
    config.timeout = 3

    credentials = BasicCredentials(ak, sk, project_id)

    client = IoTDAClient.new_builder(IoTDAClient) \
        .with_http_config(config) \
        .with_credentials(credentials) \
        .with_endpoint(endpoint) \
        .build()

    try:
        request = CreateMessageRequest()
        request.device_id = HUAWEI_DEVICE_ID
        request.body = DeviceMessageRequest(
            message=message,
            name=name
        )
        response = client.create_message(request)
        print(response)
    except exceptions.ClientRequestException as e:
        print(e.status_code)
        print(e.request_id)
        print(e.error_code)
        print(e.error_msg)