#include <stdbool.h>

/****************************************************
 * gamelib.h | Intestazione della libreria di gioco *
 ****************************************************/

/*
 *   TIPI DI DATO
 */

typedef unsigned char numero;

/*
 *   DICHIARAZIONE DELLE FUNZIONI DI GIOCO
 */

extern void impostaGioco();

extern void gioca();

extern void terminaGioco();

extern void crediti(bool min, bool classifica);

/*
 *   STRUTTURE
 */

typedef enum direzione { 
    init,
    destra, 
    sinistra, 
    sopra, 
    sotto
} Direzione;

typedef enum esito {
  	successo,
  	errore
} Esito;

typedef enum sceltaMainMenu {
  	nessunaScelta,
  	menuImposta, 
  	menuGioca, 
  	menuTermina, 
  	menuCrediti
} SceltaMainMenu;

typedef enum sceltaImpostaGioco {
  	nessunaImpostazione,
  	impGiocatori, 
  	impMappa, 
  	terminaImpostazioni
} SceltaImpostaGioco;

typedef enum sceltaMappa {
  	nessunaImp,
  	nuovaStanza, 
  	rimuoviStanza, 
  	stampaStanze,
  	generaRandom,
  	terminaImpostaMappa,
} SceltaMappa;

typedef enum tipoStanza{
 	ingresso,
  	corridoio, 
 	scala, 
  	salaBanchetto, 
  	magazzino,
  	postoDiGuardia,
  	prigione, 
  	armeria, 
  	moschea, 
  	torre, 
  	bagni
} TipoStanza;

typedef enum tipoTesoro {
  	nessunTesoro, 
  	verdeVeleno,          // -1 pVita
  	bluGuarigione,        // +1 pVita
  	rossoAumentaVita,     // +1 pVitaMax e pVita = pVitaMax
  	spadaTagliente,       // +1 dadiAttacco
  	scudo                 // +1 dadiDifesa
} TipoTesoro;

typedef enum tipoTrabocchetto {
  	nessunTrabocchetto,   
  	tegola,               // -1 pVita
  	lame,                 // -2 pVita
  	caduta,               // -1 o -2 pVita (50% di probabilità ciascuna)
  	burrone               // come caduta + turno perso
} TipoTrabocchetto;

typedef enum tipoNemico {
    nessunNemico,
  	scheletro,
  	guardia,
  	Jaffar
} TipoNemico;

typedef struct nemico {
  	TipoNemico tipo;
  	numero pVita;
  	numero dadiAttacco;
  	numero dadiDifesa;
} Nemico;

typedef struct stanza {
  	TipoStanza tipo;
  	TipoTesoro tesoro;
  	bool tesoroRaccolto;
  	bool isSegreta;
  	bool isDefault;
  	TipoTrabocchetto trabocchetto;
  	struct stanza *stDestra;
  	struct stanza *stSinistra;
  	struct stanza *stSopra;
  	struct stanza *stSotto;
  	struct stanza *precedente;
    Nemico *nemico;
} Stanza;

typedef enum classeGiocatore {
  	nessunaClasse,
  	principe,
  	doppleganger
} ClasseGiocatore;

typedef struct giocatore {
  	char nome[25];
  	ClasseGiocatore classe;
  	Stanza *posizione;
  	numero pVitaMax;
  	numero pVita;
  	numero dadiAttacco;
  	numero dadiDifesa;
  	numero scappa;
  	bool ignoraTrabocchetto;
    bool avanzato;
    bool turnoBloccato;
	numero cercataSegreta;
	bool inVita;
	bool turnoPassato;
	char azioniConcesse[10];
} Giocatore;

/*
 *   DICHIARAZIONE DELLE FUNZIONI DI BASE (non specifiche del gioco ma grafiche e di input/output)
 */

extern void attendiUtente();

extern void nuovaSchermata();

extern void stampaElencoScelte(bool numerico, const char* simboli, const char* elenco[]);

extern void logNota(const char* messaggio);

extern void logSpicy(const char* messaggio);

extern void logAvviso(const char* messaggio);

extern void logErrore(const char* messaggio);

extern void logSuccesso(const char* messaggio);

extern Esito numInput(numero *p, numero MIN_L, numero MAX_L);

extern Esito stringInput(char *p, numero MIN_L, numero MAX_L);
