#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "gamelib.h"

/**********************************
 * gamelib.c | Libreria di gioco  *
 **********************************/

/*
 * MACRO E COSTANTI DI GIOCO
 */

// Versione e crediti di gioco
const char* const C_VERSIONE = "1.0";
const char* const C_TITOLO = "Prince Of Inertia";
const char* const C_CREATORE = "Matteo Bagnoletti Tini";
const char* const C_DESCRIZIONE = "UniPG - 2024/25 - corso di Laurea triennale in Informatica";

// Colori per la stampa a video
const char* const COLORE_R = "\x1b[31m";
const char* const COLORE_G = "\x1b[33m";
const char* const COLORE_V = "\x1b[32m";
const char* const COLORE_H = "\x1b[1;35m";
const char* const COLORE_LOG = "\x1b[0;37m";
const char* const COLORE_STD = "\x1b[0m";

// Specifiche giocatore
const numero MAX_G_NOME = 25;
const numero MIN_G_NUM = 1;
const numero MAX_G_NUM = 3;
const numero MAX_PRINCIPE_SCAPPA = 1;
const numero MAX_DOPP_SCAPPA = 1;

// Specifiche gioco
const numero MAX_PVITA = 3;
const numero MIN_PVITA = 1;

const numero MIN_DADI = 1;
const numero MAX_DADI = 3;
const numero INIT_DADI = 2;

const numero MIN_STANZE = 15;

// Specifiche combattimento
#define COLPO_RIUSCITO(punti) (punti >= 4)
#define DIFESA_RIUSCITA(punti) (punti >= 4)
#define CRITICO(punti) (punti == 6)

const numero ATTACCO = 1;
const numero ATTACCO_CRITICO = 2;
const numero DIFESA = 1;
const numero DIFESA_CRITICO = 2;

// Specifiche nemici
const numero SCHELETRO_ATTACCO = 1;
const numero SCHELETRO_DIFESA = 1;
const numero SCHELETRO_PVITA = 1;

const numero GUARDIA_ATTACCO = 2;
const numero GUARDIA_DIFESA = 2;
const numero GUARDIA_PVITA = 2;

const numero JAFFAR_ATTACCO = 3;
const numero JAFFAR_DIFESA = 2;
const numero JAFFAR_PVITA = 3;

// Utilità
#define RAND(min, max) ((numero)((rand() / (RAND_MAX + 1.0)) * ((max) - (min) + 1)) + (min))
#define LANCIA_DADO() (RAND(1, 6))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define SAFE_FREE(p) { if (p && p != NULL) { free(p); p = NULL; } }
#define IS_P_VALIDO(p) (p != NULL)

#define MIN_NUMERO 0
#define MAX_NUMERO 65535

#define BOOL_TEXT(b) (b ? "sì" : "no")

#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

/*
 *   VARIABILI DI GIOCO GLOBALI
 */
bool primoAvvio = true;

Stanza *primaStanza = NULL;
Stanza *ultimaStanza = NULL;

bool check_giocatori = false;
bool check_mappa = false;
bool giocoAvviato = false;

numero roundGioco = 0;

Giocatore* giocatori[3] = { NULL, NULL, NULL };

bool jaffarSconfitto = false;

/*
 *   FUNZIONI DI UTILITÀ
 */

/**
* Inizializza il generatore di numeri casuali (srand) solo al primo avvio del programma
*/
static void initRandom() {
    if (primoAvvio == false) return;
    primoAvvio = false;
    time_t t;
    srand((unsigned) time(&t));
}

/**
* Pulisce il terminale
*/
static void pulisciCmd() {
    system(CLEAR);
}

/**
* Ripulisci il terminale e stampa il titolo del gioco
*/
extern void nuovaSchermata() {
	pulisciCmd();
    crediti(true, false);
}

/**
* Pulisce il buffer di input in modo da evitare errori di lettura
*/
static void pulisciBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
* Input di un numero opportunamente sanificato
* @param p Puntatore al numero in cui salvare il valore sanificato
*/
extern Esito numInput(numero *p, numero MIN_L, numero MAX_L) {
	if (!IS_P_VALIDO(p)) return errore;

	int n;

	if (scanf("%d", &n) != 1 || n < MIN_L || n > MAX_L || n < MIN_NUMERO || n > MAX_NUMERO) {
		pulisciBuffer();
        printf("\n");
		return errore;
	} else {
		pulisciBuffer();
		*p = (numero) n;
        printf("\n");
		return successo;
	}
}  

/**
* Input di una stringa di lunghezza compresa tra MIN_L e MAX_L opportunamente sanificata
* @param p Puntatore alla stringa in cui salvare il testo sanificato
* @param MIN_L Lunghezza minima della stringa
* @param MAX_L Lunghezza massima della stringa
*/
extern Esito stringInput(char *p, numero MIN_L, numero MAX_L) {
    if (!IS_P_VALIDO(p)) { printf("\n"); return errore; }
    if (MIN_L > MAX_L) { printf("\n"); return errore; }

    char buffer[256];

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) { printf("\n"); return errore; }

    // sanificazione
    buffer[strcspn(buffer, "\n")] = '\0';
    for (numero i = 0; buffer[i] != '\0'; i++) {
        if (!isprint((unsigned char)buffer[i])) { printf("\n"); return errore; }
    }

    // controllo
    numero len = (numero) strlen(buffer);
    if (len < MIN_L || len > MAX_L) { printf("\n"); return errore; }

    strcpy(p, buffer);
    printf("\n");
    return successo;
}

/**
 * Input di un carattere di lunghezza 1 opportunamente sanificato e che corrisponda a uno dei caratteri validi
 * @param p Puntatore al carattere in cui salvare il valore sanificato
 * @param validi Array contenente i caratteri validi (se vuoto, tutti i caratteri sono validi)
 */
static Esito charInput(char* p, const char* validi) {
    if (p == NULL) return errore;

    char c;
    if(stringInput(&c, 1, 1) == errore) return errore;

    if (!isprint((unsigned char)c)) return errore;

    if (validi != NULL && *validi != '\0' && strchr(validi, c) == NULL) return errore;

    *p = (char)c;
    return successo;
}

extern void logNota(const char* messaggio) {
    if (IS_P_VALIDO(messaggio)) {
        printf(" %s[%s!%s] %s%s\n", COLORE_LOG, COLORE_G, COLORE_LOG, messaggio, COLORE_STD);
    }
}
  
extern void logSpicy(const char* messaggio) {
	if (IS_P_VALIDO(messaggio)) printf("%s%s%s\n", COLORE_H, messaggio, COLORE_STD);
}

extern void logAvviso(const char* messaggio) {
    if (IS_P_VALIDO(messaggio)) printf("%s%s%s\n", COLORE_G, messaggio, COLORE_STD);
}

extern void logErrore(const char* messaggio) {
	if (IS_P_VALIDO(messaggio)) printf("%s%s%s\n", COLORE_R, messaggio, COLORE_STD);
}

extern void logSuccesso(const char* messaggio) {
	if (IS_P_VALIDO(messaggio)) printf("%s%s%s\n", COLORE_V, messaggio, COLORE_STD);
}

static void coloreCmd(const char* colore) {
	if (IS_P_VALIDO(colore)) printf("%s", colore);
}

/**
* Attende la pressione di un tasto da parte dell'utente per proseguire
*/
extern void attendiUtente() {
	printf("Premi INVIO per continuare...");
	getchar();
}

/**
* Chiede all'utente di confermare l'azione che sta per compiere
*/
static bool conferma(const char* messaggio) {
	if (IS_P_VALIDO(messaggio)) printf("%s.\n", messaggio);
	coloreCmd(COLORE_G);
	printf("Sei sicuro di voler proseguire? [s/N]: ");
	coloreCmd(COLORE_STD);
	char c = ' ';
	if (charInput(&c, "snSN") == errore) return false;
    if(c == 'n' || c == 'N') logAvviso("Operazione annullata.");
	return c == 's' || c == 'S';
}

/**
* Stampa l'elenco delle scelte disponibili passate come parametro utilizzando come identificatore i caratteri/numeri passati come parametro
* @param numerico Indica se utilizzare numeri come identificatori delle scelte
* @param simboli Stringa contenente i caratteri/numeri identificativi delle scelte
* @param elenco array di stringhe contenente le scelte disponibili
*/
extern void stampaElencoScelte(bool numerico, const char* simboli, const char* elenco[]) {
    if (!IS_P_VALIDO(simboli) || !IS_P_VALIDO(elenco)) return;
	
    numero s = 0;
    for (numero i = 0; elenco[i] != NULL; i++) {
        printf(" [");
        coloreCmd(COLORE_H);
        if(numerico && i >= 9) {
          	printf("%c", simboli[s]);
            s++;
          	printf("%c", simboli[s]);            
        } else {
            printf("%c", simboli[s]);
        }
        coloreCmd(COLORE_STD); printf("] %s\n", elenco[i]);
        s++;
    }
    
    printf("\n> ");
}

/**
* Restituisce la stringa corrispondente al tipo di stanza passato come parametro
* @param tipo Tipo di stanza da convertire in stringa
*/
static const char* stanzaToString(TipoStanza tipo) {
    switch (tipo) {
        case ingresso: return "ingresso";
        case corridoio: return "corridoio";
        case scala: return "scala";
        case salaBanchetto: return "sala banchetto";
        case magazzino: return "magazzino";
        case postoDiGuardia: return "posto di guardia";
        case prigione: return "prigione";
        case armeria: return "armeria";
        case moschea: return "moschea";
        case torre: return "torre";
        case bagni: return "bagni";
        default: return "tipo sconosciuto o non impostato";
    }
}

/**
* Restituisce la stringa corrispondente al tipo di trabocchetto passato come parametro
* @param trabocchetto Tipo di trabocchetto da convertire in stringa
*/
static const char* trabocchettoToString(TipoTrabocchetto trabocchetto) {
    switch (trabocchetto) {
        case nessunTrabocchetto: return "nessun trabocchetto";
        case tegola: return "tegola";
        case lame: return "lame";
        case caduta: return "caduta";
        case burrone: return "burrone";
        default: return "tipo sconosciuto o non impostato";
    }
}

/**
* Restituisce la stringa corrispondente al tipo di nemico passato come parametro
* @param nemico Tipo di nemico da convertire in stringa
*/
static const char* nemicoToString(TipoNemico nemico) {
	switch (nemico) {
		case nessunNemico: return "nessun nemico";
		case scheletro: return "scheletro";
		case guardia: return "guardia";
		case Jaffar: return "Jaffar";
		default: return "tipo sconosciuto o non impostato";
	}
}

/*
 *   DICHIARAZIONI DELLE FUNZIONI DI GIOCO (Per la documentazione si richiama la specifica definizione)
 */
 
