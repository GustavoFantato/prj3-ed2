#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "structs.h"

#define LINE_SIZE 1200 

// Protótipos das funções para cada funcionalidade

// --== @ ==-- 
// PARTE 1 DO TRABALHO
// --== @ ==-- 

// FUNCIONALIDADE [1]
void createTable(char *arquivoEntrada, char *arquivoSaida);

// FUNCIONALIDADE [2]
void listTable(char *arquivoEntrada);

// FUNCIONALIDADE [3]
void listTableWhere(char *arquivoEntrada, int n);

// FUNCIONALIDADE [4]
void listTableRRN(char *arquivoEntrada, int RRN);


// --== @ ==-- 
// PARTE 2 DO TRABALHO
// --== @ ==-- 

// FUNCIONALIDADE [5]
void createIndex(char *arquivoEntrada, char *arquivoSaida);

// FUNCIONALIDADE [6]
void listTableWhereIndex(char *arquivoDados, char *arquivoIndex, int n);

// FUNCIONALIDADE [7]
void deleteFromTable(char *arquivoDados, char *arquivoIndex, int n);

// FUNCIONALIDADE [8]
void insertIntoTable(char *arquivoDados, char *arquivoIndex, int n);

// FUNCIONALIDADE [9]
void updateTable(char *arquivoDados, char *arquivoIndex, int n);

// FUNCIONALIDADE [10]
void createAndListGraph(char *arquivoDados);

// FUNCIONALIDADE [11]
void getShortestPath(char *arquivoEntrada, char *nomeEstacaoOrigem, char *valorOrigem, char *nomeEstacaoDestino, char *valorDestino);

// FUNCIONALIDADE [12]
void improveSubwayNetwork(char *arquivoEntrada, char *nomeEstacaoOrigem, char *valorOrigem);

// FUNCIONALIDADE [13]
void countCyclesFromOrigin(char *arquivoEntrada, char *nomeEstacaoOrigem, char *valorOrigem);
#endif