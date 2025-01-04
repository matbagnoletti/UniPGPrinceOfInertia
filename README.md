<h1 align="center">UniPG: Prince of Inertia</h1>

<p align="center" style="font-family: monospace">Made by <a href="https://github.com/matbagnoletti">@matbagnoletti</a></p>
<p align="center" style="font-family: monospace">Matteo Bagnoletti Tini | matricola n° 377156</p>
<p align="center" style="font-family: monospace">Corso di Programmazione Procedurale I° anno a.a. 2024/2025, <a href="https://www.dmi.unipg.it/didattica/corsi-di-studio-in-informatica/informatica-triennale">UniPG - DMI</a></p>
<div align="center">
    <img src="https://img.shields.io/github/last-commit/matbagnoletti/UniPGPrinceOfInertia?style=for-the-badge" alt="Ultimo commit">
    <img src="https://img.shields.io/github/languages/top/matbagnoletti/UniPGPrinceOfInertia?style=for-the-badge" alt="Linguaggio">
</div>

## Sommario
- [Descrizione](#descrizione)
- [Requisiti](#requisiti)
- [Richiesta dell'esercizio](#richiesta-dellesercizio)
- [Specifiche](#specifiche)
- [Installazione e utilizzo](#installazione-e-utilizzo)
- [Licenza](#licenza)

## Descrizione
Progetto finale del corso di Programmazione Procedurale del primo anno di Informatica presso l'Università degli Studi di Perugia.

## Requisiti
- [gcc](https://www.gcc.gnu.org/) (utilizzata v13.3.0) o altro compilatore C analogo

È possibile visualizzare la versione già presente sul proprio dispositivo mediante il seguente comando:
```bash
gcc -v
```

## Richiesta dell'esercizio
Realizzare un programma in C che simuli il comportamento dell'originale videogioco [Prince of Persia](https://it.wikipedia.org/wiki/Prince_of_Persia) permettendo ad uno o più giocatori di fuggire dal castello di Jaffar e raggiungere l'amata principessa.
Per una visione più approfondita del testo e delle richieste del progetto, consultare il [seguente link](https://francescosantini.sites.dmi.unipg.it/esami/progetto2425.pdf).

## Specifiche
1. Il programma fa uso dei colori [colori ANSI](https://www.perpetualpc.net/6429_colors.html#color_list) per rendere più chiara la distinzione tra le varie tipologie di messaggi in output. Nel caso in cui il proprio terminale non visualizzi correttamente tali colori, è solitamente possibile modificare le preferenze della shell.
*Per gli utenti esperti, è possibile modificare i colori di output direttamente nel codice sorgente, modificando i valori delle costanti (`const char* const COLORE_*`) definite in `gamelib.h`*.
In particolare, i colori utilizzati sono:
   - <span style="color:rgb(233, 57, 57)">ROSSO</span> per i messaggi di errore
   - <span style="color:rgb(71, 157, 71)">VERDE</span> per i messaggi di successo
   - <span style="color:rgb(210, 201, 219)">GRIGIO</span> per i messaggi informativi
   - <span style="color: #FFFF00">GIALLO</span> per i messaggi di avvertimento

2. Il codice implementa due funzioni relative allo storico delle vittorie: `void scriviStorico(const char*)` e `void leggiStorico(void)`. La prima permette di memorizzare il nome del giocatore che ha vinto la partita in una specifica data (quella corrente) su di un file di riferimento (`storico.csv`). Poiché la seconda funzione citata provvede a leggere solo gli ultimi 3 vincitori, `scriviStorico` si avvale di un file temporaneo `tmp.csv` per memorizzare le informazioni già presenti, che verranno riscritte in `append (a)` nel file `storico.csv` mantenendo un ordine temporale di tipo `DESC` (decrescente in base alla data). Il file temporaneo viene automaticamente eliminato. 

   Poiché la scrittura e lettura su file non sono state trattate durante il corso, le funzionalità implementate si basano su ricerche online e suggerimenti automatizzati. I dati memorizzati non sono crittografati e sono facilmente accessibili. Non sono previsti attualmente meccanismi di verifica di duplicati (nei nomi) o di correttezza dei dati.

3. Il gioco considera come *vittoria* la situazione in cui un generico giocatore raggiunge e sconfigge *Jaffar*, a prescindere che sia il *principe* o un *doppleganger*. I doppleganger, nel caso in cui siano due, gareggiano insieme, per cui la vittoria di uno risulta tale per la categoria nel complesso, e come sconfitta per il principe. In caso di vittoria, il nome del giocatore verrà memorizzato nel file `storico.csv` (vedi punto 1).

4. In alcune porzioni di codice sono presenti `switch` con `case` non standard, del tipo:
   ```c
   switch (espressione) {
      case 1 ... 3:
         // codice
      break;

      case 4 ... 6:
         // codice
      break;
      
      default:
         // codice
      break;
   }
   ```
   L'avviso (*warning*) che si ottiene (*flag `-Wpedantic`*) è il seguente:
   ```
   range expressions in switch statements are non-standard [-Wpedantic]
   ```
   Questo tipo di costrutto permette di eseguire un blocco di codice per un intervallo di valori. È supportato da gcc ma, nel caso in cui si utilizzasse un compilatore differente, è opportuno verificarne la compatibilità.

## Installazione e utilizzo
Per poter utilizzare il programma, è necessario seguire i seguenti passaggi:
1. Effettua il clone della repository con il comando:
   ```bash
   git clone https://www.github.com/matbagnoletti/UniPGPrinceOfInertia.git
   ```
   In alternativa, effettua il download del file compresso del progetto ed estrailo in una cartella locale del tuo computer.
2. Nel terminale, spostati nella cartella del progetto:
   ```bash
   cd UniPGPrinceOfInertia
   ```
3. Compila il codice sorgente:
   ```bash
    gcc -c main.c -Wall -std=c11
    gcc -c gamelib.c -Wall -std=c11
    gcc -o [eseguibile] main.o gamelib.o
    ```
4. Esegui il programma:
    ```bash
    ./[eseguibile]
    ```

## Licenza
Questo progetto (e tutte le sue versioni) sono rilasciate sotto la [MB General Copyleft License](LICENSE).