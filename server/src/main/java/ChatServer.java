import org.apache.commons.cli.*;

import java.io.BufferedReader;
import java.io.Console;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ChatServer
{

    int port;
    private ConsoleManager _consoleManager;

    public ChatServer(String[] args)
    {
        try
        {
            ParseCommandLine(args);
        }
        catch(Exception ex)
        {
            System.out.println("Could not parse arguments!");
            System.exit(1);
        }


    }

    public int Run()
    {
        System.out.println("Welcome to chatserver v1.0!");
        System.out.println(String.format("Running on port: %d", port));

        _consoleManager = new ConsoleManager();

        Thread ConsoleThread = new Thread(() ->
        {
            try
            {
                _consoleManager.Run();
            }
            catch(InterruptedException ex)
            {
                //EAT
            }
        });
        ConsoleThread.start();

        try
        {
            ServerSocket socket = new ServerSocket(port);

            while(true)
            {
                Socket connection = socket.accept();
                Thread delegate = new Thread(() ->
                    {
                        try
                        {
                            GetUserHandleAndStartThreads(connection);
                        }
                        catch(IOException ex)
                        {
                            //EAT
                        }
                    }
                );
                delegate.start();
            }
        }
        catch(IOException ex)
        {
            System.out.println("Unable to create socket!");
            return 1;
        }
    }

    private void GetUserHandleAndStartThreads(Socket connection) throws IOException
    {
        ThreadSafeSocketWrapper wrapper = new ThreadSafeSocketWrapper(connection);
        _consoleManager.SystemMessage(String.format("Client at %s connected.", wrapper.RemoteAddress));

        byte[] handle = wrapper.Read();
        String str = new String(handle, StandardCharsets.UTF_8);

        _consoleManager.SystemMessage(String.format("Client at %s provides handle %s", wrapper.RemoteAddress, str));

        ConcurrentLinkedQueue<String> input = new ConcurrentLinkedQueue<>();
        ConcurrentLinkedQueue<String> output = new ConcurrentLinkedQueue<>();

        Thread ReaderThread = new Thread(() -> {

            try
            {
                Read(wrapper, output);
            }
            catch(IOException ex)
            {
                //EAT
                _consoleManager.SystemMessage(String.format("Client at %s disconnected.", wrapper.RemoteAddress));
                _consoleManager.RemoveClient(str);
            }
        });

        Thread WriterThread = new Thread(() -> {
            try
            {
                Write(wrapper, input);
            }
            catch(Exception ex)
            {
                //EAT
            }
        });

        ReaderThread.start();
        WriterThread.start();

        _consoleManager.AddClient(str, input, output);

        input.add(String.format("Server> Hello from the server, %s", str));
    }

    private void Read(ThreadSafeSocketWrapper wrapper, ConcurrentLinkedQueue<String> queue) throws IOException
    {
        while(true)
        {
            byte[] data = wrapper.Read();
            String str = new String(data, StandardCharsets.UTF_8);
            queue.add(str);
        }
    }

    private void Write(ThreadSafeSocketWrapper wrapper, ConcurrentLinkedQueue<String> queue) throws IOException, InterruptedException
    {
        while(true)
        {
            if(queue.isEmpty())
            {
                Thread.sleep(10);
                continue;
            }

            while(!queue.isEmpty())
            {
                wrapper.Write(queue.remove().getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    private void ParseCommandLine(String[] args) throws ParseException, NumberFormatException
    {
        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = parser.parse(BuildOptions(), args);

        if(cmd.hasOption('h') || cmd.hasOption("help"))
        {
            PrintHelp();
            System.exit(0);
        }

        if(cmd.hasOption('p') || cmd.hasOption("port"))
        {
            port = Integer.parseInt(cmd.getOptionValue('p'));
        }
    }

    /*
    All option building happens here!
    */
    private static Options BuildOptions()
    {
        Options options = new Options();

        options.addOption("p", "port", true, "The port that the chat server will run on.");
        options.addOption("m", "multithreading", false, "Use an implementation that allows for send/rcv at any point");
        options.addOption("h", "help", false, "Display this help.");


        return options;
    }

    public static void PrintHelp()
    {
        HelpFormatter formatter = new HelpFormatter();
        formatter.printHelp("chatserver", BuildOptions());
    }
}
