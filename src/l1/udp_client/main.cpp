#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string.h>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>


int main(int argc, char const *argv[])
{

    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <IP> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    char SERVER_IP[INET_ADDRSTRLEN];
    strcpy(SERVER_IP, argv[1]);

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[2]) };

    socket_wrapper::Socket sock = {AF_INET, SOCK_DGRAM, IPPROTO_UDP};

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    sockaddr_in addr =
    {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };

    if (!inet_aton(SERVER_IP, &addr.sin_addr))
    {
        std::cerr << "Error in server IP!" << std::endl;
        return EXIT_FAILURE;
    }

    socklen_t socket_addr_len = sizeof(sockaddr_in);

    char buffer[256] = { 0 };
    ssize_t recv_len = 0;

    while (true)
    {
        std::cout << "Enter message: ";
        std::cin >> buffer;

        if (sendto(sock, buffer, strlen(buffer), 0, reinterpret_cast<const sockaddr *>(&addr),
            socket_addr_len) == -1)
        {
            std::cout << std::endl << "Error while sending data!" << std::endl;
            return EXIT_FAILURE;
        }

        if (!memcmp(buffer,"exit",4)) return EXIT_SUCCESS;

        // Read content into buffer from an incoming client.
        recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                            reinterpret_cast<sockaddr *>(&addr),
                            &socket_addr_len);

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            std::cout << "Server response: " << buffer << std::endl;
        } else
        {
            std::cout << "Error server response: Timeout." << std::endl;
        }
    }

    return EXIT_SUCCESS;
}

