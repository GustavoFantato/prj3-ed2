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

#define INF 2147483647 // Representa o "Infinito" (INT_MAX)

/* # Algoritmo de Dijkstra #
-> Encontra o menor caminho e a menor distância entre duas estações.
-> Resolve empates priorizando a ordem alfabética (menores índices do array).
*/
void dijkstra(Grafo *g, char *origem, char *destino) {
    int idxOrigem = buscaVertice(g, origem);
    int idxDestino = buscaVertice(g, destino);

    if (idxOrigem == -1 || idxDestino == -1) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    int *dist = (int *)malloc(g->numVertices * sizeof(int));
    int *prev = (int *)malloc(g->numVertices * sizeof(int));
    int *visited = (int *)calloc(g->numVertices, sizeof(int));

    // Inicialização
    for (int i = 0; i < g->numVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[idxOrigem] = 0;

    // Loop principal para processar todos os vértices
    for (int count = 0; count < g->numVertices; count++) {
        int u = -1;
        int min_dist = INF;

        // Desempate 1 (Vértices): O array já é alfabético, então iterar de 0 a V-1
        // com o comparador "<" estrito garante a escolha do menor nome!
        for (int i = 0; i < g->numVertices; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        // Se todos os restantes são inalcançáveis, podemos parar
        if (u == -1 || min_dist == INF) break;
        
        visited[u] = 1;

        // Relaxamento das arestas vizinhas
        Aresta *atual = g->vetorVertices[u].inicio;
        while (atual != NULL) {
            int v = buscaVertice(g, atual->estacaoDestino);
            if (v != -1 && !visited[v]) {
                int peso = atual->distancia;
                
                // Se encontrou um caminho menor, atualiza
                if (dist[u] + peso < dist[v]) {
                    dist[v] = dist[u] + peso;
                    prev[v] = u;
                } 
                // Desempate 2 (Arestas): Caminho de mesmo peso, prefere a origem de menor nome
                else if (dist[u] + peso == dist[v]) {
                    if (u < prev[v]) {
                        prev[v] = u;
                    }
                }
            }
            atual = atual->prox;
        }
    }

    // Verifica se alcançou o destino e formata a saída
    if (dist[idxDestino] == INF) {
        printf("Não existe caminho entre as estações solicitadas.\n");
    } else {
        // Reconstrói o caminho de trás para frente usando os prevs
        int *caminho = (int *)malloc(g->numVertices * sizeof(int));
        int tamCaminho = 0;
        int curr = idxDestino;
        while (curr != -1) {
            caminho[tamCaminho++] = curr;
            curr = prev[curr];
        }

        // Exibe conforme a formatação do PDF
        printf("Numero de estacoes que serao percorridas: %d\n", tamCaminho - 1);
        printf("Distancia que sera percorrida: %d\n", dist[idxDestino]);
        
        // Imprime as estações na ordem certa (de trás para frente do array)
        for (int i = tamCaminho - 1; i >= 0; i--) {
            printf("%s", g->vetorVertices[caminho[i]].nomeEstacao);
            if (i > 0) printf(", ");
        }
        printf("\n");

        free(caminho);
    }

    // Limpeza de RAM
    free(dist);
    free(prev);
    free(visited);
}

// ========================================================
// FUNCIONALIDADE 12: Kruskal (MST) e Caminhamento DFS
// ========================================================

// Estrutura para ajudar na ordenação das arestas no Kruskal
typedef struct {
    int u;
    int v;
    int peso;
} Edge;

// Comparador para qsort do Kruskal (resolve os empates exigidos no PDF)
// 1º Menor Peso | 2º Menor Vértice U (Origem) | 3º Menor Vértice V (Destino)
static int cmpEdges(const void *a, const void *b) {
    Edge *e1 = (Edge *)a;
    Edge *e2 = (Edge *)b;
    if (e1->peso != e2->peso) return e1->peso - e2->peso;
    if (e1->u != e2->u) return e1->u - e2->u;
    return e1->v - e2->v;
}

// Funções clássicas de Union-Find para o Kruskal
static int findSet(int *parent, int i) {
    if (parent[i] == i) return i;
    return parent[i] = findSet(parent, parent[i]); 
}

static void unionSet(int *parent, int x, int y) {
    int xroot = findSet(parent, x);
    int yroot = findSet(parent, y);
    if (xroot != yroot) {
        parent[yroot] = xroot; // Liga as sub-árvores
    }
}

// Caminhamento em Profundidade (DFS) na Árvore Geradora Mínima
static void dfs_recursivo(Grafo *mst, int u, int *visited) {
    visited[u] = 1;
    Aresta *atual = mst->vetorVertices[u].inicio;
    
    // As arestas do nosso grafo já são guardadas em ordem alfabética!
    while (atual != NULL) {
        int v = buscaVertice(mst, atual->estacaoDestino);
        if (!visited[v]) {
            // Imprime exatamente como o run.codes quer: Origem, Destino, Distância
            printf("%s, %s, %d\n", mst->vetorVertices[u].nomeEstacao, atual->estacaoDestino, atual->distancia);
            dfs_recursivo(mst, v, visited);
        }
        atual = atual->prox;
    }
}

// Constrói a MST e dispara a DFS
void mstAndDFS(Grafo *g, char *origem) {
    int idxOrigem = buscaVertice(g, origem);
    if (idxOrigem == -1) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    // 1. Conta o número máximo de arestas para alocar o array
    int maxEdges = 0;
    for (int i = 0; i < g->numVertices; i++) {
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            maxEdges++;
            atual = atual->prox;
        }
    }

    Edge *edges = malloc(maxEdges * sizeof(Edge));
    int edgeCount = 0;

    // 2. Extrai TODAS as arestas. O pulo do gato é normalizar u e v 
    // para que 'u' seja sempre o menor alfabeticamente. O Kruskal vai 
    // tratar arestas duplicadas (ida e volta) ignorando a segunda!
    for (int i = 0; i < g->numVertices; i++) {
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            int v = buscaVertice(g, atual->estacaoDestino);
            if (i < v) { 
                edges[edgeCount].u = i;
                edges[edgeCount].v = v;
                edges[edgeCount].peso = atual->distancia;
                edgeCount++;
            } else if (i > v) { // <--- A MÁGICA ESTÁ AQUI (Garante ida e volta)
                edges[edgeCount].u = v;
                edges[edgeCount].v = i;
                edges[edgeCount].peso = atual->distancia;
                edgeCount++;
            }
            atual = atual->prox;
        }
    }

    // 3. Ordena as arestas usando as regras de desempate do Kruskal
    qsort(edges, edgeCount, sizeof(Edge), cmpEdges);

    // 4. Prepara o Union-Find e o Novo Grafo (que será a nossa Árvore MST)
    int *parent = malloc(g->numVertices * sizeof(int));
    char **nomes = malloc(g->numVertices * sizeof(char *));
    for (int i = 0; i < g->numVertices; i++) {
        parent[i] = i;
        nomes[i] = g->vetorVertices[i].nomeEstacao;
    }
    
    Grafo *mst = criarGrafo(g->numVertices, nomes);
    free(nomes);

    // 5. Executa o Kruskal: Adiciona as arestas seguras na MST
    for (int i = 0; i < edgeCount; i++) {
        int u = edges[i].u;
        int v = edges[i].v;
        int x = findSet(parent, u);
        int y = findSet(parent, v);

        if (x != y) {
            unionSet(parent, x, y);
            // Na MST, inserimos como não-direcionado (ida e volta)
            inserirAresta(mst, mst->vetorVertices[u].nomeEstacao, mst->vetorVertices[v].nomeEstacao, edges[i].peso, "MST");
            inserirAresta(mst, mst->vetorVertices[v].nomeEstacao, mst->vetorVertices[u].nomeEstacao, edges[i].peso, "MST");
        }
    }

    // 6. Roda a DFS partindo da estação origem
    int *visited = calloc(g->numVertices, sizeof(int));
    dfs_recursivo(mst, idxOrigem, visited);

    // 7. Limpeza da RAM
    free(edges);
    free(parent);
    free(visited);
    liberarGrafo(mst);
}