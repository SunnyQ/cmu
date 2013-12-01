# !/usr/bin/env python

ALLDIRS = ['flask_env/lib/python2.6/site-packages']

import sys 
import site 

# Remember original sys.path.
prev_sys_path = list(sys.path) 

# Add each new site-packages directory.
for directory in ALLDIRS:
    site.addsitedir(directory)

# Reorder sys.path so new directories at the front.
new_sys_path = [] 
for item in list(sys.path): 
    if item not in prev_sys_path: 
        new_sys_path.append(item) 
        sys.path.remove(item) 
        sys.path[:0] = new_sys_path 

from flask import Flask, redirect, url_for, request
import sys
import os
import hashlib
import socket
import urllib
from werkzeug import secure_filename

app = Flask(__name__)
	
@app.route('/')
def index():
	return redirect(url_for('static', filename='index.html'))

@app.route('/rd/<int:p>', methods=["GET"])
def rd_getrd(p):
    # 1. Figure out the <object-name> from the request
    # 2. Connect to the routing daemon on port p
    # 3. Do GETRD <object-name> 
    # 4. Parse the response from the routing daemon
    # 4 a) If response is OK <URL>, the open the URL
    # 4 b) If response is 404, then show that content is not available
    # ### You may factor out things from here and rd_getrd() function and form a separate sub-routine
    
    objectname = request.args['object']
    p = request.path[4:]
    s = socket.socket()
    s.connect(('localhost', int(p)))
    length = len(objectname.strip())
    requestContent = 'GETRD ' + str(length) + ' '+ objectname.strip()
    s.send(requestContent)
    data = s.recv(8192)
    print data
    strlist = data.split(' ')
    sign = strlist[0]        
    s.close()
    if sign == "OK":
        print 'OK'
        filehandle = urllib.urlopen(strlist[2])
        return filehandle.read()
    elif sign == "NOTFOUND":
        return 'Content unavailable' 
    return sign;

@app.route('/rd/addfile/<int:p>', methods=["POST"])
def rd_addfile(p):
    # 1. Figure out the object-name and the file details/content from the request
    # 2. Find the sha256sum of the file content
    # 3. Save the file in the static directory under the sha256sum name and compute the relative path
    # 4. Connect to the routing daemon on port p
    # 5. Do ADDFILE <object-name> <relative-path> 
    # 6. Based on the response from the routing daemon display whether the object has been successfully uploaded/added or not 
    
    objectname = request.form['object'] 
    p = request.path[12:]
    file = request.files['uploadFile'] 	
    m = hashlib.sha256()
    m.update(file.read())
    file.seek(0)
    digest = m.hexdigest()
    relativePath = '/static/' + secure_filename(digest) 
    file.save('./static/' + secure_filename(digest))
    s = socket.socket()
    s.connect(('localhost', int(p)))
    length1 = len(objectname)
    length2 = len(relativePath)
    requestContent = 'ADDFILE ' + str(length1) + ' ' +\
    objectname +' ' + str(length2) + ' ' + relativePath
    s.send(requestContent)
    data = s.recv(8192)
    strlist = data.split(' ')
    sign = strlist[0]
    if sign == "OK":
        s.close()
        return 'Add file success'
    elif sign == "ERROR":
        s.close() 
        if int(strlist[1]) > 0:
            return data[ 7 + len(strlist[1]):]
        else:
            return 'Add file failed'

@app.route('/rd/<int:p>/<obj>', methods=["GET"])
def rd_getrdpeer(p, obj):
    # 1. Connect to the routing daemon on port p
    # 2. Do GETRD <object-name> 
    # 3. Parse the response from the routing daemon
    # 3 a) If response is OK <URL>, the open the URL
    # 3 b) If response is 404, then show that content is  not available
    # ### You may factor out things from here and rd_getrd() function and form a separate sub-routine
    
    strlist = data.split('/')
    p = strlist[1]
    objectname = strlist[2]
    s = socket.socket()
    s.connect(('localhost', int(p)))
    length = len(objectname.strip())
    requestContent = 'GETRD ' + str(length) + ' ' + objectname.strip()
    s.send(requestContent)
    data = s.recv(8192)
    strlist = data.split(' ')
    sign = strlist[0]
    if sign == "OK":
        filehandle = urllib.urlopen(strlist[1])
    elif sign == "NOTFOUND":
        return 'Content unavailable'
    s.close()
    
    
if __name__ == '__main__':
    if (len(sys.argv) > 1):
        servport = int(sys.argv[1])
        app.debug = True
        app.run(host='0.0.0.0', port=servport, threaded=True, processes=1)
    else:	
        print "Usage ./webserver <server-port> \n"

