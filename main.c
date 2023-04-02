#include <stdio.h>
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define SX 68        /* Cursore sinistra */
#define DX 67        /* Cursore destra */
#define MAXX 80      /* Dimensione dello schermo di output (colonne) */
#define MAXY 20      /* Dimensione dello schermo di output (righe)   */
#define DELAY 70000/* Ritardo nel movimento delle vespe (da adattare) */
#define R 4            //righe astronave
#define C 4             //colonne astronave
#define DUE 2  //Array missili posizioni
#define MaxMissili 2 // Grandezza buffer posizioni
#define MaxNemici  3//nemici massimi
#define MaxNemiciAva 20// nemici avanzati = MaxNemici * 4
#define GruppoNemiciAva 4 // numero da quanto è composto il gruppo di nemici
#define MaxBomba 20 //numero massimo di  bombe
#define MAxDebug 50



//strutture
typedef struct{
    int x; /* Coordinata X */
    int y; /* Coordinata Y */
}Position;
typedef enum {ASTRONAVE ,NEMICO_BASE,NEMICO_AVANZATO,MISSILE,MISSILE2,BOMBA} Tipo;
typedef struct{
    Tipo tipo;
    Position posizione;
    Position direzione;
    int vivo;
    int colpito;
    int id;
    int idPadre;
    int disattiva;
    int sparato;

}Oggetto;

/* Prototipi delle funzioni adoperate */
void inizializza();
void stampaGioco();
void stampaBordi();
void stampaAstronave();
void stampaMissili();
void stampaNemici();
void stampaBombe();
void stampaNemiciAvanzati();
void spostaOggetti(Oggetto* arg,int maxy,int maxx);
void debug(Oggetto* arg,const char* stringa);

void *MissileDxThread(void *arg);
void *AstronaveThread(void *arg);
void *InputThread(void *arg);
void *NemiciThread(void *arg);
void *NemiciAvaThread(void *arg);
void *BombeThread(void *arg);
void *MissileSxThread(void *arg);

int astronaveIsColpita(Position pos);
int nemicoIsColpito(Position pos);
int nemiciAvaIsMorti(int id);
int nemicoAvanzatoIsColpito(Position pos);
int creaId();

Oggetto *getNemico(int indice);
Oggetto *getNemicoAvanzato(int indice);
Oggetto *getMissile(int indice);
Oggetto *getBomba(int indice);
Oggetto *getAstro();
Oggetto *getIdNemici(int indice);
Oggetto *getGruppoNemiciAvanzati(int id,int i);
Oggetto *getID(int indice);

Oggetto* creaBomba();
Oggetto* creaNemico();
Oggetto* creaNemicoAva();
Oggetto* creaMissile();

/* Variabili globali per passaggio dati tra threads  */
char key;

//buffer che contiene astronave,nemici,missili
Oggetto *oggetti;
int POS_ASTRO; //posizione nell'array dell'astronave
 int POS_NEMICO;//posizione nell'array del primo nemico
 int POS_MISSILI;//posizione del primo missile nell'array
 int POS_NEMICO_AVA;//posizione del primo nemico avanzato
 int POS_BOMBA;//posizione della prima bomba
 int MAX_ARRAY;//grandezza dell'array

/* Mutex per la gestione dei thread */
pthread_mutex_t game;//mutex  per stampa grafica e calcolo posizioni
pthread_mutex_t inputTastiera;//mutex per input da tastiera
pthread_t tNemico;
pthread_t tNemicoAva;
pthread_t tBomba;


int contDebug=0;//contatore per il debug grafico
int fine=0;
int contatoreId;

