import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def plot_metrics(csv_file):
    if not os.path.exists(csv_file):
        print(f"Erro: Arquivo {csv_file} não encontrado.")
        return

    # Lê o CSV
    try:
        df = pd.read_csv(csv_file)
    except Exception as e:
        print(f"Erro ao ler CSV: {e}")
        return

    # Limpa espaços em branco nos nomes das colunas
    df.columns = df.columns.str.strip()
    
    # Ajusta o tempo para começar em 0 segundos (tempo relativo)
    start_time = df['TimestampMS'].iloc[0]
    df['TimeSec'] = (df['TimestampMS'] - start_time) / 1000.0

    # Configuração da figura com 4 subplots (agora incluindo Retransmissões)
    fig, axes = plt.subplots(4, 1, figsize=(12, 20), sharex=True)
    
    # Cores padrão para consistência
    color_p2s = 'blue'   # Proxy -> Server
    color_c2p = 'green'  # Client -> Proxy

    # --- Gráfico 1: Throughput (Goodput) ---
    axes[0].plot(df['TimeSec'], df['P2S_Goodput_Mbps'], label='Proxy -> Server', color=color_p2s, linewidth=2)
    axes[0].plot(df['TimeSec'], df['C2P_Goodput_Mbps'], label='Client -> Proxy', color=color_c2p, linestyle='--', alpha=0.7)
    axes[0].set_ylabel('Goodput (Mbps)')
    axes[0].set_title('1. Throughput / Goodput da Conexão')
    axes[0].grid(True, which='both', linestyle='--', alpha=0.7)
    axes[0].legend()

    # --- Gráfico 2: RTT (Latência) ---
    axes[1].plot(df['TimeSec'], df['P2S_RTT_ms'], label='RTT (Proxy-Server)', color=color_p2s)
    axes[1].plot(df['TimeSec'], df['C2P_RTT_ms'], label='RTT (Client-Proxy)', color=color_c2p, linestyle='--', alpha=0.5)
    axes[1].set_ylabel('RTT (ms)')
    axes[1].set_title('2. Evolução da Latência (RTT)')
    axes[1].grid(True, which='both', linestyle='--', alpha=0.7)
    axes[1].legend()

    # --- Gráfico 3: Janela de Congestionamento (CWND) e SSTHRESH ---
    # CWND = Linha sólida
    axes[2].plot(df['TimeSec'], df['P2S_CWND'], label='CWND (Proxy-Server)', color=color_p2s)
    axes[2].plot(df['TimeSec'], df['C2P_CWND'], label='CWND (Client-Proxy)', color=color_c2p, alpha=0.5)
    
    # SSTHRESH = Linha tracejada (pontilhada)
    # Nota: Se o ssthresh for infinito/gigante (ex: 2147483647), ele pode distorcer o gráfico.
    # Plotamos mesmo assim para visualização.
    axes[2].plot(df['TimeSec'], df['P2S_SSTHRESH'], label='Ssthresh (Proxy-Server)', color=color_p2s, linestyle=':', linewidth=2)
    axes[2].plot(df['TimeSec'], df['C2P_SSTHRESH'], label='Ssthresh (Client-Proxy)', color=color_c2p, linestyle=':', alpha=0.5)

    axes[2].set_ylabel('Segmentos')
    axes[2].set_title('3. Janela de Congestionamento (CWND) vs SSTHRESH')
    axes[2].grid(True, which='both', linestyle='--', alpha=0.7)
    axes[2].legend()

    # --- Gráfico 4: Retransmissões (Acumuladas) ---
    axes[3].plot(df['TimeSec'], df['P2S_Retrans'], label='Retransmissões (Proxy-Server)', color=color_p2s, linewidth=2)
    axes[3].plot(df['TimeSec'], df['C2P_Retrans'], label='Retransmissões (Client-Proxy)', color=color_c2p, linestyle='--')
    
    axes[3].set_ylabel('Pacotes Retransmitidos (Total)')
    axes[3].set_xlabel('Tempo (segundos)')
    axes[3].set_title('4. Retransmissões TCP Acumuladas')
    axes[3].grid(True, which='both', linestyle='--', alpha=0.7)
    axes[3].legend()

    # Salva o gráfico
    output_img = csv_file.replace('.csv', '.png')
    plt.tight_layout()
    plt.savefig(output_img)
    print(f"[Sucesso] Gráfico salvo em: {output_img}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 plot_graphs.py <arquivo_log.csv>")
    else:
        plot_metrics(sys.argv[1])