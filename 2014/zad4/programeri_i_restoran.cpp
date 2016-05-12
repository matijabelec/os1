/*
	Program je pisan u C++-u, za promjenu u C potrebno je sve 
	cout..endl pretvoriti u printf()
*/

#include <unistd.h> // usleep() -> vjerojatno na arci ovo ne treba
#include <iostream> // cin, cout, endl
#include <cstdlib> // exit()
#include <pthread.h> // pthread_t, pthread_create(), pthread_join()
#include <signal.h> // sigset()
using namespace std;

int N; // broj programera koji mogu uci u restoran
int M; // broj programera svake vrste

pthread_mutex_t monitor; // monitor

// redovi cekanja
pthread_cond_t red_programeri[2]; //indeksi: linux=0, microsoft=1

// brojaci za pojedine redove
int br_restoran; //trenutni broj programera u restoranu
int br_restoran_odradjeno; //broj odradjenih = koliko je programera uslo u red dok programeri druge vrste cekaju
int br_programeri[2]; //indeksi: linux=0, microsoft=1; broj programera u redu cekanja
int vrsta_u_restoranu = -1; //vrsta: 0=linux_programer, 1=microsoft_programer

// funkcija koja sluzi za ispis (ulazni=vrsta programera(0 ili 1), st=niz znakova koji se ispisuju - info na kraju ispisa)
void ispis(int ulazni, string st) {
	// ispis reda linux programera
	cout << "Red Linux:";
	for(int i=0; i<M; i++)
		if(i<br_programeri[0])
			cout << "L";
		else
			cout << "-";
	
	// ispis reda microsoft programera
	cout << " Red Microsoft:";
	for(int i=0; i<M; i++)
		if(i<br_programeri[1])
			cout << "M";
		else
			cout << "-";
	
	// ispis reda programera u restoranu
	cout << " Restoran:";
	if(vrsta_u_restoranu==0)
		for(int i=0; i<br_restoran; i++)
			cout << "L";
	else
		for(int i=0; i<br_restoran; i++)
			cout << "M";
	
	// dogadjaj koji se dogodio (npr. " --> L u red cekanja")
	cout << " --> ";
	if(ulazni==0)
		cout << "L " << st;
	else
		cout << "M " << st;
	
	cout << endl;
}

// monitorska funkcija za ulaz u restoran
void udji(int vrsta) {
	pthread_mutex_lock(&monitor); // ulaz u monitor
	
	// pretpostavka je da ce programer uci u red cekanja
	br_programeri[vrsta]++;
	
	// provjera da li programer ide u red cekanja
	while(vrsta_u_restoranu == 1-vrsta || br_restoran_odradjeno >= N) {
		ispis(vrsta, "u red cekanja"); // ispisi informaciju da je programer usao u red cekanja
		pthread_cond_wait(&red_programeri[vrsta], &monitor); // postavi programera u njegov red cekanja
	}
	
	// programer izlazi iz reda cekanja
	br_programeri[vrsta]--;
	
	// postavi vrstu u restoranu (ustvari se mjenja samo kad je vrsta_u_restoranu == -1) i povecaj brojac "u restoranu"
	vrsta_u_restoranu = vrsta;
	br_restoran++;
	
	// provjeri da li ima programera druge vrste u cekanju, te ako ima uvecaj brojac "odradjeno"
	if(br_programeri[1-vrsta] > 0)
		br_restoran_odradjeno++;
	
	// ispisi informaciju da je programer usao u restoran
	ispis(vrsta, "u restoran");
	
	pthread_mutex_unlock(&monitor); // izlaz iz monitora
}

