#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [7] - Remover dados do arquivo de acordo com um criterio #
-> Utilizar da abordagem dinamica de reaproveitamento de espacos de registros logicamente removidos
-> Deve ser implementada utilizando o conceito de pilha de registros logicamente removidos
*/

void deleteFromTable(char *arquivoDados, char *arquivoIndex, int n) {

    // Abertura do arquivo de dados para leitura e escrita
    FILE *binFile = fopen(arquivoDados, "rb+");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Leitura e verificacao de consistencia do cabecalho completo
    HeaderRecord header;
    fread(&header.status, sizeof(char), 1, binFile);
    if (header.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }
    fread(&header.topo, sizeof(int), 1, binFile);
    fread(&header.proxRRN, sizeof(int), 1, binFile);
    fread(&header.nroEstacoes, sizeof(int), 1, binFile);
    fread(&header.nroParesEstacao, sizeof(int), 1, binFile);

    // Abertura e verificacao de consistencia do arquivo de indice
    FILE *indexFile = fopen(arquivoIndex, "rb");
    if (indexFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    char indexStatus;
    fread(&indexStatus, sizeof(char), 1, indexFile);
    if (indexStatus == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        fclose(indexFile);
        return;
    }

    // Marca o arquivo de dados como inconsistente durante a transacao
    header.status = '0';
    fseek(binFile, 0, SEEK_SET);
    fwrite(&header.status, sizeof(char), 1, binFile);

    // Carrega o indice inteiro para a RAM para facilitar as remocoes cruzadas
    fseek(indexFile, INDEX_HEADER_SIZE, SEEK_SET);
    IndexRecord *indexList = NULL;
    int qtdIndex = 0;
    IndexRecord tempIdx;

    while (fread(&tempIdx.codEstacao, sizeof(int), 1, indexFile) == 1) {
        fread(&tempIdx.RRN, sizeof(int), 1, indexFile);
        qtdIndex++;
        indexList = realloc(indexList, qtdIndex * sizeof(IndexRecord));
        indexList[qtdIndex - 1] = tempIdx;
    }
    fclose(indexFile); // Fechamos pois vamos recriar o indice no final

    // Loop para processar as 'n' remocoes
    for (int i = 0; i < n; i++) {

        // m eh o numero de campos a serem comparados para a busca
        int m;
        scanf("%d", &m);

        // Campos e valores a serem comparados
        char campos[8][30];
        char valores[8][100];

        // Bool para definir se usaremos o indice ou varredura sequencial
        int useIndexFile = 0;
        int indiceCodEst = -1; // facilitar saber em qual indice de campos[j] armazenamos o codEstacao

        // Loop para ler os campos e valores, de acordo com o 'm' lido
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j]);

            if (strcmp(campos[j], "codEstacao") == 0) {
                useIndexFile = 1;
                indiceCodEst = j;
            }

            // Se o campo for nomeEstacao ou nomeLinha, utiliza-se a funcao dada ScanQuoteString()
            if (strcmp(campos[j], "nomeEstacao") == 0 || strcmp(campos[j], "nomeLinha") == 0) {
                ScanQuoteString(valores[j]);
            } else {
                scanf("%s", valores[j]);

                // Se nulo, coloca a string como vazia para facilitar a comparacao mais pra frente
                if (strcmp(valores[j], "NULO") == 0) {
                    strcpy(valores[j], "");
                }
            }
        } // Fim for de leitura dos m's

        // Usamos o arquivo Index para a busca
        if (useIndexFile) {
            int codBusca = atoi(valores[indiceCodEst]);
            int targetRRN = -1;

            // Busca o RRN na lista de indice carregada na RAM
            for (int idx = 0; idx < qtdIndex; idx++) {
                if (indexList[idx].codEstacao == codBusca) {
                    targetRRN = indexList[idx].RRN;
                    break;
                }
            }

            if (targetRRN != -1) { // Se a estacao existe no indice
                long offset = DATA_HEADER_SIZE + ((long)targetRRN * DATA_REGISTER_SIZE);
                fseek(binFile, offset, SEEK_SET);

                char removed;
                fread(&removed, sizeof(char), 1, binFile);

                if (removed == '0') { // Confirma que o registro esta ativo
                    DataRecord data;
                    data.removido = removed;
                    lerRegistro(&data, binFile);

                    // Dupla checagem: garante que os outros campos do filtro tambem batem
                    int match = 1;
                    for (int f = 0; f < m; f++) {
                        checkMatch(data, campos, f, valores, &match);
                        if (!match) break;
                    }

                    if (match) {
                        // Verifica se e o ultimo registro com esse nome para atualizar o contador
                        int nomeExiste = verifyName(binFile, data.nomeEstacao, offset);
                        if (!nomeExiste && header.nroEstacoes > 0) header.nroEstacoes--;

                        // Se essa estacao ligava com outra, diminui o numero de pares
                        if (data.codProxEstacao != -1 && header.nroParesEstacao > 0) header.nroParesEstacao--;

                        // Faz a remocao logica e o encadeamento da pilha no disco
                        fseek(binFile, offset, SEEK_SET);
                        char mark = '1';
                        fwrite(&mark, sizeof(char), 1, binFile);
                        fwrite(&header.topo, sizeof(int), 1, binFile); // 'proximo' aponta para o topo antigo
                        header.topo = targetRRN; // topo passa a ser o RRN desse registro removido

                        // Remove a estacao da lista do indice na RAM (Shift Left)
                        for (int idx = 0; idx < qtdIndex; idx++) {
                            if (indexList[idx].codEstacao == codBusca) {
                                for (int k = idx; k < qtdIndex - 1; k++) {
                                    indexList[k] = indexList[k + 1];
                                }
                                qtdIndex--;
                                break;
                            }
                        }
                    }
                    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                    if (data.nomeLinha != NULL) free(data.nomeLinha);
                }
            }
        } else { // Se o codEstacao nao for criterio de busca, percorremos o arquivo de dados inteiro

            // Percorre todos os registros pelo RRN
            for (int rrnAtual = 0; rrnAtual < header.proxRRN; rrnAtual++) {
                long offset = DATA_HEADER_SIZE + ((long)rrnAtual * DATA_REGISTER_SIZE);
                fseek(binFile, offset, SEEK_SET);

                char removed;
                fread(&removed, sizeof(char), 1, binFile);

                // Pula registros ja removidos
                if (removed == '1') continue;

                DataRecord data;
                data.removido = removed;
                lerRegistro(&data, binFile);

                // Verifica se o registro atual atende a todos os criterios de busca
                int match = 1;
                for (int f = 0; f < m; f++) {
                    checkMatch(data, campos, f, valores, &match);
                    // Se nao der match, quebra o loop, evitando comparacoes desnecessarias
                    if (!match) break;
                }

                if (match) {
                    // Verifica se eh o ultimo registro com esse nome para atualizar o contador
                    int nomeExiste = verifyName(binFile, data.nomeEstacao, offset);
                    if (!nomeExiste && header.nroEstacoes > 0) header.nroEstacoes--;
                    if (data.codProxEstacao != -1 && header.nroParesEstacao > 0) header.nroParesEstacao--;

                    // Faz a remocao logica e o encadeamento da pilha no disco
                    fseek(binFile, offset, SEEK_SET);
                    char mark = '1';
                    fwrite(&mark, sizeof(char), 1, binFile);
                    fwrite(&header.topo, sizeof(int), 1, binFile); // 'proximo' aponta para o topo antigo
                    header.topo = rrnAtual; // topo passa a ser o RRN desse registro removido

                    // Remove do indice em RAM (Shift Left)
                    for (int idx = 0; idx < qtdIndex; idx++) {
                        if (indexList[idx].codEstacao == data.codEstacao) {
                            for (int k = idx; k < qtdIndex - 1; k++) {
                                indexList[k] = indexList[k + 1];
                            }
                            qtdIndex--;
                            break;
                        }
                    }
                }

                // se deu match, ou nao, precisa liberar a memoria
                if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                if (data.nomeLinha != NULL) free(data.nomeLinha);
            }

        } // Fim do IF do tipo de busca

    } // Fim do for de 'n'

    // Atualiza o cabecalho completo e marca o arquivo como consistente
    header.status = '1';
    fseek(binFile, 0, SEEK_SET);
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);
    fclose(binFile);

    // Recria e sobrescreve o arquivo de indice a partir da lista em RAM, agora sem os removidos
    indexFile = fopen(arquivoIndex, "wb");
    char indexStatusOut = '0';
    fwrite(&indexStatusOut, sizeof(char), 1, indexFile);
    for (int i = 0; i < qtdIndex; i++) {
        fwrite(&indexList[i].codEstacao, sizeof(int), 1, indexFile);
        fwrite(&indexList[i].RRN, sizeof(int), 1, indexFile);
    }
    indexStatusOut = '1';
    fseek(indexFile, 0, SEEK_SET);
    fwrite(&indexStatusOut, sizeof(char), 1, indexFile);
    fclose(indexFile);

    if (indexList != NULL) free(indexList);

    // Saida exigida pelo projeto para validar as alteracoes de ambos os arquivos
    binarioNaTela(arquivoDados);
    binarioNaTela(arquivoIndex);
}