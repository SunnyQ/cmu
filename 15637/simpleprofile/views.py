# Create your views here.

from django.core.exceptions import ObjectDoesNotExist
from django.core.urlresolvers import reverse
from django.http import Http404
from django.http import HttpResponseRedirect
from django.shortcuts import get_object_or_404, render, redirect
from django.template import RequestContext

from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User

from models import UserProfile 
from forms import ProfileForm

@login_required
def edit_profile(request, success_url=None,
                 template_name='profiles/edit_profile.html'):
    profile_obj = None
    cur_user = request.user
    try:
        profile_obj = cur_user.get_profile()
    except ObjectDoesNotExist:
        profile_obj = UserProfile.objects.create(user=cur_user)
    
    if success_url is None:
        success_url = reverse('profiles_view_profile',
                              kwargs={ 'uid': request.user.id})

    if request.method == 'POST':
        form = ProfileForm(data=request.POST, files=request.FILES, instance=profile_obj)
        if form.is_valid():
            form.save()
            return HttpResponseRedirect(success_url)
    else:
        form = ProfileForm(instance=profile_obj)
   
    return render(request, template_name, { 'form': form, })

@login_required
def profile_detail(request, uid, 
                   template_name='profiles/view_profile.html'):
    
    profile_obj = None
    cur_user = request.user

    try:
        to_view_user = User.objects.get(pk=uid)
    except User.DoesNotExist:
        return render(request, 'profiles/error.html', {'error' : 'No such user!'})

    try:
        profile_obj = to_view_user.get_profile()
    except ObjectDoesNotExist:
        profile_obj = UserProfile.objects.create(user=to_view_user)

    if profile_obj.allow_other_view == False and profile_obj.user != cur_user:
        return render(request, 'profiles/error.html', {'error' : 'You are not allowed to view this guy\'s profile!'})

    return render(request, template_name, { 'profile': profile_obj, })
