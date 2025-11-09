#ifndef PROXY_H
#define PROXY_H

#include <netinet/in.h>

// Configuração do proxy
typedef struct {
    int listen_port;       // Porta onde o proxy escuta
    char *target_host;     // IP do servidor
    int target_port;       // Porta do servidor
} ProxyConfig;

// Estrutura para registrar as métricas de uma conexão
typedef struct {
    unsigned long timestamp_ms;
    double rtt_ms;             // RTT estimado
    double throughput_mbps;    // Throughput (bytes/tempo)
    double goodput_mbps;       // Goodput (dados úteis/tempo)
    int retransmits;           // Contagem de retransmissões
    int cwnd;                  // Janela de congestionamento (se disponível)
} ConnectionMetrics;

// Estrutura para gerenciar um par de conexões (Cliente e Servidor)
typedef struct {
    int client_socket;                      // Socket do cliente
    int server_socket;                      // Socket do servidor
    struct sockaddr_in client_address;      // Endereço do cliente
    ConnectionMetrics metrics;              // Métricas associadas
    int log_fd;                             // File descriptor para o log CSV
} ConnectionPair;

#endif
