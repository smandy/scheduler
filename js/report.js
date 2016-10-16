var grid;
var data = [];
var options = {
    editable: false,
    enableAddRow: false,
    enableCellNavigation: true,
    enableColumnReorder : false
};

console.log("Bidir loading");

var columns = [
    {id : "id", name: "Id", field: "id", width: 50},
    {id : "priority", name: "Priority", field: "priority", width: 50},
    {id : "pwd", name: "pwd", field: "pwd", width: 300},
    {id : "cmdLine", name: "cmdLine", field: "cmdLine", width: 300},
    {id : "state", name: "state", field: "state", width: 180},
    {id : "dependencies", name: "dependencies", field: "dependencies", width: 180},
    {id : "env", name: "env", field: "env", width: 180}
];

grid = new Slick.Grid("#myGrid", data, columns, options);

var iid = new Ice.InitializationData();
var props = Ice.createProperties();

props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:ws -h raffles -p 4063');
props.setProperty( 'GemServer.Proxy' , 'server@GemServer');
iid.properties = props;

var communicator = Ice.initialize( iid );

var proxy = communicator.stringToProxy("server@GemServer").ice_timeout(1000);
console.log("About to ping " + proxy);

var MyCallBackReceiver = Ice.Class( Gem.GemServerListener, {
    onImage_async : function( cb, image, current) {
        console.log("On Image!");
        cb.ice_response();
    },
    onUpdate_async : function( cb, jobs, current) {
        console.log("On Update! " + jobs.map( function(j) { return j.id; } ));
        cb.ice_response();
    }
} );

var server;
Gem.GemServerPrx.checkedCast( proxy ).then( function(prx) {
    server = prx;
    server.getJobs().then( function(jobs) {
        console.log("Woot got " + jobs.length);
        for (i = 0;i<jobs.length;i++) {
            data[i] = jobs[i];
            grid.invalidateRow(i);
        }
        grid.render();
        return communicator.createObjectAdapter("").then(
            function(adapter)
            {
                console.log("Made adapter");
                //
                // Create a callback receiver servant and add it to
                // the object adapter.
                //
                var r = adapter.addWithUUID(new MyCallBackReceiver());
                
                //
                // Set the connection adapter and remember the connection.
                //
                connection = proxy.ice_getCachedConnection();
                connection.setAdapter(adapter);
                //
                // Register the client with the bidir server.
                var ident = r.ice_getIdentity();
                console.log("Going for it " + server + 'ident is ' + ident.name);
                server.addListenerWithIdent(r.ice_getIdentity()).then(
                    function() {
                        console.log('uranu');
                    },
                    function(ex) {
                        console.log('uvavu ' + ex);
                    });
                console.log("Went for it");
            });
    }, function(ex) {
        console.log('failed');
    });
});


