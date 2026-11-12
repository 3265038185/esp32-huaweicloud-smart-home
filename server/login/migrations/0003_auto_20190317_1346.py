# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('login', '0002_person_email'),
    ]

    operations = [
        migrations.AddField(
            model_name='person',
            name='comment',
            field=models.CharField(max_length=512, default=''),
        ),
        migrations.AddField(
            model_name='person',
            name='img_url',
            field=models.CharField(max_length=512, default=''),
        ),
    ]
