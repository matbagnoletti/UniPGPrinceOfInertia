#include <stdbool.h>
#include <stdio.h>
#include "gamelib.h"

/*********************************
 * main.c | Programma principale *
 *********************************/

int main() {
	crediti(true, false);
	numero scelta = nessunaScelta;
  
	logAvviso(" Benvenuto!");
	printf(" Aiuta il principe a scappare dal castello di Jaffar e salvare la principessa.\n Fai attenzione però, dovrai raggiungerla prima dei doppleganger a te ostili!\n Il castello è pieno di trabocchetti e nemici, quindi fai attenzione!\n\n");
	
	logNota("Per una esperienza di gioco migliore, ingrandisci il terminale.\n");
	logNota("Si può giocare singolarmente o in gruppo (max 3 giocatori).");
	logNota("Se ti trovi in difficoltà puoi sempre chiedere aiuto con il comando 'H' ;)\n");
	
	logAvviso(" Buon divertimento!\n\n");
  	attendiUtente();

	do { 
        nuovaSchermata();
		logAvviso("  MENU PRINCIPALE");
        logAvviso(" ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀\n");
        const char* scelte[] = {"Impostazioni", "Gioca", "Termina", "Crediti", NULL};
        stampaElencoScelte(true, "1234", scelte);

		if (numInput(&scelta, 1, 4) == errore) {
			logErrore("Valore non valido. Riprova.\n");
			attendiUtente();
			scelta = nessunaScelta;
			continue;
		}

		switch (scelta) {
			case menuImposta:
				impostaGioco();
			break;

			case menuGioca:
				gioca();
			break;

			case menuCrediti:
				crediti(false, true);
			break;

			case menuTermina:
				terminaGioco();
			break;
		}
        
	} while (scelta != menuTermina);
  
	return 0;
}
