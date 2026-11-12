"""school URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/1.8/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  url(r'^$', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  url(r'^$', Home.as_view(), name='home')
Including another URLconf
    1. Add an import:  from blog import urls as blog_urls
    2. Add a URL to urlpatterns:  url(r'^blog/', include(blog_urls))
"""
from django.conf.urls import include, url
from django.contrib import admin
from login import views as login_views
from home import views as home_views
from person import views as person_views
from fourm import views as fourm_views

urlpatterns = [
    url(r'^admin/', include(admin.site.urls)),
    url(r'^index/$', home_views.index),
    #  登录
    url(r'^login/$', login_views.login),
    url(r'login_check/$',login_views.login_check),
    # 注册
    url(r'^register/$', login_views.register),
    url(r'register_check/$',login_views.register_check),
    # 邮箱激活
    url(r'^active/(?P<token>.*)$',login_views.active),
    #  登出
    url(r'^logout/$', login_views.logout),

    # 个人中心
    url(r'^person/$',person_views.person),

    #论坛,分页

    url(r'^single/$',fourm_views.single),

    #url(r'^(?P<pIndex>[0-9]*)/fourm$', fourm_views.page_test),

    url(r'^fourm/$',fourm_views.page_test),



    #url(r'^fourm/post_meaage/$',fourm_views.post_meaage),
    url(r'^fourm/post_meaage/$',fourm_views.post_meaage),
    url(r'^fourm/post_meaage/post_meaage/$', fourm_views.post_meaage),
    url(r'^fourm/post_meaage/post_meaage/post_meaage/$', fourm_views.post_meaage),
    url(r'^fourm/post_meaage/post_meaage/post_meaage/post_meaage/$', fourm_views.post_meaage),
    url(r'^fourm/post_meaage/post_meaage/post_meaage/post_meaage/post_meaage/$', fourm_views.post_meaage),


    # url(r'^',home_views.index),
]