static Stanza* nextStanza(Stanza*, bool);
static int numStanze(void);
static void cancStanza(void);
static void cancSegreta(Stanza*, Stanza*);
static void initStanza(Stanza*, TipoStanza, TipoTesoro, bool, TipoTrabocchetto);
static void creaStanza(Stanza*, Direzione, TipoStanza, TipoTesoro, bool, TipoTrabocchetto);
static void stanzaRandom(Stanza*, bool);
static void cercaStanzaSegreta(Giocatore*);
static void dettagliStanze(void);
static void dettagliStanza(Stanza*);
static numero posStanza(Stanza*);
static void resetMappa(void);
static void controlloMappa(void);
static void impostaMappa(void);

static numero numGiocatori(void);
static bool isPrincipeImpostato(void);
static void controlloGiocatori(void);
static void resetGiocatori(void);
static Giocatore* creaGiocatore(const char*, ClasseGiocatore, Stanza*, numero, numero, numero, numero, numero, bool);
static void impostaGiocatori(void);
static bool giocatoriInVita(void);
static void riordinaGRandom(void);
static void dettagliGiocatore(Giocatore*);
static void aumentaVita(Giocatore*, numero);
static void diminuisciVita(Giocatore*, numero);

static void randomNemico(Stanza*);
static void trabocchetto(Giocatore*);
static void raccogliTesoro(Giocatore*);
static void avanza(Giocatore*, bool);
static void torna(Giocatore*);
static void stampaOpzioni(Giocatore*, char*);
static void aiuto(Giocatore*);

static void infoCombattimento(Giocatore*);
static void eseguiTurno(Giocatore*, bool);
static void combatti(Giocatore*);
static void stampaRound(numero);
static void morte(Giocatore*);
static void sconfitta(void);
static void vittoria(Giocatore*);
static char* timestamp(void);
static void leggiStorico(void);
static void scriviStorico(const char*);

/*
 *   DEFINIZIONI DELLE FUNZIONI DI GIOCO
 */

/**
* Restituisce il puntatore alla stanza successiva rispetto a quella passata come parametro
* @param pStanza Stanza di cui si vuole conoscere la stanza successiva
* @param cercaSegreta Indica se si vuole avanzare nella stanza segreta
* @return Puntatore alla stanza successiva
*/
static Stanza* nextStanza(Stanza* pStanza, bool cercaSegreta) {
    if (IS_P_VALIDO(pStanza)) {
    	if (IS_P_VALIDO(pStanza->stDestra) && pStanza->stDestra->isSegreta == cercaSegreta) return pStanza->stDestra;
        if (IS_P_VALIDO(pStanza->stSinistra) && pStanza->stSinistra->isSegreta == cercaSegreta) return pStanza->stSinistra;
        if (IS_P_VALIDO(pStanza->stSopra) && pStanza->stSopra->isSegreta == cercaSegreta) return pStanza->stSopra;
        if (IS_P_VALIDO(pStanza->stSotto) && pStanza->stSotto->isSegreta == cercaSegreta) return pStanza->stSotto;
    }
    return NULL;
}

/**
* Restituisce il numero di stanze esistenti
* @return Numero di stanze esistenti
*/
static int numStanze() {
    numero nStanze = 0;
    Stanza* pScan = nextStanza(primaStanza, false);
    while (IS_P_VALIDO(pScan)) {
        nStanze++;
        pScan = nextStanza(pScan, false);
    }
    return nStanze;
}

/**
 * Cancella l'ultima stanza creata liberando la memoria allocata e rimpiazzando il puntatore alla stanza precedente
 */
static void cancStanza() {
    if (numStanze() > 0) {
        Stanza* pCanc = ultimaStanza;
        ultimaStanza = (IS_P_VALIDO(pCanc)) ? pCanc->precedente : NULL;

        if (IS_P_VALIDO(ultimaStanza)) {
            if (IS_P_VALIDO(ultimaStanza->stDestra) && ultimaStanza->stDestra == pCanc)
                ultimaStanza->stDestra = NULL;
            if (IS_P_VALIDO(ultimaStanza->stSinistra) && ultimaStanza->stSinistra == pCanc)
                ultimaStanza->stSinistra = NULL;
            if (IS_P_VALIDO(ultimaStanza->stSopra) && ultimaStanza->stSopra == pCanc)
                ultimaStanza->stSopra = NULL;
            if (IS_P_VALIDO(ultimaStanza->stSotto) && ultimaStanza->stSotto == pCanc)
                ultimaStanza->stSotto = NULL;
        }

        if (IS_P_VALIDO(pCanc)) {
            SAFE_FREE(pCanc);
        }
    }
}

/**
 * Dealloca la stanza segreta passata come parametro e rimpiazza il puntatore alla stanza precedente
 * @param segreta Stanza segreta da cancellare
 * @param precedente Stanza precedente alla stanza segreta
 */
static void cancSegreta(Stanza* segreta, Stanza* precedente) {
    if (!IS_P_VALIDO(segreta) || !IS_P_VALIDO(precedente)) return;

    if (IS_P_VALIDO(precedente)) {
        if (IS_P_VALIDO(precedente->stDestra) && precedente->stDestra == segreta)
            precedente->stDestra = NULL;
        if (IS_P_VALIDO(precedente->stSinistra) && precedente->stSinistra == segreta)
            precedente->stSinistra = NULL;
        if (IS_P_VALIDO(precedente->stSopra) && precedente->stSopra == segreta)
            precedente->stSopra = NULL;
        if (IS_P_VALIDO(precedente->stSotto) && precedente->stSotto == segreta)
            precedente->stSotto = NULL;
    }

    if (IS_P_VALIDO(segreta)) {
        SAFE_FREE(segreta);
    }
}

/**
 * Inizializza una generica nuova stanza con i valori passati come parametro
 */
static void initStanza(Stanza* stanza, TipoStanza tipo, TipoTesoro tesoro, bool isSegreta, TipoTrabocchetto trabocchetto) {
    if (!IS_P_VALIDO(stanza)) return;

    stanza->stDestra = NULL;
    stanza->stSinistra = NULL;
    stanza->stSopra = NULL;
    stanza->stSotto = NULL;
    stanza->precedente = NULL;

    stanza->tipo = tipo;
    stanza->tesoro = tesoro;
    stanza->tesoroRaccolto = false;
    stanza->isSegreta = isSegreta;
    stanza->isDefault = false;
    stanza->trabocchetto = trabocchetto;
    stanza->nemico = nessunNemico;
}

/**
 * Crea una nuova stanza con i valori passati come parametro e la collega alla stanza passata come parametro
 */
static void creaStanza(Stanza* posizione, Direzione dir, TipoStanza tipo, TipoTesoro tesoro, bool isSegreta, TipoTrabocchetto trabocchetto) {
    Stanza* pNuova = (Stanza*) malloc(sizeof(Stanza));
    if (!IS_P_VALIDO(pNuova)) {
        logErrore("Errore di allocazione in memoria.");
        terminaGioco();
    }

    initStanza(pNuova, tipo, tesoro, isSegreta, trabocchetto);

    if (IS_P_VALIDO(posizione)) {
        switch (dir) {
            case destra: posizione->stDestra = pNuova; break;
            case sinistra: posizione->stSinistra = pNuova; break;
            case sopra: posizione->stSopra = pNuova; break;
            case sotto: posizione->stSotto = pNuova; break;
            default: primaStanza = pNuova; break;
        }
        pNuova->precedente = posizione;
    }

    if(!isSegreta) ultimaStanza = pNuova;
}

/**
 * Crea una stanza random con i valori passati come parametro
 * @param posizione Stanza in cui creare la nuova stanza
 * @param isSegreta Indica se la stanza da creare è segreta
 */
static void stanzaRandom(Stanza* posizione, bool isSegreta) {
    
    if (!IS_P_VALIDO(posizione)) return;
    
    TipoStanza tipo = (TipoStanza) RAND(1, 10);
    TipoTesoro tesoro = (TipoTesoro) RAND(0, 5);
    TipoTrabocchetto trabocchetto = nessunTrabocchetto;
    Direzione dir = init;

    bool dirValida = false;
    while (!dirValida) {
        numero posizioneRandom = RAND(0, 3);
        switch (posizioneRandom) {
            case 0: if (!IS_P_VALIDO(posizione->stDestra)) { dirValida = true; dir = destra; } break;
            case 1: if (!IS_P_VALIDO(posizione->stSinistra)) { dirValida = true; dir = sinistra; } break;
            case 2: if (!IS_P_VALIDO(posizione->stSopra)) { dirValida = true; dir = sopra; } break;
            case 3: if (!IS_P_VALIDO(posizione->stSotto)) { dirValida = true; dir = sotto; } break;
        }
    }

    if (!dirValida) {
        coloreCmd(COLORE_R);
        printf("Non è possibile creare altre stanze qui (%p). Stanze adiacenti già presenti.\n", (void*) posizione);
        return;
    }

    numero probTrab = RAND(0, 100);
    if (probTrab > 65) {
        probTrab = RAND(0, 100);
        trabocchetto = probTrab <= 75 ? tegola : probTrab <= 84 ? lame : probTrab <= 92 ? caduta : burrone;
    }

    creaStanza(posizione, dir, tipo, tesoro, isSegreta, trabocchetto);
}

/**
 * La prima volta che si cerca la probabilità di trovare una stanza segreta è del 33%, la seconda 20%, la terza 15%.
 * La stanza segreta viene aggiunta alla mappa in una delle 3 direzioni disponibili (diversa da quella per la stanza successiva e precedente).
 * All'interno della stanza non possono comparire nemici.
 * @param pGiocatore Giocatore che cerca la stanza segreta
 */
