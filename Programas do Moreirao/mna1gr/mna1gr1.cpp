//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mna1gr1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

/*
Programa de demonstra��o de an�lise nodal modificada
Por Antonio Carlos M. de Queiroz acmq@coe.ufrj.br
Vers�o 1.0 - 6/9/2000
Vers�o 1.0a - 8/9/2000 Ignora coment�rios
Vers�o 1.0b - 15/9/2000 Aumentado Yn, retirado Js
Vers�o 1.0c - 19/2/2001 Mais coment�rios
Vers�o 1.0d - 16/2/2003 Tratamento correto de nomes em min�sculas
Vers�o 1.0e - 21/8/2008 Estampas sempre somam. Ignora a primeira linha
Vers�o 1.0f - 21/6/2009 Corrigidos limites de aloca��o de Yn
Vers�o 1.0g - 15/10/2009 L� as linhas inteiras
Vers�o 1.0h - 18/6/2011 Estampas correspondendo a modelos
Vers�o 2.0 - 2/11/2013 Interface gr�fica
*/

/*
Elementos aceitos e linhas do netlist:

Resistor:  R<nome> <no+> <no-> <resist�ncia>
VCCS:      G<nome> <io+> <io-> <vi+> <vi-> <transcondut�ncia>
VCVC:      E<nome> <vo+> <vo-> <vi+> <vi-> <ganho de tens�o>
CCCS:      F<nome> <io+> <io-> <ii+> <ii-> <ganho de corrente>
CCVS:      H<nome> <vo+> <vo-> <ii+> <ii-> <transresist�ncia>
Fonte I:   I<nome> <io+> <io-> <corrente>
Fonte V:   V<nome> <vo+> <vo-> <tens�o>
Amp. op.:  O<nome> <vo1> <vo2> <vi1> <vi2>

As fontes F e H tem o ramo de entrada em curto
O amplificador operacional ideal tem a saida suspensa
Os n�s podem ser nomes
*/

#define versao "2.0 - 2/11/2013"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#define MAX_LINHA 80
#define MAX_NOME 11
#define MAX_ELEM 50
#define MAX_NOS 50
#define TOLG 1e-9

typedef struct elemento { /* Elemento do netlist */
  char nome[MAX_NOME];
  double valor;
  int a,b,c,d,x,y;
} elemento;

elemento netlist[MAX_ELEM]; /* Netlist */

int
  ne, /* Elementos */
  nv, /* Vari�veis */
  nn, /* N�s */
  i,j,k;

char
  nomearquivo[MAX_LINHA+1],
  tipo,
  na[MAX_NOME],nb[MAX_NOME],nc[MAX_NOME],nd[MAX_NOME],
  lista[MAX_NOS+1][MAX_NOME+2], /* Tem que caber jx antes do nome */
  txt[MAX_LINHA+1],buf[1000],tmp[1000],
  *p;
FILE *arquivo;

double
  g,
  Yn[MAX_NOS+1][MAX_NOS+2];

#define printf xprintf
void xprintf(char* format,...) /* Escreve no memo1 */
{
  va_list paramlist;
  char txt[2000];
  va_start(paramlist,format);
  vsprintf(txt,format,paramlist);
  Form1->Memo1->Lines->Add(txt);
}

/* Resolu��o de sistema de equa��es lineares.
   M�todo de Gauss-Jordan com condensa��o pivotal */
int resolversistema(void)
{
  int i,j,l, a;
  double t, p;

  for (i=1; i<=nv; i++) {
    t=0.0;
    a=i;
    for (l=i; l<=nv; l++) {
      if (fabs(Yn[l][i])>fabs(t)) {
	a=l;
	t=Yn[l][i];
      }
    }
    if (i!=a) {
      for (l=1; l<=nv+1; l++) {
	p=Yn[i][l];
	Yn[i][l]=Yn[a][l];
	Yn[a][l]=p;
      }
    }
    if (fabs(t)<TOLG) {
      printf("Sistema singular");
      return 1;
    }
    for (j=nv+1; j>0; j--) {  /* Basta j>i em vez de j>0 */
      Yn[i][j] /= t;
      p=Yn[i][j];
      for (l=1; l<=nv; l++) { /* Poderia n�o fazer se p=0 */
	if (l!=i)
	  Yn[l][j]-=Yn[l][i]*p;
      }
    }
  }
  return 0;
}

