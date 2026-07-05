#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [4] - Listagem de um registro a partir do RRN #
-> Recuperacao dos dados de um registro de um arquivo a partir da identificacao do RRN do registro desejado.
-> Registros marcados como logicamente removidos nao devem ser exibidos
*/

void listTableRRN(char *arquivoEntrada, int RRN) {
    
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

    // Calculo do byte offset a partir do RRN, para colocar o ponteiro no inicio do registro a ser buscado
    int byteOffset = DATA_HEADER_SIZE + (RRN * DATA_REGISTER_SIZE);
    fseek(binFile, byteOffset, SEEK_SET);

    // Leitura e verificacao se o registro foi removido ou nao
    char removed;
    if (fread(&removed, sizeof(char), 1, binFile) != 1 || removed == '1') {
        printf("Registro inexistente.\n");
        fclose(binFile);
        return;
    }

    // Criacao do data record, atribuindo o valor do campo removido que ja foi lido
    DataRecord data;
    data.removido = removed; 
    
    lerRegistro(&data, binFile);

    printRegistro(data);

    // Liberacao das memorias alocadas dos campos variaveis
    if (data.nomeEstacao != NULL) free(data.nomeEstacao);
    if (data.nomeLinha != NULL) free(data.nomeLinha);

    // Fecha o arquivo
    fclose(binFile);
}
