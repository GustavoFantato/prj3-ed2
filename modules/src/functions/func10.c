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

    // Constroi o grafo
    Grafo *grafo = construirGrafoDeArquivo(arquivoDados);
    if(grafo == NULL) return;

    // Printa e libera a mem
    imprimirGrafo(grafo);
    liberarGrafo(grafo);
}