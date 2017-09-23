# Starting up #

## Slice2java ##

Still needs to be done manually

`slice2js ../slice/scheduler.ice`

## Processes to Start ##


### Startup Grid ###

User Tiger instructions for starting grid.

Startup Schedulerserver.

`andy@raffles:~/repos/tiger$ icegridnode --Ice.Config=config/icegrid/raffles.config`

### Startup Webserver ###

Vanilla HTTP Server is okay.

Web server serves up static content then it goes directly to the C++
websocket.

No Node required! React on the client side.

`andy@raffles:/mnt/hdd/andy/repos/scheduler/js$ python -m SimpleHTTPServer`

### Startup Weblistener ###

This guys responsibility is to listen to icestorm and render pictures
when the graph is updated. He leans on the server at the moment. An
optimization would be to have him maintain state of his own (not that
big a deal, just a case of maintaining a dictionary and updating it by
key as new values come in).

`andy@raffles:/mnt/hdd/andy/repos/scheduler/python$ python schedulerListener.py`

### Startup Worker ###

The man doing the work.

`andy@raffles:/mnt/hdd/andy/repos/scheduler/python$ python worker.py --Ice.Config=../config/worker.config`

## Further TOODs ##

Add an icegrid node that would add to the server - the workers and the
listeners.

## Slice2js ##

 1997  cd generated/
 1998  slice2js ../../slice/scheduler.ice 
