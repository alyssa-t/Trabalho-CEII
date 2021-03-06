#include "global.h"
#include "funcoesAuxiliares.h"

using namespace std;

elemento netlist[MAX_ELEM];
int BotarGSNesseElemento[MAX_ELEM];
int repete;
int ne;
int nv;
int nn;
int i;
int j;
int k;
unsigned long long GIndutorCurto;
long double GCapacitorAberto;
int analisandoPontodeOp;
char nomearquivo[MAX_LINHA+1];
char tipo;
char na[MAX_NOME];
char nb[MAX_NOME];
char nc[MAX_NOME];
char nd[MAX_NOME];
char lista[MAX_NOS+1][MAX_NOME+2];
char txt[MAX_LINHA+1];
char *p;
FILE *arquivoNetlist;
FILE *arquivoSolucao;
double g, pulseRealTime, pulseOffTime;
double Yn[MAX_NOS+1][MAX_NOS+2];
double NewtonRaphsonVetor[MAX_NOS+1];
double ValoresNaoConvergindo[MAX_NOS+1];
double ValoresConvergiu[MAX_NOS+1];
double UltimaConvergenciaNoTempo[MAX_NOS+1];
//int NewtonRaphsonTentativas;
int NewtonRaphsonTentarNovamente;
int erroGrande;
double z;
int fazendoGminStepping;
 long double gs;
 long double ultimogs;
 long double fatordeDiv10;
 int counter;
