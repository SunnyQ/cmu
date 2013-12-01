from django.db import models
from django.contrib.auth.models import User
from django.db.models.signals import post_save
from storage import OverwriteStorage

def profile_upload_to(instance, filename):
    return 'images/%s/%s' % (instance.user.username, filename)

# Create your models here.
class UserProfile(models.Model):
    user = models.OneToOneField(User)
    avatar = models.ImageField(upload_to=profile_upload_to,storage=OverwriteStorage())
    first_name = models.CharField(max_length=63, blank=True)
    last_name = models.CharField(max_length=63, blank=True)
    home_addr = models.CharField(max_length=255, blank=True)
    self_desc = models.CharField(max_length=255, blank=True)
    allow_other_view = models.BooleanField(default=True)

