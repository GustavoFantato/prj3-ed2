#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [8] - Inserir registros em um arquivo de dados de entrada #
-> Abordagem dinamica de reaproveitamento de espacos de registros logicamente removidos
*/

void insertIntoTable(char *arquivoDados, char *arquivoIndex, int n) {
    
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

    // Abertura do arquivo de indice para leitura
    FILE *indexFile = fopen(arquivoIndex, "rb");
    if (indexFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Verificacao de consistencia do arquivo de indice
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

    // Carrega o indice inteiro para a RAM para agilizar as verificacoes de duplicata
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

    // Loop para processar as 'n' insercoes
    for (int i = 0; i < n; i++) {
        
        // Inicializacao do registro a ser inserido
        DataRecord data;
        memset(&data, 0, sizeof(DataRecord));
        data.removido = '0';
        data.proximo = -1;

        // Buffers auxiliares para leitura dos campos
        char strNomeEstacao[100], strNomeLinha[100];
        char strCodLinha[20], strCodProx[20], strDist[20], strCodLinInt[20], strCodEstInt[20];

        // Leitura dos campos na ordem fixa exigida pelo projeto
        scanf("%d", &data.codEstacao);

        // Se o campo for nomeEstacao ou nomeLinha, utiliza-se a funcao dada ScanQuoteString()
        ScanQuoteString(strNomeEstacao);
        if (strcmp(strNomeEstacao, "") == 0) {
            data.nomeEstacao = NULL;
            data.tamNomeEstacao = 0;
        } else {
            data.nomeEstacao = strdup(strNomeEstacao);
            data.tamNomeEstacao = strlen(strNomeEstacao);
        }

        scanf("%s", strCodLinha);
        data.codLinha = (strcmp(strCodLinha, "NULO") == 0) ? -1 : atoi(strCodLinha);

        ScanQuoteString(strNomeLinha);
        if (strcmp(strNomeLinha, "") == 0) {
            data.nomeLinha = NULL;
            data.tamNomeLinha = 0;
        } else {
            data.nomeLinha = strdup(strNomeLinha);
            data.tamNomeLinha = strlen(strNomeLinha);
        }

        scanf("%s", strCodProx);
        data.codProxEstacao = (strcmp(strCodProx, "NULO") == 0) ? -1 : atoi(strCodProx);

        scanf("%s", strDist);
        data.distProxEstacao = (strcmp(strDist, "NULO") == 0) ? -1 : atoi(strDist);

        scanf("%s", strCodLinInt);
        data.codLinhaIntegra = (strcmp(strCodLinInt, "NULO") == 0) ? -1 : atoi(strCodLinInt);

        scanf("%s", strCodEstInt);
        data.codEstIntegra = (strcmp(strCodEstInt, "NULO") == 0) ? -1 : atoi(strCodEstInt);

        // Verifica duplicata no indice em RAM antes de inserir
        // Se o codEstacao ja existe, descarta esta insercao e continua para a proxima
        int duplicata = 0;
        for (int idx = 0; idx < qtdIndex; idx++) {
            if (indexList[idx].codEstacao == data.codEstacao) {
                duplicata = 1;
                break;
            }
        }
        if (duplicata) {
            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);
            continue;
        }

        // Checa se o nome e inedito no arquivo para atualizar o contador de estacoes unicas
        int nomeExiste = verifyName(binFile, data.nomeEstacao, -1);

        int targetRRN;
        long writeOffset;

        // ==========================================
        // Logica de Insercao: Pilha vs Fim do Arquivo
        // ==========================================
        if (header.topo != -1) {
            // Reaproveitamento do espaco livre apontado pelo topo da pilha
            targetRRN = header.topo;
            writeOffset = DATA_HEADER_SIZE + ((long)targetRRN * DATA_REGISTER_SIZE);

            // Pula o 'removido' (char) para ler o 'proximo' (int) e atualizar o topo
            fseek(binFile, writeOffset + 1, SEEK_SET);
            int proximoRemovido;
            fread(&proximoRemovido, sizeof(int), 1, binFile);
            header.topo = proximoRemovido; // topo passa a apontar para o proximo buraco
        } else {
            // Sem espaco reutilizavel, insere apos o ultimo registro
            targetRRN = header.proxRRN;
            writeOffset = DATA_HEADER_SIZE + ((long)targetRRN * DATA_REGISTER_SIZE);
            header.proxRRN++;
        }

        // Escrita do registro no disco na posicao calculada
        
        fseek(binFile, writeOffset, SEEK_SET);
        fwrite(&data.removido, sizeof(char), 1, binFile);
        fwrite(&data.proximo, sizeof(int), 1, binFile);
        fwrite(&data.codEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinha, sizeof(int), 1, binFile);
        fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
        fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if (data.tamNomeEstacao > 0) fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if (data.tamNomeLinha > 0) fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);

        // Preenche os bytes nao utilizados com lixo '$' para completar o tamanho fixo do registro
        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        for (int k = 0; k < garbageBytes; k++) fputc('$', binFile);

        // Atualiza os contadores do cabecalho
        if (!nomeExiste) header.nroEstacoes++;         // estacao com nome inedito -> incrementa
        if (data.codProxEstacao != -1) header.nroParesEstacao++; // tem proximo -> novo par -> incrementa

        // Atualiza o indice em RAM com o novo registro
        qtdIndex++;
        indexList = realloc(indexList, qtdIndex * sizeof(IndexRecord));
        indexList[qtdIndex - 1].codEstacao = data.codEstacao;
        indexList[qtdIndex - 1].RRN = targetRRN;

        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    } // Fim do for de 'n'

    // Ordenacao obrigatoria do indice apos todas as insercoes para manter a busca binaria valida
    qsort(indexList, qtdIndex, sizeof(IndexRecord), compairRegisters);

    // Atualiza o cabecalho completo e marca o arquivo como consistente
    header.status = '1';
    fseek(binFile, 0, SEEK_SET);
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);
    fclose(binFile);

    // Recria e sobrescreve o arquivo de indice a partir da lista em RAM, agora ordenada
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