static void cercaStanzaSegreta(Giocatore* pGiocatore) {
    if (!IS_P_VALIDO(pGiocatore)) { 
            logErrore("Impossibile esplorare la stanza segreta :("); 
            return; 
    }

	if (pGiocatore->cercataSegreta >= 3) {
        logErrore("Non è possibile cercare altre stanze segrete qui.");
        return;
	} 

	pGiocatore->cercataSegreta++;
    pGiocatore->turnoPassato = false;

	numero prob = RAND(0, 100);
	if ((pGiocatore->cercataSegreta == 1 && prob <= 33) ||
        (pGiocatore->cercataSegreta == 2 && prob <= 20) ||
        (pGiocatore->cercataSegreta == 3 && prob <= 15)) {

        pulisciCmd();
        coloreCmd(COLORE_H);
    	printf("\n\n\n                       ooo,    .---.\n");
        printf("                      o`  o   /    |\\________________\n");
        printf("                     `   'oooo()  | ________   _   _)\n");
        printf("                     `oo   o` \\    |/        | | | |\n");
        printf("                       `ooo'   `---'         \"-\" |_|\n\n");
        
        printf("                        Hai trovato una stanza segreta!\n\n");
        coloreCmd(COLORE_STD);

        Stanza *pos = pGiocatore->posizione;
        stanzaRandom(pos, true);
        avanza(pGiocatore, true);
        printf("\n");
        
        char scelta = ' ';
        char opzioni[9];
        do {
            printf("Seleziona una delle seguenti "); coloreCmd(COLORE_H); printf("azioni"); coloreCmd(COLORE_STD); printf(":\n");
            stampaOpzioni(pGiocatore, opzioni);
            strcpy(pGiocatore->azioniConcesse, opzioni);

            if (charInput(&scelta, opzioni) == errore) {
                logErrore("Valore non valido. Riprova.\n");
                scelta = ' ';
                continue;
            } else {
                switch (scelta) {
                    case 'T':
                        raccogliTesoro(pGiocatore);
                    break;
                    
                    case 'B': {
                        Stanza* segreta = pGiocatore->posizione;
                        Stanza* precedente = segreta->precedente;
                        torna(pGiocatore);
                        nuovaSchermata();
                        cancSegreta(segreta, precedente);
                        logAvviso("Oh! La porta alla stanza segreta è scomparsa misteriosamente...");
                    }
                    break;
                                
                    case 'G':
                        dettagliGiocatore(pGiocatore);
                    break;

                    case 'Z':
                        dettagliStanza(pGiocatore->posizione);
                    break;

                    case 'M':
                        dettagliStanze();
                    break;

                    case 'H':
                        aiuto(pGiocatore);
                    break;
                    
                    case 'X':
                        logErrore("Lascia la stanza segreta prima di abbandonare il gioco");
                    break;
                }
            }	
            
            if (scelta != 'Z') printf("\n");	

        } while (scelta != 'B');	
        
    } else {
        logAvviso("Nessuna stanza segreta trovata.");
    }
}

/**
* Stampa il numero di stanza create e, per ciascuna stanza, tutti i campi della struttura (puntatori compresi)
*/
static void dettagliStanze() {
    if(!IS_P_VALIDO(primaStanza)) return;

    if (giocoAvviato) logNota("I dettagli delle stanze sono disponibili solo in fase di creazione della mappa.");
    logNota("Le stanze segrete non sono visualizzate.");
    printf("Numero di stanze create: %d\n\n", numStanze());
    Stanza* pScan = primaStanza;
    
    do {
        dettagliStanza(pScan);
        pScan = nextStanza(pScan, false);
    } while(IS_P_VALIDO(pScan));
}

/**
 * Stampa i dettagli della stanza passata come parametro
 * @param pStanza Stanza di cui si vogliono visualizzare i dettagli
 */
static void dettagliStanza(Stanza *pStanza) {
    if (!IS_P_VALIDO(pStanza)) {
        logErrore("Errore: stanza non valida");
        return;
    }

    if(pStanza->isSegreta) {
        coloreCmd(COLORE_H); printf("STANZA SEGRETA: %s ", stanzaToString(pStanza->tipo)); coloreCmd(COLORE_V);
    } else {
        coloreCmd(COLORE_H); printf("STANZA %d: %s ", posStanza(pStanza), stanzaToString(pStanza->tipo)); coloreCmd(COLORE_V);
    }

    for (numero i = 0; i < MAX_G_NUM; i++) {
        if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->posizione == pStanza) {
            printf("(%s è qui) ", giocatori[i]->nome);
        }
    }
    coloreCmd(COLORE_STD);
    printf("\n");
    printf("- Indirizzo: %p\n", (void*) pStanza);
        
    if(numStanze() > 0 && !giocoAvviato) {
        printf("- E' presente un tesoro: %s\n", BOOL_TEXT(pStanza->tesoro != nessunTesoro));
        printf("- Il tesoro è stato raccolto: %s\n", BOOL_TEXT(pStanza->tesoroRaccolto));
        printf("- Trabocchetto: %s\n", trabocchettoToString(pStanza->trabocchetto));
        printf("- E' una stanza segreta: %s\n", BOOL_TEXT(pStanza->isSegreta));
    }

    if(IS_P_VALIDO(pStanza->stDestra)) printf("- Stanza a destra: %p (%s)\n", (void*) pStanza->stDestra, stanzaToString(pStanza->stDestra->tipo)); else printf("- Stanza a destra: nessuna\n");
    if(IS_P_VALIDO(pStanza->stSinistra)) printf("- Stanza a sinistra: %p (%s)\n", (void*) pStanza->stSinistra, stanzaToString(pStanza->stSinistra->tipo)); else printf("- Stanza a sinistra: nessuna\n");
    if(IS_P_VALIDO(pStanza->stSopra)) printf("- Stanza sopra: %p (%s)\n", (void*) pStanza->stSopra, stanzaToString(pStanza->stSopra->tipo)); else printf("- Stanza sopra: nessuna\n");
    if(IS_P_VALIDO(pStanza->stSotto)) printf("- Stanza sotto: %p (%s)\n", (void*) pStanza->stSotto, stanzaToString(pStanza->stSotto->tipo)); else printf("- Stanza sotto: nessuna\n");
    if(IS_P_VALIDO(pStanza->precedente)) printf("- Stanza precedente: %p (%s)\n\n", (void*) pStanza->precedente, stanzaToString(pStanza->precedente->tipo)); else printf("- Stanza precedente: nessuna\n\n");
}

/**
 * Restituisce la posizione (un numero) della stanza passata come parametro (la stanza iniziale ha posizione 0)
 * @param s Stanza di cui si vuole conoscere la posizione
 */
static numero posStanza(Stanza *s) {
	numero pos = 0;
	Stanza* pScan = primaStanza;
	while (IS_P_VALIDO(pScan) && pScan != s) {
		pos++;
		pScan = nextStanza(pScan, false);
	}
	return pos;
}

/**
 * Resetta il gioco eliminando tutte le stanze create e liberando la memoria allocata
 */
static void resetMappa() {
	// elimina, a partire dall'ultima, tutte le stanze create liberando la memoria allocata
	while (numStanze() > 0) {
		cancStanza();
	}
	SAFE_FREE(primaStanza);
	
	Stanza* sInit = (Stanza*) malloc(sizeof(Stanza));
    if(!jaffarSconfitto) {
        if (!IS_P_VALIDO(sInit)) {
            logErrore("Errore di allocazione in memoria.");
            terminaGioco();
        } else {
            primaStanza = sInit;
            sInit->stDestra = NULL;
            sInit->stSinistra = NULL;
            sInit->stSopra = NULL;
            sInit->stSotto = NULL;
            sInit->precedente = NULL;

            sInit->tipo = ingresso;
            sInit->tesoro = nessunTesoro;
            sInit->tesoroRaccolto = false;
            sInit->isSegreta = false;
            sInit->isDefault = true;
            sInit->trabocchetto = nessunTrabocchetto;
            sInit->nemico = nessunNemico;
            ultimaStanza = primaStanza;
        }
    }
}

/**
 * Controlla se la mappa è stata impostata correttamente: deve esserci almeno una stanza iniziale e una finale e almeno 15 stanze totali. In caso positivo, imposta la variabile di controllo a true
 */
static void controlloMappa() {
    check_mappa = IS_P_VALIDO(primaStanza) && IS_P_VALIDO(ultimaStanza) && (numStanze() >= MIN_STANZE);
}

/**
 * Configura e inizializza la mappa di gioco
 */
static void impostaMappa() {
    nuovaSchermata();
    numero scelta = nessunaImp;
    
    do {
        logAvviso("  CONFIGURAZIONE MAPPA DI GIOCO");
        logAvviso(" ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀\n");    
        const char* scelte[] = { "Inserisci nuova stanza", "Rimuovi ultima stanza", "Stampa stanze", "Genera stanze random", "Termina configurazione", NULL };
        stampaElencoScelte(true, "12345", scelte);
      
    	if (numInput(&scelta, 1, 5) == errore) {
        	logErrore("Valore non valido. Riprova.\n");
            logSpicy("---------------------------------------------------------------------------------\n");
        	continue;
      	}
      
      	switch (scelta) {
        	case nuovaStanza:
        		nuovaSchermata();
                        
                logAvviso("Dove vuoi inserire la nuova stanza?\n");
                const char* scelteDir[] = { "A destra", "A sinistra", "Sopra", "Sotto", "Annulla", NULL };
                stampaElencoScelte(true, "12345", scelteDir);
          
          		if (numInput(&scelta, 1, 5) == errore) {
            		logErrore("Valore non valido. Processo di creazione ANNULLATO.");
            		break;
          		}
          
          		if (scelta == 5) break; else {
            		numero tipo;
            		numero tesoro;
            		numero trabocchetto;
                        
                    nuovaSchermata();
            
            		logAvviso("Configurazione stanza: tipologia\n");
                    const char* scelteTipo[] = {"corridoio", "scala", "sala banchetto", "magazzino", "posto di guardia", "prigione", "armeria", "moschea", "torre", "bagni", NULL };
                    stampaElencoScelte(true, "12345678910", scelteTipo);
                    
                	if (numInput(&tipo, 1, 10) == errore) {
              			logErrore("Valore non valido. Processo di creazione ANNULLATO.");
              			break;
            		}
                        
          			nuovaSchermata();
            
            		logAvviso("Configurazione stanza: tesoro\n");
                    const char* scelteTesoro[] = { "nessun tesoro", "verde veleno", "blu guarigione", "rosso aumenta vita", "spada tagliente", "scudo", NULL };
                    stampaElencoScelte(true, "012345", scelteTesoro);
            		if (numInput(&tesoro, 0, 5) == errore) {
              			logErrore("Valore non valido. Processo di creazione ANNULLATO.");
              			break;
            		}
                        
          			nuovaSchermata();
            
            		logAvviso("Configurazione stanza: trabocchetto\n");
                    const char* scelteTrab[] = { "nessun trabocchetto", "tegola", "lame", "caduta", "burrone", NULL };
                    stampaElencoScelte(true, "01234", scelteTrab);
            		if (numInput(&trabocchetto, 0, 4) == errore) {
              			logErrore("Valore non valido. Processo di creazione ANNULLATO.");
              			break;
            		}
            
            		creaStanza(ultimaStanza, (Direzione) scelta, (TipoStanza) tipo, (TipoTesoro) tesoro, false, (TipoTrabocchetto) trabocchetto);
            
            		logSuccesso("Nuova stanza inserita con successo.\n");
          		}
                controlloMappa();
        	break;
          
        	case rimuoviStanza:
          		if (numStanze() > 0) {
            		cancStanza();
            		logSuccesso("\nUltima stanza rimossa con successo.\n");
          		} else logErrore("\nNon ci sono stanze da rimuovere.\n");
                controlloMappa();
       		break;
          
        	case stampaStanze:
          		dettagliStanze();
       		break;
          
        	case generaRandom:
          		resetMappa();
          		for (numero i = 0; i < MIN_STANZE; i++) stanzaRandom(ultimaStanza, false);
                logSuccesso("Stanze generate con successo.\n");
                controlloMappa();
        	break;
		}
        
        if (scelta != terminaImpostaMappa) attendiUtente();   
        nuovaSchermata();
                
    } while (scelta != terminaImpostaMappa);
}

