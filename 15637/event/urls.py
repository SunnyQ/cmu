from django.conf.urls import patterns, url
from django.utils import timezone

urlpatterns = patterns('',
    url(r'^create/$', 'event.views.create_event', name='create-event'),
    url(r'^edit/(?P<event_id>\d+)/$', 'event.views.edit_event', name='edit-event'),
    url(r'^view/(?P<event_id>\d+)/$', 'event.views.view_event', name='view-event'),


    url(r'^$', 'event.views.list_nearby_event', name='list-nearby-event'),
    url(r'^list/all/$', 'event.views.list_all_event', name='list-all-event'),
    url(r'^list/nearby/$', 'event.views.list_nearby_event', name='list-nearby-event'),
    url(r'^list/my/$', 'event.views.list_my_event', name='list-my-event'),

    url(r'^status/change/(?P<event_id>\d+)/$', 'event.views.change_status', name='change-event-status'),
    url(r'^status/query/(?P<event_id>\d+)/$', 'event.views.query_status', name='query-event-status'),
)
