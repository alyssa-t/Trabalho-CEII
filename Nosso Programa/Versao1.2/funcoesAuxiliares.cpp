/* Monta estampas */
#include "funcoesAuxiliares.h"
#include "global.h"

using namespace std;

void estampas(char tipo)
{
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
  else if (tipo=='I')
  {
    if (strcmp(netlist[i].fonte, "DC") == 0)
    /*
    Fonte DC - Nao muda com o tempoAtual
    */
    {
        g=netlist[i].valor;
        Yn[netlist[i].a][nv+1]-=g;
        Yn[netlist[i].b][nv+1]+=g;
    }
    else
    if (strcmp(netlist[i].fonte, "SIN") == 0)
    /*
    Fonte SIN - Muda de acordo com:
    off+Vp*exp(-dec*(tempoAtual-atraso))*sin(2*pi*f*(tempoAtual-atraso)+(pi/180)*fase)
    */
    {
      g =
        (
          netlist[i].valor +
          (
            netlist[i].amplitude*exp(-1*netlist[i].amortecimento*(tempoAtual-netlist[i].atraso)) *
            sin(2*PI*netlist[i].freq*(tempoAtual-netlist[i].atraso) + (PI/180)*netlist[i].defasagem)
          )*( heaviside(tempoAtual-netlist[i].atraso) -
              heaviside((tempoAtual-netlist[i].atraso) - (2*PI/netlist[i].freq)*netlist[i].ciclo) )
        );

        Yn[netlist[i].a][nv+1]-=g;
        Yn[netlist[i].b][nv+1]+=g;

    }
    if (strcmp(netlist[i].fonte, "PULSE") == 0){
    /*
    Fonte PULSE - Muda de acordo com:
    add expressao aqui
    */
    //A fonte de tensão é sempre implementado pela estampa:
    //Yn[netlist[i].a][nv+1]-=g;
    //Yn[netlist[i].b][nv+1]+=g;
    // onde g é o valor da fonte no tempo atual

      pulseRealTime=tempoAtual-netlist[i].atraso;
      pulseRealTime= fmod(pulseRealTime,netlist[i].periodo);
      pulseOffTime=netlist[i].periodo- (netlist[i].tempoSubida+netlist[i].tempoDescida+netlist[i].tempoLigada);
      if (tempoAtual<netlist[i].atraso){

          Yn[netlist[i].a][nv+1]-=netlist[i].valor;
          Yn[netlist[i].b][nv+1]+=netlist[i].valor;
      }

      else if (tempoAtual> (netlist[i].atraso +(netlist[i].ciclo*netlist[i].periodo) ) ) {

        Yn[netlist[i].a][nv+1]-=netlist[i].valor;
        Yn[netlist[i].b][nv+1]+=netlist[i].valor;

      }

      else {
        if (pulseRealTime<netlist[i].tempoSubida ){ /* subindo*/
          Yn[netlist[i].a][nv+1]-=((((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoSubida)*pulseRealTime)+ netlist[i].valor);
          Yn[netlist[i].b][nv+1]+=((((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoSubida)*pulseRealTime)+ netlist[i].valor);
        }
        else if (pulseRealTime< (netlist[i].tempoSubida+netlist[i].tempoLigada)){
          Yn[netlist[i].a][nv+1]-=netlist[i].amplitude;
          Yn[netlist[i].b][nv+1]+=netlist[i].amplitude;
        }
        else if (pulseRealTime< (netlist[i].tempoSubida+netlist[i].tempoLigada+netlist[i].tempoDescida)){
          Yn[netlist[i].a][nv+1]-= (netlist[i].amplitude-
            (((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoDescida)*
            (pulseRealTime-(netlist[i].tempoSubida+netlist[i].tempoLigada))));

          Yn[netlist[i].b][nv+1]+= (netlist[i].amplitude-
              (((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoDescida)*
              (pulseRealTime-(netlist[i].tempoSubida+netlist[i].tempoLigada))));
        }
        else {
          Yn[netlist[i].a][nv+1]-=netlist[i].valor;
          Yn[netlist[i].b][nv+1]+=netlist[i].valor;
        }

      }
    }
  }
  else if (tipo=='V')   /*A estampa de V deve mudar de acordo com o tempoAtual*/
  {
    if (strcmp(netlist[i].fonte, "DC") == 0)
    /*
    Fonte DC - Nao muda com o tempoAtual
    */
    {
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].a]-=1;
      Yn[netlist[i].x][netlist[i].b]+=1;
      Yn[netlist[i].x][nv+1]-=netlist[i].valor;
    }
    else
    if (strcmp(netlist[i].fonte, "SIN") == 0)
    /*
    Fonte SIN - Muda de acordo com:
    off+Vp*exp(-dec*(tempoAtual-atraso))*sin(2*pi*f*(tempoAtual-atraso)+(pi/180)*fase)
    */
    {
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].a]-=1;
      Yn[netlist[i].x][netlist[i].b]+=1;
      Yn[netlist[i].x][nv+1]-=
        (
          netlist[i].valor +
          (
            netlist[i].amplitude*exp(-1*netlist[i].amortecimento*(tempoAtual-netlist[i].atraso)) *
            sin(2*PI*netlist[i].freq*(tempoAtual-netlist[i].atraso) + (PI/180)*netlist[i].defasagem)
          )*( heaviside(tempoAtual-netlist[i].atraso) -
              heaviside((tempoAtual-netlist[i].atraso) - (2*PI/netlist[i].freq)*netlist[i].ciclo) )
        );
    }
    if (strcmp(netlist[i].fonte, "PULSE") == 0){
    /*
    Fonte PULSE - Muda de acordo com:
    add expressao aqui
    */
    //Yn[netlist[i].x][nv+1]-=netlist[i].valor;
      Yn[netlist[i].a][netlist[i].x]+=1;
      Yn[netlist[i].b][netlist[i].x]-=1;
      Yn[netlist[i].x][netlist[i].a]-=1;
      Yn[netlist[i].x][netlist[i].b]+=1;

      pulseRealTime=tempoAtual-netlist[i].atraso;
      pulseRealTime= fmod(pulseRealTime,netlist[i].periodo);
      pulseOffTime=netlist[i].periodo- (netlist[i].tempoSubida+netlist[i].tempoDescida+netlist[i].tempoLigada);
      if (tempoAtual<netlist[i].atraso){
          Yn[netlist[i].x][nv+1]-=netlist[i].valor;
      }

      else if (tempoAtual> (netlist[i].atraso +(netlist[i].ciclo*netlist[i].periodo) ) ) {
        Yn[netlist[i].x][nv+1]-=netlist[i].valor;
      }

      else {
        if (pulseRealTime<netlist[i].tempoSubida ){ /* subindo*/
          Yn[netlist[i].x][nv+1]-=((((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoSubida)*pulseRealTime)+ netlist[i].valor);
        }
        else if (pulseRealTime< (netlist[i].tempoSubida+netlist[i].tempoLigada)){
          Yn[netlist[i].x][nv+1]-=netlist[i].amplitude;
        }
        else if (pulseRealTime< (netlist[i].tempoSubida+netlist[i].tempoLigada+netlist[i].tempoDescida)){
          Yn[netlist[i].x][nv+1]-= (netlist[i].amplitude-
            (((netlist[i].amplitude-netlist[i].valor)/netlist[i].tempoDescida)*
            (pulseRealTime-(netlist[i].tempoSubida+netlist[i].tempoLigada))));
        }
        else {
          Yn[netlist[i].x][nv+1]-=netlist[i].valor;
        }

      }
    }
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
  else if (tipo=='K') {     /*Transformador Ideal*/
    g=netlist[i].valor;
    Yn[netlist[i].a][netlist[i].x]-=g;
    Yn[netlist[i].b][netlist[i].x]+=g;
    Yn[netlist[i].c][netlist[i].x]+=1;
    Yn[netlist[i].d][netlist[i].x]-=1;

    Yn[netlist[i].x][netlist[i].a]+=g;
    Yn[netlist[i].x][netlist[i].b]-=g;
    Yn[netlist[i].x][netlist[i].c]-=1;
    Yn[netlist[i].x][netlist[i].d]+=1;
  }
}

int resolversistema(void)
{
  int i,j,l,a;
  double t, p;

  for (i=1; i<=nv; i++)
  {
    t=0.0;
    a=i;
    for (l=i; l<=nv; l++)
    {
      if (fabs(Yn[l][i])>fabs(t))
      {
	       a=l;
	       t=Yn[l][i];
      }
    }
    if (i!=a)
    {
      for (l=1; l<=nv+1; l++)
      {
	       p=Yn[i][l];
	       Yn[i][l]=Yn[a][l];
	       Yn[a][l]=p;
      }
    }
    if (fabs(t)<TOLG)
    {
      printf("Sistema singular\n");
      return 1;
    }
    for (j=nv+1; j>0; j--)
    {  /* Basta j>i em vez de j>0 */
      Yn[i][j]/= t;
      p=Yn[i][j];
      if (p!=0)  /* Evita operacoes com zero */
        for (l=1; l<=nv; l++)
        {
	         if (l!=i)
	         Yn[l][j]-=Yn[l][i]*p;
        }
    }
  }
  return 0;
}

int numero(char *nome)
{
  int i,achou;
  i=0; achou=0;
  while (!achou && i<=nv)
    if (!(achou=!strcmp(nome,lista[i]))) i++;
  if (!achou) {
    if (nv==MAX_NOS)
    {
      printf("O programa so aceita ate %d nos\n",nv);
      exit(1);
    }
    nv++;
    strcpy(lista[nv],nome);
    return nv; /* novo no */
  }
  else
  {
    return i; /* no ja conhecido */
  }
}

void zeraSistema (void) {
  for (i=0; i<=nv; i++)
    for (j=0; j<=nv+1; j++)
      Yn[i][j]=0;
}

double heaviside(double t)
{
  if (t >= 0)
    return 1;
  else
    return 0;
}

void salvarResultadoEmArquivo (vector <double> resultadoUmTempo)
{
  //  ofstream arquivo;
     //arquivo.open ("analise no tempo.txt", ios::app);
     for (i=0; i < static_cast <int> (resultadoUmTempo.size()); i++){
       fprintf (arquivoSolucao, "%g ",resultadoUmTempo.at(i) );
     }
     fprintf (arquivoSolucao, "\n");
    // arquivo.close();
}

int leNetlist (void){
  ne=0; nv=0; strcpy(lista[0],"0");
  printf("Nome do arquivo com o netlist (ex: mna.net): ");
  scanf("%50s",nomearquivo);
  arquivoNetlist=fopen(nomearquivo,"r");
  if (arquivoNetlist==0) {
    printf("Arquivo %s inexistente\n",nomearquivo);
    return 1;
  }

  printf("Lendo netlist:\n");
  fgets(txt,MAX_LINHA,arquivoNetlist);
  printf("Titulo: %s",txt);
  while (fgets(txt,MAX_LINHA,arquivoNetlist)) {
    ne++; /* Nao usa o netlist[0] */
    if (ne>MAX_ELEM) {
      printf("O programa so aceita ate %d elementos\n",MAX_ELEM);
      exit(1);
    }
    /*transforma o primeiro caractere da linha lida em letra maiuscula,
    para associar com algum tipo.
    ex: R = Resistor*/
    txt[0]=toupper(txt[0]);
    tipo=txt[0];
    sscanf(txt,"%10s",netlist[ne].nome);
    p=txt+strlen(netlist[ne].nome); /* Inicio dos parametros */
    /* dependendo do tipo que esta sendo lida, a forma de preencher
    a netlist eh diferente. aqui ele agrupa os tipos que tem os mesmos
    tratamentos e separa com if */
    if (tipo=='R') {
      sscanf(p,"%10s%10s%lg",na,nb,&netlist[ne].valor);
      printf("%s %s %s %g\n",netlist[ne].nome,na,nb,netlist[ne].valor);
      /*a funcao "numero" converte o nó que o usuario nomeou em um numero int
      e coloca esse numero no netlist do elemento.
      caracteriza em qual no o componente esta ligado.
      Ex: Resistor ligado a no1 e no2, a=1, b=2*/
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
    }

    else if (tipo=='I' || tipo=='V') {
      /*pega os nos que as fontes estao ligadas, e o tipo de fonte
      Ex: fonte DC, fonte senoidal*/
      sscanf(p,"%10s%10s%10s",na,nb,netlist[ne].fonte);
      /*esse +3 eh para mover o ponteiro p que aponta
      para as posicoes da linha "txt" no lugar certo.
      representa os 3 espacos q o sscanf pulou ate agr*/
      p=txt+strlen(netlist[ne].nome)+strlen(na)+strlen(nb)
      +strlen(netlist[ne].fonte)+1+1+1;

      /*se nivel DC*/
      if (strcmp(netlist[ne].fonte, "DC") == 0){
        sscanf(p,"%lg", &netlist[ne].valor);
        printf("%s %s %s %g\n",netlist[ne].nome,na,nb,netlist[ne].valor);
      }
      /*se for sin*/
      else if (strcmp(netlist[ne].fonte, "SIN") == 0){
        sscanf(p, "%lg%lg%lg%lg%lg%lg%lg", &netlist[ne].valor,
                                                &netlist[ne].amplitude,
                                                &netlist[ne].freq,
                                                &netlist[ne].atraso,
                                                &netlist[ne].amortecimento,
                                                &netlist[ne].defasagem,
                                                &netlist[ne].ciclo);
        printf("%s %s %s %g ",netlist[ne].nome,na,nb,netlist[ne].valor);
        printf("%lg %lg\n", netlist[ne].amplitude, netlist[ne].freq );
      }
      /*se for pulso*/
      else if (strcmp(netlist[ne].fonte, "PULSE") == 0){
        sscanf(p, "%lg %lg %lg %lg %lg %lg %lg %lg", &netlist[ne].valor,
                                                &netlist[ne].amplitude,
                                                &netlist[ne].atraso,
                                                &netlist[ne].tempoSubida,
                                                &netlist[ne].tempoDescida,
                                                &netlist[ne].tempoLigada,
                                                &netlist[ne].periodo,
                                              &netlist[ne].ciclo);
        printf("%s %s %s %g ",netlist[ne].nome,na,nb,netlist[ne].valor);
        printf("%lg %lg\n", netlist[ne].amplitude, netlist[ne].atraso );

      }
      /*se n for nada*/
      else{
        printf("Fonte desconhecida - Tente DC ou SIN\n");
        getch();
        exit(1);
      }
      /*a funcao "numero" converte o nó que o usuario nomeou em um numero int
      e coloca esse numero no netlist do elemento.
      caracteriza em qual no o componente esta ligado.
      Ex: Resistor ligado a no1 e no2, a=1, b=2*/
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
    }
    else if (tipo=='G' || tipo=='E' || tipo=='F' || tipo=='H') {
      sscanf(p,"%10s%10s%10s%10s%lg",na,nb,nc,nd,&netlist[ne].valor);
      printf("%s %s %s %s %s %g\n",netlist[ne].nome,na,nb,nc,nd,netlist[ne].valor);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
      netlist[ne].c=numero(nc);
      netlist[ne].d=numero(nd);
    }
    else if (tipo=='O') {
      sscanf(p,"%10s%10s%10s%10s",na,nb,nc,nd);
      printf("%s %s %s %s %s\n",netlist[ne].nome,na,nb,nc,nd);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
      netlist[ne].c=numero(nc);
      netlist[ne].d=numero(nd);
    }
    else if (tipo=='K') {         /*Transmador Ideal - 07/10/2017*/
      sscanf(p,"%10s%10s%10s%10s%lg",na,nb,nc,nd,&netlist[ne].valor);
      printf("%s %s %s %s %s %g\n",netlist[ne].nome,na,nb,nc,nd,netlist[ne].valor);
      netlist[ne].a=numero(na);
      netlist[ne].b=numero(nb);
      netlist[ne].c=numero(nc);
      netlist[ne].d=numero(nd);
    }
    else if (tipo=='*') { /* Comentario comeca com "*" */
      printf("Comentario: %s",txt);
      ne--;
    }

    else if (tipo=='.') { /* Comentario comeca com "*" */
      sscanf(p, "%lg%lg%10s%lg", &tempoFinal, &passo, netlist[ne].nome, &passoPorPt);
      printf("Configuracao: %g %g %g \n", tempoFinal, passo, passoPorPt);
      ne--;

    }

    else {
      printf("Elemento desconhecido: %s\n",txt);
      getch();
      exit(1);
    }
  }
  fclose(arquivoNetlist);
  return 0;
}
