from django.shortcuts import render
from django.shortcuts import redirect
# Create your views here.

def person(request):

    return render(request,'person.html')