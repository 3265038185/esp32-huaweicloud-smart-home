# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('login', '0003_auto_20190317_1346'),
    ]

    operations = [
        migrations.AddField(
            model_name='person',
            name='time',
            field=models.CharField(max_length=512, default=''),
        ),
    ]
