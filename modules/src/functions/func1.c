#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [1] - Criacao do arquivo binario a partir do CSV estacoes #

-> Leitura de varios registros obtidos a partir de um arquivo de entrada .csv e armazenamento desses registros em um arquivo de dados binário
*/

void createTable(char *arquivoEntrada, char *arquivoSaida) {

    // Abertura dos arquivos
    FILE *csvFile = fopen(arquivoEntrada, "r");
    if (csvFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *binFile = fopen(arquivoSaida, "wb");
    if (binFile == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(csvFile);
        return;
    }

    char line[LINE_SIZE]; // Buffer para leitura de cada linha do CSV

    // Inicializacao dos campos do cabecalho
    HeaderRecord header;
    header.status = '0';
    header.topo = -1;
    header.proxRRN = 0;
    header.nroEstacoes = 0;
    header.nroParesEstacao = 0;

    // Escrita dos campos do cabecalho
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    // A primeira linha do arquivo deve ser ignorada, pois eh o cabecalho do CSV
    readCSVLine(line, LINE_SIZE, csvFile); 
    
    // Inicializacao das variaveis para controle de quantidade de estacoes diferentes e pares
    char **diffStationNames = NULL; // Ponteiro duplo pois trata-se de um array dinamico de strings
    Par *listPares = NULL;

    // readCSVLine eh uma funcao que le a linha do CSV. Retorna NULL quando chega ao fim do arquivo
    while (readCSVLine(line, LINE_SIZE, csvFile) != NULL)   {

        // Inicializacao dos campos do registro de dados
        DataRecord data;
        data.removido = '0';
        data.proximo = -1;
        data.codEstacao = -1;
        data.codLinha = -1;
        data.codProxEstacao = -1;
        data.distProxEstacao = -1;
        data.codLinhaIntegra = -1;
        data.codEstIntegra = -1;
        data.tamNomeEstacao = 0;
        data.tamNomeLinha = 0;
        data.nomeEstacao = NULL;
        data.nomeLinha = NULL;

        // Cria-se copias da linha, para evitar que a strsep modifique o buffer original
        char *lineCopy = strdup(line);
        char *ptr = lineCopy; // Ponteiro para iterar sobre a linha

        for (int i = 0; i < DATA_REGISTER_QTD; i++){ // Loop para ler cada field da linha
            char *field = strsep(&ptr, ",");
            if (field != NULL) {
                field[strcspn(field, "\r\n")] = '\0'; // encontra o indice onde possa haver um \r ou \n e substitui por \0
                switchDataRecord(&data, i, field); // switch para associar o indice com o campo lido no momento, atribuindo esse valor a struct do datarecord
            }        
        }

        // Escrita do registro de dados no arquivo bin
        fwrite(&data.removido, sizeof(char), 1, binFile);
        fwrite(&data.proximo, sizeof(int), 1, binFile);
        fwrite(&data.codEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinha, sizeof(int), 1, binFile);
        fwrite(&data.codProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.distProxEstacao, sizeof(int), 1, binFile);
        fwrite(&data.codLinhaIntegra, sizeof(int), 1, binFile);
        fwrite(&data.codEstIntegra, sizeof(int), 1, binFile);
  

        // Nao se deve escrever nada caso o tamanho seja 0. Sera tratado, no fim, na escrita do lixo com '$'
        fwrite(&data.tamNomeEstacao, sizeof(int), 1, binFile);
        if(data.tamNomeEstacao > 0) {
            fwrite(data.nomeEstacao, sizeof(char), data.tamNomeEstacao, binFile);
        }
        
        fwrite(&data.tamNomeLinha, sizeof(int), 1, binFile);
        if(data.tamNomeLinha > 0) {
            fwrite(data.nomeLinha, sizeof(char), data.tamNomeLinha, binFile);
        }

        // Tratamento do lixo, escrevendo '$' nos bytes nao utilizados
        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
        for(int i = 0; i < garbageBytes; i++){
            fputc('$', binFile); 
        }

        // Verificacao das estacoes e pares diferentes, para controle do cabecalho
        verifyIfDiffStation(data.nomeEstacao, &diffStationNames, &header.nroEstacoes);
        if(data.codProxEstacao != -1){
        verifyIfDiffPair(data.codEstacao, data.codProxEstacao, &listPares, &header.nroParesEstacao);
        }
        
        // Soma o proximo RRN, porque o registro foi escrito no arquivo 
        header.proxRRN++;

        // Liberacao das memorias alocadas
        free(data.nomeEstacao); 
        if (data.nomeLinha != NULL) free(data.nomeLinha);
        free(lineCopy);  
    }

    // Atualizacao do header
    header.status = '1';

    // Posiciona o ponteiro no inicio para atualizar o header, ja que a quantidade de estacoes e pares diferentes so foi definida apos a leitura de todo o CSV
    fseek(binFile, 0, SEEK_SET); 
    fwrite(&header.status, sizeof(char), 1, binFile);
    fwrite(&header.topo, sizeof(int), 1, binFile);
    fwrite(&header.proxRRN, sizeof(int), 1, binFile);
    fwrite(&header.nroEstacoes, sizeof(int), 1, binFile);
    fwrite(&header.nroParesEstacao, sizeof(int), 1, binFile);

    // Liberacao das memorias alocadas para controle de estacoes e pares
    for (int i = 0; i < header.nroEstacoes; i++) {
        free(diffStationNames[i]); // cada string do vetor
    }
    free(diffStationNames); // o vetor em si
    if (listPares != NULL) free(listPares);

    // Fechamento dos arquivos
    fclose(binFile);
    fclose(csvFile);   

    // Print da soma dos binarios
    binarioNaTela(arquivoSaida);
}