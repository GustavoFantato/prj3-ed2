#include "functions.h"
#include "utils.h"
#include "grafo.h"

/*
# FUNCIONALIDADE [10] - Mapeamento do Grafo #
-> Le o arquivo bin, ignorando os registros logicamente removidos
-> Cria um vertice p/ cada estação unica, ordenando alfabeticamente
-> Insere Arestas direcionadas para "ProxEstacao" e para "EstIntegra" (com peso 0).
-> Imprime as listas de adjacências formatadas
*/

void createAndListGraph(char *arquivoDados) {
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

    // Carregar todos os registros validos p/ memoria
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
        lerRegistro(&data, binFile); // Le os registros

        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha); // calcula o lixo
        fseek(binFile, garbageBytes, SEEK_CUR);

        qtdRegs++; // soma 1 na qtd de registros
        regs = realloc(regs, qtdRegs * sizeof(DataRecord));
        regs[qtdRegs - 1] = data;
    }
    fclose(binFile);

    if (qtdRegs == 0) { // se 0 registros, falhou a funcionalidade
        printf("Falha na execução da funcionalidade.\n");
        if (regs != NULL) free(regs);
        return;
    }

    // pegar nomes unicos de estacoes para formar os Vertices
    char **nomesUnicos = NULL;
    int qtdNomes = 0;

    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao == NULL) continue;
        qtdNomes++;
        nomesUnicos = realloc(nomesUnicos, qtdNomes * sizeof(char *));
        nomesUnicos[qtdNomes - 1] = regs[i].nomeEstacao;
    }
    qsort(nomesUnicos, qtdNomes, sizeof(char *), cmpStr); // ordena

    // Remove duplicatas
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

    // Inicializar o Grafo
    Grafo *grafo = criarGrafo(nVertices, verticesFiltrados);
    free(verticesFiltrados);

    // Inserir arestas cruzando os dados da memoria
    for (int i = 0; i < qtdRegs; i++) {
        char *origem = regs[i].nomeEstacao;
        if (origem == NULL) continue;

        // Ligacao 1: Para a proxima estacao
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

        // Ligacao 2: Para a estacao de integracao
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

    // Printa e libera a mem
    imprimirGrafo(grafo);
    liberarGrafo(grafo);

    // Limpa a mem do array de registos, ja que foram alocados dinamicamente
    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao != NULL) free(regs[i].nomeEstacao);
        if (regs[i].nomeLinha != NULL) free(regs[i].nomeLinha);
    }
    free(regs);
}