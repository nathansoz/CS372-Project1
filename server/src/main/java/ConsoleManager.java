import org.jline.reader.LineReader;
import org.jline.reader.LineReaderBuilder;
import org.jline.terminal.Terminal;
import org.jline.terminal.TerminalBuilder;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/*
    Used to manage interleaved messages, etc from the console. Uses jline library
 */
public class ConsoleManager
{
    private Map<String, ConcurrentLinkedQueue<String>> _userInputMap = new ConcurrentHashMap<>();
    private Map<String, ConcurrentLinkedQueue<String>> _userOutputMap = new ConcurrentHashMap<>();
    private ConcurrentLinkedQueue<String> _globalOutput = new ConcurrentLinkedQueue<>();
    private LineReader _reader;

    public ConsoleManager()
    {
        TerminalBuilder builder = TerminalBuilder.builder();

        try
        {
            Terminal _terminal = builder.build();
            _reader = LineReaderBuilder.builder().terminal(_terminal).build();
        }
        catch(IOException ex)
        {
            //Eat
        }
    }

    public void Run() throws InterruptedException
    {
        Thread inputThread = new Thread(() -> {
            try
            {
                ParseUserInput();
            }
            catch(Exception ex)
            {
                //Eat
            }
        });

        inputThread.start();

        while(true)
        {
            _userOutputMap.forEach((k, v) ->
            {
                while(!v.isEmpty())
                {
                    _reader.callWidget(LineReader.CLEAR);
                    _reader.getTerminal().writer().println(String.format("%s> %s", k, v.remove()));
                    _reader.callWidget(LineReader.REDRAW_LINE);
                    _reader.callWidget(LineReader.REDISPLAY);
                    _reader.getTerminal().writer().flush();
                }
            });

            while(!_globalOutput.isEmpty())
            {
                _reader.callWidget(LineReader.CLEAR);
                _reader.getTerminal().writer().println(String.format("System: %s", _globalOutput.remove()));
                _reader.callWidget(LineReader.REDRAW_LINE);
                _reader.callWidget(LineReader.REDISPLAY);
                _reader.getTerminal().writer().flush();
            }

            Thread.sleep(50);
        }
    }

    public void WriteConsole(String toWrite)
    {
        _reader.callWidget(LineReader.CLEAR);
        _reader.getTerminal().writer().println(toWrite);
        _reader.callWidget(LineReader.REDRAW_LINE);
        _reader.callWidget(LineReader.REDISPLAY);
        _reader.getTerminal().writer().flush();
    }

    private void ParseUserInput() throws IOException
    {

        while(true)
        {
            String line = _reader.readLine("System> ");

            _userInputMap.forEach((k, v) -> {
                v.add(line);
            });
        }
    }

    public boolean AddClient(String name, ConcurrentLinkedQueue<String> inputQueue, ConcurrentLinkedQueue<String> outputQueue)
    {
        if(_userInputMap.containsKey(name))
        {
            return false;
        }

        _userInputMap.put(name, inputQueue);
        _userOutputMap.put(name, outputQueue);
        return true;
    }

    public void RemoveClient(String name)
    {
        if(_userInputMap.containsKey(name))
        {
            _userInputMap.remove(name);
        }

        if(_userOutputMap.containsKey(name))
        {
            _userOutputMap.remove(name);
        }
    }

    public void SystemMessage(String message)
    {
        _globalOutput.add(message);
    }
}
