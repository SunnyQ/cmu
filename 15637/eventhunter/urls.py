from django.conf.urls import patterns, include, url
from django.views.generic import TemplateView 
from django.contrib import admin
import settings
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    # url(r'^$', 'eventhunter.views.home', name='home'),
    # url(r'^eventhunter/', include('eventhunter.foo.urls')),

    url(r'^media/(?P<path>.*)$', 'django.views.static.serve', {'document_root': settings.MEDIA_ROOT}),


    url(r'^$', TemplateView.as_view(template_name="index.html"), name='index'),
    url(r'^about/$', TemplateView.as_view(template_name="about.html"), name='about'),

    url(r'^accounts/login/', 'register.views.custom_login'),
    url(r'^accounts/', include('django.contrib.auth.urls')),

    url(r'^accounts/', include('register.urls')),
    url(r'^event/', include('event.urls')),
    
    url(r'^profile/', include('simpleprofile.urls')),

    # Uncomment the admin/doc line below to enable admin documentation:
    # url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

    # Uncomment the next line to enable the admin:
    url(r'^admin/', include(admin.site.urls)),

    #url(r'^$', direct_to_template, 
    #                { 'template': 'index.html' }, 'index'),
)
