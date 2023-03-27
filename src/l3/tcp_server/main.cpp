#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

void set_reuse_addr(socket_wrapper::Socket &sock)
{
    const int flag = 1;
    // Allow reuse of port.
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0)
    {
        //throw std::logic_error("Set SO_REUSEADDR error");
        std::cerr << "Set SO_REUSEADDR error" << std::endl;
    }
}

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_STREAM, 0};

    set_reuse_addr(sock);

    std::cout << "Starting TCP echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr =
    {
        .sin_family = PF_INET,
        .sin_port = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    listen(sock, 1);


    char buffer[256];
    char nameBuf[64];

    // socket address used to store client address
    struct sockaddr_in client_address = {0};
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;
    int clientSockHandler = 0;

    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];

    clientSockHandler = accept(sock,reinterpret_cast<sockaddr*>(&client_address),
                                &client_address_len);

    if (getnameinfo(reinterpret_cast<sockaddr *>(&client_address), client_address_len, nameBuf, sizeof(nameBuf) - 1, NULL, 0, 0))
    {
        strcpy(nameBuf, "unknown");
    }

    while (true)
    {
        // Read content into buffer from an incoming client.
        recv_len = recv(clientSockHandler, buffer, sizeof(buffer) - 1, 0);

        if (recv_len > 0)
        {
            if (getnameinfo(reinterpret_cast<sockaddr*>(&client_address), client_address_len, nameBuf, sizeof(nameBuf) - 1, NULL, 0, 0))
            {
                strcpy(nameBuf, "unknown");
            }

            buffer[recv_len] = '\0';
            std::cout
                << "Client '" << nameBuf << "' with address "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << buffer
                << "\n'''"
                << std::endl;

            // Send same content back to the client ("echo").
            send(clientSockHandler, buffer, recv_len, 0);

            if (!memcmp(buffer,"exit",4)) return EXIT_SUCCESS;
        }
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

