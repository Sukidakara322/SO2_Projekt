#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // link Winsock automatically

#include "spinlock.h" // custom synchronisation primitive
#include "message.h" // struct Message

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// using CRLF for each line to start at col 0
static const char* EOL = "\r\n";

static SpinLock clients_lock;   // protects `clients` & `nicks`
static SpinLock queue_lock;     // protects `msg_queue`

static std::vector<SOCKET> clients;   // list of currently connected sockets
static std::unordered_map<SOCKET, std::string> nicks; // socket to nickname map

static std::vector<Message> msg_queue;  // outgoing broadcasts

static std::atomic_bool running{ true };    // global stop flag
static SOCKET listen_sock = INVALID_SOCKET; // accept() socket

static SOCKET create_listen_socket(unsigned short port);
static void   client_thread(SOCKET client);
static void   dispatcher();


// console_handler  —  intercept Ctrl-C / console close and trigger shutdown
static BOOL WINAPI console_handler(DWORD sig)
{
    if (sig == CTRL_C_EVENT || sig == CTRL_CLOSE_EVENT) {
        // Signal all threads to finish
        running = false;

        // Close listening socket so accept() unblocks
        if (listen_sock != INVALID_SOCKET)
            closesocket(listen_sock);

        // Close client sockets
        clients_lock.lock();
        for (SOCKET s : clients) closesocket(s);
        clients_lock.unlock();
        return TRUE;
    }
    return FALSE;
}

void run_chat_server(unsigned short port)
{
    // Handle Ctrl-C to allow clean shutdown
    SetConsoleCtrlHandler(console_handler, TRUE);

    listen_sock = create_listen_socket(port);
    std::cout << "Chat-server listening on port " << port << '\n';

    // Start a background thread that distributes messages to all clients
    std::thread(dispatcher).detach();

    // Main loop: accept incoming connections
    while (running) {
        SOCKET client = accept(listen_sock, nullptr, nullptr);
        if (!running) break;  // exit gracefully on shutdown signal

        if (client == INVALID_SOCKET) {
            if (running) std::cerr << "accept() failed\n";
            continue;
        }

        // Spawn a new thread to handle communication with this client
        std::thread(client_thread, client).detach();
    }

    // Give threads a short moment to wrap up before shutting down
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Background thread that forwards messages from the queue to all connected clients
static void dispatcher()
{
    // Keep running as long as the server is active or there are still messages to process
    while (running || !msg_queue.empty()) {
        queue_lock.lock();

        // If the queue is empty, unlock and wait before checking again
        if (msg_queue.empty()) {
            queue_lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Take the first message from the queue
        Message msg = std::move(msg_queue.front());
        msg_queue.erase(msg_queue.begin());
        queue_lock.unlock();

        // Send the message to all clients except the original sender
        clients_lock.lock();
        for (SOCKET fd : clients) {
            if (fd == msg.sender) continue; // Don't echo back to sender
            send(fd, msg.text.c_str(),
                static_cast<int>(msg.text.size()), 0);
        }
        clients_lock.unlock();
    }
}

// Utility function that removes all newline characters from a string
static inline void strip_newlines(std::string& s)
{
    s.erase(
        std::remove_if(s.begin(), s.end(),
            [](char c) { return c == '\r' || c == '\n'; }),
        s.end()  // Erase them from the string
    );
}

// Function that handles communication with a single connected client
static void client_thread(SOCKET client)
{
    constexpr size_t BUF = 1024; // Buffer size for incoming messages
    char buf[BUF];

    // Ask the client to enter a nickname
    const char* ask = "Enter nickname: ";
    send(client, ask, static_cast<int>(strlen(ask)), 0);

    // Receive the nickname from the client (n - number of bytes received)
    int n = recv(client, buf, static_cast<int>(BUF - 1), 0);
    if (n <= 0) {
        closesocket(client);
        return;
    }
    buf[n] = '\0'; // Null-terminate the received string

    // Clean the nickname and use default if empty
    std::string nick(buf);
    strip_newlines(nick);
    if (nick.empty()) nick = "Inkognito";

    // Add the client to client list and nickname map
    clients_lock.lock();
    clients.push_back(client);
    nicks[client] = nick;
    clients_lock.unlock();

    // Notify everyone that new client has joined
    {
        std::string join = "*** " + nick + " joined ***" + EOL;
        queue_lock.lock();
        msg_queue.push_back({ client, std::move(join) });
        queue_lock.unlock();
    }

    // Main loop: receive and forward messages
    for (;;) {
        // Receive data from client over the socket
        n = recv(client, buf, static_cast<int>(BUF - 1), 0);
        if (n <= 0 || !running) break;
        buf[n] = '\0';

        std::string text(buf);
        strip_newlines(text);
        if (text.empty()) continue;

        // If client types "/quit", disconnect
        if (text.rfind("/quit", 0) == 0) break;

        // Format and queue the message
        std::string payload = nick + ": " + text + EOL;
        queue_lock.lock();
        msg_queue.push_back({ client, std::move(payload) });
        queue_lock.unlock();
    }

    // Client is disconnecting, so we remove him from global state
    clients_lock.lock();
    clients.erase(std::remove(clients.begin(), clients.end(), client),
        clients.end());
    nicks.erase(client);
    clients_lock.unlock();

    // Notify everyone that the client has left
    std::string left = "*** " + nick + " left ***" + EOL;
    queue_lock.lock();
    msg_queue.push_back({ client, std::move(left) });
    queue_lock.unlock();

    // Close the client socket
    closesocket(client);
}

// Creates and sets up a TCP socket that listens on the specified port
static SOCKET create_listen_socket(unsigned short port)
{
    // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        std::exit(EXIT_FAILURE); // Exit if socket creation failed
    }

    // Enable the SO_REUSEADDR option to allow quick restart of the server
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<const char*>(&yes), sizeof yes);

    // Prepare the socket address structure
    sockaddr_in addr{};
    addr.sin_family = AF_INET;              // Use IPv4
    addr.sin_addr.s_addr = INADDR_ANY;      // Accept connections from any network
    addr.sin_port = htons(port);            // Set the port (convert to network byte order)

    // Bind the socket to the given port and address
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof addr) == SOCKET_ERROR) {
        std::cerr << "bind() failed\n";
        std::exit(EXIT_FAILURE); // Exit if binding failed
    }

    // Start listening for incoming connection requests
    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed\n";
        std::exit(EXIT_FAILURE); // Exit if listening failed
    }

    // Return the listening socket
    return s;
}
