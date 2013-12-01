from django.contrib.gis.db import models
from django.contrib.auth.models import User

# Create your models here.
class Event(models.Model):
    INACTIVE = 0
    ACTIVE = 1
    STATUSES = (
            (INACTIVE, 'Inactive'),
            (ACTIVE, 'Active'),
            )
    eid = models.AutoField(primary_key=True)
    creator = models.ForeignKey(User, verbose_name="creator of this event")
    name = models.CharField(max_length=255)
    desc = models.CharField(max_length=255)
    status = models.IntegerField(default=ACTIVE, choices=STATUSES)
    create_time = models.DateTimeField(auto_now_add=True)
    start_time = models.DateTimeField()
    end_time = models.DateTimeField()
    location = models.CharField(max_length=255)
    geo_location = models.PointField()
    objects = models.GeoManager()
    
    def __unicode__(self):
        return self.name + self.desc

class Activity(models.Model):
    JOIN = 0
    NOTJOIN = 1
    CHECKIN = 2
    CREAT = 3
    STATUSES = (
            (CHECKIN, 'Checkin'),
            (JOIN, 'Join'),
            (NOTJOIN, 'Notjoin'),
            (CREAT, 'Creat'),
            )

    class Meta:
        unique_together = ['user', 'event']

    user = models.ForeignKey(User, verbose_name="related user of this activity")
    event = models.ForeignKey(Event, verbose_name="related event of this activity")
    last_activity_time = models.DateTimeField(auto_now=True)
    status = models.IntegerField(default=JOIN, choices=STATUSES)

    def __unicode__(self):
        return self.user.username + self.event.name + self.status
