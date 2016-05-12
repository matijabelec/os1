/*
	Program je pisan u C-u, za promjenu u C++ potrebno je sve 
	printf() pretvoriti u cout..endl, te sve scanf() u cin
*/

#include <stdio.h> //scanf(), printf()
#include <stdlib.h> // exit()
#include <pthread.h> // pthread_t, pthread_create(), pthread_join()
#include <signal.h> // sigset()

pthread_mutex_t monitor; // monitor
pthread_cond_t red; // red cekanja
int N; // broj dretvi
int br; // brojac kod redova

void Barijera(int i) {
	int broj; // varijabla u koju se spremi uneseni broj
	
	// prvi dio = unos brojeva
	pthread_mutex_lock(&monitor); // zakljucavanje monitora (pocetak KO(kriticnog odsjecka) )
	br++; //uvecaj brojac
	if(br < N) { // ako se ne radi o zadnjoj dretvi od svih pokrenutih - ulazi unutar if-a inace skoci na else
		printf("Dretva %d. unesite broj\n", i);
		scanf("%d", &broj);
		pthread_cond_wait(&red, &monitor); // cekanje u redu = postavi se u red cekanja sve dok 
		                                   // sve ostale dretve ne zavrse unos broja
	} else { // u ovaj dio ulazi zadnja dretva od svih pokrenutih (ostale cekaj gore u redu)
		printf("Dretva %d. unesite broj\n", i);
		scanf("%d", &broj);
		br = 0; // postavi brojac na nulu (resetiraj brojac) da se moze odraditi drugi dio
		pthread_cond_broadcast(&red); // pusta sve iz reda cekanja = sad sve dretve nastavljaju prema drugom dijelu
	}
	pthread_mutex_unlock(&monitor); // otkljucavanje monitora (zavrsetak KO)
	
	// drugi dio = ispis brojeva na ekran
	pthread_mutex_lock(&monitor); // zakljucavanje monitora
	br++;
	if(br < N) {
		printf("Dretva %d. uneseni broj je %d\n", i, broj);
		pthread_cond_wait(&red, &monitor);
	} else {
		printf("Dretva %d. uneseni broj je %d\n", i, broj);
		br = 0;
		pthread_cond_broadcast(&red);
	}
	pthread_mutex_unlock(&monitor);
}

// funkcija dretve - argument je broj dretve (od 0 do N-1)
void* Dretva(void* i) {
	Barijera(*(int*)i); // dretva radi samo pokretanje gornje funkcije
}

// funkcija za prekid (brise monitor i red)
void Prekid(int x) {
	pthread_mutex_destroy(&monitor); // brise monitor
	pthread_cond_destroy(&red); // brise red
	exit(0); // prekid programa
}

int main(int argc, char** argv) {
	// provjera da li je unesen samo jedan argument kod pokretanja programa (argc=2 ako je 1 argument u pitanju)
	if(argc!=2) {
		printf("Netocan broj argumenata!\n");
		return 0;
	}
	
	// uzimanje prvog argumenta kao broj N, brojac br se ne treba stavit na nulu jer je kao globalna varijabla 
	// vec postavljen na nulu od strane kompajlera, al nije greska staviti br=0
	N = atoi(argv[1]);
	br = 0;
	
	// priprema monitora i reda
	pthread_mutex_init(&monitor, NULL);
	pthread_cond_init(&red, NULL);
	
	// u slucaju prekida (Ctrl+C) pozvati funkciju koja brise monitore i redove
	sigset(SIGINT, Prekid);
	
	// priprema potrebnih varijabli (polja) za dretve
	pthread_t dretva_id[N]; // opisnici dretvi
	int argument_dretve[N]; // argumenti dretve
	for(int i=0; i<N; i++) // postavljanje polja s argumentima dretve (polje sadrzi brojeve redom od 0 do N-1)
		argument_dretve[i]=i;
	
	// pokretanje svih dretvi
	for(int i=0; i<N; i++)
		pthread_create(&dretva_id[i], NULL, Dretva, (void*)&argument_dretve[i]);
	
	// cekanje svih dretvi da zavrse rad
	for(int i=0; i<N; i++)
		pthread_join(dretva_id[i], NULL);
	
	// brisanje (c/p iz funkcije Prekid)
	pthread_mutex_destroy(&monitor);
	pthread_cond_destroy(&red);
	
	return 0;
}

/*
	-------------------------------------------------------------
	funkcija barijera -> c/p iz udzbenika str 145 - Primjer 6.4.:
	-------------------------------------------------------------
		m-funkcija Barijera(m, red) {
			Zakljucati_monitor(m);
			br++;
			ako je (br < N) {
				Cekati_u_redu(red, m);
			}
			inace {
				br = 0;
				Propustiti_sve_iz_reda(red);
			}
			Otkljucati_monitor(m) ;
		}
	-------------------------------------------------------------
*/

//   date: 12th May 2014
// author: Matija Belec
//  email: matijabelec1@gmail.com
