import com.sun.xml.internal.ws.policy.privateutil.PolicyUtils;

import java.io.*;
import java.net.Socket;
import java.util.concurrent.locks.ReentrantLock;

public class ThreadSafeSocketWrapper
{
    private ReentrantLock _socketLock = new ReentrantLock();
    private ReentrantLock _readLock = new ReentrantLock();
    private ReentrantLock _writeLock = new ReentrantLock();

    private Socket _socket;

    public String RemoteAddress;

    public ThreadSafeSocketWrapper(Socket socket) throws IOException
    {
        _socket = socket;
        RemoteAddress = socket.getRemoteSocketAddress().toString();
    }

    public void Write(byte[] data) throws IOException
    {
        if(data.length <= 0)
        {
            throw new IllegalArgumentException("The provided data length is 0");
        }

        if(_socket.isClosed())
        {
            throw new IOException("The socket is closed");
        }

        _writeLock.lock();

        try
        {
            OutputStream out = _socket.getOutputStream();
            DataOutputStream dataOut = new DataOutputStream(out);

            //Size of data for recpt
            dataOut.writeInt(data.length);
            dataOut.write(data);
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    public byte[] Read() throws IOException
    {

        _readLock.lock();

        byte[] ret;

        try
        {
            InputStream in = _socket.getInputStream();
            DataInputStream dataIn = new DataInputStream(in);

            int numBytes = dataIn.readInt();

            ret = new byte[numBytes];
            int pos = 0;

            while(numBytes > 0)
            {
                int read = dataIn.read(ret, pos, numBytes);
                pos += read;
                numBytes -= read;
            }
        }
        finally
        {
            _readLock.unlock();
        }

        return ret;
    }

    public void Close() throws IOException
    {
        _socketLock.lock();
        _readLock.lock();
        _writeLock.lock();

        try
        {
            if(!_socket.isClosed())
            {
                _socket.close();
            }
        }
        finally
        {
            _socketLock.unlock();
            _readLock.unlock();
            _writeLock.unlock();
        }
    }
}
