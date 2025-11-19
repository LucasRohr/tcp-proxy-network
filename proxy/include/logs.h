#ifndef LOGS_H
#define LOGS_H

#include "proxy.h"

/**
 * Abre um arquivo de log CSV e escreve o cabeçalho
 * @return O file descriptor do arquivo, ou -1 em erro
 */
FILE* open_log_file(const char* client_ip);

// Escreve uma linha de métricas nos arquivos de log CSV
void log_metrics_csv(FILE *log_file, ConnectionMetrics *metrics_client_proxy, ConnectionMetrics *metrics_proxy_server);

// Exibe as métricas atuais no console (interface de texto)
void display_metrics_text(ConnectionMetrics *metrics_client_proxy, ConnectionMetrics *metrics_proxy_server);

#endif
