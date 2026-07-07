#include "grafo.h"
#include "utils.h"
#include "structs.h"

// Func auxiliar para o qsort ordenar as linhas em caso de empate
static int compareStrings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Busca Binaria - O(log N) - p/ encontrar o indice do vertice no array
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

// Inicializa o Grafo já com o tamanho exato de estacoes que sao unicas ordenadas
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

// FABRICA DE GRAFO
// Repete-se no inicio das 4 funcoes, modularizando-se entao para poupar codigo
Grafo* construirGrafoDeArquivo(char *arquivoDados) {
    
    // Tenta abrir o arquivo binario no modo de leitura
    FILE *binFile = fopen(arquivoDados, "rb");
    if (binFile == NULL) {
        printf("Falha na execução da funcionalidade.\n");
        return NULL;
    }

    // Le o status no cabecalho para ver se o arquivo esta consistente
    char status;
    if (fread(&status, sizeof(char), 1, binFile) != 1 || status == '0') {
        printf("Falha na execução da funcionalidade.\n");
        fclose(binFile);
        return NULL;
    }
    
    // Pula o resto do cabecalho e vai direto p/ dados
    fseek(binFile, DATA_HEADER_SIZE, SEEK_SET); 

    DataRecord *regs = NULL; // Lista para guardar os registros que vamos ler
    int qtdRegs = 0;         // Conta quantos registros validos achamos
    char removido;

    // Le o arquivo todo, registro por registro
    while (fread(&removido, sizeof(char), 1, binFile) == 1) {
        
        // Se o registro foi apagado ('1'), pula os bytes dele e vai para o proximo
        if (removido == '1') {
            fseek(binFile, DATA_REGISTER_SIZE - 1, SEEK_CUR);
            continue;
        }

        // Se nao foi apagado, lemos os dados de verdade
        DataRecord data;
        data.removido = removido;
        lerRegistro(&data, binFile); 

        // Pula o "lixo"
        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR);

        // Aumenta o tamanho da nossa lista e guarda o registro lido
        qtdRegs++;
        regs = realloc(regs, qtdRegs * sizeof(DataRecord));
        regs[qtdRegs - 1] = data;
    }
    fclose(binFile); // Ja lemos tudo, pode fechar o arquivo

    // Se nao achou nenhum registro valido, avisa e cancela
    if (qtdRegs == 0) {
        printf("Falha na execução da funcionalidade.\n");
        if (regs != NULL) free(regs);
        return NULL;
    }

    char **nomesUnicos = NULL; // Lista temporaria so para os nomes das estacoes
    int qtdNomes = 0;

    // Pega o nome de cada estacao que lemos e coloca nessa lista
    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao == NULL) continue;
        qtdNomes++;
        nomesUnicos = realloc(nomesUnicos, qtdNomes * sizeof(char *));
        nomesUnicos[qtdNomes - 1] = regs[i].nomeEstacao;
    }
    
    // Coloca os nomes em ordem alfabetica
    qsort(nomesUnicos, qtdNomes, sizeof(char *), cmpStr);

    int nVertices = 0;
    char **verticesFiltrados = malloc(qtdNomes * sizeof(char *));
    
    // Remove os nomes repetidos para descobrir as estacoes unicas (os vertices)
    if (qtdNomes > 0) {
        verticesFiltrados[0] = nomesUnicos[0]; // Guarda o primeiro
        nVertices = 1;
        for (int i = 1; i < qtdNomes; i++) {
            // Se o nome atual for diferente do anterior, guarda tambem
            if (strcmp(nomesUnicos[i], nomesUnicos[i-1]) != 0) {
                verticesFiltrados[nVertices++] = nomesUnicos[i];
            }
        }
    }
    free(nomesUnicos); // Limpa a lista de nomes que tinha repeticoes

    // Cria o grafo vazio, so com as estacoes unicas (ainda sem ligacoes)
    Grafo *grafo = criarGrafo(nVertices, verticesFiltrados);
    free(verticesFiltrados); // Limpa a lista de nomes unicos

    // Passa por todos os registros de novo para fazer as ligacoes (arestas)
    for (int i = 0; i < qtdRegs; i++) {
        char *origem = regs[i].nomeEstacao;
        if (origem == NULL) continue;

        // 1. Cria a ligacao normal para a proxima estacao
        if (regs[i].codProxEstacao != -1) {
            char *destino = NULL;
            // Procura o nome da proxima estacao usando o codigo dela
            for (int j = 0; j < qtdRegs; j++) {
                if (regs[j].codEstacao == regs[i].codProxEstacao) {
                    destino = regs[j].nomeEstacao; 
                    break; // Achou, pode parar de procurar
                }
            }
            // Se achou o nome e tem uma linha valida, faz a ligacao no grafo
            if (destino != NULL && regs[i].nomeLinha != NULL) {
                inserirAresta(grafo, origem, destino, regs[i].distProxEstacao, regs[i].nomeLinha);
            }
        }

        // 2. Cria a ligacao de integracao (quando muda para outra linha)
        if (regs[i].codEstIntegra != -1) {
            char *destinoIntegra = NULL;
            // Procura o nome da estacao de integracao pelo codigo
            for (int j = 0; j < qtdRegs; j++) {
                if (regs[j].codEstacao == regs[i].codEstIntegra) {
                    destinoIntegra = regs[j].nomeEstacao; 
                    break;
                }
            }
            // Se achou e o nome for diferente da origem, liga com distancia 0
            if (destinoIntegra != NULL && strcmp(origem, destinoIntegra) != 0) {
                inserirAresta(grafo, origem, destinoIntegra, 0, "Integração");
            }
        }
    }

    // Limpa todas as informacoes temporarias que usamos, para liberar memoria
    for (int i = 0; i < qtdRegs; i++) {
        if (regs[i].nomeEstacao != NULL) free(regs[i].nomeEstacao);
        if (regs[i].nomeLinha != NULL) free(regs[i].nomeLinha);
    }
    free(regs);

    // Devolve o grafo montado, limpo e pronto para ser usado nas funcoes 
    return grafo; 
}

