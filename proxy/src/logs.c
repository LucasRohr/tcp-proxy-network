#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> // Para mkdir
#include "../include/logs.h"

FILE* open_log_file(const char* client_ip) {
    char filename[256];
    
    // Garante que o diretório de logs exista
    mkdir("logs", 0755); // Cria o diretório, ignora erro se já existir

    snprintf(filename, sizeof(filename), "logs/log_%s_%ld.csv", client_ip, time(NULL));

    FILE *log_file = fopen(filename, "w"); // Abre arquivo de logs

    if (log_file == NULL) {
        perror("Erro ao abrir arquivo de log");
        return NULL;
    }

    // Escreve o cabeçalho do CSV
    fprintf(log_file, "TimestampMS,");
    fprintf(log_file, "C2P_RTT_ms, C2P_RTTVAR_ms, C2P_Retrans, C2P_CWND, C2P_SSTHRESH, C2P_Goodput_Mbps,");
    fprintf(log_file, "P2S_RTT_ms, P2S_RTTVAR_ms, P2S_Retrans, P2S_CWND, P2S_SSTHRESH, P2S_Goodput_Mbps\n");
    
    fflush(log_file); // Garante que o cabeçalho seja escrito

    return log_file;
}

void log_metrics_csv(FILE *log_file, ConnectionMetrics *metrics_client_proxy, ConnectionMetrics *metrics_proxy_server) {
    if (log_file == NULL) return; // Early return

    fprintf(log_file, "%lu,", metrics_client_proxy->timestamp_ms); // Escreve timestamp

    // Escreve métricas da conexão Cliente <-> Proxy
    fprintf(log_file, "%.3f,%.3f,%d,%d,%d,%.3f,",
            metrics_client_proxy->rtt_ms, metrics_client_proxy->rtt_var_ms, metrics_client_proxy->retransmits,
            metrics_client_proxy->cwnd_segments, metrics_client_proxy->ssthresh, metrics_client_proxy->goodput_mbps);
    
    // Escreve métricas da conexão Proxy <-> Servidor
    fprintf(log_file, "%.3f,%.3f,%d,%d,%d,%.3f\n",
            metrics_proxy_server->rtt_ms, metrics_proxy_server->rtt_var_ms, metrics_proxy_server->retransmits,
            metrics_proxy_server->cwnd_segments, metrics_proxy_server->ssthresh, metrics_proxy_server->goodput_mbps);
            
    fflush(log_file); // Garante que os dados sejam escritos
}

void display_metrics_text(ConnectionMetrics *metrics_client_proxy, ConnectionMetrics *metrics_proxy_server) {
    // Limpa a tela (ANSI escape code) para uma exibição em "tempo real"
    printf("\033[2J\033[H");

    printf("--- Métricas da Conexão (Proxy) | Atualizado em %lu ---\n", metrics_client_proxy->timestamp_ms);
    
    printf("----------------------------------------------------------------\n");
    printf("| Métrica          | Cliente -> Proxy   | Proxy -> Servidor |\n");
    printf("----------------------------------------------------------------\n");
    printf("| RTT (ms)          | %-18.3f | %-17.3f |\n", metrics_client_proxy->rtt_ms, metrics_proxy_server->rtt_ms);
    printf("| RTT Var (ms)      | %-18.3f | %-17.3f |\n", metrics_client_proxy->rtt_var_ms, metrics_proxy_server->rtt_var_ms);
    printf("| Retransmissões    | %-18d | %-17d |\n", metrics_client_proxy->retransmits, metrics_proxy_server->retransmits);
    printf("| CWND (segmentos)  | %-18d | %-17d |\n", metrics_client_proxy->cwnd_segments, metrics_proxy_server->cwnd_segments);
    printf("| Throughput (Mbps) | %-18.3f | %-17.3f |\n", metrics_client_proxy->throughput_mbps, metrics_proxy_server->throughput_mbps);
    printf("| Goodput (Mbps)    | %-18.3f | %-17.3f |\n", metrics_client_proxy->goodput_mbps, metrics_proxy_server->goodput_mbps);
    printf("----------------------------------------------------------------\n");
    printf("Log salvo em: logs/...\n");
}
