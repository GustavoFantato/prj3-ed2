#ifndef GRAFO_H
#define GRAFO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// infinito usado no Dijkstra e Prim
#define INF 2147483647 // tamanho max do int

typedef struct aresta {
    char *estacaoDestino;
    int distancia;
    int qtdLinhas;
    char **nomeLinhas; // Array de strings (usado p/ lidar com o empate)
    struct aresta *prox;
} Aresta;

typedef struct vertice {
    char *nomeEstacao;
    Aresta *inicio;
} Vertice;

typedef struct grafo {
    int numVertices;
    Vertice *vetorVertices;
} Grafo;

// funcoes base do Grafo
Grafo *criarGrafo(int numVertices, char **nomesEstacoes);
Grafo* construirGrafoDeArquivo(char *arquivoDados);

void inserirAresta(Grafo *g, char *origem, char *destino, int dist, char *linha);
void imprimirGrafo(Grafo *g);
void liberarGrafo(Grafo *g);

// Funcionalidade 11: Caminho Mais Curto
void dijkstra(Grafo *g, char *origem, char *destino);

// Funcionalidade 12: Arvore Geradora Mínima (Prim)
void buildAGM(Grafo *g, char *origem);

// Funcionalidade 13: Busca de Ciclos Simples
void buscarCiclos(Grafo *g, char *origem);

#endif