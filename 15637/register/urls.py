from django.conf.urls import patterns, url
from django.utils import timezone
from register import views

urlpatterns = patterns('',
    url(r'^register/$', 'register.views.register', name='register'),
)