/**
* Restituisce il numero di giocatori attualmente presenti
* @return Numero di giocatori attualmente presenti
*/
static numero numGiocatori() {
    numero nGiocatori = 0;
    for (numero i = 0; i < MAX_G_NUM; i++) {
        if (IS_P_VALIDO(giocatori[i])) nGiocatori++;
    }
    return nGiocatori;
}

/**
* Controlla se è stato impostato uno e un solo principe
* @return true se è stato impostato, false altrimenti
*/
static bool isPrincipeImpostato() {
    for (numero i = 0; i < numGiocatori(); i++) {
        if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->classe == principe) return true;
    }
    return false;
}

/**
* Controlla se i giocatori sono stati impostati correttamente: deve esserci un principe e, i restanti, doppleganger. In caso positivo, imposta la variabile di controllo a true
*/
static void controlloGiocatori() {
	numero contaDopp = 0;
    for (numero i = 0; i < numGiocatori(); i++) {
        if (IS_P_VALIDO(giocatori[i])) {
            if (giocatori[i]->classe == doppleganger) contaDopp++;
        }
    }
    
    check_giocatori = isPrincipeImpostato() && contaDopp == (numGiocatori() - 1);
    if(!check_giocatori) {
        logErrore("Giocatori non impostati correttamente.\nIl gioco richiede un principe e al massimo 2 doppleganger.\n");
    }
}

/**
 * Resetta i giocatori eliminando tutti i giocatori creati e liberando la memoria allocata
 */
static void resetGiocatori() {
    for (numero i = 0; i < MAX_G_NUM; i++) {
        if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->inVita) {
            giocatori[i]->inVita = false;
            SAFE_FREE(giocatori[i]);
        }
    }
    
    check_giocatori = false;
}

/**
* Crea un nuovo giocatore con i parametri passati come argomento
* @param nome Nome del giocatore
* @param classe Classe del giocatore (principe o doppleganger)
* @param posizione Stanza in cui si trova il giocatore
* @param pVitaMax Punti vita massimi del giocatore
* @param pVita Punti vita attuali del giocatore
* @param dadiAttacco Numero di dadi per l'attacco del giocatore
* @param dadiDifesa Numero di dadi per la difesa del giocatore
* @param scappa Indica il numero di volte che il giocatore può scappare
* @param ignoraTrabocchetto Indica se il giocatore ignora il primo trabocchetto
* @return Puntatore al giocatore creato
*/
static Giocatore* creaGiocatore(const char* nome, ClasseGiocatore classe, Stanza* posizione, numero pVitaMax, numero pVita, numero dadiAttacco, numero dadiDifesa, numero scappa, bool ignoraTrabocchetto) {
    Giocatore* pGiocatore = (Giocatore*) malloc(sizeof(Giocatore));
    if (!IS_P_VALIDO(pGiocatore)) {
        logErrore("Attenzione: si è verificato un errore di allocazione della memoria. Il programma verrà terminato.");
        terminaGioco();
    }
    
    strcpy(pGiocatore->nome, nome);
    pGiocatore->classe = classe;
    pGiocatore->posizione = posizione;
    pGiocatore->pVitaMax = pVitaMax;
    pGiocatore->pVita = pVita;
    pGiocatore->dadiAttacco = dadiAttacco;
    pGiocatore->dadiDifesa = dadiDifesa;
    pGiocatore->scappa = scappa;
    pGiocatore->ignoraTrabocchetto = ignoraTrabocchetto;
    pGiocatore->avanzato = false;
    pGiocatore->turnoBloccato = 0;
    pGiocatore->cercataSegreta = 0;
    pGiocatore->inVita = true;
    pGiocatore->turnoPassato = true;
    
    return pGiocatore;
}

/**
* Configura e inizializza i giocatori
* I giocatori possono essere minimo 1 (principe) e massimo 3 (principe e 2 doppleganger)
*/
static void impostaGiocatori() {
    logSpicy("---------------------------------------------------------------------------------");
    
	do {
        // reset e inizializzazione dei giocatori
    	resetGiocatori();
    	numero nGiocatori = 0;

        printf("Inserisci il "); coloreCmd(COLORE_G); printf("numero di giocatori ");  coloreCmd(COLORE_STD); printf("(minimo 1, massimo 3): ");
        if (numInput(&nGiocatori, MIN_G_NUM, MAX_G_NUM) == errore) {
            logErrore("Valore non valido. Riprova.");
            continue;
        }

        for (numero i = 0; i < nGiocatori; i++) {
            char nome[MAX_G_NOME];
            numero classe;

            printf("Inserisci il "); coloreCmd(COLORE_G); printf("nome ");  coloreCmd(COLORE_STD); printf("del giocatore %d: ", i + 1);
            if (stringInput(nome, 1, MAX_G_NOME) == errore) {
                logErrore("Nome non valido. Riprova.\n");
                i--; 
                continue;
            }

            printf("Seleziona la "); coloreCmd(COLORE_G); printf("tipologia "); coloreCmd(COLORE_STD); printf("di giocatore:\n");
            
            const char* scelteClasse[] = { "Principe", "Doppleganger", NULL };
            stampaElencoScelte(true, "12", scelteClasse);
            if (numInput(&classe, 1, 2) == errore) {
                logErrore("Valore non valido. Riprova.");
                i--;
                classe = nessunaClasse;
            } else {
            	if (classe == principe) {
            		giocatori[i] = creaGiocatore(nome, principe, primaStanza, MAX_PVITA, MAX_PVITA, INIT_DADI, INIT_DADI, MAX_PRINCIPE_SCAPPA, true);
            		logSuccesso("Principe creato con successo.");
            	} else {
            		giocatori[i] = creaGiocatore(nome, doppleganger, primaStanza, MAX_PVITA, MAX_PVITA, INIT_DADI, INIT_DADI, MAX_DOPP_SCAPPA, false);
            		logSuccesso("Doppleganger creato con successo.");
            	}
            }
            
            printf("\n");
        }
        
        controlloGiocatori();
        
    } while (!check_giocatori);
    
    attendiUtente();
}

/**
* Controlla se i giocatori sono ancora in vita
* @return true se almeno un giocatore è in vita, false altrimenti
*/
static bool giocatoriInVita() {
	for (numero i = 0; i < MAX_G_NUM; i++) {
		if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->pVita > 0) return true;
	}
	return false;
}

/**
 * Riordina in modo randomico l'array dei giocatori, ignorando eventuali valori NULL.
 */
static void riordinaGRandom() {
    for (numero i = MAX_G_NUM - 1; i > 0; i--) {
        // Trova l'ultimo elemento valido (non NULL)
        while (i > 0 && giocatori[i] == NULL) {
            i--;
        }
        if (i == 0) break;

        // Genera un indice casuale tra 0 e i
        numero j;
        do {
            j = RAND(0, i);
        } while (giocatori[j] == NULL); // Trova un indice valido

        // Scambia array[i] con array[j]
        Giocatore *temp = giocatori[i];
        giocatori[i] = giocatori[j];
        giocatori[j] = temp;
    }
}

/**
 * Stampa i dettagli del giocatore passato come parametro
 * @param pGiocatore Giocatore di cui si vogliono visualizzare i dettagli
 */
static void dettagliGiocatore(Giocatore *pGiocatore) {
	if (!IS_P_VALIDO(pGiocatore)) {
		logErrore("Errore: giocatore non valido");
		return;
	}
	
	coloreCmd(COLORE_H); printf("GIOCATORE: %s\n", pGiocatore->nome); coloreCmd(COLORE_STD);
	printf("- Tipologia: %s\n", (pGiocatore->classe == principe) ? "principe" : "doppleganger");
	printf("- Stanza: %p\n", (void*) pGiocatore->posizione);
	printf("- Punti vita : %d su %d\n", pGiocatore->pVita, pGiocatore->pVitaMax);
	printf("- Dadi attacco: %d\n", pGiocatore->dadiAttacco);
	printf("- Dadi difesa: %d\n", pGiocatore->dadiDifesa);
	printf("- Può avanzare: %s\n", BOOL_TEXT(IS_P_VALIDO(nextStanza(pGiocatore->posizione, false)) && !pGiocatore->avanzato && (pGiocatore->cercataSegreta == 0 || pGiocatore->turnoPassato)));
    printf("- Può cercare stanze segrete: %s\n", BOOL_TEXT(pGiocatore->posizione != ultimaStanza && pGiocatore->posizione != primaStanza && (!pGiocatore->avanzato || pGiocatore->turnoPassato)));
}

/**
 * Aumenta i punti vita del giocatore passato come parametro
 * @param pGiocatore Giocatore a cui aumentare i punti vita
 * @param punti Punti vita da aggiungere
 */
static void aumentaVita(Giocatore *pGiocatore, numero punti) {
    if (IS_P_VALIDO(pGiocatore)) {
        pGiocatore->pVita += punti;
        if (pGiocatore->pVita > pGiocatore->pVitaMax) {
            pGiocatore->pVita = pGiocatore->pVitaMax;
            logAvviso("Punti vita massimi raggiunti. Non è possibile aumentarli ulteriormente.");
        } 
    }
}

/**
 * Diminuisce i punti vita del giocatore passato come parametro
 * @param pGiocatore Giocatore a cui diminuire i punti vita
 * @param punti Punti vita da sottrarre
 */
static void diminuisciVita(Giocatore *pGiocatore, numero punti) {
    if (IS_P_VALIDO(pGiocatore)) {
        if (pGiocatore->pVita > punti) {
            pGiocatore->pVita -= punti;
        } else {
            pGiocatore->pVita = 0;
        }
    }
}

/**
* Quando un giocatore avanza c'è il 25% di probabilità che incontri un nemico. In caso positivo, viene generato un nemico random (60% scheletro, 40% guardia). Solo nell'ultima stanza è possibile trovare Jaffar
*/
static void randomNemico(Stanza *pStanza) {
	if (IS_P_VALIDO(pStanza)) {
		Nemico *pNemico = (Nemico*) malloc(sizeof(Nemico));
		if (!IS_P_VALIDO(pNemico)) {
			logErrore("Attenzione: si è verificato un errore di allocazione della memoria. Il programma verrà terminato.");
			terminaGioco();
		}

		if (pStanza == ultimaStanza) {
            pNemico->tipo = Jaffar;
            pNemico->pVita = JAFFAR_PVITA;
            pNemico->dadiAttacco = JAFFAR_ATTACCO;
            pNemico->dadiDifesa = JAFFAR_DIFESA;
            
            printf("Attenzione! Hai raggiunto "); coloreCmd(COLORE_R); printf("%s\n", nemicoToString(pNemico->tipo)); coloreCmd(COLORE_STD);
			pStanza->nemico = pNemico;
        } else {
			numero prob = RAND(0, 100);
			if (prob < 25) {            
            	prob = RAND(0, 100);
                        
            	if (prob < 60) {
            		pNemico->tipo = scheletro;
            		pNemico->pVita = SCHELETRO_PVITA;
            		pNemico->dadiAttacco = SCHELETRO_ATTACCO;
            		pNemico->dadiDifesa = SCHELETRO_DIFESA;
            	} else {
            		pNemico->tipo = guardia;
            		pNemico->pVita = GUARDIA_PVITA;
            		pNemico->dadiAttacco = GUARDIA_ATTACCO;
            		pNemico->dadiDifesa = GUARDIA_DIFESA;
            	}
            
            	printf("Oh noo! E' comparso un nemico nella stanza: "); coloreCmd(COLORE_R); printf("%s\n", nemicoToString(pNemico->tipo)); coloreCmd(COLORE_STD);
				pStanza->nemico = pNemico;
			} else {
				logSuccesso("Nessun nemico in vista!");
				SAFE_FREE(pStanza->nemico);
			}
		}

	}
}

