# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('login', '0005_auto_20190322_1849'),
    ]

    operations = [
        migrations.CreateModel(
            name='User',
            fields=[
                ('id', models.AutoField(verbose_name='ID', primary_key=True, serialize=False, auto_created=True)),
                ('name', models.CharField(max_length=128, unique=True, null=True)),
                ('password', models.CharField(max_length=128, null=True)),
                ('email', models.EmailField(max_length=254, null=True, default='')),
                ('shchool_num', models.CharField(max_length=128, null=True)),
                ('is_active', models.CharField(max_length=128, null=True)),
                ('comment', models.CharField(max_length=512, null=True, default='')),
                ('img_url', models.CharField(max_length=512, null=True, default='')),
                ('time', models.CharField(max_length=512, null=True, default='')),
            ],
            options={
                'verbose_name': '用户',
                'verbose_name_plural': '用户',
            },
        ),
    ]
