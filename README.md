# LocalChatApp (WIP)

A multi-client TCP chat application written in C++ that allows multiple clients to connect to a server and exchange messages in real-time.



## Features

- **Multi-client support**: Server can handle multiple simultaneous connections
- **Real-time messaging**: Messages are instantly broadcast to all connected clients
- **Thread-safe**: Uses modern C++ threading and mutex for concurrent operations
- **Memory safe**: Utilizes smart pointers for automatic memory management
- **Cross-platform**: Compatible with Linux/Unix systems

## Project Structure

```
ChatLocal/
├── client/           # Client application
│   └── main.cpp      # Client source code
├── server/           # Server application
│   └── main.cpp      # Server source code
├── lib/              # Shared socket utility library
│   ├── socketUtil.h      # Header file with function declarations
│   └── socketUtilImp.cpp # Implementation of socket utilities
└── README.md         # This file
```

## Requirements

- **Compiler**: g++ with C++17 support or later
- **Operating System**: Linux/Unix (uses POSIX sockets)
- **Dependencies**: 
  - pthread library (for threading)
  - Standard C++ libraries

## Building the Application

### Manual Compilation

1. **Build the socket utility library**:
   ```bash
   cd lib/
   g++ -c socketUtilImp.cpp -o socketUtil.o -std=c++17
   ```

2. **Build the server**:
   ```bash
   cd server/
   g++ main.cpp ../lib/socketUtil.o -o server -std=c++17 -pthread
   ```

3. **Build the client**:
   ```bash
   cd client/
   g++ main.cpp ../lib/socketUtil.o -o client -std=c++17
   ```

### Quick Build Scripts

You can also compile directly without separate object files:

**Server:**
```bash
cd server/
g++ main.cpp ../lib/socketUtilImp.cpp -o server -std=c++17 -pthread
```

**Client:**
```bash
cd client/
g++ main.cpp ../lib/socketUtilImp.cpp -o client -std=c++17
```

## Usage

### Starting the Server

1. Navigate to the server directory:
   ```bash
   cd server/
   ```

2. Run the server:
   ```bash
   ./server
   ```

The server will start listening on `localhost:2000` and display "Server was bound successfully!" when ready.

### Connecting Clients

1. Open a new terminal and navigate to the client directory:
   ```bash
   cd client/
   ```

2. Run a client:
   ```bash
   ./client
   ```

3. If connection is successful, you'll see "Connection good!"

4. Type messages and press Enter to send them to all other connected clients

5. Type `exit` to disconnect from the server

### Example Session

**Terminal 1 (Server):**
```
$ ./server
Server was bound successfully!
hello from client 1
hello from client 2
Client stopped connection!
```

**Terminal 2 (Client 1):**
```
$ ./client
Connection good!
send some messages to host ("exit" to stop program)
hello from client 1
hello from client 2
exit
```

**Terminal 3 (Client 2):**
```
$ ./client
Connection good!
send some messages to host ("exit" to stop program)
hello from client 1
hello from client 2
exit
```

## Architecture

### Socket Utility Library (`lib/`)

The shared library provides:

- **`createTCPIPv4Socket()`**: Creates a TCP socket for IPv4 communication
- **`createIPv4Address(const char* ip, int port)`**: Creates and configures a sockaddr_in structure

### Server (`server/main.cpp`)

- Binds to port 2000 on all available interfaces
- Accepts multiple client connections concurrently
- Creates a separate thread for each client connection
- Broadcasts messages from one client to all other connected clients
- Thread-safe client management using mutex locks

### Client (`client/main.cpp`)

- Connects to server at `127.0.0.1:2000`
- Provides interactive command-line interface
- Sends user input to server
- Automatically detects server disconnection

## Technical Details

### Threading Model

The server uses **C++ std::thread** for concurrent client handling:
- Main thread accepts new connections
- Each client gets a dedicated worker thread
- Thread-safe access to shared client list using `std::mutex`
- Detached threads for automatic cleanup

### Memory Management

The application uses modern C++ memory management:
- **Smart pointers** (`std::unique_ptr`) for automatic cleanup
- **RAII** (Resource Acquisition Is Initialization) principles
- No manual `new`/`delete` operations in main code paths

### Network Protocol

- **Protocol**: TCP (reliable, connection-oriented)
- **Port**: 2000
- **Message Format**: Raw text strings
- **Encoding**: ASCII/UTF-8

## Limitations

- **Platform**: Currently supports Linux/Unix only
- **Security**: No authentication or encryption
- **Scalability**: Limited to 10 concurrent connections (configurable)
- **Protocol**: Simple text-based, no message framing

## Future Enhancements

- [ ] Cross-platform support (Windows)
- [ ] SSL/TLS encryption
- [ ] User authentication
- [ ] Message history/logging
- [ ] Configuration file support
- [ ] Graceful server shutdown
- [ ] Client reconnection handling

## Troubleshooting

### Common Issues

1. **"Address already in use" error**:
   - Wait a few seconds and try again
   - Or use `sudo netstat -tulpn | grep :2000` to check if port is in use

2. **Connection refused**:
   - Make sure the server is running before starting clients
   - Check that firewall allows connections on port 2000

3. **Compilation errors**:
   - Ensure you have g++ with C++17 support
   - Make sure all source files are present
   - Use the `-pthread` flag when compiling the server

### Debug Mode

To enable debug output, add debug prints to the source code or use a debugger like `gdb`.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is for educational purposes. Feel free to use and modify as needed.

## Author

Created as a learning project for socket programming and multi-threading in C++.
