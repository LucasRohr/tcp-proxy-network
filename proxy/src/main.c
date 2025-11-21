#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../include/proxy.h"
#include "../include/connection_handler.h"

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Uso: %s <porta_local> <host_servidor_real> <porta_servidor_real> [--optimize]\n", argv[0]);
        fprintf(stderr, "Exemplo sem otimização: %s 8080 192.168.1.100 9090\n", argv[0]);
        fprintf(stderr, "Exemplo com otimização: %s 8080 192.168.1.100 9090 --optimize\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 1. Armazena a configuração do proxy passada por args
    ProxyConfig config;
    config.listen_port = atoi(argv[1]);
    config.target_host = argv[2];
    config.target_port = atoi(argv[3]);

    // Padrão: Otimização desligada
    config.enable_optimization = 0;

    // Verifica se o 5º argumento existe e se é a flag de otimização
    if (argc == 5) {
        if (strcmp(argv[4], "--optimize") == 0 || strcmp(argv[4], "-o") == 0) {
            config.enable_optimization = 1;
        } else {
            fprintf(stderr, "Aviso: Argumento '%s' desconhecido. Use '--optimize' ou '-o' para ativar otimizações.\n", argv[4]);
        }
    }

    // 2. Cria o socket listener do proxy
    int listen_fd;
    struct sockaddr_in proxy_address;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0); // STREAM para conexão TCP

    if (listen_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Permite que o socket seja reutilizado imediatamente (bom para testes)
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. Configura o endereço do proxy
    memset(&proxy_address, 0, sizeof(proxy_address));
    proxy_address.sin_family = AF_INET;
    proxy_address.sin_addr.s_addr = INADDR_ANY; // Escuta em todos os IPs locais
    proxy_address.sin_port = htons(config.listen_port); // Porta

    // 4. Faz o Bind do socket para o proxy
    if (bind(listen_fd, (struct sockaddr *)&proxy_address, sizeof(proxy_address)) < 0) {
        perror("Erro no bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // 5. Coloca o socket em modo Listen
    if (listen(listen_fd, 10) < 0) { // 10 é o backlog limite de conexões pendentes
        perror("Erro no listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Proxy TCP escutando na porta %d, encaminhando para %s:%d\n\n",
           config.listen_port, config.target_host, config.target_port);

    // Feedback visual
    printf("----------------------------------------------------------------\n");
    printf("Proxy TCP Iniciado\n");
    printf("Escutando em: 192.168.0.145:%d\n", config.listen_port);
    printf("Destino:      %s:%d\n", config.target_host, config.target_port);
    printf("Otimização:   [%s]\n", config.enable_optimization ? "\033[1;32mATIVADA\033[0m" : "\033[1;33mDESATIVADA\033[0m");
    printf("----------------------------------------------------------------\n");

    // 6. Loop principal: aceita e despacha conexões
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        // Aguarda um cliente conectar
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_address, &client_len);
        if (client_fd < 0) {
            perror("Erro no accept");
            continue;
        }

        // Prepara os argumentos para a nova thread se accept feito com sucesso
        ConnectionThreadArgs *connection_args = malloc(sizeof(ConnectionThreadArgs));

        if (!connection_args) {
            perror("Erro ao alocar argumentos da thread");
            close(client_fd);
            continue;
        }

        connection_args->client_socket = client_fd;
        connection_args->config = &config; // Passa um ponteiro para a config do proxy
        connection_args->client_address = client_address;

        // Cria a thread para gerenciar a conexão
        pthread_t thread;

        if (pthread_create(&thread, NULL, handle_connection, (void*)connection_args) != 0) {
            perror("Erro ao criar thread");
            free(connection_args);
            close(client_fd);
        }

        // Desvincula a thread pra que ela libere recursos quando terminar
        pthread_detach(thread);
    }

    close(listen_fd);

    return 0;
}