int main(){
    pthread_t tAstronave;
    pthread_t tMissile;
    pthread_t tInput;
    int i,j;

 POS_ASTRO=0; //posizione nell'array dell'astronave
 POS_NEMICO=(POS_ASTRO+1);//posizione nell'array del primo nemico
 POS_BOMBA=POS_NEMICO+MaxNemici;
 POS_MISSILI=POS_BOMBA+MaxBomba;//posizione del primo missile nell'array
 POS_NEMICO_AVA=POS_MISSILI+MaxMissili;//posizione del primo nemico avanzato nell'array
 MAX_ARRAY=POS_NEMICO_AVA+MaxNemiciAva;//grandezza massima  dell'array (astronave,nemici,missili)

 oggetti= (Oggetto *)malloc(MAX_ARRAY*sizeof(Oggetto ));//allocco in memoria array di oggetti grande MAX_ARRAY
 if(oggetti==NULL){
     perror("memoria finita");
     exit(0);
 }

    /* Inizializzo e configuro la finestra di output */
    initscr();
    noecho();
    endwin();
    curs_set(0);
    /* Inizializzo mutex */
    pthread_mutex_init(&game, NULL);
    pthread_mutex_init(&inputTastiera, NULL);
    stampaBordi();
    inizializza();

    //Creo il thread Astronave
    if (pthread_create(&tAstronave, NULL, AstronaveThread ,NULL)) {
        endwin();
        exit;
    }
    //creo i thread Missile

     if (pthread_create(&tMissile, NULL, MissileSxThread,NULL)) {
        endwin();
        exit;
        }

    //Creo il thread Input
    if (pthread_create(&tInput, NULL, InputThread,NULL)) {
        endwin();
        exit;
    }


    stampaGioco();


    /* Elimino mutex */
    pthread_mutex_destroy(&game);
    pthread_mutex_destroy(&inputTastiera);
    /* Ripristino la modalità di funzionamento usuale */
    endwin();
    /* Segnalo la fine del gioco e termino il programma */
    printf("\n\n\nGAME OVER\n\n\n");
    return 0;
}


//processo consumatore
void stampaGioco(){
    int i,k,j,cont=0,nuovax;
    clock_t start,end;
    double times=0;

    int stop=0;
    Oggetto *astronave=getAstro();
    Oggetto *nemici;

    stampaAstronave();
    start=clock();
    do {


        pthread_mutex_lock(&game);
        for( i = 0; i < R; i++){
            for( j = 0; j < C; j++) {
                mvaddch(astronave->posizione.y + i,astronave->posizione.x + j, ' ');
            }
        }

    pthread_mutex_unlock(&game);
        Oggetto* nemici=creaNemico();
        if (nemici!=NULL  ) {
            end=clock();
            times=((double)(end-start))/CLOCKS_PER_SEC;
            //ogni tot tempo spowna una nave nemica
             if(times>1){

                if (pthread_create(&tNemico, NULL, NemiciThread,NULL)) {
                endwin();
                exit;
                }


            start=clock();
            }
        }




        //Segnalo collisione e tipo (Astronave )

        spostaOggetti(astronave,MAXY,MAXX-3);

        pthread_mutex_lock(&game);
        stampaAstronave();
        stampaNemiciAvanzati();
        stampaNemici();

        stampaBombe();
        stampaMissili();


        pthread_mutex_unlock(&game);

        //refresh debug
        if(key=='l'){
       for(int i = 0; i < MAxDebug; i++) {
            mvprintw((i),MAXX+5,"                                                                           ");
       }
       key='/';

   }
        refresh();

        usleep(DELAY);
    } while (fine!=1);
}


//funzione che stampa area di gioco
void stampaBordi(){
    int  y;

    for (y = 0; y <= MAXY; y++){
        mvaddch(y, 1 , '|');
    }
    for (y = 0; y <= MAXY; y++){
        mvaddch(y, MAXX , '|');
    }
    for (y = 1; y <= MAXX; y++){
        mvaddch(MAXY + 1, y, '-');
    }
     refresh();
}

//funziona che inizializza astronave
void inizializza(){
    int i;

    contatoreId=0;

    Oggetto* astronave=getAstro();
    astronave->posizione.x=MAXX/2;
    astronave->posizione.y=MAXY-(R-1);
    astronave->tipo=ASTRONAVE;
    astronave->id=1111;
    astronave->vivo=1;
    astronave->colpito=0;
}


