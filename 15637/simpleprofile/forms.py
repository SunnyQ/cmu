from django.contrib.auth.models import User
from django import forms
from django.utils.translation import ugettext_lazy as _
from datetime import datetime    
from models import UserProfile
from django.core.files.uploadedfile import InMemoryUploadedFile
import StringIO
from PIL import Image

def is_jpeg(file):
    try:
        img = Image.open(file)
        #return img.format == 'JPEG'
        return True
    except IOError:
        return False

def make_thumb_nail(file):
    try:
        img = Image.open(file)
        img.thumbnail((128, 128), Image.ANTIALIAS)
        thumbnailString = StringIO.StringIO()
        img.save(thumbnailString, 'JPEG')
        newFile = InMemoryUploadedFile(thumbnailString, None, 'temp.jpg', 'image/jpeg', thumbnailString.len, None)
        return newFile
    except IOError:
        return None


class ProfileForm(forms.ModelForm):
    """
    Form for creating && editing a user's profile.
    """
    first_name = forms.CharField(max_length=30,
                                widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "First Name"}))
 
    last_name = forms.CharField(max_length=30,
                                widget=forms.TextInput(attrs={'class': 'input-xlarge', 'placeholder': "Last Name"}))
    
    self_desc = forms.CharField(max_length=255,
                                widget=forms.Textarea(attrs={'class': "field span6" , 'rows': "4", 'placeholder': "Enter the description"}))
    
    home_addr = forms.CharField(max_length=255,
                                widget=forms.TextInput(attrs={'class': 'span6', 'placeholder': "Enter the home address"}))

    image = forms.ImageField(required=False)

    def clean_image(self):
        file = self.cleaned_data['image']
        if file:
            try:
                img = Image.open(file)
                img.thumbnail((128, 128), Image.ANTIALIAS)
                thumbnailString = StringIO.StringIO()
                img.save(thumbnailString, 'JPEG')
                newFile = InMemoryUploadedFile(thumbnailString, None, 'temp.jpg', 'image/jpeg', thumbnailString.len, None)
                return newFile
            except IOError:
                raise forms.ValidationError(_("Only JPEG file is supported."))
        else:
            return file 

    class Meta:
        model = UserProfile
        exclude = ('user', 'avatar')

    def save(self, force_insert=False, force_update=False, commit=True):
        m = super(ProfileForm, self).save(commit=False)
        if self.cleaned_data['image']:
            #suf = make_thumb_nail(self.cleaned_data['image'])
            #if suf:
            #    m.avatar.save('%s_thumbnail.%s'%(m.user.username, 'jpeg'), suf, save=False)
            m.avatar.save('%s_thumbnail.%s'%(m.user.username, 'jpeg'), self.cleaned_data['image'], save=False)

        if commit:
            m.save()

        return m

