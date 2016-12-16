package bamma;

import Ice.Communicator;
import Ice.ObjectAdapter;
import Ice.Properties;
import Ice.Util;

public class Runner {
    public static void main(String[] args) {
        System.out.println("Create communicator");
        Communicator communicator = Util.initialize(args);
        Properties props = communicator.getProperties();

        String adapterName = props.getProperty("adapterName");
        String objectIdentity = props.getProperty("objectIdentity");

        System.out.println("objectIdentity = " + objectIdentity);
        System.out.println("adapterName = " + adapterName);

        ObjectAdapter adapter = communicator.createObjectAdapter(adapterName);
        GemServer server = new GemServer();

        System.out.println("server = " + server);
        adapter.add(server, communicator.stringToIdentity(objectIdentity));
        adapter.activate();

        System.out.println("Waiting for communicator shutdown");
        communicator.waitForShutdown();

        System.out.println("Destroying");

        communicator.destroy();
        System.out.println("Destroyed - exiting");
    }
}
