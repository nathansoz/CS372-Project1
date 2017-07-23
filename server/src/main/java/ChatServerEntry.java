import org.apache.commons.cli.*;

public class ChatServerEntry
{
    public static void main(String[] args)
    {
        if(args.length == 0)
        {
            ChatServer.PrintHelp();
            System.exit(0);
        }

        ChatServer server = new ChatServer(args);

        int exit = server.Run();

        System.exit(exit);
    }
}