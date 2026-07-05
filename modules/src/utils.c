#include "structs.h"
#include "utils.h"

/*
# UTILIDADES GERAIS - Codigos reutilizaveis para auxiliar na execucao das funcionalidades #
*/


// FUNCOES GERAIS

void printRegistro(DataRecord data){
        
        printf("%d ", data.codEstacao);

        if (data.nomeEstacao != NULL) printf("%s ", data.nomeEstacao); else printf("NULO ");
        if (data.codLinha == -1) printf("NULO "); else printf("%d ", data.codLinha);
        if (data.nomeLinha != NULL) printf("%s ", data.nomeLinha); else printf("NULO ");
        if (data.codProxEstacao == -1) printf("NULO "); else printf("%d ", data.codProxEstacao);
        if (data.distProxEstacao == -1) printf("NULO "); else printf("%d ", data.distProxEstacao);
        if (data.codLinhaIntegra == -1) printf("NULO "); else printf("%d ", data.codLinhaIntegra);
        if (data.codEstIntegra == -1) printf("NULO\n"); else printf("%d\n", data.codEstIntegra);
}

void lerRegistro(DataRecord *data, FILE *binFile){

    // Leitura dos demais campos, seguindo a ordem definida no projeto
    fread(&data->proximo, sizeof(int), 1, binFile);
    fread(&data->codEstacao, sizeof(int), 1, binFile);
    fread(&data->codLinha, sizeof(int), 1, binFile);
    fread(&data->codProxEstacao, sizeof(int), 1, binFile);
    fread(&data->distProxEstacao, sizeof(int), 1, binFile);
    fread(&data->codLinhaIntegra, sizeof(int), 1, binFile);
    fread(&data->codEstIntegra, sizeof(int), 1, binFile);

    // Se tamanho eh 0, nao deve ler o nomeEstacao, apenas atribuir NULL
    fread(&data->tamNomeEstacao, sizeof(int), 1, binFile);
    if (data->tamNomeEstacao > 0) {
        // Aloca memoria ao nomeEstacao de acordo com o tamanho lido + 1, para o '\0' do final
        data->nomeEstacao = malloc((data->tamNomeEstacao + 1) * sizeof(char));
        fread(data->nomeEstacao, sizeof(char), data->tamNomeEstacao, binFile);
        data->nomeEstacao[data->tamNomeEstacao] = '\0'; 
    } else {
        data->nomeEstacao = NULL;
    }
    // Se tamanho eh 0, nao deve ler o nomeLinha, apenas atribuir NULL
    fread(&data->tamNomeLinha, sizeof(int), 1, binFile);
    if (data->tamNomeLinha > 0) {
        // Aloca memoria o nomeLinha de acordo com o tamanho lido + 1, para o '\0' do final
        data->nomeLinha = malloc((data->tamNomeLinha + 1) * sizeof(char));
        fread(data->nomeLinha, sizeof(char), data->tamNomeLinha, binFile);
        data->nomeLinha[data->tamNomeLinha] = '\0';
    } else {
        data->nomeLinha = NULL;
    }

}


// FUNCOES UTILIZADAS NA FUNCIONALIDADE [1]
// Ao fim da funcionalidade, printa a soma dos bytes do arquivo binario
void binarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

// Leitura de cada linha do arquivo CSV, retornando NULL quando chega ao fim do arquivo
char *readCSVLine(char line[], int line_size, FILE *csvFile){   
    return fgets(line, line_size, csvFile); // Se chega ao fim ja retorna NULL naturalmente
}

// Verifica se o field da linha do CSV eh nulo
int verifyIfNullField(char *field){
    if (field == NULL || field[0] == '\0'){
        return 1;
    }
    return 0;
}

