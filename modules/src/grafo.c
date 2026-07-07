#include "grafo.h"

/* Função auxiliar para o qsort ordenar as linhas em caso de empate */
static int compareStrings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* Busca Binária O(log N) para encontrar o índice do vértice no array */
static int buscaVertice(Grafo *g, char *nome) {
    int esq = 0, dir = g->numVertices - 1;
    while (esq <= dir) {
        int meio = esq + (dir - esq) / 2;
        int cmp = strcmp(g->vetorVertices[meio].nomeEstacao, nome);
        if (cmp == 0) return meio;
        if (cmp < 0) esq = meio + 1;
        else dir = meio - 1;
    }
    return -1; 
}

/* Inicializa o Grafo já com o tamanho exato de estações únicas ordenadas */
Grafo* criarGrafo(int numVertices, char **nomesEstacoes) {
    Grafo *g = (Grafo *)malloc(sizeof(Grafo));
    g->numVertices = numVertices;
    g->vetorVertices = (Vertice *)malloc(numVertices * sizeof(Vertice));

    for (int i = 0; i < numVertices; i++) {
        g->vetorVertices[i].nomeEstacao = strdup(nomesEstacoes[i]);
        g->vetorVertices[i].inicio = NULL;
    }
    return g;
}

/* Insere a Aresta garantindo ordem alfabética do destino e concatenando as linhas */
void inserirAresta(Grafo *g, char *origem, char *destino, int dist, char *linha) {
    int idx = buscaVertice(g, origem);
    if (idx == -1) return;

    Vertice *v = &(g->vetorVertices[idx]);
    Aresta *atual = v->inicio;
    Aresta *ant = NULL;

    // Procura a posição exata de inserção alfabética
    while (atual != NULL && strcmp(atual->estacaoDestino, destino) < 0) {
        ant = atual;
        atual = atual->prox;
    }

    // Caso a aresta para este destino já exista, basta adicionar a nova linha (se não existir)
    if (atual != NULL && strcmp(atual->estacaoDestino, destino) == 0) {
        int linhaExiste = 0;
        for (int i = 0; i < atual->qtdLinhas; i++) {
            if (strcmp(atual->nomeLinhas[i], linha) == 0) {
                linhaExiste = 1;
                break;
            }
        }
        // Se é uma linha nova entre as mesmas estações, adiciona e reordena
        if (!linhaExiste) {
            atual->qtdLinhas++;
            atual->nomeLinhas = realloc(atual->nomeLinhas, atual->qtdLinhas * sizeof(char *));
            atual->nomeLinhas[atual->qtdLinhas - 1] = strdup(linha);
            qsort(atual->nomeLinhas, atual->qtdLinhas, sizeof(char *), compareStrings);
        }
        return;
    }

    // Caso não exista, cria uma nova Aresta
    Aresta *nova = (Aresta *)malloc(sizeof(Aresta));
    nova->estacaoDestino = strdup(destino);
    nova->distancia = dist;
    nova->qtdLinhas = 1;
    nova->nomeLinhas = (char **)malloc(sizeof(char *));
    nova->nomeLinhas[0] = strdup(linha);
    nova->prox = atual;

    // Encadeia no local certo
    if (ant == NULL) v->inicio = nova;
    else ant->prox = nova;
}

/* Exibe o Grafo exatamente como exigido no PDF */
/* Exibe o Grafo exatamente como exigido no PDF */
void imprimirGrafo(Grafo *g) {
    for (int i = 0; i < g->numVertices; i++) {
        
        // Pula as estações isoladas (que não têm nenhuma aresta saindo delas)
        if (g->vetorVertices[i].inicio == NULL) continue;

        // Imprime a estação de origem
        printf("%s", g->vetorVertices[i].nomeEstacao);
        
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            
            // Imprime o destino e a distância separados por vírgula e espaço
            printf(", %s, %d", atual->estacaoDestino, atual->distancia);
            
            // Imprime as linhas separadas por vírgula e espaço
            for (int j = 0; j < atual->qtdLinhas; j++) {
                printf(", %s", atual->nomeLinhas[j]);
            }
            atual = atual->prox;
        }
        printf("\n");
    }
}

/* Limpeza completa da RAM */
void liberarGrafo(Grafo *g) {
    if (g == NULL) return;
    for (int i = 0; i < g->numVertices; i++) {
        free(g->vetorVertices[i].nomeEstacao);
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            Aresta *prox = atual->prox;
            free(atual->estacaoDestino);
            for (int j = 0; j < atual->qtdLinhas; j++) free(atual->nomeLinhas[j]);
            free(atual->nomeLinhas);
            free(atual);
            atual = prox;
        }
    }
    free(g->vetorVertices);
    free(g);
}