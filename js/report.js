(function () {
    var data = [];
    var lookup = [];
    var options = {
        editable: false,
        enableAddRow: true,
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

    var grid = new Slick.Grid("#myGrid", data, columns, options);
    var iid = new Ice.InitializationData();
    var props = Ice.createProperties();
    
    props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:ws -h raffles -p 4063');
    iid.properties = props;
    
    var communicator = Ice.initialize( iid );
    
    function clearObject(obj) {
        for (var x in obj) delete obj[x];
    }
    var MyCallBackReceiver = Ice.Class( Gem.GemServerListener, {
        onImage_async : function(cb, image, current) {
            console.log("On Image! " + image.jobs.length);
            clearObject(data);
            clearObject(lookup);
            for (i = 0;i<image.jobs.length;i++) {
                data[i] = image.jobs[i];
                lookup[image.jobs[i].id] = i;
                grid.invalidateRow(i);
            }
            grid.render();
            cb.ice_response();
        },
        onUpdate_async : function(cb, jobs, current) {
            console.log("On Update! " + jobs.map( function(j) { return j.id; } ));
            for (i = 0;i<jobs.length;++i) {
                var job = jobs[i];
                var id = job.id;
                var idx;
                if (id in lookup) {
                    idx = lookup[id];
                    console.log("FOund idx " + id + " " + idx);
                } else {
                    idx = lookup.length;
                    console.log("New idx " + idx);
                    lookup[id] = idx;
                };
                data[idx] = job;
                grid.invalidateRow(idx);
            };
            grid.render();
            cb.ice_response();
        },
        onReset_async : function(cb , current) {
            grid.clear();
            clearObject(data);
            clearObject(lookup);
            grid.render();
            cb.ice_response();
        }
    } );

    var proxy = communicator.stringToProxy("server@GemServer").ice_timeout(1000);
    console.log("About to ping " + proxy);
    Gem.GemServerPrx.checkedCast( proxy ).then( function(server) {
        return communicator.createObjectAdapter("").then(
            function(adapter)
            {
                console.log("Made adapter");
                var r = adapter.addWithUUID(new MyCallBackReceiver());
                connection = proxy.ice_getCachedConnection();
                connection.setAdapter(adapter);
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
    } );
})();
