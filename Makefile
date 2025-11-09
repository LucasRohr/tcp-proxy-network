# Compilador
CC = gcc
# Flags: -Wall (todos os warnings), -pthread (para as threads), -I (diretório de includes)
CFLAGS = -Wall -pthread -I./proxy/include -g
# Flags de Linkagem: -pthread
LDFLAGS = -pthread

# Diretórios
SRC_DIR = proxy/src
OBJ_DIR = obj
INCLUDE_DIR = proxy/include
TARGET = proxy_app

# Arquivos fonte
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/connection_handler.c

# Arquivos objeto (calculados a partir dos fontes)
# $(patsubst ...): Substitui "proxy_tcp/src/%.c" por "obj/%.o"
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Regra principal
all: $(TARGET)

# Regra para linkar o executável final
$(TARGET): $(OBJS)
	@echo "Ligando o executável final: $(TARGET)..."
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)
	@echo "Compilação concluída!"

# Regra para compilar cada .c em um .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@# Cria o diretório de objetos se ele não existir
	@mkdir -p $(OBJ_DIR)
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos compilados
clean:
	@echo "Limpando arquivos compilados..."
	rm -f $(TARGET) $(OBJ_DIR)/*.o
	@rmdir $(OBJ_DIR) 2>/dev/null || true
