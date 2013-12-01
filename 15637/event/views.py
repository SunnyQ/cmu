# Create your views here.

from datetime import datetime    
from django.utils import timezone

from django.core import serializers
from django.contrib.auth.decorators import login_required
from django.shortcuts import get_object_or_404, render, redirect
from django.http import HttpResponseRedirect, HttpResponse
from django.core.urlresolvers import reverse
from models import Event, Activity
from forms import CreateEventForm, EditEventForm
from django.core.context_processors import csrf
from django.contrib.gis.geos import fromstr
from django.contrib.gis.measure import D 
from django.utils import simplejson

@login_required
def create_event(request, success_url=None,
        template_name='event/create_event.html'):
    if request.method == 'POST':
        form = CreateEventForm(data=request.POST)
        if form.is_valid():
            new_event = form.save(request.user)
            if success_url is None:
                success_url = reverse('view-event', kwargs={'event_id' : new_event.eid })
            return redirect(success_url)
    else:
        form = CreateEventForm()

    return render(request, template_name, {'form':form,})

@login_required
def view_event(request, event_id,
        template_name='event/view_event.html'):
    cur_event = get_object_or_404(Event, eid=event_id)

    activities = Activity.objects.select_related().filter(event=cur_event,
            status__in=[Activity.JOIN, Activity.CHECKIN, Activity.CREAT],)

    return render(request, template_name, {'event':cur_event, 'activities' : activities,})

@login_required
def edit_event(request, event_id, success_url=None,
        template_name='event/edit_event.html'):

    cur_user = request.user
    event_obj = get_object_or_404(Event, eid=event_id)

    if success_url is None:
        success_url = reverse('view-event', kwargs={'event_id' : event_id})

    if (event_obj.creator != cur_user):
        return render(request, 'event/event_error.html', {'error' : 'You are not the creator of this event!'})

    if request.method == 'POST':
        form = EditEventForm(data=request.POST, files=request.FILES, instance=event_obj)
        if form.is_valid():
            form.save()
            return HttpResponseRedirect(success_url)
    else:
        form = EditEventForm(instance=event_obj)
 
    return render(request, template_name, {'form': form, })

@login_required
def list_all_event(request, template_name='event/list_all_event.html'):
    """
    return all active events.
    """
    all_events = Event.objects.filter(status=Event.ACTIVE)
    return render(request, template_name, {'events':all_events})

@login_required
def list_nearby_event(request, template_name='event/list_nearby_event.html'):
    """
    Given a location and a radius (meter), return the closest number events.
    """
    lat = None
    lon=None 
    radius=5
    number=10
    if request.method == 'POST':
        lat = request.POST.get('lat', 0.0)
        lon = request.POST.get('lon', 0.0)
        radius = request.POST.get('radius', 5000)
        number = request.POST.get('number', 10)
    elif request.method == 'GET':
        lat = request.GET.get('lat', 0.0)
        lon = request.GET.get('lon', 0.0)
        radius = request.GET.get('radius', 5000)
        number = request.GET.get('number', 10)
    
    ref_pnt = fromstr("POINT(%s %s)" % (lon, lat))
    distance_from_point = {'m': radius}
    now_time = datetime.now()
    close_events = Event.objects.filter(geo_location__distance_lte=(ref_pnt, D(**distance_from_point)), end_time__gte=now_time)[:number]
    close_events_status = []

    if request.is_ajax():
        return HttpResponse(simplejson.dumps({
        'success': True,
        'events': serializers.serialize('json', close_events),
        'lat':lat, 
        'lon':lon,
        'uid': request.user.id
        }))

    return render(request, template_name, {'events':close_events, 'lat':lat, 'lon':lon})

@login_required
def list_my_event(request, template_name='event/list_my_event.html'):
    """
    Given a user, return the events created by or join by the user.
    """

    now_time = datetime.now()

    cur_user = request.user

    activities_expired = Activity.objects.select_related().filter(user=cur_user,
            status__in=[Activity.JOIN, Activity.CHECKIN, Activity.CREAT],
            event__end_time__lt=now_time)

    activities_happening = Activity.objects.select_related().filter(user=cur_user,
            status__in=[Activity.JOIN, Activity.CHECKIN, Activity.CREAT],
            event__start_time__lte=now_time,
            event__end_time__gte=now_time)

    activities_future= Activity.objects.select_related().filter(user=cur_user,
            status__in=[Activity.JOIN, Activity.CHECKIN, Activity.CREAT],
            event__start_time__gt=now_time)

    return render(request, template_name, {
        'activities_expired': activities_expired,
        'activities_happening': activities_happening,
        'activities_future': activities_future,
        }
        )

@login_required
def change_status(request, event_id, success_url=None,):
    """
    Given a user and an event, change the status of the related activity.
    """
    cur_user = request.user
    success = True
    error = ''
    status = Activity.NOTJOIN
    try:
        cur_event = Event.objects.get(eid=event_id)
        activity, created = Activity.objects.get_or_create(user=cur_user, event=cur_event)

        status = Activity.JOIN
        if request.method == 'POST':
            status = request.POST.get('status', Activity.JOIN)
        elif request.method == 'GET':
            status = request.GET.get('status', Activity.JOIN)
    
        activity.status = status
        status = activity.status
        activity.save()
    except Event.DoesNotExist:
        success = False
        error = 'Event does not exist.'

    if request.is_ajax():
        return HttpResponse(simplejson.dumps({
        'success': True,
        'error' : error,
        'status' : status
        }))


    if success_url is None:
        success_url = reverse('view-event', kwargs={'event_id': event_id})

    return redirect(success_url)

@login_required
def query_status(request, event_id, template_name='event/query_event_status.html'):
    """
    Given a user and an event, return the status of the related activity.
    """
    cur_user = request.user

    success = True
    error = ''
    joined_number = 0
    checkin_number = 0
    status = Activity.NOTJOIN
    is_creator = False

    try:
        cur_event = Event.objects.get(eid=event_id)
        if cur_event.creator == cur_user:
            is_creator = True

        joined_number = Activity.objects.filter(event__exact=cur_event, status__exact=Activity.JOIN).count()
        checkin_number = Activity.objects.filter(event__exact=cur_event, status__exact=Activity.CHECKIN).count()
        status = Activity.JOIN
        try:
            activity = Activity.objects.get(user=cur_user, event=cur_event)
            status = activity.status
        except Activity.DoesNotExist:
            status = Activity.NOTJOIN
    except Event.DoesNotExist:
        success = False
        error = 'Event does not exist.'

    if request.is_ajax():
        return HttpResponse(simplejson.dumps({
        'success': success,
        'error' : error,
        'is_creator': is_creator, 
        'status': status, 
        'joined_number' : joined_number,
        'checkin_number' : checkin_number,
        }))

    return HttpResponse(simplejson.dumps({
    'success': success,
    'error' : error,
    'is_creator': is_creator, 
    'status': status, 
    'joined_number' : joined_number,
    'checkin_number' : checkin_number,
    }))



    return render(request, template_name, {'status':status,})
