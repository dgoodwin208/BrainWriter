import re
from pymongo import MongoClient
import os

def connect_to_db():
    # add the handlers to the logger
    regex = re.compile(r'^mongodb\:\/\/(?P<username>[_\w]+):(?P<password>[\w]+)@(?P<host>[\.\w]+):(?P<port>\d+)/(?P<database>[_\w]+)$')

    mongolab_url = os.environ['MONGOLAB_URI']

    #get our match
    match = regex.search(mongolab_url)
    data = match.groupdict()
    #print data
    # now connect
    connection = MongoClient(os.environ['MONGOLAB_URI'], int(data['port']))
    db = connection[data['username']]
    db.authenticate(data['username'], data['password'])
    return db, connection

