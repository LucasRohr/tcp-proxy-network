#ifndef PROXY_H
#define PROXY_H

#include <netinet/in.h>
#include <stdio.h>

// Configuração do proxy
typedef struct {
    int listen_port;       // Porta onde o proxy escuta
    char *target_host;     // IP do servidor
    int target_port;       // Porta do servidor
} ProxyConfig;

// Estrutura para registrar as métricas de uma conexão
typedef struct {
    unsigned long timestamp_ms; // Momento da medição

    // Métricas diretas do TCP_INFO
    double rtt_ms;             // RTT estimado
    double rtt_var_ms;         // Variação do RTT
    int retransmits;           // Contagem de retransmissões
    int cwnd_segments;         // Janela de congestionamento (se disponível)
    int ssthresh;              // Limiar de ssthresh

    // Métricas calculadas
    double throughput_mbps;    // Throughput (bytes/tempo)
    double goodput_mbps;       // Goodput (dados úteis/tempo)

    // Campos auxiliares de cálculo
    unsigned long bytes_transferred_total;
    unsigned long last_bytes_total;
    unsigned long last_timestamp_ms;
} ConnectionMetrics;

// Estrutura para gerenciar um par de conexões (Cliente e Servidor)
typedef struct {
    int client_socket;                          // Socket do cliente
    int server_socket;                          // Socket do servidor
    struct sockaddr_in client_address;          // Endereço do cliente

    ConnectionMetrics metrics_client_proxy;     // Métricas da conexão Cliente <-> Proxy
    ConnectionMetrics metrics_proxy_server;     // Métricas da conexão Proxy <-> Servidor

    FILE *log_file;                             // Arquivo de log CSV para a conexão
} ConnectionPair;

#endif
