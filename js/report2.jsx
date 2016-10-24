function clearObject(obj) {
    //for (var x in obj) delete obj[x];
    obj.length = 0;
}

var iid = new Ice.InitializationData();
var props = Ice.createProperties();

var CallbackReceiver = Ice.Class( Gem.GemServerListener, {
    onImage_async      : function(cb, image, current) { this.parent.onImage_async(cb, image, current); },
    onReset_async      : function(cb , current) { this.parent.onReset_async( cb, current); },
    onImageReady_async : function(cb, s, current) { this.parent.onImageReady_async( cb,s,current); },
    onUpdate_async     : function(cb, jobs, current) { this.parent.onUpdate_async( cb, jobs, current ); }
} );

props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:ws -h raffles -p 4063');
iid.properties = props;
var communicator = Ice.initialize( iid );
var proxy = communicator.stringToProxy("server@GemServer").ice_timeout(1000);
console.log("About to ping " + proxy );

var Job = React.createClass( {
    render : function() {
        return (<div className='middle aligned content'>
        {this.props.id} {this.props.value}
        </div>);
    }
});

var RenderedImage = React.createClass( {
    render : function() {
        return ( <img src={this.props.image + "?dt=" + new Date().getTime()} />);
    }
});


var StateFormatter = React.createClass({
    render:function() {
        console.log( "State is " + this.props.value + " p=" + this.props);
        return ( <div>{this.props.value.name}</div>);
    }
    });

var IceGridListener = React.createClass( {
    getInitialState : function() {
        console.log("Get initial state")
        return {
            data : [],
            lookup : [],
            image : ''
        };
    },
    getParent : function() {
        console.log("Get parent");
        return this;
    },
    onImage_async : function(cb, image, current) {
        console.log("On Image! " + image.jobs.length + " " + image.currentImage);
        clearObject(this.state.data);
        clearObject(this.state.lookup);
        for (var i = 0;i<image.jobs.length;i++) {
            this.state.data[i] = image.jobs[i];
            this.state.lookup[image.jobs[i].id] = i;
        }
        console.log("Now have " + this.state.data.length);
        this.state.image = image.currentImage;
        this.setState(this.state);
        cb.ice_response();
    },
    onUpdate_async : function(cb, jobs, current) {
        console.log("On Update! " + jobs.map( function(j) { return j.id; } ));
        for (var i = 0;i<jobs.length;++i) {
            var job = jobs[i];
            var id = job.id;
            var idx;
            if (id in this.state.lookup) {
                idx = this.state.lookup[id];
                console.log("Found idx " + id + " " + idx);
            } else {
                idx = this.state.lookup.length;
                console.log("New idx " + idx);
                this.state.lookup[id] = idx;
            };
            this.state.data[idx] = job;
        };
        console.log("Setting state");
        this.setState(this.state);
        console.log("On Update! exiting");
        cb.ice_response();
    },
    onImageReady_async : function(cb, s, current) {
        console.log("On image ready " + s);
        this.setImage(s);
        cb.ice_response();
    },

    setImage : function(s) {
        this.state.image = s;
        this.setState(this.state);
    },
    onReset_async : function(cb , current) {
        clearObject(this.state.data);
        clearObject(this.state.lookup);
        this.setState(this.state);
        cb.ice_response();
    },
    componentDidMount : function() {
        console.log("Did mount");
        var outer = this;
        Gem.GemServerPrx.checkedCast( proxy ).then( function(server) {
            console.log("Checked cast : " + server);
            return communicator.createObjectAdapter("").then(
                function(adapter)
                {
                    console.log("Made adapter");
                    var receiver = new CallbackReceiver();
                    receiver.parent = outer;
                    console.log("Made adapter " + this);
                    var r = adapter.addWithUUID(receiver);
                    console.log("Made r");
                    var connection = proxy.ice_getCachedConnection();
                    connection.setAdapter(adapter);
                    var ident = r.ice_getIdentity();
                    console.log("Going for it " + server + ' ident is ' + ident.name);
                    server.addListenerWithIdent(r.ice_getIdentity()).then(
                        function() {
                            console.log('uranu');
                        },
                        function(ex) {
                            console.log('uvavu ' + ex);
                        });
                    console.log("Went for it");
                });
        } )
    },
    rowGetter : function(i) {
        return this.state.data[i];
    },
    render : function() {
        // var ret = (<div>{this.state.data.length}</div>);
        // console.log( "State is " + this.state.data.map( (x) => { return x.id;} ));
        // var products = this.state.data.map( (row) => { return (<Job id={row.id} key={row.id} job={row} />) } );
        // return <div className='ui items'>
        // {products}
        // </div>;
        var columns = [
            { key : 'id',
                name : 'ID' },
                        { key : 'pwd',
                            name : 'PWD' },
                { key : 'state',
                    name : 'state',
                    formatter : StateFormatter} ];
        //console.log("ReactDataGrid is " + ReactDataGrid);
        var ret = ( <div><ReactDataGrid
            columns={columns}
            rowGetter={this.rowGetter}
            rowsCount={this.state.data.length}
            minHeight={300}/>
            <RenderedImage image={this.state.image}/>
        </div>);
        
        console.log("Ret is " + ret);
        return ret;
        
    }
});

ReactDOM.render(<IceGridListener/>, document.getElementById('grid'));