/**
* Attiva l'effetto del trabocchetto presente nella stanza in cui si trova il giocatore passato come parametro
* @param pGiocatore Giocatore su cui far scattare l'effetto del trabocchetto
*/
static void trabocchetto(Giocatore *pGiocatore) {
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione)) {
		switch (pGiocatore->posizione->trabocchetto) {
			case nessunTrabocchetto:
				logSuccesso("Fin'ora nessun problema! Chissà cosa si nasconde in questa stanza...");
                break;
			case tegola:
                if (pGiocatore->ignoraTrabocchetto) {
                    logSuccesso("Hai evitato il trabocchetto della tegola! Fai più attenzione la prossima volta.");
                    pGiocatore->ignoraTrabocchetto = false;
                } else {
                    logErrore("Attenzione! Hai calpestato una tegola trabocchetto. Perdi 1 punto vita.");
                    diminuisciVita(pGiocatore, 1);
                }
				break;
			case lame:
				if (pGiocatore->ignoraTrabocchetto) {
                    logSuccesso("Hai evitato il trabocchetto delle lame! Fai più attenzione la prossima volta.");
                    pGiocatore->ignoraTrabocchetto = false;
                } else {
                    logErrore("Attenzione! Hai attivato un meccanismo di lame. Perdi 2 punti vita.");
                    diminuisciVita(pGiocatore, 2);
                }
				break;
			case caduta:
                if (RAND(0, 1)) { 
                    if (pGiocatore->ignoraTrabocchetto) {
                        logSuccesso("Hai evitato il trabocchetto della caduta! Fai più attenzione la prossima volta.");
                        pGiocatore->ignoraTrabocchetto = false;
                    } else {
                        logErrore("Attenzione! Sei caduto. Perdi 1 punto vita.");
                        diminuisciVita(pGiocatore, 1);
                    }
                } else {
                    if(pGiocatore->ignoraTrabocchetto) {
                        logSuccesso("Hai evitato il trabocchetto della caduta! Fai più attenzione la prossima volta.");
                        pGiocatore->ignoraTrabocchetto = false;
                    } else {
                        logErrore("Attenzione! Sei caduto gravemente. Perdi 2 punti vita.");
                        diminuisciVita(pGiocatore, 2);
                    }
				}
				break;
			case burrone:
				if (RAND(0, 1)) { 
                    if (pGiocatore->ignoraTrabocchetto) {
                        logSuccesso("Hai evitato il trabocchetto del burrone! Fai più attenzione la prossima volta.");
                        pGiocatore->ignoraTrabocchetto = false;
                        pGiocatore->turnoBloccato = true;
                    } else {
                        logErrore("Attenzione! Sei caduto in un burrone. Perdi 1 punto vita.");
                        diminuisciVita(pGiocatore, 1);
                    }
				} else {
                    if (pGiocatore->ignoraTrabocchetto) {
                        logSuccesso("Hai evitato il trabocchetto del burrone! Fai più attenzione la prossima volta.");
                        pGiocatore->ignoraTrabocchetto = false;
                    } else {
					    logErrore("Attenzione! Sei caduto in un burrone. Perdi 2 punti vita.\nPer riprenderti dovrai passare il prossimo turno :(");
					    diminuisciVita(pGiocatore, 2);
                        pGiocatore->turnoBloccato = true;
                    }
				}
				break;
		}
		attendiUtente();
	}
}

/**
* Permette al giocatore passato come parametro di raccolgiere il tesoro presente nella stanza in cui si trova
* @param pGiocatore Giocatore che vuole raccogliere il tesoro
*/
static void raccogliTesoro(Giocatore* pGiocatore) {
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione)) {
        if (pGiocatore->posizione->tesoro != nessunTesoro && !pGiocatore->posizione->tesoroRaccolto) {
            switch (pGiocatore->posizione->tesoro) {
                case verdeVeleno:
                    logErrore("Attenzione! Hai raccolto un veleno verde. Perdi 1 punto vita.");
                    diminuisciVita(pGiocatore, 1);
                    break;
                case bluGuarigione:
                    logSuccesso("Hai raccolto una pozione blu. Guadagni 1 punto vita.");
                    aumentaVita(pGiocatore, 1);
                    break;
                case rossoAumentaVita:
                    logSuccesso("Hai raccolto una pozione rossa. La tua vita massima aumenta di 1 punto e la tua vita viene completamente ripristinata.");
                    pGiocatore->pVitaMax++;
		    pGiocatore->pVita = pGiocatore->pVitaMax;
                    break;
                case spadaTagliente:
                    logSuccesso("Hai raccolto una spada tagliente. Guadagni un dado attacco.");
                    pGiocatore->dadiAttacco++;
                    break;
                case scudo:
                    logSuccesso("Hai raccolto uno scudo. Guadagni un dado difesa.");
                    pGiocatore->dadiDifesa++;
                    break;
				default: break;
            }
            pGiocatore->posizione->tesoroRaccolto = true;
        }
    }
}

/**
* Avanza la posizione del giocatore nell'unica posizione disponibile, se disponibile
* @param pGiocatore Giocatore da far avanzare
*/
static void avanza(Giocatore *pGiocatore, bool segreta) {
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione)) {
		Stanza* pScan = nextStanza(pGiocatore->posizione, segreta);
		if (IS_P_VALIDO(pScan)) {
            pScan->precedente = pGiocatore->posizione;
			pGiocatore->posizione = pScan;
			coloreCmd(COLORE_V);
			if (!pGiocatore->posizione->isSegreta) {
			    printf("%s è avanzato nella stanza (%d): %s.\n", pGiocatore->nome, posStanza(pGiocatore->posizione), stanzaToString(pGiocatore->posizione->tipo));
            } else {
		        printf("%s è avanzato nella stanza (SEGRETA): %s.\n", pGiocatore->nome, stanzaToString(pGiocatore->posizione->tipo));
            }
            coloreCmd(COLORE_STD);
			pGiocatore->avanzato = true;
            pGiocatore->turnoPassato = false;
            trabocchetto(pGiocatore);

            if (pGiocatore->pVita == 0) return;
			if(!segreta) randomNemico(pScan);
		} else {
			logErrore("Non ci sono stanze disponibili in cui avanzare.");
		}
	}
}

/**
* Stampa le opzioni disponibili per il giocatore passato come parametro ed aggiorna l'array delle azioni disponibili.
* Il giocatore può: [A] avanzare (se ci sono stanze disponibili (nextStanza() != NULL) e non è già avanzato (pGiocatore->avanzato)), [C] combattere se è presente un nemico, [F] fuggire se è presente un nemico, [S] cercare una stanza segreta, [T] raccogliere il tesoro
* @param pGiocatore Giocatore di cui stampare le opzioni
* @param azioni Array contenente le azioni disponibili
*/
static void stampaOpzioni(Giocatore* pGiocatore, char* azioni) {
  	numero opz = 0;
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione)) {
		if(IS_P_VALIDO(pGiocatore->posizione->nemico)) {
			azioni[opz++] = 'C';
			printf(" ["); coloreCmd(COLORE_H); printf("C"); coloreCmd(COLORE_STD); printf("] Combatti\n");
			
            azioni[opz++] = 'F';
			printf(" ["); coloreCmd(COLORE_H); printf("F"); coloreCmd(COLORE_STD); printf("] Fuggi\n");
		} else {
			azioni[opz++] = 'G';      
			printf(" ["); coloreCmd(COLORE_H); printf("G"); coloreCmd(COLORE_STD); printf("] Info giocatore\n");
			
            azioni[opz++] = 'Z';
			printf(" ["); coloreCmd(COLORE_H); printf("Z"); coloreCmd(COLORE_STD); printf("] Info zona\n");

            azioni[opz++] = 'M';
            printf(" ["); coloreCmd(COLORE_H); printf("M"); coloreCmd(COLORE_STD); printf("] Info mappa\n");
			
			azioni[opz++] = 'X';      
			printf(" ["); coloreCmd(COLORE_H); printf("X"); coloreCmd(COLORE_STD); printf("] Abbandona partita\n");

            // Il giocatore può avanzare se: (1) c'è una stanza disponibile, (2) non è già avanzato e (3) non ha già cercato la stanza segreta         
			if (IS_P_VALIDO(nextStanza(pGiocatore->posizione, false)) && !pGiocatore->avanzato && (pGiocatore->cercataSegreta == 0 || pGiocatore->turnoPassato)) {
				azioni[opz++] = 'A';
				printf(" ["); coloreCmd(COLORE_H); printf("A"); coloreCmd(COLORE_STD); printf("] Avanza\n");
			}  
                
			if (pGiocatore->posizione != ultimaStanza && pGiocatore->posizione != primaStanza && (!pGiocatore->avanzato || pGiocatore->turnoPassato )) {
				azioni[opz++] = 'S';
				printf(" ["); coloreCmd(COLORE_H); printf("S"); coloreCmd(COLORE_STD); printf("] Cerca stanza segreta\n");
			}
        
			if (pGiocatore->posizione->tesoro != nessunTesoro && !pGiocatore->posizione->tesoroRaccolto) {
				azioni[opz++] = 'T';
				printf(" ["); coloreCmd(COLORE_H); printf("T"); coloreCmd(COLORE_STD); printf("] Raccogli tesoro\n");
			}
			
			if (pGiocatore->posizione->isSegreta) {
                azioni[opz++] = 'B';
                printf(" ["); coloreCmd(COLORE_H); printf("B"); coloreCmd(COLORE_STD); printf("] Torna indietro\n");
			} else {
                azioni[opz++] = 'P';
			    printf(" ["); coloreCmd(COLORE_H); printf("P"); coloreCmd(COLORE_STD); printf("] Passa turno\n");
            }
        }

        azioni[opz++] = 'H';
        printf(" ["); coloreCmd(COLORE_H); printf("H"); coloreCmd(COLORE_STD); printf("] Aiuto o suggerimento\n");

        azioni[opz] = '\0';
        printf("\n> ");
    }              
}