/* Rotina que conta os n�s e atribui n�meros a eles */
int numero(char *nome)
{
  int i,achou;

  i=0; achou=0;
  while (!achou && i<=nv)
    if (!(achou=!strcmp(nome,lista[i]))) i++;
  if (!achou) {
    if (nv==MAX_NOS) {
      printf("O programa s� aceita at� %d n�s",nv);
      exit(1);
    }
    nv++;
    strcpy(lista[nv],nome);
    return nv; /* novo n� */
  }
  else {
    return i; /* n� j� conhecido */
  }
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Abrir1Click(TObject *Sender)
{
 denovo:
  /* Leitura do netlist */
  ne=0; nv=0; strcpy(lista[0],"0");
  if (!OpenDialog1->Execute()) return;
  strcpy(nomearquivo,OpenDialog1->FileName.c_str());
  arquivo=fopen(nomearquivo,"r");
  if (arquivo==0) {
    printf("Arquivo %s inexistente",nomearquivo);
    goto denovo;
  }
  printf("Lendo netlist:");
  fgets(txt,MAX_LINHA,arquivo);
  printf("T�tulo: %s",txt);
  while (fgets(txt,MAX_LINHA,arquivo)) {
    ne++; /* N�o usa o netlist[0] */
    if (ne>MAX_ELEM) {
      printf("O programa so aceita at� %d elementos",MAX_ELEM);
      exit(1);
    }
    txt[0]=toupper(txt[0]);
    tipo=txt[0];
    sscanf(txt,"%10s",netlist[ne].nome);
    p=txt+strlen(netlist[ne].nome); /* In�cio dos par�metros */
    /* O que � lido depende do tipo */
    if (tipo=='R' || tipo=='I' || tipo=='V') {
      sscanf(p,"%10s%10s%lg",na,nb,&netlist[ne].valor);
      printf("%s %s %s %g",netlist[ne].nome,na,nb,netlist[ne].valor);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
    }
    else if (tipo=='G' || tipo=='E' || tipo=='F' || tipo=='H') {
      sscanf(p,"%10s%10s%10s%10s%lg",na,nb,nc,nd,&netlist[ne].valor);
      printf("%s %s %s %s %s %g",netlist[ne].nome,na,nb,nc,nd,netlist[ne].valor);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
      netlist[ne].c=numero(nc);
      netlist[ne].d=numero(nd);
    }
    else if (tipo=='O') {
      sscanf(p,"%10s%10s%10s%10s",na,nb,nc,nd);
      printf("%s %s %s %s %s",netlist[ne].nome,na,nb,nc,nd);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
      netlist[ne].c=numero(nc);
      netlist[ne].d=numero(nd);
    }
    else if (tipo=='*') { /* Coment�rio come�a com "*" */
      printf("Coment�rio: %s",txt);
      ne--;
    }
    else {
      printf("Elemento desconhecido: %s",txt);
      getch();
      exit(1);
    }
  }
  fclose(arquivo);
  /* Acrescenta vari�veis de corrente acima dos n�s, anotando no netlist */
  nn=nv;
  for (i=1; i<=ne; i++) {
    tipo=netlist[i].nome[0];
    if (tipo=='V' || tipo=='E' || tipo=='F' || tipo=='O') {
      nv++;
      if (nv>MAX_NOS) {
        printf("As correntes extra excederam o n�mero de vari�veis permitido (%d)",MAX_NOS);
        exit(1);
      }
      strcpy(lista[nv],"j"); /* Tem espa�o para mais dois caracteres */
      strcat(lista[nv],netlist[i].nome);
      netlist[i].x=nv;
    }
    else if (tipo=='H') {
      nv=nv+2;
      if (nv>MAX_NOS) {
        printf("As correntes extra excederam o n�mero de vari�veis permitido (%d)",MAX_NOS);
        exit(1);
      }
      strcpy(lista[nv-1],"jx"); strcat(lista[nv-1],netlist[i].nome);
      netlist[i].x=nv-1;
      strcpy(lista[nv],"jy"); strcat(lista[nv],netlist[i].nome);
      netlist[i].y=nv;
    }
  }
  /* Lista tudo */
  printf("Vari�veis internas: ");
  for (i=0; i<=nv; i++)
    printf("%d -> %s",i,lista[i]);
  if (Mostrarestampas1->Checked) {
    printf("Netlist interno final");
    for (i=1; i<=ne; i++) {
      tipo=netlist[i].nome[0];
      if (tipo=='R' || tipo=='I' || tipo=='V') {
        printf("%s %d %d %g",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].valor);
      }
      else if (tipo=='G' || tipo=='E' || tipo=='F' || tipo=='H') {
        printf("%s %d %d %d %d %g",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].c,netlist[i].d,netlist[i].valor);
      }
      else if (tipo=='O') {
        printf("%s %d %d %d %d",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].c,netlist[i].d);
      }
      if (tipo=='V' || tipo=='E' || tipo=='F' || tipo=='O')
        printf("Corrente jx: %d",netlist[i].x);
      else if (tipo=='H')
        printf("Correntes jx e jy: %d, %d",netlist[i].x,netlist[i].y);
    }
  }
  /* Monta o sistema nodal modificado */
  printf("O circuito tem %d n�s, %d vari�veis e %d elementos",nn,nv,ne);
  getch();
  /* Zera sistema */
  for (i=0; i<=nv; i++) {
    for (j=0; j<=nv+1; j++)
      Yn[i][j]=0;
  }
  /* Monta estampas */
  for (i=1; i<=ne; i++) {
    tipo=netlist[i].nome[0];
    if (tipo=='R') {
      g=1/netlist[i].valor;
      Yn[netlist[i].a][netlist[i].a]+=g;
      Yn[netlist[i].b][netlist[i].b]+=g;
      Yn[netlist[i].a][netlist[i].b]-=g;
      Yn[netlist[i].b][netlist[i].a]-=g;
    }
    else if (tipo=='G') {
      g=netlist[i].valor;
      Yn[netlist[i].a][netlist[i].c]+=g;
      Yn[netlist[i].b][netlist[i].d]+=g;
      Yn[netlist[i].a][netlist[i].d]-=g;
      Yn[netlist[i].b][netlist[i].c]-=g;
    }
    else if (tipo=='I') {
      g=netlist[i].valor;
      Yn[netlist[i].a][nv+1]-=g;
      Yn[netlist[i].b][nv+1]+=g;
    }
    else if (tipo=='V') {
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].a]-=1;
      Yn[netlist[i].x][netlist[i].b]+=1;
      Yn[netlist[i].x][nv+1]-=netlist[i].valor;
    }
    else if (tipo=='E') {
      g=netlist[i].valor;
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].a]-=1;
      Yn[netlist[i].x][netlist[i].b]+=1;
      Yn[netlist[i].x][netlist[i].c]+=g;
      Yn[netlist[i].x][netlist[i].d]-=g;
    }
    else if (tipo=='F') {
      g=netlist[i].valor;
      Yn[netlist[i].a][netlist[i].x]+=g;
      Yn[netlist[i].b][netlist[i].x]-=g;
      Yn[netlist[i].c][netlist[i].x]+=1;
      Yn[netlist[i].d][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].c]-=1;
      Yn[netlist[i].x][netlist[i].d]+=1;
    }
    else if (tipo=='H') {
      g=netlist[i].valor;
      Yn[netlist[i].a][netlist[i].y]+=1;
      Yn[netlist[i].b][netlist[i].y]-=1;
      Yn[netlist[i].c][netlist[i].x]+=1;
      Yn[netlist[i].d][netlist[i].x]-=1;
      Yn[netlist[i].y][netlist[i].a]-=1;
      Yn[netlist[i].y][netlist[i].b]+=1;
      Yn[netlist[i].x][netlist[i].c]-=1;
      Yn[netlist[i].x][netlist[i].d]+=1;
      Yn[netlist[i].y][netlist[i].x]+=g;
    }
    else if (tipo=='O') {
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].c]+=1;
      Yn[netlist[i].x][netlist[i].d]-=1;
    }
    /* Mostra sistema ap�s a montagem da estampa */
    if (Mostrarestampas1->Checked) {
      printf("Sistema ap�s a estampa de %s",netlist[i].nome);
      for (k=1; k<=nv; k++) {
        strcpy(buf,"");
        for (j=1; j<=nv+1; j++) {
          if (Yn[k][j]!=0) sprintf(tmp,"%+6.2f ",Yn[k][j]);
          else sprintf(tmp,"...... ");
          strcat(buf,tmp);
        }
        printf(buf);
      }
    }
  }
  /* Resolve o sistema */
  if (resolversistema()) return;
  /* Mostra o sistema resolvido */
  if (Mostrarestampas1->Checked) {
    printf("Sistema resolvido:");
    for (k=1; k<=nv; k++) {
      strcpy(buf,"");
      for (j=1; j<=nv+1; j++) {
        if (Yn[k][j]!=0) sprintf(tmp,"%+6.2f ",Yn[k][j]);
        else sprintf(tmp,"...... ");
        strcat(buf,tmp);
      }
      printf(buf);
    }
  }
  /* Mostra solu��o */
  printf("Solu��o:");
  strcpy(txt,"Tens�o");
  for (i=1; i<=nv; i++) {
    if (i==nn+1) strcpy(txt,"Corrente");
    printf("%s %s: %g",txt,lista[i],Yn[i][nv+1]);
  }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
  printf("Vers�o %s",versao);
}
//---------------------------------------------------------------------------

