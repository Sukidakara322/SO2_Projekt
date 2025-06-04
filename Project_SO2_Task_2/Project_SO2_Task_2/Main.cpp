/* chat_server.cpp  -----------------------------------------------------------
   Multithreaded TCP chat-server for Windows
   * One thread per client
   * Custom spin-lock for critical sections
   * No project-property tweaks required
   * Build & run: F5   –  test with multiple  PowerShell  windows:
       PS> nc 127.0.0.1 9000        (or 'ncat' / 'telnet')
--------------------------------------------------------------------------- */

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")      // links Winsock automatically

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

/* ??????????? tiny spin-lock (project requires self-made sync) ??????????? */
class SpinLock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() noexcept { while (flag.test_and_set(std::memory_order_acquire)) {/*busy*/ } }
    void unlock() noexcept { flag.clear(std::memory_order_release); }
};

/* ??????????? shared state guarded by SpinLocks ???????????????????????????? */
struct Message { SOCKET sender; std::string text; };

SpinLock               clients_lock;
SpinLock               queue_lock;
std::vector<SOCKET>    clients;     // connected sockets
std::vector<Message>   msg_queue;   // pending broadcasts

/* ??????????? dispatcher: broadcasts messages to everyone but sender ??????? */
void dispatcher()
{
    for (;;) {
        /* pop one message (critical section) */
        queue_lock.lock();
        if (msg_queue.empty()) {
            queue_lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        Message msg = std::move(msg_queue.front());
        msg_queue.erase(msg_queue.begin());
        queue_lock.unlock();

        /* broadcast -------------------------------------------------------- */
        clients_lock.lock();
        for (SOCKET fd : clients) {
            if (fd == msg.sender) continue;
            send(fd, msg.text.c_str(), static_cast<int>(msg.text.size()), 0);
        }
        clients_lock.unlock();
    }
}

/* ??????????? one thread per client ???????????????????????????????????????? */
void client_thread(SOCKET client)
{
    constexpr size_t BUF = 1024;
    char             buf[BUF];

    for (;;) {
        int n = recv(client, buf, static_cast<int>(BUF - 1), 0);
        if (n <= 0) break;                  // disconnect / error
        buf[n] = '\0';

        queue_lock.lock();
        msg_queue.push_back({ client, std::string(buf) });
        queue_lock.unlock();
    }

    /* remove from client list & clean up */
    clients_lock.lock();
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    clients_lock.unlock();

    closesocket(client);
}

/* ??????????? helper: create listening socket ?????????????????????????????? */
SOCKET create_listen_socket(unsigned short port)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) { std::cerr << "socket() failed\n"; std::exit(EXIT_FAILURE); }

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof yes);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof addr) == SOCKET_ERROR) {
        std::cerr << "bind() failed\n"; std::exit(EXIT_FAILURE);
    }
    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed\n"; std::exit(EXIT_FAILURE);
    }
    return s;
}

/* ??????????? main / accept loop ??????????????????????????????????????????? */
int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }
    const unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));

    /* Winsock start-up ----------------------------------------------------- */
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n"; return EXIT_FAILURE;
    }

    SOCKET server = create_listen_socket(port);
    std::cout << "Chat-server listening on port " << port << '\n';

    std::thread(dispatcher).detach();       // background broadcasting thread

    for (;;) {                              // accept loop
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) { std::cerr << "accept() failed\n"; continue; }

        clients_lock.lock();
        clients.push_back(client);
        clients_lock.unlock();

        std::thread(client_thread, client).detach();
    }
}
