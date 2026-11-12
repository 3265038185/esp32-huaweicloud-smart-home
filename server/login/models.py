import mongoengine
from django.db import models


# Create your models here.



class person(models.Model):
    _id = models.AutoField
    name = models.CharField(max_length=128,unique=True,null=True)
    password = models.CharField(max_length=128,null=True)
    email = models.EmailField(unique=False,default="",null=True)
    shchool_num = models.CharField(max_length=128,null=True)
    is_active =models.CharField(max_length=128,null=True)
    #评论
    comment = models.CharField(max_length=512,default="",null=True)
    # 头像地址
    img_url = models.CharField(max_length=512,default="",null=True)
    # 日期
    time = models.CharField(max_length=512,default="",null=True)

    def __str__(self):
        return self.name

    class Meta:

        verbose_name = "用户"
        verbose_name_plural = "用户"