#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [9] - Atualizacao de registro de dados #
-> Desde que os registros de dados sao de tamanho fixo, a atualizacao deve ser feita diretamente no registro existente que nao esteja marcado como removido
*/

void updateTable(char *arquivoDados, char *arquivoIndex, int n) {
    
    // Abertura do arquivo binario de dados para leitura e escrita
    FILE *binFile = fopen(arquivoDados, "rb+");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verificacao se o status do arquivo de dados eh '1' (consistente)
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Abertura do arquivo binario de indice para leitura
    FILE *indexFile = fopen(arquivoIndex, "rb");
    if (indexFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Verificacao se o status do arquivo de indice eh '1' (consistente)
    char indexStatus;
    fread(&indexStatus, sizeof(char), 1, indexFile);
    if (indexStatus == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        fclose(indexFile);
        return;
    }

    // 1. Marcar arquivo de dados como inconsistente ('0') durante a transacao
    status = '0';
    fseek(binFile, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, binFile);

    // 2. Carregar o Indice para a RAM
    // Posiciona o ponteiro no inicio dos registros de indice, apos o header
    fseek(indexFile, INDEX_HEADER_SIZE, SEEK_SET);
    IndexRecord *indexList = NULL;
    int qtdIndex = 0;
    IndexRecord tempIdx;
    
    // Loop para ler todos os registros do arquivo de indice e armazenar no array dinamico
    while (fread(&tempIdx.codEstacao, sizeof(int), 1, indexFile) == 1) {
        fread(&tempIdx.RRN, sizeof(int), 1, indexFile);
        qtdIndex++;
        indexList = realloc(indexList, qtdIndex * sizeof(IndexRecord));
        indexList[qtdIndex - 1] = tempIdx;
    }
    fclose(indexFile); // Fecha arquivo de indice, pois ele sera recriado e reescrito no final

    // 3. Processar as 'n' atualizacoes
    // Loop para iterar os casos de update solicitados pelo usuario
    for (int i = 0; i < n; i++) {
        
        int m; // Quantidade de criterios de busca
        if (scanf("%d", &m) != 1) break;

        char camposBusca[8][30];
        char valoresBusca[8][100];
        
        // Bool para definir se usaremos a busca direta no array de indice em RAM
        int useIndexFile = 0;
        int indiceCodEst = -1; // Facilitar saber qual indice guarda o codEstacao

        // Scanner Hibrido para os campos de BUSCA
        // Loop para ler os 'm' campos e valores a serem comparados
        for (int j = 0; j < m; j++) {
            scanf("%s", camposBusca[j]); 
            if (strcmp(camposBusca[j], "codEstacao") == 0) {
                useIndexFile = 1;
                indiceCodEst = j;
            }     
            
            // Se o campo for string, utiliza-se a funcao ScanQuoteString()
            if (strcmp(camposBusca[j], "nomeEstacao") == 0 || strcmp(camposBusca[j], "nomeLinha") == 0) {
                ScanQuoteString(valoresBusca[j]); 
            } else { 
                scanf("%s", valoresBusca[j]);
                // Se nulo, coloca a string como vazia para facilitar a comparacao de match
                if (strcmp(valoresBusca[j], "NULO") == 0) {
                    strcpy(valoresBusca[j], "");
                }
            }
        }

        int p; // Quantidade de campos a serem atualizados
        scanf("%d", &p);

        char camposAtualiza[8][30];
        char valoresAtualiza[8][100];

        // Scanner Hibrido para os campos de ATUALIZACAO
        // Loop para ler os 'p' campos e os novos valores a serem escritos
        for (int j = 0; j < p; j++) {
            scanf("%s", camposAtualiza[j]); 
            if (strcmp(camposAtualiza[j], "nomeEstacao") == 0 || strcmp(camposAtualiza[j], "nomeLinha") == 0) {
                ScanQuoteString(valoresAtualiza[j]); 
            } else { 
                scanf("%s", valoresAtualiza[j]);
                if (strcmp(valoresAtualiza[j], "NULO") == 0) {
                    strcpy(valoresAtualiza[j], "");
                }
            }
        }

        // ==========================================
        // Logica de Varredura e Atualizacao
        // ==========================================
        
        // Posiciona o ponteiro no inicio dos registros de dados, apos o header
        fseek(binFile, DATA_HEADER_SIZE, SEEK_SET);
        int currentRRN = 0;
        char removed;

        // Vamos varrer o arquivo inteiro. Se usar indice, podemos otimizar o salto para O(1).
        if (useIndexFile) {
            int codBusca = atoi(valoresBusca[indiceCodEst]);
            int targetRRN = -1;
            
            // Busca o RRN correspondente ao codEstacao procurado na nossa RAM
            for (int idx = 0; idx < qtdIndex; idx++) {
                if (indexList[idx].codEstacao == codBusca) {
                    targetRRN = indexList[idx].RRN;
                    break;
                }
            }
            if (targetRRN == -1) continue; // Nao achou no indice, pula pra proxima iteracao de 'n'
            
            currentRRN = targetRRN;
            // Posiciona ponteiro no registro desejado, de acordo com o RRN
            fseek(binFile, DATA_HEADER_SIZE + (targetRRN * DATA_REGISTER_SIZE), SEEK_SET);
        }

        // Enquanto nao chegar ao fim do arquivo, le-se o campo de removido para verificar se o registro esta ativo
        while (fread(&removed, sizeof(char), 1, binFile) == 1) {
            
            // Se o campo foi removido pula para o proximo registro
            if (removed == '1') {
                fseek(binFile, DATA_REGISTER_SIZE - 1, SEEK_CUR);
                currentRRN++;
                continue;
            }

            // Salva o offset do registro atual para podermos voltar e sobrescreve-lo In-Place depois
            long currentOffset = ftell(binFile) - 1; 

            // Criacao do data record, atribuindo o valor do campo removido que ja foi lido
            DataRecord data;
            data.removido = removed;
            lerRegistro(&data, binFile);

            // Guardar a quantidade de bytes de lixo, para posicionar o ponteiro no inicio do proximo registro
            int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
            fseek(binFile, garbageBytes, SEEK_CUR);

            // Verifica se o registro atual da match com os criterios de busca
            int match = 1; 
            for (int f = 0; f < m; f++) {
                checkMatch(data, camposBusca, f, valoresBusca, &match);
                // Se nao der match, quebra o loop, evitando comparacoes desnecessarias
                if (!match) break;
            }

            // Se deu match (registro foi encontrado), aplicamos os novos valores!
            if (match) {
                long nextRegOffset = ftell(binFile); // Salva de onde o while deve continuar a varredura
                
                int alterouCodEstacao = 0; // Flag para atualizar a chave primaria no indice
                int oldCodEstacao = data.codEstacao;

                // Aplica os novos valores de 'p' na struct que esta na RAM
                for (int k = 0; k < p; k++) {
                    if (strcmp(camposAtualiza[k], "codEstacao") == 0) {
                        data.codEstacao = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                        alterouCodEstacao = 1;
                    } 
                    else if (strcmp(camposAtualiza[k], "codLinha") == 0) {
                        data.codLinha = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                    }
                    else if (strcmp(camposAtualiza[k], "codProxEstacao") == 0) {
                        data.codProxEstacao = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                    }
                    else if (strcmp(camposAtualiza[k], "distProxEstacao") == 0) {
                        data.distProxEstacao = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                    }
                    else if (strcmp(camposAtualiza[k], "codLinhaIntegra") == 0) {
                        data.codLinhaIntegra = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                    }
                    else if (strcmp(camposAtualiza[k], "codEstIntegra") == 0) {
                        data.codEstIntegra = (strcmp(valoresAtualiza[k], "") == 0) ? -1 : atoi(valoresAtualiza[k]);
                    }
                    else if (strcmp(camposAtualiza[k], "nomeEstacao") == 0) {
                        // Limpa a string antiga e aloca a nova
                        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                        if (strcmp(valoresAtualiza[k], "") == 0) {
                            data.nomeEstacao = NULL;
                            data.tamNomeEstacao = 0;
                        } else {
                            data.nomeEstacao = strdup(valoresAtualiza[k]);
                            data.tamNomeEstacao = strlen(valoresAtualiza[k]);
                        }
                    }
                    else if (strcmp(camposAtualiza[k], "nomeLinha") == 0) {
                        if (data.nomeLinha != NULL) free(data.nomeLinha);
                        if (strcmp(valoresAtualiza[k], "") == 0) {
                            data.nomeLinha = NULL;
                            data.tamNomeLinha = 0;
                        } else {
                            data.nomeLinha = strdup(valoresAtualiza[k]);
                            data.tamNomeLinha = strlen(valoresAtualiza[k]);
                        }
                    }
                }

                // Volta o ponteiro para o inicio do registro atual e sobrescreve In-Place
                fseek(binFile, currentOffset, SEEK_SET);

                // Escrita sequencial dos campos de tamanho fixo
                fwrite(&data.removido, sizeof(char), 1, binFile);
                fwrite(&data.proximo, sizeof(int), 1, binFile);
                fwrite(&data.codEstacao, sizeof(int), 1, binFile);
                fwrite(&data.codLinha, sizeof(int), 1, binFile);
                fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
                fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
                fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
                fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
                
                // Escrita sequencial dos campos de tamanho variavel
                fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);
                if (data.tamNomeEstacao > 0) fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
                
                fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
                if (data.tamNomeLinha > 0) fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);

                // Recalcula o lixo com base nos novos tamanhos de string e preenche a sobra com '$'
                int newGarbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
                for(int k = 0; k < newGarbageBytes; k++){
                    fputc('$', binFile); 
                }

                // Atualiza a chave no Indice em RAM caso o codEstacao tenha sido modificado
                if (alterouCodEstacao) {
                    for (int idx = 0; idx < qtdIndex; idx++) {
                        if (indexList[idx].codEstacao == oldCodEstacao && indexList[idx].RRN == currentRRN) {
                            indexList[idx].codEstacao = data.codEstacao;
                            break;
                        }
                    }
                }

                // Restaura o ponteiro para continuar lendo os proximos registros do while
                fseek(binFile, nextRegOffset, SEEK_SET); 

                // Se a busca inicial usou Indice (que garante achado unico), podemos quebrar o while aqui
                if (useIndexFile) {
                    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                    if (data.nomeLinha != NULL) free(data.nomeLinha);
                    break; 
                }
            }
            
            // Liberacao de memoria das strings iteradas
            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);

            currentRRN++;
        }
    }

    // Ordenacao obrigatoria do Indice (Pois chaves primarias podem ter sido alteradas)
    qsort(indexList, qtdIndex, sizeof(IndexRecord), compairRegisters);

    // 4. FLUSH FINAL DAS INFORMACOES NO DISCO
    
    // Atualiza cabecalho de dados para status '1' (consistente)
    status = '1';
    fseek(binFile, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, binFile);
    fclose(binFile);

    // Abertura do arquivo de indice para reescrita total (wb)
    indexFile = fopen(arquivoIndex, "wb");
    
    // Escrevemos o header do index file como '0'
    char indexStatusOut = '0';
    fwrite(&indexStatusOut, sizeof(char), 1, indexFile);
    
    // Escrita de todos codEstacao e RRN atualizados no arquivo binario index
    for (int i = 0; i < qtdIndex; i++) {
        fwrite(&indexList[i].codEstacao, sizeof(int), 1, indexFile);
        fwrite(&indexList[i].RRN, sizeof(int), 1, indexFile);
    }
    
    // Finalizamos a escrita, agora mudamos o status para '1'
    indexStatusOut = '1';
    fseek(indexFile, 0, SEEK_SET);
    fwrite(&indexStatusOut, sizeof(char), 1, indexFile);
    fclose(indexFile); // Fecha arquivo index

    // Liberacao de memoria da lista index
    if (indexList != NULL) free(indexList);

    // Saida obrigatoria pro runCodes
    binarioNaTela(arquivoDados);
    binarioNaTela(arquivoIndex);
}