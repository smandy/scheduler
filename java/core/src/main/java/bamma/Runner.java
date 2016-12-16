package bamma;

import Ice.Communicator;
import Ice.ObjectAdapter;
import Ice.Properties;
import Ice.Util;

public class Runner {
    public static void main(String[] args) {
        Communicator communicator = Util.initialize(args);

        Properties props = communicator.getProperties();
        String adapterName = props.getProperty("adapterName");
        String objectIdentity = props.getProperty("objectIdentity");
        ObjectAdapter adapter = communicator.createObjectAdapter(adapterName);
        GemServer server = new GemServer();
        adapter.add(server, communicator.stringToIdentity(objectIdentity));
    }
}