//debug grafico
void debug(Oggetto* arg,const char* stringa){

    mvprintw((contDebug++%MAxDebug),MAXX+5,"%d/%s - pos:%d/%d , dir:%d/%d  vivo:%d tipo:%d  id:%d id padre: %d ",contDebug%50,stringa,arg->posizione.x,arg->posizione.y,arg->direzione.x,arg->direzione.y,arg->vivo ,arg->tipo,arg->id,arg->idPadre);

    usleep(DELAY);
    // refresh();
}

void *AstronaveThread(void *arg){

    Oggetto *astronave=getAstro();
    while (astronave->vivo!=0){


        pthread_mutex_lock(&game);
              // Gestisco movimento tasti cursore
            if (key == SX  ){
                key='/';

                astronave->direzione.x=-1;

            }else if(key== DX){
               key='/';

                astronave->direzione.x=1;
            }
            if(astronave->colpito==3){
                astronave->vivo=0;
                fine=1;
            }
        pthread_mutex_unlock(&game);

    }
    pthread_exit(NULL);

}

void *MissileSxThread(void *arg){
    Oggetto* missile;
    Oggetto* astronave=getAstro();

    while(fine!=1){

        if(key == ' '){
            key='/';

            //primo missile (sinistra)
            missile=creaMissile();
            pthread_mutex_lock(&game);
            if(missile==NULL){
                pthread_mutex_unlock(&game);
                continue ;
            }



            missile->posizione.x=astronave->posizione.x+3;
            missile->posizione.y=astronave->posizione.y-1;
            missile->direzione.x=1;
            missile->direzione.y=-1;
            missile->vivo=1;
            missile->tipo=MISSILE;
            missile->id=1111;
             pthread_mutex_unlock(&game);



               missile=creaMissile();
            pthread_mutex_lock(&game);
            if(missile==NULL){
                pthread_mutex_unlock(&game);
                continue ;
            }


            missile->posizione.x=astronave->posizione.x;
            missile->posizione.y=astronave->posizione.y-1;
            missile->direzione.x=-1;
            missile->direzione.y=-1;
            missile->vivo=1;
            missile->tipo=MISSILE;
            missile->id=1111;
             pthread_mutex_unlock(&game);
            }
    }
    pthread_exit(NULL);


}


int creaId(){
    return contatoreId++;
}

void *NemiciThread(void *arg){

    Oggetto* nemico;
    nemico=creaNemico();

    double times;
    clock_t start,end;


        nemico->vivo=1;
        nemico->posizione.y=0;
        nemico->posizione.x=2;
        nemico->direzione.x=1;
        nemico->direzione.y=1;
        nemico->tipo=NEMICO_BASE;
        nemico->id=creaId();
        nemico->idPadre=00;
        nemico->colpito=0;
        nemico->disattiva=0;
        nemico->sparato=0;
        start=clock();

        while(nemico->vivo!=0 && fine!=1){
            if(key=='z'){
                key='/';
                nemico->colpito=1;
            }
            //se il nemico non ha ancora sparato entra nella funzione
            if (nemico->sparato==0) {
                end=clock();
                times=((double)(end-start))/CLOCKS_PER_SEC;
                if(times>2){
                    nemico->sparato=1;

                    pthread_mutex_lock(&game);
                    if (pthread_create(&tBomba, NULL, BombeThread ,&nemico->id)) {
                        endwin();
                        exit;
                    }
                     pthread_mutex_unlock(&game);
                    start=clock();
                }
            }

            //se il nemico viene colpito ed non è ancora stato disativato si creano i nemici di secondo livello
            if(nemico->colpito==1 && nemico->disattiva==0){
                 nemico->disattiva=1;
                 nemico->sparato=1;
                //creo il thread nemici avanzati
                 pthread_mutex_lock(&game);
                if (pthread_create(&tNemicoAva, NULL, NemiciAvaThread ,&nemico->id)) {
                    endwin();
                    exit;
                }
                pthread_mutex_unlock(&game);
            }
            usleep(DELAY);
        }

        debug(nemico,"muore");

        pthread_exit(NULL);



}

