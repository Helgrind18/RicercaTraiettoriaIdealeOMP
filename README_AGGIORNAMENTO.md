# APEX LAB — Confronto visuale delle traiettorie (Qt 6)

Aggiornamento della GUI **già installata**. Non modifica l'algoritmo genetico, il valutatore, i parametri del motore, CMake, i test né i dati delle piste.

## Installazione consigliata (Ubuntu + CLion)

1. Estrai l'archivio in una cartella a scelta, per esempio `~/Scaricati/apex_confronto/`.
2. Apri il terminale **nella cartella estratta**.
3. Esegui `bash aggiorna_gui.sh ~/CLionProjects/RicercaTraiettoriaIdealeOMP`.
4. In CLion, seleziona il target **RicercaTraiettoriaGUI** e scegli **Build → Rebuild Project**, quindi **Run**.

In alternativa, copia o sovrascrivi manualmente **solo** i quattro file di `gui/` dell'archivio all'interno di `gui/` del progetto. Il CMakeLists esistente e' gia' compatibile e non deve essere modificato.

Lo script conserva un backup dei quattro sorgenti precedenti in `gui/.backup_prima_confronto/`.

## Utilizzo

- **MODALITÀ → AFFIANCATE**: due piste nella stessa finestra: a sinistra la migliore traiettoria iniziale (arancione tratteggiata), a destra la migliore ottimizzata (turchese continua); sopra ciascuna e' riportato il tempo sul giro corrispondente.
- **MODALITÀ → SOVRAPPOSTE**: iniziale e ottimizzata sulla stessa pista; i collegamenti grigi tra alcune sezioni aiutano a vedere lo spostamento.
- **SCOSTAMENTI → ×1 · REALI**: posizione laterale realmente calcolata, sulla scala del circuito.
- **SCOSTAMENTI → ×10, ×25, ×50**: moltiplica *solo nel disegno* lo spostamento di ciascun punto rispetto al centro della pista. **Non rappresenta una traiettoria fisica, non modifica il calcolo e puo' sembrare fuori dalla pista.** La scelta predefinita e' ×25 per rendere visibili le differenze a circuito intero.
- **Rotellina**: zoom sincronizzato; **trascina**: sposta la vista; **Centra vista**: reimposta zoom e spostamento.
- Le caselle **Iniziale** e **Ottimizzata** mostrano/nascondono le rispettive traiettorie.
- In modalita' sovrapposta compare anche la distanza media e massima fra le traiettorie (in metri, calcolata sulle coordinate non amplificate).

La vettura e' solo un'animazione illustrativa, non usa la velocita' fisica calcolata dal valutatore.

## File modificati

```
gui/VistaPista.hpp
gui/VistaPista.cpp
gui/ApexMainWindow.hpp
gui/ApexMainWindow.cpp
```

**Nessuna dipendenza aggiuntiva:** Qt 6 Widgets come nella versione gia' installata.
