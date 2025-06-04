#include <winsock2.h> // Windows Sockets API
#include <iostream>
#include <string>

// Declaration of the chat server function
void run_chat_server(unsigned short port);

int main(int argc, char* argv[])
{
    // Check if the user provided exactly one argument (port number)
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE; // Exit if incorrect usage
    }

    // Convert the input argument to an unsigned short (port number)
    const unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));

    // Initialize the Winsock library
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return EXIT_FAILURE;  // Exit if Winsock initialization failed
    }

    // Start the chat server on the specified port
    run_chat_server(port);

    // Cleanup Winsock resources before exiting
    WSACleanup();
    return 0;  // Indicate successful execution
}