// Switch para atribuir os campos do data record de acordo com o index do campo lido do CSV, ja que a ordem dos campos no CSV eh fixa e conhecida
void switchDataRecord(DataRecord *data, int i, char *field){
    switch(i){
        case 0: 
            data->codEstacao = atoi(field);
            break;

        case 1: 
            // Dado no projeto que o nomeEstacao nao aceita valor nulo, por isso nao verifica-se
            data->tamNomeEstacao = strlen(field);
            data->nomeEstacao = strdup(field); // Cria-se uma copia do field para evitar que seja sobrescrito posteriormente
            break;

        case 2: 
            if(verifyIfNullField(field)) data->codLinha = -1; 
            else data->codLinha = atoi(field);
            break;

        case 3: 
            if(verifyIfNullField(field)){
                data->tamNomeLinha = 0; 
                data->nomeLinha = NULL;
            } else {
                data->tamNomeLinha = strlen(field); 
                data->nomeLinha = strdup(field); // Cria-se uma copia do field para evitar que seja sobrescrito posteriormente
            }
            break;

        case 4:
            if(verifyIfNullField(field)) data->codProxEstacao = -1; 
            else data->codProxEstacao = atoi(field);
            break;

        case 5: 
            if(verifyIfNullField(field)) data->distProxEstacao = -1; 
            else data->distProxEstacao = atoi(field);
            break;

        case 6:
            if(verifyIfNullField(field)) data->codLinhaIntegra = -1; 
            else data->codLinhaIntegra = atoi(field);
            break;

        case 7: 
            if(verifyIfNullField(field)) data->codEstIntegra = -1; 
            else data->codEstIntegra = atoi(field);
            break;
    }
}

// Funcao para verificar se o nome da estacao lida do arquivo binario ja foi encontrada anteriormente, para controle do nroEstacoes no header
void verifyIfDiffStation(char *name, char ***diffStationNames, int *stationsQtd){
   
    // Loop para verificar se o nome da estacao ja esta no array de estacoes diferentes
    for (int i = 0; i < *stationsQtd; i++){
        if (strcmp(name, (*diffStationNames)[i]) == 0){ 
            return; // Se encontrado apenas retorna
        }
    }

    // Se saiu do loop, nao foi encontrado. Entao eh uma estacao nova
    (*stationsQtd)++; // Incrementa a quantidade de estacoes diretamente no ponteiro para que seja atualizada na funcao que chamou

    // Cria um ponteiro temporario para realocar o tamanho do array de estacoes diferentes, para conseguir adicionar o novo nome encontrado
    char **temp = realloc(*diffStationNames, sizeof(char*) * (*stationsQtd)); 
    
    // Se ocorrer um problema ao realocar, nao atualiza o ponteiro e retorna
    if (temp == NULL){
        printf("Falha ao alocar ponteiro para controle de estacoes diferentes.\n");
        (*stationsQtd)--;
        return;   
    }
    
    // Atualiza o ponteiro do array de estacoes diferentes e adiciona o novo nome
    *diffStationNames = temp;
    (*diffStationNames)[(*stationsQtd) - 1] = strdup(name);
    return;
}

// Funcao para verificar se o par de estacao ja foi encontrado anteriormente no CSV
void verifyIfDiffPair(int orig, int dest, Par **listPar, int *nPares){

    // Loop para verificar se o par ja esta no array de pares diferentes
    for (int i = 0; i < *nPares; i++){
        if ((*listPar)[i].orig == orig && (*listPar)[i].dest == dest){
            return; 
        }
    }

    // Se nao estiver no array, eh um par diferente e deve ser adicionado
    (*nPares)++;

    // Cria-se um ponteiro temporario para realocar o tamanho do array de pares, para conseguir adicionar o novo par encontrado
    Par *temp = realloc(*listPar, sizeof(Par) * (*nPares));
    
    // Se ocorrer um problema ao realocar, nao atualiza o ponteiro e retorna
    if (temp == NULL){
        printf("Falha ao alocar ponteiro para controle de pares diferentes.\n");
        (*nPares)--;
        return;   
    }
    
    // Atualiza o ponteiro do array de pares diferentes e adiciona o novo par
    *listPar = temp;
    (*listPar)[(*nPares) - 1].orig = orig;
    (*listPar)[(*nPares) - 1].dest = dest;
    return;
}


// Usadas na funcionalidade [3]

// Funcao dada para ler os campos iniciados com ""
void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

// Funcao para pegar e retornar o valor inteiro de um campo, verificando se eh nulo ou nao
int getValue(char *valor) {
    if (strcmp(valor, "") == 0) {
        return -1;
    }
    return atoi(valor); // converte para int
}

