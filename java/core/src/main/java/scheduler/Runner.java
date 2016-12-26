package scheduler;

import Ice.Communicator;
import Ice.ObjectAdapter;
import Ice.Properties;
import Ice.Util;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;

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

        ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();

        ScalaSchedulerServer server = new ScalaSchedulerServer(communicator, executor);

        System.out.println("server = " + server);
        adapter.add(server, communicator.stringToIdentity(objectIdentity));
        adapter.activate();

        System.out.println("Waiting for communicator shutdown");
        communicator.waitForShutdown();

        System.out.println("Destroying");

        adapter.deactivate();

        executor.shutdown();

        communicator.destroy();
        System.out.println("Destroyed - exiting");
    }
}
