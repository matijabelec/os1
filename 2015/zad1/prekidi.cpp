#include <unistd.h> // sleep()
#include <csignal>  // sigset(), SIGINT
#include <iostream> // cin, cout, endl
#include <ctime>    // time(), ctime()
using namespace std;

#define KORAK_SEC 1 // broj sekundi po koraku petlje
#define GLAVNI_N 20 // broj koraka petlje za obradu u GLAVNOM DIJELU PROGRAMA (broj koraka petlje u main-u)
#define PREKID_N 5 // broj koraka petlje u PREKIDNOM DIJELU PROGRAMA (broj koraka petlje u obradi prekida)

int tekuci_prekid = -1;
int razine[] = {
    SIGINT, // (SIGINT = 2) Ctrl+C
    SIGQUIT, // (SIGQUIT = 3) Ctrl+Ž ili Ctrl+AltGr+Q
    SIGKILL, // (SIGKILL = 9)
    SIGTERM, // (SIGTERM = 15)
    SIGTSTP // (SIGTSTP = 20) Ctrl+Z
};
int br_razina = sizeof(razine)/sizeof(int); // automatsko odredjivanje broja elemenata (brojeva) u polju "razine"

void prekidna_rutina(int s) { // funkcija koja se pokrece kada se dogodi neki od signala u gornjem polju
    int prethodni_prekid; // razina signala kojeg je tekuci prekid prekinuo
    
    time_t vrijeme = time(NULL); // trenutno vrijeme sustava
    cout << "[INFO] pozvana prekidna rutina u " << ctime(&vrijeme) << endl; //ispis vremena
    
    for(int i=tekuci_prekid+1; i<br_razina; i++) //prolaz kroz sve vise razine prekida od tekuceg prioriteta
        if(razine[i] == s) { //ako je pronadjen signal unutar neke od visih razina prekida:
            prethodni_prekid = tekuci_prekid;   // tekuci se prioritet sprema u "prethodni"
            tekuci_prekid = i; // "novi" tekuci prioritet postaje upravo pronadjeni prekid razine "i"
            sigrelse(s);    // dopusta novi prekid signala tipa "s" (argument funkcije "s" je id_signala)
            
            cout << "pocela je obrada prekida razine " << i+1 << endl;
            for(int j=1; j<=PREKID_N; j++) {
                sleep(KORAK_SEC);   // spavanje (cekanje KORAK_SEC sekundi)
                cout << "traje obrada prekida razine " << i+1 << " ... " << j << "/" << PREKID_N << endl;
            }
            cout << "zavrsila je obrada prekida razine " << i+1 << endl;
            
            sighold(s);     // onemogucuje prekid signala tipa "s"
            tekuci_prekid = prethodni_prekid; //nakon zavrsetka obrade prekida natrag se vraca na prethodno prekinuti prekid
            return; // prekid funkcije (prekid je zavrsio obradu)
        }
    cout << "[INFO] prekid nije vece razine od tekuceg pa se ne obradjuje" << endl;
}

int main(int argc, char** argv) { //argc i argv nisu potrebni u ovom programu, al budu u kasnijim zadacima (za sad moze i samo main() )
    cout << "PID procesa: " << ::getpid() << endl; //ispis ID-a procesa (PID = Process ID)
    
    cout << "broj razina prekida: " << br_razina << endl; //ispis ukupnog broja razina prekida u programu
    cout << "prekidi koji se obradjuju: " << endl;
    for(int i=0; i<br_razina; i++) { // prolaz kroz sve razine prekida (polje "razine")
        cout << "razina " << i+1 << ": signal(" << razine[i] << ")" << endl;    // ispis informacija o prekidu na razini
        sigset(razine[i], prekidna_rutina); // dodjeljivanje funkcije "prekidna rutina" za svaki prekid koji se nalazi u polju "razine"
    }
    
    cout << "program je pokrenut" << endl;
    for(int i=1; i<=GLAVNI_N; i++) {
        sleep(KORAK_SEC);
        cout << "program se izvodi ... " << i << "/" << GLAVNI_N << endl;
    }
    cout << "program je zavrsio" << endl;
    return 0;
}

/*********************************************************
    
    pokrenuti novi terminal i od tamo se šalje signal
    na pokrenuti proces (pomoću PID broja)
    
    - naredba:
        kill -{N} {PID}
    
    - umjesto {PID} ide ID procesa (npr. 20012)
    - umjesto {N} ide id_signala (npr. 3)
    
    - za navedene brojeve primjer naredbe izgleda:
        kill -3 20012
    
    *********************************************************
    - za popis signala:
        kill -l
    (*ovo zadnje je malo slovo 'L')
    
*********************************************************/