// Funcao para verificar se a string lida da match com a string de busca, considerando o caso de busca por campo nulo (string de busca vazia)
int checkStringMatch(char *data_string, char *valor_busca) {
    if (strcmp(valor_busca, "") == 0) {
        return (data_string == NULL);
    }
    if (data_string == NULL) {
        return 0;
    }
    return (strcmp(data_string, valor_busca) == 0);
}

void checkMatch(DataRecord data, char campos[][30], int f, char valores[][100], int *match){
    
    if (strcmp(campos[f], "codEstacao") == 0) {
    if (data.codEstacao != getValue(valores[f])) *match = 0;
    } 
    else if (strcmp(campos[f], "codLinha") == 0) {
    if (data.codLinha != getValue(valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "codProxEstacao") == 0) {
    if (data.codProxEstacao != getValue(valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "distProxEstacao") == 0) {
    if (data.distProxEstacao != getValue(valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "codLinhaIntegra") == 0) {
    if (data.codLinhaIntegra != getValue(valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "codEstIntegra") == 0) {
    if (data.codEstIntegra != getValue(valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "nomeEstacao") == 0) {
    if (!checkStringMatch(data.nomeEstacao, valores[f])) *match = 0;
    }
    else if (strcmp(campos[f], "nomeLinha") == 0) {
    if (!checkStringMatch(data.nomeLinha, valores[f])) *match = 0;
    }
    
}



// Usadas na funcionalidade [5]

// Funcao usada para o qsort() conseguir aplicar a logica de ordenacao dele
int compairRegisters(const void *a, const void *b){

    // Criamos dois ponteiros para forçarmos os genericos a serem do tipo struct do indexRecord

    IndexRecord *registerA = (IndexRecord *) a;
    IndexRecord *registerB = (IndexRecord *) b;

    return registerA->codEstacao - registerB->codEstacao; 
}



// Usadas na funcionalidade[6]

int getRRNIndexFile(char *arquivoIndex, int codEstacao){
    
    FILE *indexFile = fopen(arquivoIndex, "rb"); // abre arquivo index
    
    // Checando consistencia do arquivo index
    char status;
    fread(&status, sizeof(char), 1, indexFile);
    if(status == '0'){
        fclose(indexFile);
        return -2;
    }

    IndexRecord reg; // Aloca struct de register
    while(fread(&reg, sizeof(IndexRecord), 1, indexFile) == 1){ // Ja le e atribui os valores a struct do register
        if(reg.codEstacao == codEstacao){ // Encontramos o codEstacao desejado, retorna o RRN
            fclose(indexFile);
            return reg.RRN;
        }
    }

    // Se saiu do while, nao temos estacao com esse codigo
    fclose(indexFile);
    return -1; // valor nao encontrado 

}

// Usadas na funcionalidade[7]

int verifyName(FILE *binFile, const char *nomeBuscado, long offsetIgnorado) {

    if (nomeBuscado == NULL) return 0;

    int exist = 0;

    long posAtual = ftell(binFile);
    fseek(binFile, DATA_HEADER_SIZE, SEEK_SET);


    char removed;
    while (fread(&removed, sizeof(char), 1, binFile) == 1) {

        long currentOffset = ftell(binFile) - 1;

        if (removed == '1' || currentOffset == offsetIgnorado) {
            fseek(binFile, DATA_REGISTER_SIZE - 1, SEEK_CUR);
            continue;
        }

        DataRecord data;
        data.removido = removed;

        lerRegistro(&data, binFile);

        int garbageBytes = DATA_REGISTER_SIZE - (DATA_FIX_SIZE_FIELDS + data.tamNomeEstacao + data.tamNomeLinha);

        fseek(binFile, garbageBytes, SEEK_CUR);

        if (data.nomeEstacao != NULL && strcmp(data.nomeEstacao, nomeBuscado) == 0) {
            exist = 1;
            if (data.nomeEstacao != NULL) free(data.nomeEstacao);
            if (data.nomeLinha != NULL) free(data.nomeLinha);
            break;
        }


        if (data.nomeEstacao != NULL) free(data.nomeEstacao);
        if (data.nomeLinha != NULL) free(data.nomeLinha);
    }

    fseek(binFile, posAtual, SEEK_SET);

    return exist;
}