void *NemiciAvaThread(void *arg){
    int *id=(int *)arg;
    clock_t start,end;
    int i;
    double times;
    Oggetto* nemicoAVa;
     pthread_mutex_lock(&game);
    Oggetto* nemico=getIdNemici(*id);


    for(i = 0; i < GruppoNemiciAva; i++){

        nemicoAVa=creaNemicoAva();
        nemicoAVa->tipo=NEMICO_AVANZATO;
        nemicoAVa->posizione.x=nemico->posizione.x+(i*2);
        nemicoAVa->posizione.y=nemico->posizione.y;
        nemicoAVa->direzione.x=1;
        nemicoAVa->direzione.y=1;
        nemicoAVa->vivo=1;
        nemicoAVa->idPadre=nemico->id;
        nemicoAVa->id=creaId();
        nemicoAVa->colpito=0;
        nemicoAVa->disattiva=0;
        nemicoAVa->sparato=0;
       debug(nemicoAVa,"creato avanzato");

    }
    start=clock();
     pthread_mutex_unlock(&game);

    while(!nemiciAvaIsMorti(nemico->id && fine!=1)){

        for( i = 0; i < GruppoNemiciAva; i++){
            nemicoAVa=getGruppoNemiciAvanzati(nemico->id,i);

            if (nemicoAVa->colpito==2 && nemicoAVa->vivo==1) {
                debug(nemicoAVa,"ucciso");
                nemicoAVa->vivo=0;
                nemicoAVa->disattiva=1;
            }
                 //se il nemico non ha ancora sparato entra nella funzione
                if (nemicoAVa->sparato==0) {
                end=clock();
                times=((double)(end-start))/CLOCKS_PER_SEC;
                if(times>1){
                    nemicoAVa->sparato=1;

                    pthread_mutex_lock(&game);
                    if (pthread_create(&tBomba, NULL, BombeThread ,&nemicoAVa->id)) {
                        endwin();
                        exit;
                    }
                     pthread_mutex_unlock(&game);
                    start=clock();
                }

            }
        }
    }
    debug(nemico,"gruppo morto");
    nemico->vivo=0;

    pthread_exit(NULL);

}

void *BombeThread(void *arg){
    int *id=(int *)arg;
    clock_t start,end;
    int i;
     pthread_mutex_lock(&game);
    Oggetto* nemico=getIdNemici(*id);
    Oggetto* bomba=creaBomba();

    bomba->vivo=1;
    bomba->posizione.x=nemico->posizione.x;
    bomba->posizione.y=nemico->posizione.y;
    bomba->direzione.x=1;
    bomba->direzione.y=1;
    bomba->tipo=BOMBA;
     pthread_mutex_unlock(&game);
    while(bomba->vivo!=0 && fine!=1){


    }
    if(nemico->disattiva==0){
    nemico->sparato=0;
    }
    pthread_exit(NULL);


}

//funzione che restituisce true se il gruppo di navicelle avanzate è morto
int nemiciAvaIsMorti(int id){
    int gruppoDead=0;
    for(int i = 0; i <MaxNemiciAva; i++) {
       Oggetto* nemici= getNemicoAvanzato(i);

        if(nemici->idPadre==id && nemici->disattiva==1){
            gruppoDead++;

        }
        if (gruppoDead==GruppoNemiciAva) {
            return 1;
        }

    }

        return 0;

}


Oggetto* creaBomba(){
    for(int i = 0; i < MaxBomba; i++){
        Oggetto* bomba=getBomba(i);
        if (bomba->vivo==0) {
            return bomba;
        }
    }
    return NULL;
}

Oggetto* creaNemicoAva(){
      for(int i = 0; i <MaxNemiciAva; i++) {
       Oggetto* nemici= getNemicoAvanzato(i);
       if(nemici->vivo==0){
        return nemici;
       }
    }
    return NULL;
}

