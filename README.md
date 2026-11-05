# APEX / LAB · GUI C++ / Qt6

Applicazione desktop in **C++ + Qt 6 Widgets** per il progetto `RicercaTraiettoriaIdealeOMP`.
Non utilizza Python, JavaScript, HTML, server o browser. Chiama direttamente le classi originali `AlgoritmoGenetico`, `ValutatoreTraiettoria`, `Traiettoria`, `Veicolo` e `Gomma`.

## Installazione su Ubuntu 24.04 / CLion

**1.** Installa Qt 6 (se manca):

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-base-dev-tools
```

**2.** Estrai **tutto il contenuto** dello ZIP nella **radice del progetto** `~/CLionProjects/RicercaTraiettoriaIdealeOMP`, in modo che il file `installa_gui.sh` sia accanto al tuo `CMakeLists.txt` e che la cartella `gui/` sia alla stessa altezza di `include/` e `src/`.

**3.** Dalla cartella del progetto, esegui:

```bash
bash installa_gui.sh
```

Lo script aggiunge in coda al tuo `CMakeLists.txt` un blocco separato. Non sostituisce `main.cpp`, non modifica il motore e non elimina i test. Puoi anche copiare manualmente il contenuto di `CMake_AGGIUNTA_GUI.txt` in fondo al CMakeLists.

**4.** Torna in CLion: **Tools → CMake → Reload CMake Project** (oppure il pulsante Reload). Dal menu dei target in alto seleziona **`RicercaTraiettoriaGUI`** e premi **Run ▶**.

Se preferisci un build da terminale:

```bash
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug --target RicercaTraiettoriaGUI -j 4
./cmake-build-debug/RicercaTraiettoriaGUI
```

> **Importante:** nello screenshot del tuo CLion il file è `GeneraTraiettorie.hpp`, mentre la classe al suo interno è `GeneratoreTraiettorie`. La GUI usa esattamente questa combinazione. Se nel tuo progetto la denominazione cambia, adatta soltanto l'`#include` in `gui/MotoreApex.cpp`.

## Funzioni

- Piste **Spa** e **Monza** (campioni GPS in `gui/data/*.csv`), pan e zoom.
- Visualizzazione bordi pista stimati, traiettoria iniziale e traiettoria ottimizzata.
- Auto in movimento: **animazione visiva**, non telemetria o profilo di velocità.
- Parametri genetici: sezioni, popolazione, generazioni, crossover, mutazione, intensità, torneo, élite, seed.
- Parametri veicolo: massa, potenza, accelerazioni, frenata, Cd, area frontale, vmax, grip.
- Esecuzione di `AlgoritmoGenetico::ottimizza()` su un thread separato (interfaccia reattiva).
- Tempo sul giro iniziale e finale in **secondi simulati**, durata dell'ottimizzazione in **millisecondi reali**.
- Grafico dei geni (posizioni laterali), esportazione dei dati in JSON e CSV.

## Struttura

```text
RicercaTraiettoriaIdealeOMP/
  CMakeLists.txt                    # tuo CMake, con il nuovo target in fondo
  src/                              # tuo motore C++, NON modificato
  include/                          # tuoi header, NON modificati
  test/                              # tuoi test, NON modificati
  gui/
    main_gui.cpp
    ApexMainWindow.hpp/.cpp         # finestra principale, controlli e risultati
    MotoreApex.hpp/.cpp             # interfaccia col tuo GA C++
    VistaPista.hpp/.cpp             # disegno circuito e animazione
    GraficoGeni.hpp/.cpp           # grafico del cromosoma
    data/spa.csv
    data/monza.csv
  installa_gui.sh
  CMake_AGGIUNTA_GUI.txt
```

## Note sul modello

- Le coordinate dei circuiti sono un **campione** di dati GPS, non una ricostruzione ingegneristica esatta. I bordi sono creati da una carreggiata approssimata di 12 m di larghezza; **non sono bordi FIA ufficiali**.
- Per il confronto iniziale, la GUI ricrea la stessa popolazione prodotta all'inizio di `ottimizza()` a parità di seed. È coerente con l'implementazione discussa del tuo metodo.
- Il tempo del giro è una **stima del tuo `ValutatoreTraiettoria`**, non un tempo ufficiale F1.
- Non sono visualizzati tempi per singola generazione perché `ottimizza()` restituisce soltanto la traiettoria migliore; non viene inventata una cronologia di convergenza.
- Il codice della GUI **non introduce OpenMP**: lo integreremo quando la versione parallela del motore sarà pronta.

## Licenza dei dati pista

I CSV provengono da un sottoinsieme di coordinate trasformate dal progetto `bacinger/f1-circuits` (licenza MIT, Copyright Tomislav Bacinger), dataset di circuiti F1. L'interfaccia non è affiliata alla Formula 1.

## Diagnostica

**`Qt6Config.cmake` non trovato**: installa `qt6-base-dev` e ricarica CMake.

**`GeneraTraiettorie.hpp` non trovato**: verifica il nome dell'header nella tua cartella `include` e adegua `gui/MotoreApex.cpp`.

**Errore di link su un metodo del motore**: controlla che la cartella `src/` contenga tutte le implementazioni `.cpp`, con gli stessi nomi dichiarati negli header.

**La finestra mostra pista vuota**: assicurati che `gui/data/spa.csv` e `gui/data/monza.csv` siano stati estratti. La directory viene definita da CMake per leggere i file in modo indipendente dalla working directory.

**La finestra sembra bloccata durante la chiusura**: chiudendola mentre gira l'ottimizzazione, il programma attende che il worker finisca per evitare una distruzione anticipata del thread.
