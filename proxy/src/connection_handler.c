#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "../include/connection_handler.h"

// Função auxiliar para encaminhar dados de um socket para outro (cliente <-> servidor)
// Retorna o número de bytes lidos, 0 caso desconexão, -1 caso erro
ssize_t forward_data(int src_fd, int dest_fd, char* buffer, size_t buffer_size) {
    ssize_t bytes_read = recv(src_fd, buffer, buffer_size, 0); // Recupera dados do buffer

    if (bytes_read > 0) {
        // Encaminha os dados lidos
        send(dest_fd, buffer, bytes_read, 0);
    }

    return bytes_read;
}

// Essa é a função que será executada pela thread
void* handle_connection(void* args) {
    ConnectionThreadArgs *thread_args = (ConnectionThreadArgs*)args;
    
    int client_socket = thread_args->client_socket;
    ProxyConfig *config = thread_args->config;
    
    char client_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(thread_args->client_address.sin_addr), client_ip_str, INET_ADDRSTRLEN); // Checagem do endereço
    
    printf("[+] Nova conexão de %s:%d\n", client_ip_str, ntohs(thread_args->client_address.sin_port));

    // 1. Conectar ao servidor real usando novo socket depois de conectar com o cliente
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Erro ao criar socket para o servidor");
        close(client_socket);
        free(thread_args);
        return NULL;
    }

    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(config->target_port); // Porta do servidor

    // Checagem do endereço
    if (inet_pton(AF_INET, config->target_host, &server_address.sin_addr) <= 0) {
        perror("Endereço do servidor real inválido");
        close(client_socket);
        close(server_socket);
        free(thread_args);
        return NULL;
    }

    // Se conecta ao servidor usando seu socket
    if (connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Erro ao conectar ao servidor real");
        close(client_socket);
        close(server_socket);
        free(thread_args);
        return NULL;
    }

    printf("[+] Conexão (Cliente %d <-> Servidor %d) estabelecida.\n", client_socket, server_socket);

    // Libera os argumentos da thread, já temos os dados que precisamos
    free(thread_args);

    // 2. Configurar o poll() para monitorar os dois sockets agora que temos a conexão no meio do cliente e servidor
    struct pollfd poll_fd[2]; // 0 = cliente, 1 = servidor
    poll_fd[0].fd = client_socket;
    poll_fd[0].events = POLLIN; // Monitorar por dados de entrada (leitura)
    poll_fd[1].fd = server_socket;
    poll_fd[1].events = POLLIN; // Monitorar por dados de entrada (leitura)

    char buffer[4096]; // Buffer de 4KB para o tráfego

    // 3. Loop de encaminhamento de dados
    while (1) {
        // Espera indefinidamente (-1) até que um dos sockets tenha dados
        int poll_count = poll(poll_fd, 2, -1);

        if (poll_count < 0) {
            perror("Erro no poll");
            break; // Encerra o loop e a conexão
        }

        // Verifica se o cliente enviou dados e encaminha para o servidor caso sim
        if (poll_fd[0].revents & POLLIN) {
            if (forward_data(client_socket, server_socket, buffer, sizeof(buffer)) <= 0) {
                break; // Cliente desconectou ou erro
            }
        }

        // Verifica se o servidor enviou dados e encaminha para o cliente caso sim
        if (poll_fd[1].revents & POLLIN) {
            if (forward_data(server_socket, client_socket, buffer, sizeof(buffer)) <= 0) {
                break; // Servidor desconectou ou erro
            }
        }
    }

    // 4. Limpeza
    printf("[-] Conexão (Cliente %d <-> Servidor %d) encerrada.\n", client_socket, server_socket);
    close(client_socket);
    close(server_socket);

    return NULL;
}
