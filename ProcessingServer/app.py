
from flask import Flask, send_file, url_for, Response
from pymongo import MongoClient
#import gridfs
#from flask import render_template, request, redirect

# setup mongo
#this currently runs locally, will change to the mongolab instance when we're ready
MONGODB_HOST = 'localhost'
MONGODB_PORT = 27017

# connect to the database & get a gridfs handle
client = MongoClient(MONGODB_HOST, MONGODB_PORT)
db = client.bwtest

def start():
    """start the flask service"""

    # create app
    app = Flask(__name__)
    app.debug = True


    #Data comes into this method:
    #'sessionStartTime' is the number of seconds since 1970 when the experiment started
    #'data' is the latest raw data, served up as a csv (each line contains up to 8 channels)
    @app.route('/data', methods=['POST'])
    def saveRatingsToGene():
        print "starting"
        print request.form

        data = request.form['data']
        sessionStartTime = request.form['sessionStartTime']

        #Do we already have a DB entry for this?
        experiment_obj = db.experiments.find_one({"sessionStartTime": sessionStartTime})

        #The data is coming in as a text CSV.
        rows = data.split('\n')
        for row in rows:
            channels = row.split(',')
            #somehow put this data into a mongo object


        return "OK"

    #This server must expose URLs to allow the user to dload the csv from the server
    #Flask gives us what we need to dload files. This sample code might be overly complicated
    #As it is built for streaming large files, which we don't necessarily need
    #http://flask.pocoo.org/docs/patterns/streaming/
    @app.route('/large.csv')
    def generate_large_csv():
        def generate():
            for row in iter_all_rows():
                yield ','.join(row) + '\n'
        return Response(generate(), mimetype='text/csv')

    # This is where we should be able to display a list of all experiments in the mongodb
    # And use them to fill in a template (.tpl file) to allow either dloaded files
    # or perhaps, even visualized on the screen
    # Very excited about this plotting library (if it's easy to implement):
    # http://code.shutterstock.com/rickshaw/
    @app.route('/', methods=['GET'])
    def hello_world():

        experiments = db.experiments.find() #get all

        for experiment in experiments:
            print "Yep we see an experiment"

        print "yep working"


    # let's go!
    app.run()


def main():
    start()


if __name__ == "__main__":
    main()


#Sample reference code from a different project to show how files can be delivered.
#In this case, images:
# def serve_pil_image(pil_img):
#     """
#     see:
#         https://groups.google.com/forum/?fromgroups=#!topic/python-tornado/B19D6ll_uZE
#         http://stackoverflow.com/questions/7877282/how-to-send-image-generated-by-pil-to-browser
#     """
#     img_io = cStringIO.StringIO()
#     pil_img.save(img_io, 'JPEG', quality=70)
#     img_io.seek(0)
#     return send_file(img_io, mimetype='image/jpeg')

 #    The other cool thing to note about this function as reference code is that it uses the <>
    #notation to serve a parameter in as part of the URL
 #    @app.route('/image/<lims1_id>')
 #    def get_image(lims1_id):
 #        """retrieve an image from mongodb gridfs"""
 #        specific_slice = db.images.find_one({"lims1_id": str(lims1_id), "img_fsid": {"$exists": True}})
 #
 #        #NOTE: this is a hack, but there is a dif between passing in strings and ints to mongodb
 #        if not specific_slice:
 #            specific_slice = db.images.find_one({"lims1_id": int(lims1_id), "img_fsid": {"$exists": True}})
 #
 #        img_fsid = specific_slice["img_fsid"]
 #
 #        im_stream = grid_fs.get(ObjectId(img_fsid))
 #        im = Image.open(im_stream)
 #        return serve_pil_image(im)