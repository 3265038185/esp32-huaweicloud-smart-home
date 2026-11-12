from django.contrib import admin
from . import models
# Register your models here.

from .models import person


@admin.register(person)
class PersonAdmin(admin.ModelAdmin):
    list_per_page = 100
    list_display = ('name','comment','time')

    search_fields = ('name', 'comment', 'time')  # 搜索字段

    list_filter = ['time']