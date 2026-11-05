# APEX LAB — Piste grandi e confronto reale (Qt 6)

Aggiornamento grafico che usa le traiettorie prodotte dal tuo progetto C++.
Non cambia l'algoritmo genetico, il valutatore, i dati della pista o CMake.

## Novità

- **Piste più grandi:** il grafico geni inizialmente è nascosto; premi **MOSTRA GRAFICO GENI** per riaprirlo.
- **VISTA GRANDE:** finestra indipendente massimizzata con due tracciati affiancati.
- **Scala reale ×1** predefinita: non modifica né deforma le traiettorie.
- **Lente di dettaglio** nella mappa ottimizzata: arancione = iniziale, turchese = ottimizzata, mostra la differenza locale in metri nel punto di massimo scostamento.
- **DETTAGLIO Δ MAX:** ingrandisce automaticamente il tratto con la maggiore differenza reale.
- **Zoom sincronizzato** con rotellina, ancorato al cursore; **doppio clic** ingrandisce più rapidamente; **trascinamento** sposta entrambe le mappe.
- **MOSTRA TUTTO / CENTRA VISTA:** ripristina la vista completa.
- Modalità **SOVRAPPOSTE** ancora disponibile; amplificazione laterale ×10/×25/×50 facoltativa.

**Nota geometrica:** la pista è lunga diversi chilometri, ma larga circa 12 metri. La sola vista dell'intero circuito non può mostrare chiaramente uno scostamento di pochi metri su uno schermo; lo zoom locale è la soluzione fedele alla geometria.

## Installazione

1. Estrai l'archivio ZIP in una cartella a tua scelta.
2. Apri un terminale **nella cartella estratta**.
3. Esegui:

```bash
bash aggiorna_gui_vista_grande.sh ~/CLionProjects/RicercaTraiettoriaIdealeOMP
```

4. In CLion: seleziona il target `RicercaTraiettoriaGUI`, fai **Build → Rebuild Project**, poi premi **Run**.

In alternativa, puoi copiare direttamente i quattro file della sottocartella `gui/` nella cartella `gui/` del progetto, sovrascrivendo quelli esistenti.

## Prova suggerita

Spa, 80 sezioni, 80 individui, seed 2026, 100 generazioni.
Premi **AVVIA OTTIMIZZAZIONE**, quindi **VISTA GRANDE**: vedrai le due traiettorie, il dettaglio della massima differenza reale e potrai ingrandire una curva con la rotellina. Premi **DETTAGLIO Δ MAX** per saltare direttamente al punto più diverso. Premi **MOSTRA TUTTO** per tornare all'intero circuito.

## Note tecniche

Il pacchetto sostituisce solo:

- `gui/VistaPista.hpp`
- `gui/VistaPista.cpp`
- `gui/ApexMainWindow.hpp`
- `gui/ApexMainWindow.cpp`

Il codice è stato controllato staticamente, ma non è stato compilato in questo ambiente, dove Qt 6 SDK non è installato. Una compilazione finale su Ubuntu/CLion resta necessaria.
