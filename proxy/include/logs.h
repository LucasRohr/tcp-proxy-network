#ifndef LOGS_H
#define LOGS_H

#include "proxy.h"

/**
 * Abre um arquivo de log CSV e escreve o cabeçalho
 * @return O file descriptor do arquivo, ou -1 em erro
 */
int open_log_file(struct sockaddr_in *client_address);

// Escreve uma linha de métricas no arquivo de log CSV
void log_metrics_csv(int log_fd, ConnectionMetrics *metrics);

// Exibe as métricas atuais no console (interface de texto)
void display_metrics_text(ConnectionMetrics *metrics);

#endif
