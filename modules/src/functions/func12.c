#include "functions.h"
#include "utils.h"
#include "grafo.h"

/*
# FUNCIONALIDADE [12] - Árvore Geradora Mínima (Kruskal) e Caminhamento em DFS #
-> Mapeia o binário para a memória no formato de Grafo (Listas de Adjacência).
-> Aplica o algoritmo de Kruskal para criar a MST baseada nas distâncias.
-> Realiza um caminhamento DFS recursivo para imprimir os passos partindo da origem.
*/

static int cmpStr(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void minimumSpanningTree(char *arquivoDados, char *arquivoIndex) {
    
    char lixo1[50];
    char valorOrigem[100];

    // O terminal envia: nomeEstacao "Mogi das Cruzes"
    scanf("%s", lixo1); // Absorve o "nomeEstacao" (vamos usá-lo apenas para limpar o buffer)
    ScanQuoteString(valorOrigem); // Extrai apenas a estação "Mogi das Cruzes"

    // ===============================================
    // LÓGICA DE MONTAGEM DO GRAFO (IDÊNTICA À FUNC 11)
    // ===============================================
    FILE *binFile = fopen(arquivoDados, "rb");
    if (binFile == NULL) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    char status;
    if (fread(&status, sizeof(char), 1, binFile) != 1 || status == '0') {
        printf("Falha na execução da funcionalidade.\n");
        fclose(binFile);
        return;
    }
    fseek(binFile, DATA_HEADER_SIZE, SEEK_SET);

    DataRecord *regs = NULL;
    int qtdRegs = 0;
    char removido;

    while (fread(&removido, sizeof(char), 1, binFile) == 1) {
        if (removido == '1') {
            fseek(binFile, DATA_REGISTER_SIZE - 1, SEEK_CUR);
            continue;
        }

        DataRecord data;
        data.removido = removido;
        lerRegistro(&data, binFile); 

        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR);

        qtdRegs++;
        regs = realloc(regs, qtdRegs * sizeof(DataRecord));
        regs[qtdRegs - 1] = data;
    }
    fclose(binFile);

    if (qtdRegs == 0) {
        printf("Falha na execução da funcionalidade.\n");
        if (regs != NULL) free(regs);
        return;
    }

    char **nomesUnicos = NULL;
    int qtdNomes = 0;

    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao == NULL) continue;
        qtdNomes++;
        nomesUnicos = realloc(nomesUnicos, qtdNomes * sizeof(char *));
        nomesUnicos[qtdNomes - 1] = regs[i].nomeEstacao;
    }
    qsort(nomesUnicos, qtdNomes, sizeof(char *), cmpStr);

    int nVertices = 0;
    char **verticesFiltrados = malloc(qtdNomes * sizeof(char *));
    if (qtdNomes > 0) {
        verticesFiltrados[0] = nomesUnicos[0];
        nVertices = 1;
        for (int i = 1; i < qtdNomes; i++) {
            if (strcmp(nomesUnicos[i], nomesUnicos[i-1]) != 0) {
                verticesFiltrados[nVertices++] = nomesUnicos[i];
            }
        }
    }
    free(nomesUnicos);

    Grafo *grafo = criarGrafo(nVertices, verticesFiltrados);
    free(verticesFiltrados);

    for (int i = 0; i < qtdRegs; i++) {
        char *origem = regs[i].nomeEstacao;
        if (origem == NULL) continue;

        if (regs[i].codProxEstacao != -1) {
            char *destino = NULL;
            for (int j = 0; j < qtdRegs; j++) {
                if (regs[j].codEstacao == regs[i].codProxEstacao) {
                    destino = regs[j].nomeEstacao; break;
                }
            }
            if (destino != NULL && regs[i].nomeLinha != NULL) {
                inserirAresta(grafo, origem, destino, regs[i].distProxEstacao, regs[i].nomeLinha);
            }
        }

        if (regs[i].codEstIntegra != -1) {
            char *destinoIntegra = NULL;
            for (int j = 0; j < qtdRegs; j++) {
                if (regs[j].codEstacao == regs[i].codEstIntegra) {
                    destinoIntegra = regs[j].nomeEstacao; break;
                }
            }
            if (destinoIntegra != NULL && strcmp(origem, destinoIntegra) != 0) {
                inserirAresta(grafo, origem, destinoIntegra, 0, "Integração");
            }
        }
    }

    // ===============================================
    // EXECUÇÃO DO KRUSKAL (MST) E DFS
    // ===============================================
    
    // A lógica matemática densa foi isolada no nosso módulo.
    mstAndDFS(grafo, valorOrigem);

    // Limpeza rigorosa da memória RAM
    liberarGrafo(grafo);
    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao != NULL) free(regs[i].nomeEstacao);
        if (regs[i].nomeLinha != NULL) free(regs[i].nomeLinha);
    }
    free(regs);
}