function clearObject(obj) {
    //for (var x in obj) delete obj[x];
    obj.length = 0;
}

var iid = new Ice.InitializationData();
var props = Ice.createProperties();

var CallbackReceiver = Ice.Class( scheduler.SchedulerServerListener, {
    onImage_async      : function(cb, image, current) { this.parent.onImage_async(cb, image, current); },
    onReset_async      : function(cb, current)        { this.parent.onReset_async( cb, current); },
    onImageReady_async : function(cb, s, current)     { this.parent.onImageReady_async( cb,s,current); },
    onUpdate_async     : function(cb, jobs, current)  { this.parent.onUpdate_async( cb, jobs, current ); }
});

props.setProperty('Ice.Default.Locator', 'IceGrid/Locator:ws -h raffles -p 4063');
iid.properties = props;
var communicator = Ice.initialize(iid);
//var proxy = communicator.stringToProxy("server@SchedulerServer").ice_timeout(1000);
var proxy = communicator.stringToProxy("server@SchedulerJava").ice_timeout(1000);
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
        return ( <img width={this.props.gridVisible ? "1200" : "1600"}
                             height={this.props.gridVisible ? "500" : "600"} src={this.props.image + "?dt=" + new Date().getTime()} />);
    }
});

var StateFormatter = React.createClass({
    render:function() {
        //console.log( "State is " + this.props.value + " p=" + this.props);
        return ( <div>{this.props.value.name}</div>);
    }
});

//Custom Formatter component
var IdFormatter = React.createClass({
    render : function() {
        return ( <div>{this.props.value.id.id}</div> );
    }});

var StateFormatter = React.createClass({
    render : function() {
        return ( <div>{this.props.value.state.name}</div> );
    }});

var IceGridListener = React.createClass( {
    getInitialState : function() {
        console.log("Get initial state")
        return {
            data : [],
            lookup : [],
            image : '',
            gridVisible : true,
            graphVisible : true
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
        scheduler.SchedulerServerPrx.checkedCast( proxy ).then( function(server) {
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
        } , function(ex) {
            console.log("Failed " + ex);
        })
    },
    rowGetter : function(i) {
        return this.state.data[i];
    },

    onGridVisible : function(x) {
        console.log("OnGridVisible : " + x );
        this.state.gridVisible = ! this.state.gridVisible;
        this.setState(this.state);
    },
    onGraphVisible : function(x) {
        console.log("OnGraphVisible  : " + x );
        this.state.graphVisible = ! this.state.graphVisible;
        this.setState(this.state);
    },
    render : function() {
        // var ret = (<div>{this.state.data.length}</div>);
        // console.log( "State is " + this.state.data.map( (x) => { return x.id;} ));
        // var products = this.state.data.map( (row) => { return (<Job id={row.id} key={row.id} job={row} />) } );
        // return <div className='ui items'>
        // {products}
        // </div>;
        var columns = [
            { key : 'job',
                name : 'Job',
                formatter : IdFormatter},
                { key : 'state',
                    name : 'State',
                    formatter : StateFormatter },
    
                    ];
        var grid = this.state.gridVisible ? <ReactDataGrid
        columns={columns}
        rowGetter={this.rowGetter}
        rowsCount={this.state.data.length}
        minHeight={this.state.graphVisible ? 200 : 900}/> : null;
        var graph = this.state.graphVisible ? <RenderedImage gridVisible={this.state.gridVisible} image={this.state.image}/> : null;
        var ret = ( <div>
            <input type="checkbox" checked={this.state.gridVisible} onChange={this.onGridVisible}/>
            <input type="checkbox" checked={this.state.graphVisible} onChange={this.onGraphVisible}/>
            {grid}
            <br/>
            {graph}
            </div>);
        console.log("Ret is " + ret);
        return ret;
    }
});

ReactDOM.render(<IceGridListener/>, document.getElementById('grid'));