/**
 * Fornisce un suggerimento al giocatore passato come parametro in base alla stanza in cui si trova,
 * la sua salute, la presenza di un nemico e la sua forza, la presenza di un tesoro e di altri giocatori (avversari).
 * Inoltre, valuta lo stato delle azioni disponibili.
 * @param pGiocatore Giocatore a cui fornire il suggerimento
 */
static void aiuto(Giocatore *pGiocatore) {
    if (pGiocatore == NULL || pGiocatore->posizione == NULL) {
        logNota("Non c'è molto da fare senza sapere dove ti trovi...");
        return;
    }

    Stanza *stanza = pGiocatore->posizione;
    Nemico *nemico = stanza->nemico;

    // Controlli sulle azioni permesse
    bool azioneAvanzaPresente = false;
    bool azioneCercaPresente = false;
    int conteggioAzioni = 0;

    for (int i = 0; i < 8; i++) {
        if (pGiocatore->azioniConcesse[i] != '\0') {
            conteggioAzioni++;
            if (pGiocatore->azioniConcesse[i] == 'A') azioneAvanzaPresente = true;
            if (pGiocatore->azioniConcesse[i] == 'S') azioneCercaPresente = true;
        }
    }

    int controlloCasuale = RAND(0, 2);
    bool contestoStd = !IS_P_VALIDO(nemico) || nemico->tipo == nessunNemico;

    if (controlloCasuale == 0 && !azioneAvanzaPresente && contestoStd) {
        logNota("Non puoi avanzare ulteriormente in questo turno. Riconsidera i tuoi passi.");
    } else if (controlloCasuale == 1 && !azioneCercaPresente && contestoStd) {
        logNota("Hai forse cercato fin troppi segreti qui? O è tempo di avanzare?");
    } else if (controlloCasuale == 2 && conteggioAzioni < 8 && contestoStd) {
        logNota("Ti senti limitato nelle tue mosse? Usa 'P' per fermarti e riflettere.");
    }

    // Controlli basati sul contesto
    if (IS_P_VALIDO(nemico) && nemico->tipo != nessunNemico) {
        switch (nemico->tipo) {
            case scheletro:
                if (pGiocatore->pVita < nemico->pVita) {
                    logNota("Il tuo avversario è parecchio ossuto, sembra più forte di te. Forse è meglio fuggire.");
                } else {
                    logNota("Uno scheletro si para davanti. Non dovrebbe essere difficile abbatterlo.");
                }
            break;

            case guardia:
                if (pGiocatore->pVita < nemico->pVita) {
                    logNota("Questa guardia armata sembra possente. Una ritirata potrebbe essere saggia.");
                } else if (pGiocatore->pVita == nemico->pVita) {
                    logNota("La guardia è un avversario equilibrato. Sii pronto per un combattimento intenso.");
                } else {
                    logNota("Combatti con coraggio! Nulla è impossibile per un avventuriero come te.");
                }
            break;

            case Jaffar:
                if (pGiocatore->pVita < nemico->pVita) {
                    logNota("Jaffar è un avversario temibile e sembra più forte di te. Fuggire potrebbe essere l'opzione migliore.");
                } else if (pGiocatore->pVita == nemico->pVita) {
                    logNota("Jaffar è alla tua altezza. Questo sarà uno scontro memorabile.");
                } else {
                    logNota("Hai un vantaggio su Jaffar, ma non sottovalutarlo. È il nemico finale!");
                }
            break;

            default:
                logNota("Un nemico sconosciuto si avvicina. Valuta attentamente la situazione.");
            break;
        }
    } else {
        numero tipoSuggerimento = RAND(1, 4);
        switch (tipoSuggerimento) {
            case 1: // Suggerimento sulla stanza attuale
                switch (stanza->tipo) {
                    case ingresso:
                        logNota("Ogni viaggio inizia da qui. Ma non è detto che sia facile.");
                    break;

                    case scala:
                        logNota("Una scala può portarti in alto o in basso, ma sempre verso un cambiamento.");
                    break;

                    case armeria:
                        if (stanza->tesoro == spadaTagliente || stanza->tesoro == scudo) {
                            logNota("Un'arma o uno scudo potrebbero essere qui per chi osa cercarli.");
                        } else {
                            logNota("L'armeria sembra spoglia. Forse qualcuno è stato qui prima di te.");
                        }
                    break;

                    default:
                        logNota("La stanza in cui sei potrebbe nascondere segreti. Fidati del tuo istinto.");
                    break;
                }
            break;

            case 2: // Suggerimento sulla salute del giocatore
                if (pGiocatore->pVita <= pGiocatore->pVitaMax / 2) {
                    logNota("La tua salute è bassa. Cerca un modo per curarti prima che sia troppo tardi.");
                } else if (pGiocatore->pVita < pGiocatore->pVitaMax) {
                    logNota("Non sei al massimo della forma, ma puoi ancora combattere. Preparati.");
                } else {
                    logNota("La tua salute è al massimo. Usa questo vantaggio saggiamente.");
                }
            break;

            case 3: // Suggerimento sul tesoro nella stanza attuale
                if (stanza->tesoro != nessunTesoro && !stanza->tesoroRaccolto) {
                    switch (stanza->tesoro) {
                        case verdeVeleno:
                            logNota("C'è un liquido verde qui. Attento: potrebbe essere un veleno.");
                            break;
                        case bluGuarigione:
                            logNota("Un liquido blu potrebbe aiutarti a recuperare le forze.");
                            break;
                        case rossoAumentaVita:
                            logNota("Un tesoro rosso brillante potrebbe aumentare la tua forza vitale.");
                            break;
                        case spadaTagliente:
                            logNota("Una spada affilata potrebbe darti un vantaggio in battaglia.");
                            break;
                        case scudo:
                            logNota("Uno scudo robusto ti aiuterà a proteggerti dai danni.");
                            break;
                        default:
                            logNota("Un oggetto misterioso potrebbe cambiare le sorti del gioco.");
                            break;
                    }
                } else {
                    logNota("Nessun tesoro visibile qui. Ma potrebbe esserci altro da scoprire.");
                }
            break;

            case 4: { // Suggerimento sul trabocchetto nella stanza successiva 
                Stanza *stanzaSuccessiva = nextStanza(stanza, false);
                if (IS_P_VALIDO(stanzaSuccessiva) && stanzaSuccessiva->trabocchetto != nessunTrabocchetto) {
                    logNota("Nella stanza successiva potrebbe esserci una trappola. Procedi con cautela.");
                } else {
                    logNota("La stanza successiva sembra sicura, ma non abbassare la guardia.");
                }
            }
            break;

            default:
                logNota("Il destino è incerto. Ma ogni mossa conta.");
            break;
        }
    }
}

/**
* Torna indietro nella posizione precedente, se disponibile
* @param pGiocatore Giocatore da far tornare indietro
*/
static void torna(Giocatore *pGiocatore) {
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione)) {
		if (IS_P_VALIDO(pGiocatore->posizione->precedente)) {
            // dealloca il nemico presente nella stanza da cui si è fuggiti
            SAFE_FREE(pGiocatore->posizione->nemico);
			pGiocatore->posizione = pGiocatore->posizione->precedente;
            pGiocatore->avanzato = true;
            pGiocatore->cercataSegreta = 3;
			coloreCmd(COLORE_V);
			printf("%s è tornato nella stanza (%d): %s.\n", pGiocatore->nome, posStanza(pGiocatore->posizione), stanzaToString(pGiocatore->posizione->tipo));
			coloreCmd(COLORE_STD);
			pGiocatore->avanzato = true;
		} else {
			logErrore("Non ci sono stanze disponibili in cui tornare.");
		}
	}
}

/**
 * Stampa la schermata di gioco relativa ad un combattimento tra un giocatore e un nemico
 * @param pGiocatore Giocatore coinvolto nel combattimento
 */
static void infoCombattimento(Giocatore* pGiocatore) {
    if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione) && IS_P_VALIDO(pGiocatore->posizione->nemico)) {
        coloreCmd(COLORE_H);
    	printf("\n\n\n                          |\\                     /)\n");
    	printf("                        /\\_\\\\__               (_//\n");
    	printf("                       |   `>\\-`     _._       //`)\n");
    	printf("                        \\ /` \\\\  _.-`:::`-._  //\n");
    	printf("                         `    \\|`    :::    `|/\n");
    	printf("                               |     :::     |\n");
    	printf("                               |.....:::.....|\n");
    	printf("                               |:::::::::::::|\n");
    	printf("                               |     :::     |\n");
    	printf("                               \\     :::     /\n");
    	printf("                                \\    :::    /\n");
    	printf("                                 `-. ::: .-'\n");
    	printf("                                  //`:::`\\\\\n");
    	printf("                                //   '   \\\\\n");
    	printf("                                |/         \\\\\n\n");
        coloreCmd(COLORE_STD);
    }
}

static void dadiCombattimento(numero* riferimento) {
    numero random = LANCIA_DADO();
    printf(" [*] È uscito %d: ", random);
    switch (random) {
        case 0 ... 3:
            printf("un dado sprecato!\n");
        break;
        
        case 4 ... 5:
            printf("il lancio ha avuto successo\n");
            (*riferimento)++;
        break;
        
        case 6:
            printf("lancio critico!\n");
            (*riferimento) += 2;
        break;
    }
}

