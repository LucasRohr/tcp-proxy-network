#ifndef TCP_MONITOR_H
#define TCP_MONITOR_H

#include "proxy.h"

// Coleta métricas TCP de um socket
int monitor_get_tcp_info(int sock_fd, ConnectionMetrics *metrics);

// Inicializa os valores da estrutura de métricas
void monitor_init_metrics(ConnectionMetrics *metrics);

// Calcula o throughput e goodput com base nos bytes transferidos
void monitor_calculate_throughput(ConnectionMetrics *metrics, unsigned long total_bytes_forwarded);

// Retorna o timestamp atual em milissegundos
unsigned long get_timestamp_ms();

#endif
