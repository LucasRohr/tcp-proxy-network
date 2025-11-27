# tcp-proxy-network

---

# TCP Proxy Inteligente - Monitoramento e Otimização

Este projeto consiste no desenvolvimento de um **Proxy TCP Intermediário** capaz de interceptar conexões entre um cliente e um servidor, coletar métricas de desempenho em tempo real (como RTT, CWND e Throughput) e aplicar políticas de otimização dinâmicas (Buffer Tuning e TCP Pacing) para melhorar a qualidade da transmissão em cenários de rede adversos.

Trabalho desenvolvido para a disciplina de **Redes de Computadores I**.

## 📋 Índice

1.  Arquitetura e Funcionamento
2.  Compilação e Execução
3.  Políticas de Otimização (Justificativa Técnica)
4.  Metodologia de Testes e Cenários
5.  Visualização de Dados

---

## 🏗 Arquitetura e Funcionamento

O Proxy atua na **Camada de Aplicação**, estabelecendo duas conexões TCP distintas para cada sessão:

1.  **Cliente ↔ Proxy:** O cliente se conecta ao proxy.
2.  **Proxy ↔ Servidor:** O proxy abre uma nova conexão com o servidor de destino.

### Componentes Principais:

- **Main Thread (`main.c`):** Responsável por inicializar o socket _listener_ e aceitar novas conexões (`accept`). Para cada cliente conectado, uma nova _thread_ é disparada.
- **Connection Handler (`connection_handler.c`):** O núcleo do proxy. Utiliza a chamada de sistema `poll()` para multiplexar a entrada e saída de dados entre os dois sockets. Possui um _timer_ interno que, a cada intervalo (ex: 1000ms), coleta métricas e aplica otimizações.
- **Monitor (`tcp_monitor.c`):** Utiliza a estrutura `tcp_info` do Kernel Linux (via `getsockopt`) para extrair dados precisos da pilha TCP, como RTT (Round Trip Time), variação do RTT, contagem de retransmissões e tamanho da Janela de Congestionamento (CWND).
- **Optimizer (`tcp_optimizer.c`):** Módulo responsável por alterar parâmetros do socket em tempo real (`setsockopt`), ajustando buffers e taxas de envio.

---

## 🚀 Compilação e Execução

O projeto utiliza um `Makefile` para automação.

### Pré-requisitos

- Ambiente Linux (para suporte completo a `TCP_INFO` e `SO_MAX_PACING_RATE`).
- Compilador `gcc` e `make`.

### Compilando

Na raiz do projeto, execute:

```bash
make clean
make
```

Isso gerará o executável `proxy_app`.

### Executando o Proxy

A sintaxe de execução é:

```bash
./proxy_app <porta_local> <ip_servidor_real> <porta_servidor_real> [--optimize]
```

- **Modo Monitoramento (Sem Otimização):**
  Apenas repassa os pacotes e gera logs. Útil para estabelecer o _baseline_ do trabalho.

  ```bash
  ./proxy_app 8080 192.168.0.226 9090
  ```

- **Modo Otimizado (Com Otimização):**
  Ativa os algoritmos de Buffer Tuning e Pacing.

  ```bash
  // Flag de otimização aceita: --optimize ou -o
  ./proxy_app 8080 192.168.0.226 9090 --optimize
  ```

---

## ⚙️ Políticas de Otimização (Justificativa Técnica)

Quando a flag `--optimize` (ou `-o`) é ativada, o proxy aplica duas estratégias principais para mitigar problemas de latência e perda de pacotes.

### 1\. Dynamic Buffer Tuning (Ajuste Dinâmico de Buffer)

**Conceito:** O desempenho do TCP é limitado pelo **BDP (Bandwidth-Delay Product)**. O BDP representa a quantidade de dados "em voo" (não confirmados) que a rede pode comportar.
$$BDP = Throughput \times RTT$$

