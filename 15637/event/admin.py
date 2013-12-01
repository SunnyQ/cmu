from django.contrib import admin

from models import Event, Activity

class EventAdmin(admin.ModelAdmin):
    list_display        = ('creator', 'desc', 'status', 'start_time', 'end_time', 'location')
    search_fields       = ('creator', )
admin.site.register(Event, EventAdmin)

class ActivityAdmin(admin.ModelAdmin):
    list_display        = ('user', 'event', 'status', )
    search_fields       = ('user', )
admin.site.register(Activity, ActivityAdmin)