int PrimeiraVezNR;
int circuitolinear;
/*variavel para analise no tempo*/
double tempoAtual, tempoFinal, passo, passoPorPt;
double contadorGS;
int convergiu;
int main()
{

  cout << endl << "Programa de Analise Nodal Modificada - AlyTSPICE" << endl;
  cout << "Analise no dominio do tempo com metodo de integracao dos trapezios" << endl;
  cout << "\t\t\tAlyssa de Oliveira Takazume\n\t\t\tMatheus Silva de Lima" << endl;
  printf("\t\t __/\\  /\\  /\\  /\\__\n");
  printf("\t\t     \\/  \\/  \\/    \n");

  cout <<"\t    /_\\ |  \\ /  ---  S I E"<<endl;
  cout <<"\t___/   \\|__ |    |    P C "<<endl;


  /* 1 - Leitura do netlist */
  //denovo:
  int erro;
  vector <double> resultadoUmTempo;
  vector <char*> primeiraLinhaTab;
  //abri o aarquivo para escrever os resultados
  //GIndutorCurto = 1844674407370955161;    <- Esse numero eh o maior possivel, mas da problema de convergencia
  //GCapacitorAberto = 1.11e-16;       <- Esse numero eh o menor possivel, mas da problema de convergencia
  GIndutorCurto = GINDUTORCURTO;
  GCapacitorAberto = GCAPACITORABERTO;
  fazendoGminStepping=0;
  convergiu = 0;
  circuitolinear=1;

  do{
    erro = leNetlist();
  }while (erro != 0);

  /* Acrescenta variaveis de corrente acima dos nos, anotando no netlist */
  nn=nv;
  for (i=1; i<=ne; i++) {
    tipo=netlist[i].nome[0];

    /*se forem os tipos que precisam de UMA corrente pra montar o mna,
    o numero de variavel vai aumentar em 1, alem dos variaveis de nos*/
    if (tipo=='V' || tipo=='E' || tipo=='F' || tipo=='O' || tipo=='K') {
      nv++;
      if (nv>MAX_NOS) {
        printf("As correntes extra excederam o numero de variaveis permitido (%d)\n",MAX_NOS);
        exit(1);
      }
      strcpy(lista[nv],"j"); /* Tem espaco para mais dois caracteres */
      strcat(lista[nv],netlist[i].nome);
      netlist[i].x=nv;
    }
    /*o tipo H, q eh uma tensao ligada a corrente, precisam de 2 variaveis
    de corrente para construir a estampa*/
    else if (tipo=='H') {
      nv=nv+2;
      if (nv>MAX_NOS) {
        printf("As correntes extra excederam o numero de variaveis permitido (%d)\n",MAX_NOS);
        exit(1);
      }
      strcpy(lista[nv-1],"jx"); strcat(lista[nv-1],netlist[i].nome);
      netlist[i].x=nv-1;
      strcpy(lista[nv],"jy"); strcat(lista[nv],netlist[i].nome);
      netlist[i].y=nv;
    }
  }
  getch();
  /* Lista tudo */
  #ifdef DEBUG
      printf("'Variaveis internas: \n");
        for (i=0; i<=nv; i++)
          printf("%d -> %s\n",i,lista[i]);
        getch();
        printf("Netlist interno final\n");
        for (i=1; i<=ne; i++) {
          tipo=netlist[i].nome[0];
          if (tipo=='R' || tipo=='I' || tipo=='V' || tipo =='C' || tipo == 'L') {
            printf("%s %d %d %g\n",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].valor);
          }
          else if (tipo=='G' || tipo=='E' || tipo=='F' || tipo=='H' || tipo=='K') {
            printf("%s %d %d %d %d %g\n",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].c,netlist[i].d,netlist[i].valor);
          }
          else if (tipo=='O') {
            printf("%s %d %d %d %d\n",netlist[i].nome,netlist[i].a,netlist[i].b,netlist[i].c,netlist[i].d);
          }
          if (tipo=='V' || tipo=='E' || tipo=='F' || tipo=='O')
            printf("Corrente jx: %d\n",netlist[i].x);
          else if (tipo=='H')
            printf("Correntes jx e jy: %d, %d\n",netlist[i].x,netlist[i].y);
        }
        getch();
  #endif
  /* Monta o sistema nodal modificado */
  #ifdef DEBUG
      printf("O circuito tem %d nos, %d variaveis e %d elementos\n",nn,nv,ne);
      getch();
  #endif

  /*Loop responsavel pelo TEMPO*/

  arquivoSolucao=fopen(NOME_ARQUIVO_TAB,"w");

  tempoAtual = 0;

  //Inicia a analise com um calculo de ponto de operacao. Estava com preguiça e por enquanto sistema inicia com jt0 = 0 e vt0 = 0

    zeraSistema();
    analisePontoOperacao();
    CopiaUltimaSolucaoNoTempo();

    analisandoPontodeOp=0;
    /*for (i=0; i<=nv; i++){
      cout<<Yn[i] <<endl;
    }cout<<"\n"<<endl;*/


  while(tempoAtual <= tempoFinal)
  {
    //  Metodo de resolucao
    //  0 - Inicializa valores de tensao e corrente nos capacitores e indutores com uma analise de ponto de operacao com Newton-Raphson
    //  1 - Zera sistema nodal
    //  2 - Copia Solucao anterior do Newton-Raphson
    //  3 - Caso o Newton-Raphson esteja demorando para convergir, reinicia o Newton-Raphson com valores chutados
    //  4 - Caso o número de reinicializacoes do Newton-Raphson exceda um limite, inicia gminstepping
    //  5 - Monta & Resolve sistema nodal modificado
    //  6 - Caso o Newton-Raphson não convirja, voltar a '2'
    //  7 - Atualiza valor das memorias de corrente e tensao nos capacitores e indutores
    //  8 - Salva resultado em tabela
    //  9 - Volta a (1) e repete até o fim da analise


    /* Zera sistema */
    zeraSistema();

    /* Monta estampas & Resolve com algorítimo de Newton-Raphson*/
    /*Ainda possui espaço para melhoras, para análise no tempo sem elementos nao lineares
      ele acaba tendo que resolver o mesmo sistema duas vezes para poder passar o instante de tempo*/
    /*a b c d sao posicoes relacionadas aos nos
      x e y sao relacionadas as correntes, que entram na estampa pelos
      "pontilhadinhos" qnd a gente faz mna na mao*/

      if (circuitolinear==1)
      {
        zeraSistema();  //zera, monta e resolve
        montarEstampas();
        if (resolversistema())
        {
          getch();
          exit(0);
        }
      }

      else{
        PrimeiraVezNR =1;
        if (!analiseNR()){
          cout <<"nao convergiu"<<endl;
          getch();
          exit(0);
        }
        CopiaUltimaSolucaoNoTempo();
        //cout<<"copiado!";
        //mostraResultadoParcial();
        //NOTA1: ADICIONAR UM VETOR DE "ULTIMA CONVERGENCIA" AQUI

      }

        /*
      do{
          if ((NewtonRaphsonTentativas==0) && (NewtonRaphsonTentarNovamente ==0)){
            ChutaValorNR();
          //  jaFizIsso+=1;
          }
          else{
            CopiaSolucaoNR();   //copia solucao anterior
          }

          if(NewtonRaphsonTentativas > NEWTONRAPHSON_NUMERO_MAX_TENTATIVAS)
          {   //Caso o numero de interacoes do newton-raphson tenha excedido um valor, chuta valores aleatorios
            ChutaValorNR();
            NewtonRaphsonTentativas = 0;
            NewtonRaphsonTentarNovamente++;
            if(NewtonRaphsonTentarNovamente == NEWTONRAPHSON_NUMERO_MAX_TENTARNOVAMENTE)
            { //caso ele tenha reiniciado o algoritimo vezes demais, inicia gminstepping
              ZeraValorNR();
              gs = CONDUTANCIA_INICIAL_GS;
              gminstepping();
              fazendoGminStepping=0;
              //getch();
            //  exit(0);
          //  cout << "sai do gminstep mesmooooo, gs="<< gs<< endl;
          //  fazendoGminStepping = 0;
            }
          }
          zeraSistema();  //zera, monta e resolve
          montarEstampas();
          if (resolversistema())
          {
            getch();
            exit(0);
          }

          NewtonRaphsonTentativas++;
            //cout << NewtonRaphsonTentativas<< endl;
        }while(ComparaValorNR() == 0 );  //repete isso ate newton-raphson convergir
*/

    /* Atualiza as memorias nos capacitores e indutores */
    /*apos a resolucao do sistema nodal, a gente precisa atualizar o valor dos parametros vto e jto de cada capacitor/indutor;*/
    atualizarMemoriasCapacitorIndutor();

    #ifdef DEBUG  /* Opcional: Mostra o sistema resolvido */
    printf("Sistema resolvido:\n");
    for (i=1; i<=nv; i++)
    {
      for (j=1; j<=nv+1; j++)
        if (Yn[i][j]!=0) printf("%+3.1f ",Yn[i][j]);
        else printf(" ... ");
        printf("\n");
    }
    getch();
    #endif

    /*aqui vc escreve oo cabecalho para tab*/
    if (tempoAtual==0){
      fprintf (arquivoSolucao, "%s ", "t" );
      for (i=1; i<=nv; i++){
        fprintf (arquivoSolucao, "%s ", lista[i] );
      }
      fprintf (arquivoSolucao, "\n");
    }

    printProgresso(PROGRESSO_PORCENTAGEM,PROGRESSO_SIMBOLO); //printa quantos porcento da simulacao ja ocorreu (a cada 1%, com simbolo '*')

    resultadoUmTempo.push_back(tempoAtual);
    for (i=1; i<=nv; i++){
    //printf("tempo atual = %g, %g\n", tempoAtual, Yn[i][nv+1]);
    resultadoUmTempo.push_back(Yn[i][nv+1]);
    }

    salvarResultadoEmArquivo(resultadoUmTempo);
    resultadoUmTempo.clear();
/*    resultadoDeTodosOsPassos.push_back(resultadoUmTempo);

    for (i=0; i < resultadoDeTodosOsPassos.size(); i++){
      for (j=0; j< resultadoUmTempo.size(); j++){
        printf("%g ", resultadoUmTempo.at(j));
      }
      printf("\n");
    }
*/

  tempoAtual+=passo;

  } /*FIM loop de tempo*/

  /* Mostra solucao */
  fclose(arquivoSolucao);

  cout << "Circuito simulado com sucesso!" << endl;
  //printf("Solucao:\n");
  //strcpy(txt,"Tensao");
  //for (i=1; i<=nv; i++) {
  //  if (i==nn+1) strcpy(txt,"Corrente");
  //  printf("%s %s: %g\n",txt,lista[i],Yn[i][nv+1]);
  //}
  plotarGrafico();
  getch();
  cout << "おはよう！" << endl;
  return 0;
}
