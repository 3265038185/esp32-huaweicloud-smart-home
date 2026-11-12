# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations


class Migration(migrations.Migration):

    dependencies = [
        ('login', '0006_user'),
    ]

    operations = [
        migrations.RenameModel(
            old_name='User',
            new_name='person',
        ),
    ]
