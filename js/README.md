# Starting up #


## Processes to Start ##

### Startup Grid ###

User Tiger instructions for starting grid.

Startup Gemserver.

`andy@raffles:~/repos/tiger$ icegridnode --Ice.Config=config/icegrid/raffles.config`


### STartup Webserver ###

Vanilla HTTP Server is okay.

Web server serves up static content then it goes directly to the C++ websocket.

No Node required! React on the client side.

`andy@raffles:/mnt/hdd/andy/repos/gem/js$ python -m SimpleHTTPServer`

### Startup Weblistener ###

This guys responsibility is to listen to icestorm and render pictures
when the graph is updated. He leans on the server at the moment. An
optimization would be to have him maintain state of his own ( not that
big a deal, just a case of maintaining a dictionary and updating it by
key as new values come in).

`andy@raffles:/mnt/hdd/andy/repos/gem/python$ python gemListener.py`

### Startup Worker ###

The man doing the work.

`andy@raffles:/mnt/hdd/andy/repos/gem/python$ python gemListener.py`

## Further TOODs ##

Add an icegrid node that would add to the server - the workers and the
listeners.