/**
* Simula un turno di combattimento tra un attaccante e un difensore
* @param pGiocatore Giocatore che attacca o si difende
* @param attaccante Segnala se il giocatore passato come parametro attacca o se si difende
*/
static void eseguiTurno(Giocatore* pGiocatore, bool attaccante) {
    if (!IS_P_VALIDO(pGiocatore) || !IS_P_VALIDO(pGiocatore->posizione) || !IS_P_VALIDO(pGiocatore->posizione->nemico)) return;

    numero danniTotali = 0, difesaTotale = 0, danniNetti = 0;
    Nemico* pNemico = pGiocatore->posizione->nemico;

    if (attaccante) {
        for (numero i = 0; i < pGiocatore->dadiAttacco; i++) {
            coloreCmd(COLORE_G);
	        printf("\n [!] Lancia il %d dado (su %d disponibili) e attacca... [Premi INVIO]", i+1, pGiocatore->dadiAttacco);
            coloreCmd(COLORE_STD);
            getchar();
            
            dadiCombattimento(&danniTotali);
        }
        
        // Calcolo della difesa
        for (numero i = 0; i < pNemico->dadiDifesa; i++) {
            coloreCmd(COLORE_G);
	        printf("\n [!] Il nemico lancia il %d dado (su %d disponibili) e si difende... [PREMI INVIO]", i+1, pNemico->dadiDifesa);
            coloreCmd(COLORE_STD);
            getchar();
            
            dadiCombattimento(&difesaTotale);
        }

        danniNetti = (danniTotali > difesaTotale) ? (danniTotali - difesaTotale) : 0;
        if (danniNetti > 0) {
            coloreCmd(COLORE_V);

            printf("\n [!] BRAVO! I tuoi lanci ti hanno permesso di colpire il nemico! Gli infliggi %u ", danniNetti);
            if(danniNetti == 1) printf("danno.\n");
            else printf("danni.\n");

            if (danniNetti > pNemico->pVita) pNemico->pVita = 0; else pNemico->pVita -= danniNetti;
            coloreCmd(COLORE_STD);
        } else {
            logErrore("\n [!] UFFA! Il nemico si è difeso troppo bene! NON infliggi danni.");
        }
    } else {
    	// attacca il nemico
        for (numero i = 0; i < pNemico->dadiAttacco; i++) {
            coloreCmd(COLORE_G);
	        printf("\n [!] %s lancia il %d dado (su %d disponibili) e attacca... [PREMI INVIO]", nemicoToString(pNemico->tipo), i+1, pNemico->dadiAttacco);
            coloreCmd(COLORE_STD);
            getchar();
            
            dadiCombattimento(&danniTotali);
        }
        
        for (numero i = 0; i < pGiocatore->dadiDifesa; i++) {
            coloreCmd(COLORE_G);
	        printf("\n [!] Lancia il %d dado (su %d disponibili) e difenditi... [Premi INVIO]", i+1, pGiocatore->dadiDifesa);
            coloreCmd(COLORE_STD);
            getchar();
            
            dadiCombattimento(&difesaTotale);
        }
        
        danniNetti = (danniTotali > difesaTotale) ? (danniTotali - difesaTotale) : 0;
        if (danniNetti > 0) {
        	coloreCmd(COLORE_R);
            
            printf("\n [!] ATTENZIONE! Il nemico ha realizzato ottimi lanci e ti ha colpito! Subisci %u ", danniNetti);
            if(danniNetti == 1) printf("danno.\n");
            else printf("danni.\n");
        
            diminuisciVita(pGiocatore, danniNetti);
            coloreCmd(COLORE_STD);
        } else {
            logSuccesso("\n [!] MAGNIFICO! Ti sei difeso troppo bene ;) NON subisci danni.");
        }
    }
    
    printf("\n");
}

/**
* Simula il combattimento tra il giocatore passato come parametro e il nemico presente nella stanza in cui si trova
* @param pGiocatore Giocatore che deve combattere. Chi fa un numero più alto attacca per primo, l'altro si difende e poi ci si inverte.
* I due combattenti tirano 1 dado a 6 facce, chi ottiene il punteggio più alto attacca per primo e l’altro si difende, poi attacca l’altro ed il primo si difende.
* Ad ogni turno di combattimento si ritira il dado per sapere chi attacca per primo.
* Un valore >= 4 è un colpo a segno (attacco o difesa). Un valore == 6 è un colpo critico (doppio attacco o difesa).
* Per ogni combattimento vinto si guadagna 1 punto vita
*/
static void combatti(Giocatore *pGiocatore) {
	if (IS_P_VALIDO(pGiocatore) && IS_P_VALIDO(pGiocatore->posizione) && IS_P_VALIDO(pGiocatore->posizione->nemico)) {       
		Nemico *pNemico = pGiocatore->posizione->nemico;
        numero tipoNemico = pNemico->tipo;

		while (pNemico->pVita > 0 && pGiocatore->pVita > 0) {
            pulisciCmd();
			infoCombattimento(pGiocatore);
            printf(" ["); coloreCmd(COLORE_H); printf("PV"); coloreCmd(COLORE_STD); printf("] giocatore: %d\n", pGiocatore->pVita);
            printf(" ["); coloreCmd(COLORE_H); printf("PV"); coloreCmd(COLORE_STD); printf("] nemico: %d\n", pNemico->pVita);
			
            // Determina chi attacca per primo
            coloreCmd(COLORE_G);
	        printf("\n\n [?] Lancia il dado per determinare chi attacca per primo... [Premi INVIO]");
            coloreCmd(COLORE_STD);
            getchar();
            printf("\n");
			numero dadoG = LANCIA_DADO();
			numero dadoN = LANCIA_DADO();

			if (dadoG >= dadoN) {
				printf(" [*] %s inizia per primo!\n", pGiocatore->nome);
				eseguiTurno(pGiocatore, true);
                
                if (pNemico->pVita <= 0 || pGiocatore->pVita <= 0) break;
                
                attendiUtente();
                printf("\n [*] Ora è il turno del nemico!\n");
                eseguiTurno(pGiocatore, false);
                attendiUtente();
			} else {
				printf(" [*] Il nemico inizia per primo!\n");
				eseguiTurno(pGiocatore, false);
                                
                if (pNemico->pVita <= 0 || pGiocatore->pVita <= 0) break;
                                
                attendiUtente();
                printf("\n [*] Ora è il turno di %s!\n", pGiocatore->nome);
                eseguiTurno(pGiocatore, true);
                attendiUtente();
			}
		}

        // Dealloca il nemico
        SAFE_FREE(pGiocatore->posizione->nemico);

		if (pGiocatore->pVita <= 0) {
            logErrore("\n [!] Sei stato sconfitto...\n");
        } else if (tipoNemico == Jaffar) {
            vittoria(pGiocatore);
            jaffarSconfitto = true;
        } else {
            logSuccesso("\n [!] Hai sconfitto il nemico! Guadagni 1 punto vita.\n");
            aumentaVita(pGiocatore, 1);
        }

        attendiUtente();
        nuovaSchermata();
	}
}

static void stampaRound(numero n) {
	coloreCmd(COLORE_H);
    printf("\n\n\n---------------------------------------------------------------------------------\n\n");
    printf("                      ▗▄▄▖  ▗▄▖ ▗▖ ▗▖▗▖  ▗▖▗▄▄▄\n"); 
	printf("                      ▐▌ ▐▌▐▌ ▐▌▐▌ ▐▌▐▛▚▖▐▌▐▌  █     %02d\n", n);
	printf("                      ▐▛▀▚▖▐▌ ▐▌▐▌ ▐▌▐▌ ▝▜▌▐▌  █   ▀▀▀▀▀▀\n");
	printf("                      ▐▌ ▐▌▝▚▄▞▘▝▚▄▞▘▐▌  ▐▌▐▙▄▄▀\n\n");
    printf("---------------------------------------------------------------------------------\n\n");
    coloreCmd(COLORE_STD);
}

/**
 * Implementa la morte di un giocatore passato come parametro.
 * Un giocatore muore quando i suoi punti vita sono pari a 0 o quando si seleziona l'opzione 'X' per abbandonare la partita nel menu di gioco.
 * @param pGiocatore Giocatore che è morto
 */
static void morte(Giocatore *pGiocatore) {
    if (IS_P_VALIDO(pGiocatore) && pGiocatore->inVita) {
        coloreCmd(COLORE_R);
        printf("Oh nooo! %s è morto :(\n", pGiocatore->nome);
        coloreCmd(COLORE_STD);
        pGiocatore->pVita = 0;
        pGiocatore->inVita = false;
        
        // Rimuovi il giocatore dall'array
        for (numero i = 0; i < MAX_G_NUM; i++) {
            if (giocatori[i] == pGiocatore) {
                SAFE_FREE(giocatori[i]);
                break;
            }
        }
    }
}

/**
 * Tutti i giocatori sono morti, quindi la partita è terminata.
 */
static void sconfitta() {
	pulisciCmd();
    coloreCmd(COLORE_R);
    printf("\n\n\n---------------------------------------------------------------------------------\n\n");
    printf("                    ▗▖ ▗▖ ▗▄▖ ▗▄▄▄▖    ▗▄▄▖ ▗▄▄▄▖▗▄▄▖  ▗▄▄▖ ▗▄▖\n"); 
	printf("                    ▐▌ ▐▌▐▌ ▐▌  █      ▐▌ ▐▌▐▌   ▐▌ ▐▌▐▌   ▐▌ ▐▌\n");
	printf("                    ▐▛▀▜▌▐▛▀▜▌  █      ▐▛▀▘ ▐▛▀▀▘▐▛▀▚▖ ▝▀▚▖▐▌ ▐▌\n");
	printf("                    ▐▌ ▐▌▐▌ ▐▌▗▄█▄▖    ▐▌   ▐▙▄▄▖▐▌ ▐▌▗▄▄▞▘▝▚▄▞▘\n\n");
	printf("---------------------------------------------------------------------------------\n\n");
    printf("Tutti i giocatori sono morti o hanno abbandonato la partita.\nLa principessa è caduta nelle mani di Jaffar :(\n\n");
    coloreCmd(COLORE_STD);
    attendiUtente();

    resetMappa();
    check_mappa = false;
    resetGiocatori();
    check_giocatori = false;

    nuovaSchermata();
}

/**
 * Jaffar è stato sconfitto, quindi la partita è terminata.
 * @param pGiocatore Giocatore che ha sconfitto Jaffar
 */
static void vittoria(Giocatore *pGiocatore) {
    if (!IS_P_VALIDO(pGiocatore)) return;
	pulisciCmd();
    coloreCmd(COLORE_V);
    printf("---------------------------------------------------------------------------------\n\n");
    printf("                    ▗▖ ▗▖ ▗▄▖ ▗▄▄▄▖    ▗▖  ▗▖▗▄▄▄▖▗▖  ▗▖▗▄▄▄▖▗▄▖ \n"); 
	printf("                    ▐▌ ▐▌▐▌ ▐▌  █      ▐▌  ▐▌  █  ▐▛▚▖▐▌  █ ▐▌ ▐▌\n");
	printf("                    ▐▛▀▜▌▐▛▀▜▌  █      ▐▌  ▐▌  █  ▐▌ ▝▜▌  █ ▐▌ ▐▌\n");
	printf("                    ▐▌ ▐▌▐▌ ▐▌▗▄█▄▖     ▝▚▞▘ ▗▄█▄▖▐▌  ▐▌  █ ▝▚▄▞▘\n\n");
	printf("---------------------------------------------------------------------------------\n\n");
    if (pGiocatore->classe == principe) {
        printf("Complimenti %s! Hai sconfitto Jaffar e salvato la principessa!\n\n", pGiocatore->nome);
    } else {
        printf("Oh no! Il principe è stato battuto dai doppleganger del Visir, %s ha raggiunto la principessa.\n\n", pGiocatore->nome);
    }
    scriviStorico(pGiocatore->nome);
    coloreCmd(COLORE_STD);
    attendiUtente();

    resetMappa();
    check_mappa = false;
    resetGiocatori();
    check_giocatori = false;

    nuovaSchermata();
}

/**
* Restituisce una stringa del tipo "gg-mm-aaaa" contenente la data attuale
*/
static char* timestamp() {
	static char data[15];
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	strftime(data, sizeof(data), "%d-%m-%Y", tm_info);
	return data;
}

