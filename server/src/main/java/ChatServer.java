import org.apache.commons.cli.*;

import java.io.Console;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class ChatServer
{

    int port;

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

        try
        {
            ServerSocket socket = new ServerSocket(port);

            while(true)
            {
                Socket connection = socket.accept();
                ThreadSafeSocketWrapper wrapper = new ThreadSafeSocketWrapper(connection);

                Thread ReaderThread = new Thread(new Runnable() {

                    @Override
                    public void run() {
                        try
                        {
                            Read(wrapper);
                        }
                        catch(IOException ex)
                        {
                            //EAT
                            System.out.println(String.format("Client at %s disconnected.", wrapper.RemoteAddress));
                        }
                    }

                });

                ReaderThread.start();
            }
        }
        catch(IOException ex)
        {
            System.out.println("Unable to create socket!");
            return 1;
        }
    }

    private void Read(ThreadSafeSocketWrapper wrapper) throws IOException
    {
        while(true)
        {
            byte[] data = wrapper.Read();
            String str = new String(data, StandardCharsets.UTF_8);
            System.out.println(str);
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
        options.addOption("h", "help", false, "Display this help.");


        return options;
    }

    public static void PrintHelp()
    {
        HelpFormatter formatter = new HelpFormatter();
        formatter.printHelp("chatserver", BuildOptions());
    }
}