// monitorska funkcija za izlaz iz restorana
void izadji(int vrsta) {
	pthread_mutex_lock(&monitor);
	
	// zabiljezi izlaz programera iz restorana
	br_restoran--;
	
	// ako je restoran sada prazan
	if(br_restoran==0) {
		
		// provjeri da li druga vrsta programera ceka na ulazak
		if(br_programeri[1-vrsta] > 0) {
			// dopusti ulaz drugoj vrsti programera
			vrsta_u_restoranu = 1-vrsta;
			
			// oslobodi sve dretve druge vrste programera iz njihovog reda
			pthread_cond_broadcast(&red_programeri[1-vrsta]);
		} else {
			// ako ne ceka ni jedan programer druge vrste postavi vrsta_u_restoranu na -1
			// (pa ko prvi od programera udje - odredjuje vrstu u restoranu)
			vrsta_u_restoranu = -1;
		}
		
		// broj odradjenih iste vrste postavi na nulu ("reset brojaca")
		br_restoran_odradjeno = 0;
	}
	
	// ispisi informaciju da je programer izasao iz restorana
	ispis(vrsta, "iz restorana");
	pthread_mutex_unlock(&monitor);
}

// dretva programera (c/p iz moodle-a, pa prilagodjeno)
void* Programer(void* vrsta) {
	usleep(rand()%100000); // programiraj;  //ovaj poziv usleep teoretski nije potreban - al pomocu njega se moze 
	                       // "podesiti" da se ispis odradi priblizno kao na moodle primjeru (jer inace dretve
						   // prebrzo udju unutar monitora i ne stigne se dobiti "slucajni" poredak)
	udji(*((int*)vrsta) );
	usleep(rand()%100000); // jedi; //isto kao i s gornjim usleep pozivom - treba eksperimentirati brojevima unutar
	                       // npr. umjesto 100000 staviti neki drugi broj 1000-1000000 (milijun = jedna sekunda cekanja)
	izadji(*((int*)vrsta) );
}

// funkcija za prekid (brise monitor i red)
void Prekid(int x) {
	pthread_mutex_destroy(&monitor); // brise monitor
	
	// brisanje redova
	for(int i=0; i<2; i++)
		pthread_cond_destroy(&red_programeri[i]);
	
	exit(0); // prekid programa
}

int main(int argc, char** argv) {
	// provjera da li je unesen samo jedan argument kod pokretanja programa (argc=3 ako su 2 argumenta u pitanju)
	if(argc!=3) {
		cout << "Netocan broj argumenata!" << endl;
		return 0;
	}
	
	// uzimanje prvog argumenta kao broj N, a drugog kao broj M; globalna varijable su standardno postavljene na 
	// nulu (teoretski ovisi od kompajlera do kompajlera - svi danasnji prate standard da se globalne varijable stave 
	// na nulu) pa se ne moraju (ali mogu) ovdje postaviti na nulu
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	
	// priprema monitora i redova
	pthread_mutex_init(&monitor, NULL);
	for(int i=0; i<2; i++)
		pthread_cond_init(&red_programeri[i], NULL);
	
	// u slucaju prekida (Ctrl+C) pozvati funkciju koja brise monitore i redove
	sigset(SIGINT, Prekid);
	
	// priprema potrebnih varijabli (polja) za dretve
	pthread_t dretva_id[M*2]; // opisnici dretvi
	int argument_dretve[M*2]; // argumenti dretve
	for(int i=0; i<M*2; i++) // polje s argumentima dretve (sadrzi brojeve redom od 0,1,0,1,... sto predstavlja vrstu programera)
		argument_dretve[i] = i%2;
	
	// pokretanje svih dretvi
	for(int i=0; i<M*2; i++)
		pthread_create(&dretva_id[i], NULL, Programer, (void*)&argument_dretve[i]);
	
	// cekanje svih dretvi da zavrse rad
	for(int i=0; i<M*2; i++)
		pthread_join(dretva_id[i], NULL);
	
	// brisanje (c/p iz funkcije Prekid)
	pthread_mutex_destroy(&monitor);
	for(int i=0; i<2; i++)
		pthread_cond_destroy(&red_programeri[i]);
	
	return 0;
}

//   date: 12th May 2014
// author: Matija Belec
//  email: matijabelec1@gmail.com