/**
* Memorizza in un file CSV il nome del giocatore che ha vinto la partita
* @param nome Nome del giocatore che ha vinto la partita
* @file storico.csv File in cui vengono memorizzati i nomi e le date dei giocatori che hanno vinto la partita
* @file tmp.csv File temporaneo utilizzato per la scrittura del file storico.csv
*/
static void scriviStorico(const char *nome) {
    FILE *fileStorico = fopen("storico.csv", "r");
    if (fileStorico == NULL) {
        fileStorico = fopen("storico.csv", "w");
        if (fileStorico == NULL) {
            logErrore("Errore nella creazione di storico.csv.");
            return;
        }

        fprintf(fileStorico, "> %s, il giorno %s\n", nome, timestamp());
        fclose(fileStorico);
        return;
    }

    FILE *fileTmp = fopen("tmp.csv", "w");
    if (fileTmp == NULL) {
        logErrore("Errore nella creazione del file temporaneo.");
        fclose(fileStorico);
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fileStorico) != NULL) {
        fputs(buffer, fileTmp);
    }

    fclose(fileStorico);
    fclose(fileTmp);

    fileStorico = fopen("storico.csv", "w");
    if (fileStorico == NULL) {
        logErrore("Errore nel riaprire storico.csv.");
        remove("tmp.csv");
        return;
    }

    fprintf(fileStorico, "> \x1b[1;33m%s\x1b[1;0m, il giorno %s\n", nome, timestamp());

    fileTmp = fopen("tmp.csv", "r");
    if (fileTmp == NULL) {
        logErrore("Errore nel riaprire tmp.csv.");
        fclose(fileStorico);
        return;
    }

    while (fgets(buffer, sizeof(buffer), fileTmp) != NULL) {
        fputs(buffer, fileStorico);
    }

    fclose(fileStorico);
    fclose(fileTmp);
    remove("tmp.csv");
}

/**
* Legge il file storico.csv e stampa a video i nomi dei giocatori che hanno vinto la partita
* @file storico.csv File in cui vengono memorizzati i nomi e le date dei giocatori che hanno vinto la partita
*/
static void leggiStorico() {
    FILE *file = fopen("storico.csv", "r");
    if (file == NULL) {
        printf("Nessun dato memorizzato.\n");
        return;
    }

    char buffer[256];
    numero conta = 0;
    bool valido = false;
    while (fgets(buffer, sizeof(buffer), file) != NULL && conta < 3) {
        printf("%s", buffer);
        valido = true;
        conta++;
    }

    if (!valido) {
        printf("Nessun dato memorizzato.\n");
    }

    fclose(file);
}

/*
 *   DEFINIZIONI DELLE FUNZIONI DI GIOCO
 */

/**
* Funzione di inizializzazione del gioco
*/
extern void impostaGioco() {
    initRandom();
    numero scelta = nessunaImp;
    
    // reset e inizializzazione della prima stanza
    jaffarSconfitto = false;
    resetMappa();
    
    do {
      	nuovaSchermata();
    	logAvviso("  CONFIGURAZIONE GIOCO ");
        logAvviso(" ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀\n");        
     	printf(" ["); coloreCmd(COLORE_H); printf("1"); coloreCmd(COLORE_STD); printf("] Imposta giocatori ");
        if(check_giocatori) {
            logSuccesso("(già impostati)");
        } else {
            logErrore("(da impostare)");
        }
               
        printf(" ["); coloreCmd(COLORE_H); printf("2"); coloreCmd(COLORE_STD); printf("] Imposta mappa ");
        if (check_mappa) {
            logSuccesso("(già impostata - modificabile)");
        } else {
            logErrore("(da impostare)");
        }
        
        printf(" ["); coloreCmd(COLORE_H); printf("3"); coloreCmd(COLORE_STD); printf("] Termina impostazioni\n\n> ");
      
        if (numInput(&scelta, 1, 3) == errore) {
            logErrore("Valore non valido. Riprova.\n");
            attendiUtente();
            continue;
        }
        
        switch (scelta) {
            case impGiocatori:
              	if (check_giocatori && conferma("Proseguendo i giocatori precedentemente impostati verranno sovrascritti") == false) {
                    break;
                } else impostaGiocatori();
                break;
                
            case impMappa:
                impostaMappa();
                break;
        }
        
    } while (scelta != menuTermina);
    
    nuovaSchermata();
}

extern void gioca() {
    if(!check_giocatori) {
        logErrore("Giocatori non impostati correttamente.\nIl gioco richiede un principe e al massimo 2 doppleganger.\n");
        attendiUtente();
        return;
    }
    
    if(!check_mappa) {
        logErrore("Mappa non impostata correttamente.\nIl gioco necessita di almeno 15 stanze.\n");
        attendiUtente();
        return;
    }

    giocoAvviato = true;
    
    // definisce fino a quando il gioco è in corso (tutti i giocatori sono in vita e Jaffar non è stato sconfitto), suddividendolo in Round
    while (giocatoriInVita() && jaffarSconfitto == false) {
        pulisciCmd();
        stampaRound(++roundGioco);
        printf("Giocatori ancora in vita: "); coloreCmd(COLORE_V); printf("%d\n\n", numGiocatori()); coloreCmd(COLORE_STD);
        attendiUtente();
        
        nuovaSchermata();
        
        // Definisce turni random per ogni Round
    	riordinaGRandom();
        for (numero i = 0; i < MAX_G_NUM; i++) {
        	if(IS_P_VALIDO(giocatori[i]) && giocatori[i]->pVita > 0 && giocatori[i]->turnoBloccato == false) {
                
				numero salute = (numero)((giocatori[i]->pVita / giocatori[i]->pVitaMax) * 100);
				printf("È il turno di "); coloreCmd(COLORE_H); printf("%s", giocatori[i]->nome); coloreCmd(COLORE_STD); printf("!\n\n");
        		printf("- Stato: ");
	            switch (salute) {
    				case 0 ... 50:
       					coloreCmd(COLORE_R);
        				printf("ferito\n");
        				coloreCmd(COLORE_STD);
        			break;
    				case 51 ... 100:
					    coloreCmd(COLORE_V);
        				printf("in salute\n");
        				coloreCmd(COLORE_STD);
        			break;
				}
	            printf("- Stanza attuale: %s\n\n", stanzaToString(giocatori[i]->posizione->tipo));

                if(roundGioco == 1) {
					avanza(giocatori[i], false);
				}
                                
                giocatori[i]->avanzato = false;
	            
	            char azione = ' ';
        		char opzioni[9];

                // Ad ogni turno il giocatore può scegliere tra le seguenti azioni fino a quando non decide di passare il turno        
                while (IS_P_VALIDO(giocatori[i]) && giocatori[i]->pVita >= MIN_PVITA && azione != 'P') {
                    printf("\nSeleziona una delle seguenti "); coloreCmd(COLORE_H); printf("azioni"); coloreCmd(COLORE_STD); printf(":\n");
                    stampaOpzioni(giocatori[i], opzioni);
                    strcpy(giocatori[i]->azioniConcesse, opzioni);

                    if (charInput(&azione, opzioni) == errore) {
                        logErrore("Valore non valido. Riprova.");
                        azione = ' ';
                        continue;
                    } else {
                        switch (azione) {
                            case 'A':
                                avanza(giocatori[i], false);
                                giocatori[i]->cercataSegreta = 0;
                            break;
                    
                            case 'C':
                                giocatori[i]->cercataSegreta = 0;
                                combatti(giocatori[i]);
                            break;
                    
                            case 'F':
                                torna(giocatori[i]);
                            break;
                                            
                            case 'S':
                                cercaStanzaSegreta(giocatori[i]);
                            break;
                                                                    
                            case 'T':
                                raccogliTesoro(giocatori[i]);
                            break;
                    
                            case 'G':
                                dettagliGiocatore(giocatori[i]);
                            break;
                    
                            case 'Z':
                                dettagliStanza(giocatori[i]->posizione);
                            break;

                            case 'M':
                                dettagliStanze();
                            break;

                            case 'H':
                                aiuto(giocatori[i]);
                            break;
                            
                            case 'X':
                                if (conferma("Proseguendo il tuo giocatore verrà eliminato")) {
                                    giocatori[i]->pVita = 0;
                                    azione = 'P';
                                }
                            break;
                        }
                    }
                }

                giocatori[i]->turnoPassato = true;
                        
	            if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->pVita == 0) morte(giocatori[i]);

	            logSpicy("---------------------------------------------------------------------------------");
            } else if (IS_P_VALIDO(giocatori[i]) && giocatori[i]->turnoBloccato == true) {
                giocatori[i]->turnoBloccato = false;
            }
        }
    }

    giocoAvviato = false;
    if(!jaffarSconfitto) sconfitta();
}

extern void terminaGioco() {
    // libera tutta la memoria allocata
    resetGiocatori();
    check_giocatori = false;
    resetMappa();
    check_mappa = false;
    SAFE_FREE(primaStanza);
    logSpicy("---------------------------------------------------------------------------------");
    logSpicy("                Grazie per aver giocato a Prince Of Inertia!");
    logSpicy("---------------------------------------------------------------------------------");
}

extern void crediti(bool min, bool classifica) {
    pulisciCmd();
    // creato con http://patorjk.com/software/taag/
    coloreCmd(COLORE_H);
	printf("\n           /\\                                                        /\\\n");
	printf("          |  |                                                      |  |\n");
	printf("         /----\\                                                    /----\\\n");
	printf("        [______]                Prince of Inertia                 [______]\n");
	printf("         |    |         _____                        _____         |    |\n");
	printf("         |[]  |        [     ]                      [     ]        |  []|\n");
	printf("         |    |       [_______][ ][ ][ ][][ ][ ][ ][_______]       |    |\n");
	printf("         |    [ ][ ][ ]|     |  ,----------------,  |     |[ ][ ][ ]    |\n");
	printf("         |             |     |/'    ____..____    '\\|     |             |\n");
	printf("          \\  []        |     |    /'    ||    '\\    |     |        []  /\n");
	printf("           |      []   |     |   |o     ||     o|   |     |  []       |\n");
	printf("           |   []      | (_) |   |     (||)     |   | (_) |       []  |\n");
	printf("         /''           |     |   |o     ||     o|   |     |           ''\\\n");
	printf("        [_____________[_______]--'------''------'--[_______]_____________]\n");
        
    // versione minimizzata
    coloreCmd(COLORE_STD);
    if (!min) {
        printf("\n                               * Versione: "); coloreCmd(COLORE_G); printf("%s *\n", C_VERSIONE); coloreCmd(COLORE_STD);
        printf("                      * Creatore: "); coloreCmd(COLORE_G); printf("%s *\n", C_CREATORE); coloreCmd(COLORE_STD);
        printf("         * %s *\n", C_DESCRIZIONE);
    }
    printf("\n");

    // mostra classifica
    if (classifica) {
        coloreCmd(COLORE_G);
    	printf("                           * Storico ultime VITTORIE * \n");
        printf("                       ----------------------------------\n\n");
        coloreCmd(COLORE_STD);
        leggiStorico();
        printf("\n");
        attendiUtente();
        nuovaSchermata();
    } else {
      	printf("\n");
    	coloreCmd(COLORE_STD);
    }
}  
