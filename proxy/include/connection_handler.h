#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "proxy.h"

typedef struct {
    int client_socket;                  // Socket do cliente que acabou de conectar
    ProxyConfig *config;                // Ponteiro para a configuração do proxy
    struct sockaddr_in client_address;  // Endereço do cliente (para logs)
} ConnectionThreadArgs;

// Função principal da thread.
// Gerencia o ciclo de vida de um par de conexões (cliente e servidor) e encaminha os dados
void* handle_connection(void* args);

#endif