Oggetto* creaNemico(){
      for(int i = 0; i <MaxNemici ; i++) {
       Oggetto* nemici= getNemico(i);
       if(nemici->vivo==0){
        return nemici;
       }
    }
    return NULL;
}


//funzione che restituisce oggetto dell'array oggetti (zona missili) libero(non ancora attivo)
Oggetto* creaMissile(){
    for(int i = 0; i <MaxMissili ; i++) {
       Oggetto* missile= getMissile(i);
       if(missile->vivo==0){
        return missile;
       }
    }
    return NULL;

}

void stampaAstronave(){
    Oggetto* astronave=getAstro();
    char charastro[R][C]={  {'/', '\\', '/', '\\'},
                             {'|', '|', '|', '|'},
                             {'|', '\\', '/', '|'},
                             {'\\', '_', '_', '/'}};

    for(int i = 0; i < R; i++){
        for(int j = 0; j < C; j++) {
             mvaddch(astronave->posizione.y + i,astronave->posizione.x + j, charastro[i][j]);
        }
    }
}

void stampaNemiciAvanzati(){
    char charNemiciAva[DUE]={'\\','/'};
    int i,j;
    Oggetto* nemicoAvanzato;
    for(i = 0; i < MaxNemiciAva; i++){
        nemicoAvanzato=getNemicoAvanzato(i);
        for( j = 0; j < DUE; j++){

                mvaddch(nemicoAvanzato->posizione.y ,nemicoAvanzato->posizione.x + j, ' ');

        }
    }
    for( i = 0; i < MaxNemiciAva; i++){
       nemicoAvanzato=getNemicoAvanzato(i);

       if(nemicoAvanzato->vivo!=1){
           continue;
       }
       spostaOggetti(nemicoAvanzato,MAXY,MAXX);

       if(nemicoAvanzato->vivo!=1){
           continue;
       }


        for(j=0;j < DUE;j++){
               mvaddch(nemicoAvanzato->posizione.y  ,nemicoAvanzato->posizione.x + j, charNemiciAva[j]);

        }
    }
}

void stampaNemici(){
    char charNemico='v';
    int i;
    Oggetto* nemico;
    for( i = 0; i < MaxNemici; i++){
      nemico=getNemico(i);
      if(nemico->vivo==0 ||  nemico->disattiva==1){
          continue;
      }

      mvaddch(nemico->posizione.y ,nemico->posizione.x , ' ');
    }


    for(i = 0; i <MaxNemici ; i++) {
        nemico= getNemico(i);

        if(nemico->vivo==0 || nemico->disattiva==1){

           continue;
        }

        spostaOggetti(nemico,MAXY,MAXX);

        if(nemico->vivo==0 || nemico->disattiva==1){

           continue;
        }

        mvaddch(nemico->posizione.y ,nemico->posizione.x ,  charNemico);
    }

}

void stampaBombe(){
    char charBomba='|';
    Oggetto* bomba;
    int i;

    for( i = 0; i < MaxBomba; i++){
        bomba=getBomba(i);
        if(bomba->vivo==0){
            continue;
        }
        mvaddch(bomba->posizione.y ,bomba->posizione.x , ' ');
    }

    for( i = 0; i < MaxBomba; i++){
         bomba=getBomba(i);
        if(bomba->vivo==0){
            continue;
        }

        if(astronaveIsColpita(bomba->posizione)){
            bomba->vivo=0;
        }
        spostaOggetti(bomba,MAXY,MAXX);

        if(astronaveIsColpita(bomba->posizione)){
            bomba->vivo=0;
        }

        if(bomba->vivo==0){
            continue;
        }

        mvaddch(bomba->posizione.y ,bomba->posizione.x , charBomba);
    }


}

