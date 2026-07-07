#include "functions.h"
#include "utils.h"
#include "grafo.h"

/*
# FUNCIONALIDADE [12] - Melhoria nas linhas de metro #
-> Gastar menor quantia possivel pra reprojetar as linhas de metro, garantindo que as pessoas possam sair de qualquer estacao e chegar em qualquer outra estacao
*/

void improveSubwayLines(char *arquivoDados, char *arquivoIndex) {
    
    // Variaveis auxiliares p/ leitura da entrada
    char lixo1[50];
    char valorOrigem[100];

    // Formato do input eh: nomeEstacao "Nome da Estação"
    // ignora o "nomeEstacao"
    scanf("%s", lixo1); 
    
    // Le o "Nome da Estação" usando a func de ignorar as aspas ("")
    ScanQuoteString(valorOrigem); 

    // Criacao do grafo
    Grafo *grafo = construirGrafoDeArquivo(arquivoDados);
    if (grafo == NULL) return;

    // Constroi a Árvore Geradora Mínima 
    // a partir da estacao de origem, e ja printa
    buildAGM(grafo, valorOrigem);

    // Libera memoria
    liberarGrafo(grafo);
}