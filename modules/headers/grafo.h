#ifndef GRAFO_H
#define GRAFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura de uma Aresta (Lista encadeada para os destinos)
typedef struct aresta {
    char *estacaoDestino;
    int distancia;
    int qtdLinhas;
    char **nomeLinhas; // Array de strings (para lidar com o empate de múltiplas linhas)
    struct aresta *prox;
} Aresta;

// Estrutura de um Vértice (Array)
typedef struct vertice {
    char *nomeEstacao;
    Aresta *inicio;
} Vertice;

// Estrutura principal do Grafo (Lista de Adjacências)
typedef struct grafo {
    int numVertices;
    Vertice *vetorVertices;
} Grafo;

// Protótipos das funções do Grafo
Grafo* criarGrafo(int numVertices, char **nomesEstacoes);
void inserirAresta(Grafo *g, char *origem, char *destino, int dist, char *linha);
void imprimirGrafo(Grafo *g);
void liberarGrafo(Grafo *g);

void dijkstra(Grafo *g, char *origem, char *destino);
#endif