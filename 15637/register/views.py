# Create your views here.
from django.contrib import auth
import sys
from django.shortcuts import get_object_or_404, render, redirect
from django.http import HttpResponseRedirect, HttpResponse
from django.core.urlresolvers import reverse
from forms import RegistrationForm
from django.core.context_processors import csrf
from django.contrib.auth.views import login

def custom_login(request, **kwargs):

    """
    redirect logged in user to home
    """
    if request.user.is_authenticated():
        index_url = reverse('index')
        return redirect(index_url)
    else:
        return login(request, **kwargs)

def register(request, success_url=None,
        template_name='registration/registration_form.html'):

    if request.method == 'POST':
        form = RegistrationForm(data=request.POST)

        if form.is_valid():
            new_user = form.save()
            user = auth.authenticate(username=form.cleaned_data['username'],
                                password=form.cleaned_data['password1'])
            user.backend = 'django.contrib.auth.backends.ModelBackend'
            auth.login(request, user)
            if success_url is None:
                success_url = reverse('profiles_edit_profile')

            return redirect(success_url)
    else:
        form = RegistrationForm()

    return render(request, template_name, {'form':form,})
