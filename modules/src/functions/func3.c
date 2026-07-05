#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [3] - Recuperacao dos dados dos registros que satisfacam um criteiro de busca, determinado pelo usuario
-> Qualquer campo pode ser usado como forma de busca
-> A busca deve ser feita considerando um ou mais campos 
-> Essa funcao pode retornar 0 registros (quando nenhum registro satisfaz o criterio), 1 registro (quando apenas um satisfaz) ou mais registros (quando um ou mais registros satisfazem o criterio)
-> Registros marcados como logicamente removidos nao devem ser exibidos
*/

void listTableWhere(char *arquivoEntrada, int n) {
    
    // Abertura do arquivo binario para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verificacao se o status do arquivo eh '1' (consistente)
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Loop para ler os n casos de busca
    for (int i = 0; i < n; i++) {
        
        // m eh o numero de campos a serem comparados para a busca
        int m;
        scanf("%d", &m); 

        // Campos e valores a serem comparados 
        char campos[8][30];
        char valores[8][100];
        int foundCodEst = 0;
        
        // Loop para ler os campos e valores, de acordo com o 'm' lido 
        for (int j = 0; j < m; j++) {
            scanf("%s", campos[j]); 

            if (strcmp(campos[j], "codEstacao") == 0) {
                foundCodEst = 1; 
            }

            // Se o campo for nomeEstacao ou nomeLinha, utiliza-se a funcao dada ScanQuoteString()
            if (strcmp(campos[j], "nomeEstacao") == 0 || strcmp(campos[j], "nomeLinha") == 0) {
                ScanQuoteString(valores[j]); 
            } 
            else { 
                scanf("%s", valores[j]);

                // Se nulo, coloca a string como vazia para facilitar a comparacao mais pra frente
                if (strcmp(valores[j], "NULO") == 0) {
                    strcpy(valores[j], "");
                }
            }
        }

        // Posiciona o ponteiro no inicio dos registros, apos o header
        fseek(binFile, DATA_HEADER_SIZE, SEEK_SET);

        int foundReg = 0; // Bool para controle de registro encontrado ou nao
        char removed; // armazenar o campo se o registro foi removido ou nao

        // Enquanto nao chegar ao fim do arquivo, le-se o campo de removido para verificar se o registro esta ativo ou nao.    
        while (fread(&removed, sizeof(char), 1, binFile) == 1) {
            
            // Se o campo foi removido pula para o proximo registro
            if (removed == '1') {
                fseek(binFile, 79, SEEK_CUR);
                continue;
            }

            // Criacao do data record, atribuindo o valor do campo removido que ja foi lido
            DataRecord data;
            data.removido = removed;

            lerRegistro(&data, binFile);

            // Guardar a quantidade de bytes de lixo, para posicionar o ponteiro no inicio do proximo registro
            int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
            fseek(binFile, garbageBytes, SEEK_CUR);

            int match = 1; 

            // Loop para comparar os campos lidos com os campos de busca, verificando se o registro atual do arquivo binario corresponde aos criterios de busca
            for (int f = 0; f < m; f++) {
                checkMatch(data, campos, f, valores, &match);
                
                // Se nao der match, quebra o loop, evitando comparacoes desnecessarias
                if (match == 0) break; 
            }

            // Se deu match, registro foi encontrado, printa os campos do registro
            if (match == 1) {
                foundReg = 1;
                printRegistro(data);
                
                // Libera a memoria alocada pelas strings antes do break ou fim do laco
                if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                if (data.nomeLinha != NULL) free(data.nomeLinha);

                // Se a busca tinha o ID unico e ele deu match, para a busca imediatamente
                if (foundCodEst) {
                    break; 
                }
            } else {
                // se nao deu match tambem precisa liberar a memoria
                if (data.nomeEstacao != NULL) free(data.nomeEstacao);
                if (data.nomeLinha != NULL) free(data.nomeLinha);
            }
        }

        // Se nao encontrou nenhum registro APÓS percorrer o arquivo todo, printa a mensagem
        if (!foundReg) {
            printf("Registro inexistente.\n");
        }

        // Formatacao adequada de print do '\n' para o runCodes
        if (!(i == n-1)){
            printf("\n");
        }
    }

    // Fecha o arquivo
    fclose(binFile);
}