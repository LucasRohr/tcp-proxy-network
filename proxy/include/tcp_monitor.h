#ifndef TCP_MONITOR_H
#define TCP_MONITOR_H

#include "proxy.h"

/**
 * Coleta métricas TCP de um socket usando getsockopt(TCP_INFO)
 * @param socket O socket para monitorar
 * @param metrics A estrutura para preencher com os dados
 * @return 0 caso sucesso, -1 caso erro
 */
int get_tcp_metrics(int socket, ConnectionMetrics *metrics);

#endif