void stampaMissili(){
    int i;
    char charMissili='*';
    Oggetto* missile;
    //Prima cancello tutti i missili per far si che un missile dopo l'altro non cancelli il precedente

    for(i = 0; i <MaxMissili ; i++) {
        missile= getMissile(i);

        if(missile->vivo==0){
           continue;
        }
        mvaddch(missile->posizione.y ,missile->posizione.x , ' ');
    }
    //sposto e disegno tutti i missili vivi
    for(i = 0; i <MaxMissili ; i++) {
       missile= getMissile(i);


        if(missile->vivo==0){

           continue;
        }

         if(nemicoIsColpito(missile->posizione)){

            missile->vivo=0;
        }

        if (nemicoAvanzatoIsColpito(missile->posizione)) {
            missile->vivo=0;
        }



        spostaOggetti(missile,MAXY,MAXX);
        if(missile->vivo==0){
           continue;
        }
        mvaddch(missile->posizione.y ,missile->posizione.x ,  charMissili);



    }


 }

 //funzione che gestisce gli spostamenti dei vari oggetti sulla mappa e controlla anche le collisioni con i limiti della mappa

void spostaOggetti(Oggetto* arg,int maxy,int maxx){
    int nuovax,nuovay;
    nuovax=arg->posizione.x+arg->direzione.x;
    nuovay=arg->posizione.y+arg->direzione.y;

    if (arg->disattiva==0) {



    switch (arg->tipo){
        case ASTRONAVE:

        if(nuovax>1 && nuovax < maxx){
            //la nuova posizione è valida
            arg->posizione.x=nuovax;
        }else{
            arg->vivo=0;
        }
        if(nuovay>0 && nuovay < maxy){
            arg->posizione.y=nuovay;
        }else{
            arg->vivo=0;
        }


        arg->direzione.x=0;//rimposto la direzione  di x a 0
        arg->direzione.y=0;//rimposto la direzione  di y a 0

            break;

        case MISSILE:

          if(nuovax>1 && nuovax < maxx){
            //la nuova posizione è valida
            arg->posizione.x=nuovax;
            }else{
                arg->vivo=0;
            }
            if(nuovay>0 && nuovay < maxy){
                arg->posizione.y=nuovay;
            }else{
                arg->vivo=0;
            }


            break;

        case NEMICO_BASE:

        //prendo la posizione attuale della stronave+direzionex
         //se posizione.y è pari entra
         if(nuovay%2==0){
            //la coordinata x è minore di massimo il nemico si sposta di xaltrimenti si sposta di y
            if(nuovax<maxx){
                arg->posizione.x=nuovax;
            }else{
                if(nuovay==MAXY-4){
                arg->vivo=0;
                fine=1;
                }else{
                     arg->posizione.y=nuovay;
                }

            }

        }else if(nuovax>3){

            //sposto la nave di direzione-1
                nuovax=arg->posizione.x-arg->direzione.x;
                arg->posizione.x=nuovax;
            }else{
                 if(nuovay==MAXY-4){
                arg->vivo=0;
                fine=1;
                }else{
                     arg->posizione.y=nuovay;
                }
            }

            break;

        case NEMICO_AVANZATO:

        if(nuovax>MAXX){
            arg->posizione.x=MAXX-1;
            arg->posizione.y=nuovay;
        }
        if(nuovax<3){
            arg->posizione.x=3;
            arg->posizione.y=nuovay;
        }
            if(nuovay%2==0){

            //la coordinata x è minore di massimo il nemico si sposta di xaltrimenti si sposta di y
                if(nuovax+1<maxx){
                    arg->posizione.x=nuovax;
                }else{
                    if(nuovay==MAXY-4){
                    arg->vivo=0;
                    fine=1;
                    }else{
                       // debug(arg,67);
                         arg->posizione.y=nuovay;
                    }

                }
            //se posizione.y è dispari
            //prendo la posizione attuale della stronave+direzionex controllo se è > di 0

            }else if(nuovax>3){

            //sposto la nave di direzione-1
                nuovax=arg->posizione.x-arg->direzione.x;
                arg->posizione.x=nuovax;
            }else{
                if(nuovay==MAXY-4){
                    arg->vivo=0;
                    fine=1;
                }else{

                     arg->posizione.y=nuovay;
                }
            }
        break;

        case BOMBA:
            if(nuovay>0 && nuovay < maxy){

                arg->posizione.y=nuovay;
            }else{

                arg->vivo=0;

            }


            break;



    }

    }

}

