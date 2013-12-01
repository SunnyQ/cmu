from django.conf.urls import patterns, url
from django.views.generic import TemplateView 
from django.utils import timezone

urlpatterns = patterns('',
    url(r'^edit/$', 'simpleprofile.views.edit_profile', name='profiles_edit_profile'),
    url(r'^view/(?P<uid>\d+)/$', 'simpleprofile.views.profile_detail', name='profiles_view_profile'),
)
