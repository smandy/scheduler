var Ice      = require('ice').Ice;
var Glacier2 = require('ice').Glacier2;
var IceStorm = require('ice').IceStorm;
var IceGrid  = require('ice').IceGrid;
var Gem      = require('./generated/gem').Gem;
var iid = new Ice.InitializationData();
var props = Ice.createProperties();

//props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:tcp -h raffles -p 4061:ws -h raffles -p 4063');

props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:tcp -h raffles -p 4061:ws -h raffles -p 4063');
props.setProperty( 'GemServer.Proxy' , 'server@GemServer');
iid.properties = props;

var communicaator = Ice.initialize( iid );
console.log("P is " + communicator.getProperties().getProperty("GemServer.Proxy"));

var proxy = communicator.stringToProxy("server@GemServer").ice_timeout(1000);

var server;
Gem.GemServerPrx.checkedCast( proxy ).then( function(prx) {
    server = prx;
});

server.getJobs().then( function(jobs) {
    console.log( jobs.map( function(job) {
        return job.id;
    }));
}, function(ex) {
    console.log('failed');
});

console.log("Proxy is " + proxy);
console.log("About to ping");

proxy.ice_ping().then(
    function() {
        console.log("Success!!");
        //communicator.destroy();
    },
    function(ex) { console.log("Failure" + ex); } );
console.log('Woot');
