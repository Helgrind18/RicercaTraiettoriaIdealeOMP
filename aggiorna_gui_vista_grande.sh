#!/usr/bin/env bash
set -euo pipefail

PACCHETTO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROGETTO="${1:-$HOME/CLionProjects/RicercaTraiettoriaIdealeOMP}"

if [[ ! -f "$PROGETTO/CMakeLists.txt" || ! -f "$PROGETTO/gui/main_gui.cpp" ]]; then
    echo "[ERRORE] Progetto APEX LAB non trovato in: $PROGETTO"
    echo "Uso: bash aggiorna_gui_vista_grande.sh ~/CLionProjects/RicercaTraiettoriaIdealeOMP"
    exit 1
fi

FILE=(VistaPista.hpp VistaPista.cpp ApexMainWindow.hpp ApexMainWindow.cpp)
for f in "${FILE[@]}"; do
    if [[ ! -f "$PACCHETTO/gui/$f" ]]; then
        echo "[ERRORE] File aggiornamento mancante: gui/$f"
        exit 1
    fi
done

# Se l'archivio e' stato estratto direttamente nella cartella del progetto,
# i quattro file sono gia' quelli nuovi. Negli altri casi fai backup e copia.
if [[ "$(realpath "$PACCHETTO")" == "$(realpath "$PROGETTO")" ]]; then
    echo "[OK] Sorgenti estratti direttamente nel progetto. Non serve copiarli."
else
    BACKUP="$PROGETTO/gui/.backup_vista_precedente_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$BACKUP"
    for f in "${FILE[@]}"; do
        if [[ -f "$PROGETTO/gui/$f" ]]; then cp "$PROGETTO/gui/$f" "$BACKUP/$f"; fi
        cp "$PACCHETTO/gui/$f" "$PROGETTO/gui/$f"
        cmp -s "$PACCHETTO/gui/$f" "$PROGETTO/gui/$f" || {
            echo "[ERRORE] Verifica copia fallita: gui/$f"; exit 1;
        }
        echo "[OK] gui/$f aggiornato"
    done
    echo "[INFO] Backup vecchia GUI: $BACKUP"
fi

echo
echo "[OK] APEX LAB aggiornato: piste grandi, confronto reale, lente Δ max e zoom sincronizzato."
echo "[INFO] Motore src/, include/, test/ e CMakeLists.txt invariati."
echo "[INFO] In CLion seleziona RicercaTraiettoriaGUI -> Rebuild Project -> Run."
