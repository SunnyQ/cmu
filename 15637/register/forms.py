from django.contrib.auth.models import User
from django import forms
from django.utils.translation import ugettext_lazy as _

class RegistrationForm(forms.Form):
    """
    Form for registering a new user account.
    
    Validates that the requested username is not already in use, and
    requires the password to be entered twice to catch typos.
    """
    username = forms.RegexField(regex=r'^[\w.@+-]+$',
                                max_length=30,
                                widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Username"}),
                                error_messages={'invalid': _("This value may contain only letters, numbers and @/./+/-/_ characters.")})

    #first_name = forms.RegexField(regex=r'^[A-Za-z]+$',
    #                            max_length=30,
    #                            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "First Name"}),
    #                            error_messages={'invalid': _("This value may contain only letters.")})

    #last_name = forms.RegexField(regex=r'^[A-Za-z]+$',
    #                            max_length=30,
    #                            widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Last Name"}),
    #                            error_messages={'invalid': _("This value may contain only letters.")})

    email = forms.EmailField(widget=forms.TextInput(attrs={'maxlength': 75, 'class': 'input-xlarge', 'placeholder': "Email"}))
    password1 = forms.CharField(widget=forms.PasswordInput(render_value=False, attrs={'class': 'input-xlarge', 'placeholder': "Password"}))
    password2 = forms.CharField(widget=forms.PasswordInput(render_value=False, attrs={'class': 'input-xlarge', 'placeholder': "Password again"}))
    
    def clean_username(self):
        """
        Validate that the username is alphanumeric and is not already
        in use.
        
        """
        existing = User.objects.filter(username__iexact=self.cleaned_data['username'])
        if existing.exists():
            raise forms.ValidationError(_("A user with that username already exists."))
        else:
            return self.cleaned_data['username']

    def clean_email(self):
        """
        Validate that the supplied email address is unique for the
        site.
        
        """
        existing = User.objects.filter(email__iexact=self.cleaned_data['email'])
        if existing.exists(): 
            raise forms.ValidationError(_("This email address is already in use. Please supply a different email address."))
        else: 
            return self.cleaned_data['email']

    def clean(self):
        """
        Verifiy that the values entered into the two password fields
        match. Note that an error here will end up in
        ``non_field_errors()`` because it doesn't apply to a single
        field.
        
        """
        if 'password1' in self.cleaned_data and 'password2' in self.cleaned_data:
            if self.cleaned_data['password1'] != self.cleaned_data['password2']:
                raise forms.ValidationError(_("Two passwords didn't match."))
        return self.cleaned_data

    def save(self):
        new_user = User.objects.create_user(self.cleaned_data['username'],
                                        self.cleaned_data['email'],
                                        self.cleaned_data['password1'])

        return new_user
