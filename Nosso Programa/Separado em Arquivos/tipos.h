#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "constantes.h"

#ifndef _TIPOS_H_
#define _TIPOS_H_

typedef struct elemento /* Elemento do netlist */
{
  char nome[MAX_NOME];
  double valor; /*usado tbm como DC "valor", SIN "nivel continuo", PULSE "amplitude1"*/
  int a,b,c,d,x,y;
  /*a partir daqui so vai usar se for uma fonte que n eh dc: I ou V que tem <parametro>*/
  char fonte[10]; //se eh DC, SIN, ou PULSE
  double amplitude /*anplitude2 para pulse*/, freq, atraso, amortecimento,
          defasagem, ciclo, tempoSubida, tempoDescida, tempoLigada, periodo;
} elemento;

#endif
