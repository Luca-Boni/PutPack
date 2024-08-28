# PutPack

## Description

Putpack is a Dropbox-like application, that allows synchronization between the files of a user through the internet.
The server stores the user's files and coordinates the file reading/writing. The client, on the other hand, keeps the folder sync_dir_\<username\> updated with the server, while also sending chages made to the files.
This project was built according to the [specifiations](https://drive.google.com/file/d/1mGSftnRPeBUqdq149x4QQhogdRv7P0Po/view?usp=sharing) of the Operating Systems II course.

## How to compile

If you're running a Linux system, or Windows Subsystem for Linux, you can just run the scipt `compileAll.sh`, and the binaries for the server and the client should be created in the `bin` directory.

## How to run

To run the server, you can simply run the executable `PutPackServer`, or specify a port on which the server will run: `./PutPackServer <port>`.

For the client, the executable `PutPackClient` takes three arguments in the following order: username, server's address, and server's port, e.g. `./PutPackClient myUsername 192.0.2.0 5000`.

## Details

### The Server

The server listens to connections in the ConnectionManager thread, which then redirects the first message received to the ServerDaemon, the main thread of the program. If it is the user's first client to connect, it will create both the UserManager and ClientManager threads. The former is responsible for managing requests from all clients of a user, sending the response, and assuring some level of synchronization (beware that it does not fully support more than one client writing the same file at the same time, as it was not required), while the latter reads messages from the client and redirects the messages to the UserManager. If the user was already connected in another device, the ServerDaemon simply creates the ClientManager and informs the UserManager.

When the request to write to a file is received by the server, a FileWriter thread is spawned to write the file until it is told to stop (message size zero), and the same message is propagated to all other clients of that user. When a client connects, a FileReader is spawned for each file in the user's directory, which read the files and send their data to the caller (UserManager).

To stop the server, type `exit` in the terminal. The server will wait, however, for all the clients to disconnect first. To stop the server immeadiately, press `Ctrl+C` in the terminal.

### The Client

The client uses the inotify library to monitor changes in the files (FileMonitor), and when it perceives a change, sends the file (read by a FileReader) to the server (via the ClientDaemon).

When it receives a file change from the server (via the ServerReceiver thread), it will spawn a FileWriter. The ClientMenu accepts the following commands:

Command                  | Description
-------------------------|-------------------------------------------------------------------------------
`help`                   | Lists the available commands
`upload <path/filename>` | Sends the file to the server, and propagates to all clients (including itself)
`download <filename>`    | Copies the file from the server, to the directory the terminal is currently on
`delete <filename>`      | Deletes the file in the server and all clients
`list_server`            | Lists the files in the server
`list_client`            | Lists the files in the client
`get_sync_dir`           | Forces a synchronization with the server
`exit`                   | Ends the connection with the server and stops the program