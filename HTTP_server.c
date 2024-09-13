#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // Open the file
    FILE *html_data = fopen("index.html", "r");
    if (html_data == NULL) {
        perror("Unable to open file");
        return 1;
    }

    char buffer[128];
    char response_data[2048] = {'\0'};
    while (fgets(buffer, sizeof(buffer), html_data)) {
        strcat(response_data, buffer);
    }
    fclose(html_data);

    // Construct the HTTP response
    char http_header[4096];
    snprintf(http_header, sizeof(http_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %lu\r\n"
             "Connection: close\r\n\r\n",
             strlen(response_data));
    strcat(http_header, response_data);

    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Define the address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9001);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    int client_socket;
    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        send(client_socket, http_header, strlen(http_header), 0);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
