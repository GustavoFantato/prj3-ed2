/*
 * TRABALHO PRÁTICO 2 - ESTRUTURA DE DADOS II 
,kl * INTEGRANTES DO GRUPO:
 * - Gustavo Fantato Fernandes (16986132)
 * - Victor Kayky Zaneti (15491132)
 */

#include <stdio.h>
#include "functions.h"

int main() {
    int funcionalidade;

    // Leitura da funcionalidade escolhida pelo usuario ou enviada pelo run.codes
    if (scanf("%d", &funcionalidade) != 1) {
        return 0; // Encerra o programa caso nao consiga ler a funcionalidade
    }

    char arquivoDados[100];
    char arquivoSaida[100];
    char arquivoIndex[100];
    int n, rrn;

    // Roteamento para a funcionalidade correspondente
    switch (funcionalidade) {
        
        // --== PARTE 1 ==--
        
        case 1:
            // [1] - createTable: csv -> bin
            scanf("%s %s", arquivoDados, arquivoSaida);
            createTable(arquivoDados, arquivoSaida);
            break;

        case 2:
            // [2] - listTable: Exibe todos os registros
            scanf("%s", arquivoDados);
            listTable(arquivoDados);
            break;

        case 3:
            // [3] - listTableWhere: Busca sequencial por criterios
            scanf("%s %d", arquivoDados, &n);
            listTableWhere(arquivoDados, n);
            break;

        case 4:
            // [4] - listTableRRN: Busca direta (O(1)) pelo RRN
            scanf("%s %d", arquivoDados, &rrn);
            listTableRRN(arquivoDados, rrn);
            break;

        // --== PARTE 2 ==--

        case 5:
            // [5] - createIndex: Cria arquivo de indice primario (B-Tree em RAM / Array Ordenado)
            scanf("%s %s", arquivoDados, arquivoIndex);
            createIndex(arquivoDados, arquivoIndex);
            break;

        case 6:
            // [6] - listTableWhereIndex: Busca com apoio do indice primario
            scanf("%s %s %d", arquivoDados, arquivoIndex, &n);
            listTableWhereIndex(arquivoDados, arquivoIndex, n);
            break;

        case 7:
            // [7] - deleteFromTable: Remocao logica reaproveitando espaco (Pilha de removidos)
            scanf("%s %s %d", arquivoDados, arquivoIndex, &n);
            deleteFromTable(arquivoDados, arquivoIndex, n);
            break;

        case 8:
            // [8] - insertIntoTable: Insercao utilizando a pilha de removidos
            scanf("%s %s %d", arquivoDados, arquivoIndex, &n);
            insertIntoTable(arquivoDados, arquivoIndex, n);
            break;

        case 9:
            // [9] - updateTable: Atualizacao in-place de campos
            scanf("%s %s %d", arquivoDados, arquivoIndex, &n);
            updateTable(arquivoDados, arquivoIndex, n);
            break;

        case 10:
            // [10] - createAndListGraph: Cria Grafo e exibe a lista de adjacência
            scanf("%s", arquivoDados);
            createAndListGraph(arquivoDados);
            break;
        
        case 11:
            // [11] - Caminho mais curto (Dijkstra)
            {
                char arquivoDados[100], arquivoIndex[100];
                // Lemos os DOIS ficheiros que o run.codes manda (mesmo ignorando o index)
                scanf("%s %s", arquivoDados, arquivoIndex);
                shortestPath(arquivoDados, arquivoIndex);
            }
            break;
        
        case 12:
        
            {
                char arquivoDados[100], arquivoIndex[100];
                scanf("%s %s", arquivoDados, arquivoIndex);
                improveSubwayLines(arquivoDados, arquivoIndex);
            }
            break;
        case 13:
            // [13] - Busca de Ciclos Simples
            {
                char arquivoDados[100], arquivoIndex[100];
                scanf("%s %s", arquivoDados, arquivoIndex);
                countCyclesFromOrigin(arquivoDados, arquivoIndex);
            }
            break;
        

        default:
            // Funcionalidade inexistente
            break;

        
    }

    return 0;
}