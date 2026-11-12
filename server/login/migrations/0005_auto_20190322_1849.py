# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('login', '0004_person_time'),
    ]

    operations = [
        migrations.DeleteModel(
            name='person',
        ),
        migrations.DeleteModel(
            name='User',
        ),
    ]
