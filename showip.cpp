#include <iostream>
#include <string>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <print>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <IP address> <port>" << std::endl;
    }

    char* hostname = argv[1];
    char* port = argv[2];

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    struct addrinfo *results;

    int status = getaddrinfo(hostname, port, &hints, &results);
    if (status != 0) {
        std::printf("getaddrinfo error: %s\n", gai_strerror(status));
    }

    char ipstr[INET6_ADDRSTRLEN];
    memset(ipstr, 0, INET6_ADDRSTRLEN);

    // now info is there
    for (auto p = results; p != NULL; p = p->ai_next) {
        void *addr;
        std::string ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        auto str = std::string(ipstr, strlen(ipstr));
        std::println("{}: {}", ipver, str);
    }

    freeaddrinfo(results);

    return 0;
}