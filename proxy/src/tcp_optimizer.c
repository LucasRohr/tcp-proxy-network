#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "../include/tcp_optimizer.h"

void apply_tcp_pacing(int sock_fd, long rate_bytes_per_sec) {
    // TCP Pacing é uma funcionalidade específica do Linux, por isso a condição para testar no Mac
    #ifdef SO_MAX_PACING_RATE
        if (setsockopt(sock_fd, SOL_SOCKET, SO_MAX_PACING_RATE, &rate_bytes_per_sec, sizeof(rate_bytes_per_sec)) < 0) {
            perror("[Optimizer] Erro ao aplicar TCP Pacing");
        } else {
            // Descomente para debugar se necessário
            // printf("[Optimizer] Pacing aplicado: %ld bytes/s\n", rate_bytes_per_sec);
        }
    #else
        // No macOS ou sistemas sem essa opção, não aplica nada
        // Isso permite que o código compile e rode, mas sem o efeito do pacing
    #endif
}

void apply_buffer_tuning(int sock_fd, int send_buffer_size, int recv_buffer_size) {
    // Ajuste de buffer funciona em Linux e macOS
    
    // 1. Ajusta buffer de envio (Send Buffer)
    if (send_buffer_size > 0) {
        if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, sizeof(send_buffer_size)) < 0) {
            perror("[Optimizer] Erro ao ajustar SO_SNDBUF");
        }
    }

    // 2. Ajusta buffer de recebimento (Receive Buffer)
    if (recv_buffer_size > 0) {
        if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &recv_buffer_size, sizeof(recv_buffer_size)) < 0) {
            perror("[Optimizer] Erro ao ajustar SO_RCVBUF");
        }
    }
}