**Problema:** Se o buffer do socket for menor que o BDP, a conexão não utiliza toda a banda disponível. Se for muito maior, ocorre _Bufferbloat_ (latência excessiva).<br/>
**Solução Implementada:** O proxy calcula o BDP em tempo real usando o _throughput_ atual e o RTT medido. Em seguida, ajusta os tamanhos dos buffers de envio e recebimento (`SO_SNDBUF`, `SO_RCVBUF`) para **2x o BDP**, garantindo que o "tubo" esteja sempre cheio, mas sem desperdício excessivo de memória.

### 2\. TCP Pacing (Controle de Ritmo)

**Conceito:** O TCP padrão tende a enviar pacotes em rajadas (_bursts_). Em redes com gargalos ou alta latência, essas rajadas enchem filas de roteadores rapidamente, causando descartes e retransmissões.<br/>
**Solução Implementada:** O proxy monitora o RTT. Se o **RTT exceder 100ms** (indicativo de rede congestionada ou longa distância), o Pacing é ativado via `SO_MAX_PACING_RATE`. Isso instrui o Kernel a espaçar o envio de pacotes uniformemente, suavizando o tráfego e reduzindo a probabilidade de perdas por congestionamento.

---

## 🧪 Metodologia de Testes e Cenários

Os testes foram realizados utilizando o software **`tc` (Traffic Control)** do Linux na máquina servidora (via VM) para emular diferentes condições de rede.

### Nomenclatura dos Arquivos de Log

Os logs gerados na pasta `logs/` seguem o padrão (indicando cenário de teste e uso ou não de otimização):
`teste_<nome_cenario>_[sem|COM]_otim.csv`

Isso permite identificar facilmente qual cenário e qual modo de operação gerou os dados.

### Cenários Executados

#### 1\. Rede Ideal (Baseline)

- **Descrição:** Rede local sem interferências.
- **Comando `tc`:** _Nenhum (reset)_.
- **Arquivo:** `teste_rede_ideal...`

#### 2\. Cenário Leve

- **Descrição:** Pequena latência e perda mínima. Simula uma conexão Wi-Fi estável mas distante.
- **Comando `tc`:** `sudo tc qdisc add dev eth0 root netem delay 50ms loss 1%`
- **Arquivo:** `teste_leve...`

#### 3\. Cenário Moderado

- **Descrição:** Latência considerável e perda moderada. Simula WAN ou internet congestionada.
- **Comando `tc`:** `sudo tc qdisc add dev eth0 root netem delay 100ms loss 2%`
- **Arquivo:** `teste_moderado...`

#### 4\. Cenário Gargalo de Banda

- **Descrição:** Limitação artificial da largura de banda para 5Mbps.
- **Comando `tc`:** `sudo tc qdisc add dev eth0 root tbf rate 5mbit burst 32kbit latency 400ms`
- **Arquivo:** `teste_gargalo...`

#### 5\. Cenário "Long Network" (Alta Latência)

- **Descrição:** Alto RTT sem perda de pacotes (ex: link intercontinental). Testa a capacidade do TCP de abrir a janela.
- **Comando `tc`:** `sudo tc qdisc add dev eth0 root netem delay 200ms`
- **Arquivo:** `teste_long...`

#### 6\. Cenário "Rede Caótica" (Estresse)

- **Descrição:** Jitter (variação de atraso) elevado e alta taxa de perda. O pior cenário possível.
- **Comando `tc`:** `sudo tc qdisc add dev eth0 root netem delay 100ms 50ms distribution normal loss 5%`
- **Arquivo:** `teste_caotica...`

---

## 📊 Visualização de Dados

Um script em Python foi desenvolvido para gerar gráficos comparativos a partir dos CSVs.

### Dependências

```bash
pip install pandas matplotlib
```

### Gerando Gráficos

```bash
python3 scripts/plot_graphs.py logs/teste_moderado_COM_otim.csv
```

O script gera uma imagem PNG contendo 4 gráficos:

1.  **Throughput/Goodput:** Comparação da taxa de transferência.
2.  **RTT:** Evolução da latência.
3.  **CWND vs ssthresh:** Comportamento da janela de congestionamento.
4.  **Retransmissões:** Acúmulo de pacotes perdidos.
