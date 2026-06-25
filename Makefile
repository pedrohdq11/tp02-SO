CC = gcc
CFLAGS = -Wall -Wextra -g

# O executável final
TARGET = simulador

# Diretórios
SRC_DIR = src
INC_DIR = src

# Fontes e objetos (agora incluindo comandos.c e fs.c)
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/comandos.c $(SRC_DIR)/fs.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

run-test: $(TARGET)
	./$(TARGET) testes/comandos.txt
