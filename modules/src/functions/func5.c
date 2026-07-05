#include "functions.h"
#include "utils.h"

/*
# FUNCIONALIDADE [5] - Criacao do arquivo de indice primario #
-> Usado para criar um índice sobre um campo (ou um conjunto de campos) de busca.
-> No momento da criação, o arquivo de índice primário deve possuir um registro de cabeçalho e vários registros de dados
*/

void createIndex(char *arquivoEntrada, char *arquivoSaida){

    // Alocacao e verificacao do arquivo de dados

    FILE *stationsFile = fopen(arquivoEntrada, "rb"); // declara e abre o arquivo passado
    if(stationsFile == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verificacao de consistencia do arquivo de dados
    char dataHeaderStatus;
    fread(&dataHeaderStatus, sizeof(char), 1, stationsFile);

    if(dataHeaderStatus == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(stationsFile);
        return;
    }

    // Cria structs para guardar os dados do arquivo novo a ser criado
    IndexRecord *indexList = NULL;
    int qtdRegisters = 0;
    int currentRRN = 0;

    // Precisamos pular o cabecalho, ir para o primeiro RRN
    fseek(stationsFile, DATA_HEADER_SIZE, SEEK_SET); // ponteiro do arquivo esta no campo de 'removido' do primeiro registro, de RRN 0

    char removed;

    // Loop vai ir lendo sempre o removido do RRN. Quando != 1, eh pq chegamos ao fim 
    while(fread(&removed, sizeof(char), 1, stationsFile) == 1){

    // verifica se registro removido. Se estiver removido nao escrevemos no indexFile
    if(removed == '1'){
        fseek(stationsFile, DATA_REGISTER_SIZE - 1, SEEK_CUR); // pula para o proximo RRN
        currentRRN++; 
        continue;
    }

    // Se nao removido, precisamos ler o registro atual
    DataRecord data;
    data.removido = removed;

    lerRegistro(&data, stationsFile);

    // Aloca novo elemento do array de structs
    qtdRegisters++;
    indexList = realloc(indexList, qtdRegisters * sizeof(IndexRecord));

    // Atribui os valores ao elemento atual
    indexList[qtdRegisters - 1].codEstacao = data.codEstacao;
    indexList[qtdRegisters - 1].RRN = currentRRN;

    // Calcula o lixo restante do registro e pula para o proximo registro
    int garbage = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);
    fseek(stationsFile, garbage, SEEK_CUR);

    currentRRN++;

    // Liberacao das memorias alocadas para os campos de string de tamanho variavel
    if(data.nomeEstacao != NULL){
        free(data.nomeEstacao);
    }
    if(data.nomeLinha != NULL){
        free(data.nomeLinha);
    }

    }

    // Fecha arquivo de entrada, ja que finalizamos a leitura dos dados 
    fclose(stationsFile);


    // Ordenacao do indexList 
    qsort(indexList, qtdRegisters, sizeof(IndexRecord), compairRegisters);

    // Agora que temos o vetor ordenado, basta escrevermos no arquivoSaida

    FILE *indexFile = fopen(arquivoSaida, "wb");

    if (indexFile == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    IndexHeader indexHeader;
    indexHeader.status = '0';

    // Escrevemos o header do index file
    fwrite(&indexHeader.status, sizeof(char), 1, indexFile);
    
    // Escrita de todos codEstacao e RRN no arquivo binario index
    for(int i = 0; i < qtdRegisters; i++){
        fwrite(&indexList[i].codEstacao, sizeof(int), 1, indexFile);
        fwrite(&indexList[i].RRN, sizeof(int), 1, indexFile);
    }

    // Finalizamos a escrita, agora mudamos o status para '1' 
    indexHeader.status = '1';
    fseek(indexFile, 0, SEEK_SET); // Posiciona ponteiro no primeiro byte
    fwrite(&indexHeader.status, sizeof(char), 1, indexFile); // Atualizamos status
    fclose(indexFile); // Fecha arquivo index

    // Liberacao de memoria da lista index
    free(indexList);

    binarioNaTela(arquivoSaida);
        
}