#ifndef TCP_OPTIMIZER_H
#define TCP_OPTIMIZER_H

/**
 * Aplica TCP Pacing (controle de taxa) a um socket
 * @param socket O socket para aplicar o pacing
 * @param rate_bytes_per_sec Taxa máxima em bytes por segundo
 */
void apply_tcp_pacing(int socket, long rate_bytes_per_sec);

/**
 * Ajusta dinamicamente os buffers de envio e recebimento
 * @param socket O socket para ajustar
 * @param buffer_size O novo tamanho do buffer em bytes
 */
void apply_buffer_tuning(int socket, int send_buffer_size, int recv_buffer_size);

#endif
