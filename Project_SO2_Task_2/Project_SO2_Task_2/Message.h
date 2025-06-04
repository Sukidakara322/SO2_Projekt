#pragma once
#include <winsock2.h>  // Windows socket definitions
#include <string>

// Represents a single chat message sent by a client
struct Message {
    SOCKET sender;     // The socket of the client who sent the message
    std::string text;  // The actual text content of the message
};
