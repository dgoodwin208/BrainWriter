import bottle
import os

@bottle.route('/')
def index():
    return "Hello World In a Bottle"

if __name__ == '__main__':
    # if len(sys.argv) != 2:
    #     usage(sys.argv)

    bottle.run(host="0.0.0.0", port=os.environ.get('PORT', 5000))
