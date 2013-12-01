from django.contrib.auth.models import User
from django import forms
from django.utils.translation import ugettext_lazy as _
from datetime import datetime    
from models import Event, Activity
from django.contrib.gis.geos import GEOSGeometry

class CreateEventForm(forms.Form):
    """
    Form for creating a new event.
    
    Validates that the requested username is not already in use, and
    requires the password to be entered twice to catch typos.
    """
    name = forms.CharField(required=True,
            max_length=255,
            label=_("Event Name"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Event name"})
            )

    desc = forms.CharField(max_length=255,
            label=_("Description"),
            widget=forms.Textarea(attrs={'class': "field span6" , 'rows': "4", 'placeholder': "Enter the description"})
            )


    start_time = forms.DateTimeField(required=True,
            label=_("Start Time"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Start date and Time"})
            )

    end_time = forms.DateTimeField(required=True,
            label=_("End Time"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "End date and Time"})
            )

    location = forms.CharField(required=True,
            max_length=255,
            label=_("Location"),
            widget=forms.TextInput(attrs={'class': 'span6', 'placeholder': "Enter the location"})
            )

    lat = forms.FloatField(required=True,
            max_value=90.0,
            min_value= -90.0,
            widget=forms.TextInput(attrs={'type': 'hidden'})
            )

    lon = forms.FloatField(required=True,
            max_value=180.0,
            min_value= -180.0,
            widget=forms.TextInput(attrs={'type': 'hidden'})
            )

    def clean(self):
        """
        Verifiy that the start_time is no later than the end_time
        """
        cur_time = datetime.now()
        if 'start_time' in self.cleaned_data and 'end_time' in self.cleaned_data :
            if self.cleaned_data['start_time'].replace(tzinfo=None) < cur_time:
                raise forms.ValidationError(_("Start Time could not be earlier than the current time."))

            if self.cleaned_data['start_time'] > self.cleaned_data['end_time']:
                raise forms.ValidationError(_("Start Time could not be later than End Time."))
        return self.cleaned_data
 
    def save(self, user):
        lat = self.cleaned_data['lat']
        lon = self.cleaned_data['lon']
        new_event = Event(creator=user,
                name=self.cleaned_data['name'],
                desc=self.cleaned_data['desc'],
                status=Event.ACTIVE,
                start_time=self.cleaned_data['start_time'],
                end_time=self.cleaned_data['end_time'],
                location=self.cleaned_data['location'],
                geo_location=GEOSGeometry('POINT(%f %f)' % (lon, lat)),
                )

        new_event.save()
        activity, created = Activity.objects.get_or_create(user=user, event=new_event)
        activity.status = Activity.CREAT
        activity.save()
        return new_event;

class EditEventForm(forms.ModelForm):
    """
    Form for && editing an event.
    """
    name = forms.CharField(required=True,
            max_length=255,
            label=_("Event Name"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Event name"})
            )

    desc = forms.CharField(max_length=255,
            label=_("Description"),
            widget=forms.Textarea(attrs={'class': "field span6" , 'rows': "4", 'placeholder': "Enter the description"})
            )


    start_time = forms.DateTimeField(required=True,
            label=_("Start Time"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Start date and Time"})
            )

    end_time = forms.DateTimeField(required=True,
            label=_("End Time"),
            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "End date and Time"})
            )

    def clean(self):
        """
        Verifiy that the start_time is no later than the end_time
        """
        cur_time = datetime.now()
        if 'start_time' in self.cleaned_data and 'end_time' in self.cleaned_data :
            if self.cleaned_data['start_time'].replace(tzinfo=None) < cur_time:
                raise forms.ValidationError(_("Start Time could not be later than the current time."))

            if self.cleaned_data['start_time'] > self.cleaned_data['end_time']:
                raise forms.ValidationError(_("Start Time could not be later than End Time."))
        return self.cleaned_data

    class Meta:
        model = Event 
        exclude = ('creator', 'geo_location', 'location', )
