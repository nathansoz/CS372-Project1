import org.apache.commons.cli.*;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

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

                break;
            }
        }
        catch(IOException ex)
        {
            System.out.println("Unable to create socket!");
            return 1;
        }

        return 0;
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
