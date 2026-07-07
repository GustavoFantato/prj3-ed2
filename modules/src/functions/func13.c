#include "functions.h"
#include "utils.h"
#include "grafo.h"

/*
# FUNCIONALIDADE [13] - Busca de Ciclos Simples #
-> Mapeia o binario p/ mem no formato de Grafo
-> Aplica a busca em profundidade p/ encontrar caminhos que voltem p/ origem
*/

void countCyclesFromOrigin(char *arquivoDados, char *arquivoIndex) {
    
    char lixo1[50];
    char valorOrigem[100];

    // Ignora a chave da busca (ex: "nomeEstacao")
    scanf("%s", lixo1);
    
    // Pega o valor desejado da entrada
    ScanQuoteString(valorOrigem); 

    // Criacao do grafo
    Grafo *grafo = construirGrafoDeArquivo(arquivoDados);
    if (grafo == NULL) return;

    // Contagem dos ciclos com a busca em profundidade
    // a partir do vértice de origem e imprime a quantidade encontrada
    buscarCiclos(grafo, valorOrigem);

    // Limpa memoria
    liberarGrafo(grafo);
}