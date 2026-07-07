# Nome do executável exigido pelo projeto
BINARY = programaTrab

# Nome do arquivo ZIP para a submissão do Projeto 2
ZIP_NAME = submissao.zip

# Caminhos exatos de acordo com a sua árvore
SRC_DIR = ./src
MODULES_SRC = ./modules/src
HEADERS_DIR = ./modules/headers

# Compilador e Flags (o -I aponta para onde estão os seus arquivos .h)
CC = gcc
FLAGS = -g -Wall -Werror -I$(HEADERS_DIR)

# Captura o programaTrab.c, o utils.c e TODOS os funcX.c dentro da subpasta de funções
SOURCES = $(SRC_DIR)/programaTrab.c \
          $(MODULES_SRC)/utils.c \
          $(wildcard $(MODULES_SRC)/functions/*.c) \
		  $(MODULES_SRC)/grafo.c


# 1. REGRA DE COMPILAÇÃO (Obrigatória: 'make all')
all:
	$(CC) $(FLAGS) $(SOURCES) -o $(BINARY) -lm

# 2. REGRA DE EXECUÇÃO (Obrigatória: 'make run')
run:
	./$(BINARY)

# 3. REGRA PARA CRIAR O ZIP (Garante que limpa tudo antes e gera o zip perfeito)
zip: clean
	@echo "Criando o arquivo compactado $(ZIP_NAME) para o Projeto 2..."
	zip -r $(ZIP_NAME) Makefile modules/ src/
	@echo "Pronto! Arquivo $(ZIP_NAME) gerado sem lixo de compilação."

# 4. LIMPEZA
clean:
	rm -f $(BINARY) $(ZIP_NAME)

.PHONY: all run zip clean