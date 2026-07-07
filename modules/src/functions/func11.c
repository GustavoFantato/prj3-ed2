#include "functions.h"
#include "utils.h"
#include "grafo.h"

/*
# FUNCIONALIDADE [11] - Caminho mais curto (Dijkstra) #
-> Lê as estações de origem e destino da entrada padrão.
-> Constrói o grafo a partir do ficheiro binário usando a fábrica.
-> Executa o algoritmo de Dijkstra.
*/
void shortestPath(char *arquivoDados, char *arquivoIndex) {
    
    char lixo1[50], lixo2[50];
    char origem[100], destino[100];

    // O input eh: nomeEstacao "Origem" nomeEstacao "Destino"

    // Le o input
    scanf("%s", lixo1); // ignorar o primeiro "nomeEstacao"
    ScanQuoteString(origem); 

    // Lê o destino
    scanf("%s", lixo2); // ignorar o segundo "nomeEstacao"
    ScanQuoteString(destino); 

    // Usa a logica de criar o grafo
    Grafo *grafo = construirGrafoDeArquivo(arquivoDados);
    if (grafo == NULL) return;

    // Roda o algoritmo de dijkstra
    dijkstra(grafo, origem, destino);

    // Libera a mem
    liberarGrafo(grafo);
}