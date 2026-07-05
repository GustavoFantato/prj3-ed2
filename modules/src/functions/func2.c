#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [2] - Listagem de todos os registros nao removidos #

-> Recuperacao dos dados de TODOS os registros armazenados em um arquivo de dados de entrada, mostrando os dados de forma organizada na saída padrão (dada graficamente no arquivo de instrucoes do projeto) para permitir a distincao dos campos e registros. 
-> Registros marcados como logicamente removidos nao devem ser exibidos 

*/

void listTable(char *arquivoEntrada) {
    
    // Abertura do arquivo binario para leitura
    FILE *binFile = fopen(arquivoEntrada, "rb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Leitura do status para verificar se o arquivo eh consistente
    char status;
    fread(&status, sizeof(char), 1, binFile);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(binFile);
        return;
    }

    // Posiciona o ponteiro apos o header, no inicio dos registros
    fseek(binFile, 16, SEEK_CUR);

    int foundReg = 0; // bool para controle de registro encontrado ou nao
    char removed; 

    // Enquanto nao chegar ao fim do arquivo, le-se o campo de removido para verificar se o registro esta ativo ou nao.
    while (fread(&removed, sizeof(char), 1, binFile) == 1) { 
        
        // Se == 1, eh porque foi removido
        if (removed == '1') {
            fseek(binFile, 79, SEEK_CUR); 
            continue; // Pula para o proximo registro
        }

        // Criacao do data record
        foundReg = 1; // Registro encontrado, sera utilizado posteriormente
        
        // Inicializacao do data record, atribuindo o valor do campo removido que ja foi lido
        DataRecord data;
        data.removido = removed; 
        
        lerRegistro(&data, binFile);

        // Guardar a quantidade de bytes de lixo
        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        fseek(binFile, garbageBytes, SEEK_CUR); // Posiciona o ponteiro para o inicio do proximo registro, pulando o lixo

        // Prints dos campos do registro

        printRegistro(data);

        // Liberacao das memorias alocadas dos campos variaveis
        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    }

    // Se o registros nao foi encontrado, printa a mensagem
    if (!foundReg) {
        printf("Registro inexistente.\n");
    }

    // Fechamento do arquivo
    fclose(binFile);
}