// Insere a aresta garantindo ordem alfabetica do destino e concatenando as linhas
void inserirAresta(Grafo *g, char *origem, char *destino, int dist, char *linha) {
    int idx = buscaVertice(g, origem);
    if (idx == -1) return;

    Vertice *v = &(g->vetorVertices[idx]);
    Aresta *atual = v->inicio;
    Aresta *ant = NULL;

    // Procura a pos exata de insercao alfabetica
    while (atual != NULL && strcmp(atual->estacaoDestino, destino) < 0) {
        ant = atual;
        atual = atual->prox;
    }

    // Caso a aresta p/ esse destino ja exista, eh so adicionar a nova linha (se n existir)
    if (atual != NULL && strcmp(atual->estacaoDestino, destino) == 0) {
        int linhaExiste = 0;
        for (int i = 0; i < atual->qtdLinhas; i++) {
            if (strcmp(atual->nomeLinhas[i], linha) == 0) {
                linhaExiste = 1;
                break;
            }
        }
        // Se eh uma linha nova entre as mesmas estacoes, adiciona e reordena
        if (!linhaExiste) {
            atual->qtdLinhas++;
            atual->nomeLinhas = realloc(atual->nomeLinhas, atual->qtdLinhas * sizeof(char *));
            atual->nomeLinhas[atual->qtdLinhas - 1] = strdup(linha);
            qsort(atual->nomeLinhas, atual->qtdLinhas, sizeof(char *), compareStrings);
        }
        return;
    }

    // Caso nao exista, cria uma nova aresta
    Aresta *nova = (Aresta *)malloc(sizeof(Aresta));
    nova->estacaoDestino = strdup(destino);
    nova->distancia = dist;
    nova->qtdLinhas = 1;
    nova->nomeLinhas = (char **)malloc(sizeof(char *));
    nova->nomeLinhas[0] = strdup(linha);
    nova->prox = atual;

    // encadeia no local certo
    if (ant == NULL) v->inicio = nova;
    else ant->prox = nova;
}

