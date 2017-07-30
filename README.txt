You can build the project using ./build.sh on flip.engr.oregonstate.edu

The server is in java, so execute like this after building (using port 8080 in this example):

java -jar server/target/chatserver-1.0-SNAPSHOT.jar 8080

The client is in c++. It is a native program that you can execute like this:

client_build/client localhost 8080



Details:

If the server isn't running, this is what you would see:

nathan@nathan-ub:~/source/chat$ client_build/client localhost 800
Chat client v1.0
User handle: nathan
Handle set to: nathan
Connecting on port 800
Failed to connect to server.


On DNS lookup failure:

nathan@nathan-ub:~/source/chat$ client_build/client localhost.blahblah 800
Chat client v1.0
User handle: nathan
Handle set to: nathan
Connecting on port 800
getaddrinfo failed with: Name or service not known


This is what the server looks like

nathan@nathan-ub:~/source/chat$ java -jar server/target/chatserver-1.0-SNAPSHOT.jar --port 8080
Welcome to chatserver v1.0!
Running on port: 8080
System> hello
System> test
System> 

*Note that up to this point, no client has connected. The server is acting as a chat room and nothing is connected. The following output is what happens when a client connects. 

System: Client at /127.0.0.1:56538 connected.
System: Client at /127.0.0.1:56538 provides handle nathan
nathan> Hello, how are you?
System> I'm well, yourself?
nathan> I'm good!
System> 

*For extra credit I added the ability to connect more than one client


System: Client at /127.0.0.1:56546 connected.
System: Client at /127.0.0.1:56546 provides handle nathan1
nathan1> I'm nathan!
System> No, I already know about a nathan.
System> 


*There is no error handling done around duplicate handles, but when a handle disconnects, it is freed up. The only mode available for the server is a broadcast mode of communication. All users see everything the server writes. The server sees what individual users write. users don't see what each other write. This could be fixed, but I only had so much time and it does satisfy the requirements of back and forth communication

*The server knows about disconnections

System: Client at /127.0.0.1:56546 disconnected.
System> 

*The server can disconnect everyone by calling \quit

System> \quit
System: Client at /127.0.0.1:56546 disconnected.
System> 


*Two ctrl-c commands will exit the server


EXTRA CREDIT

* Threaded client and server
* Server supports more than one client
* Messages can be sent in any order (not just client-server-client-etc)

