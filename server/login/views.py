from django.shortcuts import render
from django.shortcuts import redirect
# from django.shortcuts import reverse
from django.conf import settings
from . import models
from django.http import HttpResponse

from itsdangerous import  TimedJSONWebSignatureSerializer as Serializer
from itsdangerous import SignatureExpired
from django.core.mail import send_mail

# Create your views here.




def login(request):

    return render(request, 'login.html')

def login_check(request):
    if request.method == "POST":
        username = request.POST.get('username',None)
        password = request.POST.get('password',None)
        message = "请填写完所有的字段"
        if username and password:
            try:
                user = models.person.objects.get(name=username)



                if user.password == password:
                    if user.is_active == "0":
                        message ="您没有通过邮箱验证哦"
                        ### 懒了,关闭邮箱验证
                        request.session['user_name'] = username
                        return render(request, 'index.html')
                    else:
                        request.session['user_name'] = username
                        return render(request,'index.html')
                else:
                    message = "密码不正确!"
            except:
                message = "用户名不存在!"


            return render(request,'login.html',{"message":message})

        return render(request, 'login.html', {"message": message})


def register(request):
    pass
    return render(request, 'register.html')

def register_check(request):
    if request.method == 'POST':
        username = request.POST.get('username',None)
        password = request.POST.get('password',None)
        email = request.POST.get("email",None)
        shcool_num = request.POST.get("school_num",None)
        same_user_name = models.person.objects.filter(name =username)
        if  username and password and email and shcool_num :

            if same_user_name:
                message = '用户名已经存在,请重新注册'
                return render(request,'register.html',{"message":message})

            #  通过验证开始注册
            new_user = models.person()
            new_user.name = username
            new_user.password = password
            new_user.email = email
            new_user.shchool_num = shcool_num
            new_user.is_active = "0"
            new_user.save()
            print("注册成功")
            #
            # # 发送激活链接,包含所需要的连接  http:127.0.0.1:8848/active
            # # 激活连接中要包含着 人的信息
            # serializer =Serializer(settings.SECRET_KEY,3600)
            # info = {'confirm':new_user.name}
            # token = serializer.dumps(info)
            # token = token.decode()
            #
            # # 发邮件
            # subject = '桂电帮欢迎信息'
            # html_message = '%s,欢迎你成为桂电帮的注册会员<h1>请点击一下连接来激活您的账户 http://guidianbang.qicp.vip:22154/active/%s ' % (
            #     username, token)
            # sender = settings.EMAIL_FROM
            # reciver = [new_user.email]
            #
            # send_mail(subject,html_message,sender,reciver)




            return render(request,'login.html')
        else:
            message = "信息填写不完整"
            return render(request, 'register.html', {"message": message})


def active(request,token):
    serializer = Serializer(settings.SECRET_KEY, 3600)

    try:
        info = serializer.loads(token)
        user_name = info['confirm']
        # 开始激活用户
        print("开始激活")
        user =models.person.objects.get(name =user_name)
        user.is_active = "1"
        user.save()
        return render(request, 'login.html')
    except SignatureExpired as e:
        # 激活连接已经过期
        return HttpResponse("激活连接已经过期")



def logout(request):

        if not request.session.get('user_name', None):
            # 如果本来就未登录，也就没有登出一说
            return redirect("/index")
        request.session.flush()
        # 或者使用下面的方法
        # del request.session['is_login']
        # del request.session['user_id']
        # del request.session['user_name']
        return redirect("/index")