// Printar o grafo de acordo com as especif. dos projeto
void imprimirGrafo(Grafo *g) {
    for (int i = 0; i < g->numVertices; i++) {
        
        // Pula as estacoes isoladas (que nao tem nenhuma aresta saindo delas)
        if (g->vetorVertices[i].inicio == NULL) continue;

        // Imprime a estacao de origem
        printf("%s", g->vetorVertices[i].nomeEstacao);
        
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            
            // Imprime o destino e a dist separados por virgula e espaco
            printf(", %s, %d", atual->estacaoDestino, atual->distancia);
            
            // Imprime as linhas separadas por virgula e espaco
            for (int j = 0; j < atual->qtdLinhas; j++) {
                printf(", %s", atual->nomeLinhas[j]);
            }
            atual = atual->prox;
        }
        printf("\n");
    }
}

/* Liberacao da memoria */
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

/* # Algoritmo de Dijkstra #
-> Encontra o menor caminho e a menor dist entre duas estacoes.
-> Resolve empates, priorizando a ordem alfabetica
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

    // inicializacao
    for (int i = 0; i < g->numVertices; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[idxOrigem] = 0;

    // Loop principal para processar todos os vertices
    for (int count = 0; count < g->numVertices; count++) {
        int u = -1;
        int min_dist = INF;

        // Desempate 1 (vertices): O array ja eh alfabetico, então ir de 0 a V-1
        for (int i = 0; i < g->numVertices; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        // se todos os restantes nao sao possiveis de chegar, para
        if (u == -1 || min_dist == INF) break;
        
        visited[u] = 1;

        Aresta *atual = g->vetorVertices[u].inicio;
        while (atual != NULL) {
            int v = buscaVertice(g, atual->estacaoDestino);
            if (v != -1 && !visited[v]) {
                int peso = atual->distancia;
                
                // Se encontrou um caminho menor, att
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

    // Verifica se chegou no destino e formata a saida
    if (dist[idxDestino] == INF) {
        printf("Não existe caminho entre as estações solicitadas.\n");
    } else {
        // Reconstroi o caminho de tras p/ frente usando os prevs
        int *caminho = (int *)malloc(g->numVertices * sizeof(int));
        int tamCaminho = 0;
        int curr = idxDestino;
        while (curr != -1) {
            caminho[tamCaminho++] = curr;
            curr = prev[curr];
        }

        // Print
        printf("Numero de estacoes que serao percorridas: %d\n", tamCaminho - 1);
        printf("Distancia que sera percorrida: %d\n", dist[idxDestino]);
        
        // Imprime as estacoes na ordem certa
        for (int i = tamCaminho - 1; i >= 0; i--) {
            printf("%s", g->vetorVertices[caminho[i]].nomeEstacao);
            if (i > 0) printf(", ");
        }
        printf("\n");

        free(caminho);
    }

    // Liberacao de mem
    free(dist);
    free(prev);
    free(visited);
}


// FUNCIONALIDADE 12

// Func recursiva auxiliar para imprimir a arvore geradora minima
static void printAGM(Grafo *g, int u, int *ant, int *chave, int n) {
    for (int v = 0; v < n; v++) {
        // Se u for o pai direto de v na arvore gerada
        if (ant[v] == u) {
            printf("%s, %s, %d\n", g->vetorVertices[u].nomeEstacao, g->vetorVertices[v].nomeEstacao, chave[v]);
            
            // chamada recursiva pros descentendentes de v
            printAGM(g, v, ant, chave, n);
        }
    }
}

// Constroi a arvore geradora minima
void buildAGM(Grafo *g, char *origem) {
    int idxOrigem = buscaVertice(g, origem);
    if (idxOrigem == -1) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    int n = g->numVertices;
    int max_arestas = 0;
    
    // Conta o total de arestas para alocacao da struct aux criada
    for(int i = 0; i < n; i++) {
        Aresta *atual = g->vetorVertices[i].inicio;
        while(atual) { 
            max_arestas++; 
            atual = atual->prox; 
        }
    }

    ArestaND *arestas = malloc(max_arestas * sizeof(ArestaND));
    int m = 0;
    
    // Coletar as arestas
    for (int i = 0; i < n; i++) {
        Aresta *atual = g->vetorVertices[i].inicio;
        while (atual != NULL) {
            int v = buscaVertice(g, atual->estacaoDestino);
            // ignora arestas em que o destino nao tem possui saidas
            if (v != -1 && g->vetorVertices[v].inicio != NULL) {
                arestas[m].u = i;
                arestas[m].v = v;
                arestas[m].peso = atual->distancia;
                m++;
            }
            atual = atual->prox;
        }
    }

    int *chave = malloc(n * sizeof(int));
    int *ant = malloc(n * sizeof(int));
    int *visitado = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++){
        chave[i] = INF;
        ant[i] = -1;
    }

    chave[idxOrigem] = 0;
    visitado[idxOrigem] = 1;
    int count = 1;

    // Algoritmo de Prim na lista de arestas
    while (count < n) {
        int melhorU = -1, melhorV = -1;
        int menorPeso = INF;

        // Percorre a lista procurando a melhor aresta de fronteira
        for (int i = 0; i < m; i++) {
            int u = arestas[i].u;
            int v = arestas[i].v;
            int peso = arestas[i].peso;

            // Aresta deve ter exatamente um extremo dentro da arvore (visitado) e outro fora
            if (visitado[u] == visitado[v]) continue;

            int interno = visitado[u] ? u : v;
            int externo = visitado[u] ? v : u;

            // Criterios de desempate
            // menor peso -> menor ID do vertice interno -> menor ID do vertice externo
            if (peso < menorPeso ||
               (peso == menorPeso && interno < melhorU) ||
               (peso == menorPeso && interno == melhorU && externo < melhorV)) {
                menorPeso = peso;
                melhorU = interno;
                melhorV = externo;
            }
        }

        // Se nao encontrou vizinho valido, o componente conexo esgotou
        if (melhorV == -1) break;

        visitado[melhorV] = 1;
        chave[melhorV] = menorPeso;
        ant[melhorV] = melhorU;
        count++;
    }

    // printa com a func auxiliar
    printAGM(g, idxOrigem, ant, chave, n);

    free(arestas);
    free(chave); 
    free(ant); 
    free(visitado);
}

// FUNCIONALIDADE 13: Busca de Ciclos Simples

// busca em profundidade
static void buscaProfundidadeCiclos(Grafo *g, int origin, int u, int *visited, int *count) {
    Aresta *atual = g->vetorVertices[u].inicio;
    
    while (atual != NULL) {
        int v = buscaVertice(g, atual->estacaoDestino);
        
        if (v != -1) {
            if (v == origin) {
                (*count)++; // encontrou um ciclo de volta p/ origem
            } else if (!visited[v]) {
                visited[v] = 1;
                buscaProfundidadeCiclos(g, origin, v, visited, count);
                visited[v] = 0; // backtracking para encontrar todos os ciclos
            }
        }
        atual = atual->prox;
    }
}

void buscarCiclos(Grafo *g, char *origem) {
    int idxOrigem = buscaVertice(g, origem);
    if (idxOrigem == -1) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    int *visited = calloc(g->numVertices, sizeof(int));
    int count = 0;

    visited[idxOrigem] = 1;
    buscaProfundidadeCiclos(g, idxOrigem, idxOrigem, visited, &count);

    if (count == 0) {
        printf("Quantidade de ciclos: -1\n");
    } else {
        printf("Quantidade de ciclos: %d\n", count);
    }
    
    free(visited);
}