//pos = coordinate bomba , se pos astronave è uguale a pos bomba allora ritorna true

int astronaveIsColpita(Position pos){
    Oggetto* astronave=getAstro();
    if(pos.x>=astronave->posizione.x && pos.x<=astronave->posizione.x+C && pos.y==astronave->posizione.y){
        astronave->colpito++;
        return TRUE;
    }
    return FALSE;
}

//pos = coordinate missili, se pos nemico è uguale a pos missile allora ritorna true
int nemicoIsColpito(Position pos){
    Oggetto* nemico;

    for(int i = 0; i < MaxNemici; i++) {
        nemico=getNemico(i);
        if (nemico->disattiva==0) {

            if(pos.x == nemico->posizione.x && pos.y == nemico->posizione.y ){
                nemico->colpito++;
                return TRUE;
                }
        }

    }
    return FALSE;

}

int nemicoAvanzatoIsColpito(Position pos){
    Oggetto* nemicoAvanzato;

    for(int i = 0; i < MaxNemiciAva; i++) {
        nemicoAvanzato=getNemicoAvanzato(i);
        if (nemicoAvanzato->disattiva==0) {

            if((pos.x == nemicoAvanzato->posizione.x && pos.y == nemicoAvanzato->posizione.y) || (pos.x == nemicoAvanzato->posizione.x+1 && pos.y == nemicoAvanzato->posizione.y)){
                nemicoAvanzato->colpito++;
                debug(nemicoAvanzato,"colpito");


                return TRUE;
                }
        }

    }
    return FALSE;

}

//cerca e ritorna un astronave di secondo livello con lo stesso id del padre

Oggetto *getGruppoNemiciAvanzati(int id,int i){
    Oggetto* nemiciAvanzati;
    for(int j = 0; j < MaxNemiciAva; j++){
        nemiciAvanzati=getNemicoAvanzato(j);
        if(nemiciAvanzati->idPadre==id){

            return &oggetti[j+i+POS_NEMICO_AVA];
        }
    }
    return NULL;

}

Oggetto *getBomba(int indice){
    if(indice>=MaxBomba){
        perror("indice bomba non valido");
        exit(-1);
    }
    return &oggetti[indice+POS_BOMBA];
}

Oggetto *getAstro(){
    return &oggetti[POS_ASTRO];

}

Oggetto *getNemico(int indice){
    if(indice>=MaxNemici){
        perror("indice nemici non valido");
        exit(-1);
    }
    return &oggetti[indice+POS_NEMICO];

}

Oggetto *getNemicoAvanzato(int indice){
    if(indice>=MaxNemiciAva){
        perror("indice nemici avanzati non valido");
        exit(-1);
    }
    return &oggetti[indice+POS_NEMICO_AVA];
}

Oggetto *getMissile(int indice){
    if(indice>=MaxMissili){
        perror("indice missile non valido");
        exit(-1);
    }
    return &oggetti[indice+POS_MISSILI];
}

Oggetto *getIdNemici(int id){
    Oggetto *nemico;
    for(int  i = 0; i < MAX_ARRAY; i++){
        nemico=getID(i);
        if(nemico->id == id){
            return nemico;
        }
    }
    return NULL;


}

Oggetto *getID(int indice){
    if(indice>=MAX_ARRAY){
        perror("indice id non valido");
        exit(-1);
    }
    return &oggetti[indice];
}


void *InputThread(void *arg){

    while(fine!=1){
    pthread_mutex_lock(&inputTastiera);
    key=getch();
    pthread_mutex_unlock(&inputTastiera);



    }
    pthread_exit(NULL);
}
