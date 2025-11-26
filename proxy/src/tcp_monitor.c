#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
// Inclui cabeçalhos diferentes dependendo do SO
#ifdef __linux__
    #include <netinet/tcp.h> // Linux: tem TCP_INFO e struct tcp_info
#else
    // macOS/Outros: Não tem TCP_INFO padrão do Linux
    // Definir um dummy ou ignorar
#endif
#include <unistd.h>
#include <sys/time.h>
#include "../include/tcp_monitor.h"

unsigned long get_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (unsigned long)(tv.tv_sec * 1000) + (unsigned long)(tv.tv_usec / 1000);
}

void monitor_init_metrics(ConnectionMetrics *metrics) {
    memset(metrics, 0, sizeof(ConnectionMetrics));
    metrics->timestamp_ms = get_timestamp_ms(); // Atribui novo timestamp
    metrics->last_timestamp_ms = metrics->timestamp_ms; // Registra timestamp antigo
}

int monitor_get_tcp_info(int sock_fd, ConnectionMetrics *metrics) {
    #ifdef __linux__
        // === CÓDIGO REAL (LINUX) ===
        struct tcp_info info;
        socklen_t info_len = sizeof(info);

        // Chamada ao kernel usando TCP_INFO para obter dados
        if (getsockopt(sock_fd, IPPROTO_TCP, TCP_INFO, &info, &info_len) < 0) {
            perror("Erro ao obter TCP_INFO");
            return -1;
        }

        // Kernel do Linux retorna os valores em microssegundos, necessitando conversão
        // Registra informações retornadas
        metrics->rtt_ms = (double)info.tcpi_rtt / 1000.0;
        metrics->rtt_var_ms = (double)info.tcpi_rttvar / 1000.0;
        metrics->retransmits = info.tcpi_retrans;
        metrics->cwnd_segments = info.tcpi_snd_cwnd;
        metrics->ssthresh = info.tcpi_snd_ssthresh;

        return 0;
    #else
        // === CÓDIGO "MOCK" (MACOS / DEV LOCAL) ===
        // Retorna valores falsos apenas para permitir a compilação e teste da lógica do proxy no meu MacOS
        
        // Valores fictícios
        metrics->rtt_ms = 15.0;
        metrics->rtt_var_ms = 2.0;
        metrics->retransmits = 0;
        metrics->cwnd_segments = 10;
        metrics->ssthresh = 65535;
        
        printf("[Aviso] Monitoramento simulado (não-Linux detectado)\n");

        return 0;
    #endif
}

void monitor_calculate_throughput(ConnectionMetrics *metrics, unsigned long total_bytes_forwarded) {
    metrics->timestamp_ms = get_timestamp_ms(); // Registra timestamp
    
    unsigned long interval_ms = metrics->timestamp_ms - metrics->last_timestamp_ms;
    if (interval_ms == 0) return; // Evita divisão por zero

    metrics->bytes_transferred_total = total_bytes_forwarded;
    unsigned long bytes_this_interval = metrics->bytes_transferred_total - metrics->last_bytes_total;

    // Calcula Kbps: (bytes * 8 bits/byte) / (intervalo_em_ms / 1000 s/ms) / 1.000.000 bits/Mbit
    double interval_sec = (double)interval_ms / 1000.0;
    double bits_per_sec = (double)(bytes_this_interval * 8) / interval_sec;
    double kbps = bits_per_sec / 1000.0;

    // No proxy, os bytes encaminhados são o "goodput" (payload)
    metrics->throughput_kbps = kbps;
    metrics->goodput_kbps = kbps;

    // Atualiza valores para o próximo cálculo
    metrics->last_bytes_total = metrics->bytes_transferred_total;
    metrics->last_timestamp_ms = metrics->timestamp_ms;
}
