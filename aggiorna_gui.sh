#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DESTINAZIONE="${1:-$HOME/CLionProjects/RicercaTraiettoriaIdealeOMP}"
if [[ ! -f "$DESTINAZIONE/CMakeLists.txt" || ! -f "$DESTINAZIONE/gui/main_gui.cpp" ]]; then
    echo "[ERRORE] Non trovo il progetto Qt installato in: $DESTINAZIONE"
    echo "Uso: bash aggiorna_gui.sh /percorso/del/progetto"
    exit 1
fi
if [[ "$(realpath "$SCRIPT_DIR")" == "$(realpath "$DESTINAZIONE")" ]]; then
    echo "[INFO] File gia' estratti nella cartella del progetto: nessuna copia necessaria."
else
    mkdir -p "$DESTINAZIONE/gui/.backup_prima_confronto"
    for file in VistaPista.hpp VistaPista.cpp ApexMainWindow.hpp ApexMainWindow.cpp; do
        [[ -f "$SCRIPT_DIR/gui/$file" ]] || { echo "[ERRORE] File mancante: gui/$file"; exit 1; }
        # Backup pre-aggiornamento, fatto solo la prima volta.
        if [[ -f "$DESTINAZIONE/gui/$file" && ! -f "$DESTINAZIONE/gui/.backup_prima_confronto/$file" ]]; then
            cp "$DESTINAZIONE/gui/$file" "$DESTINAZIONE/gui/.backup_prima_confronto/$file"
        fi
        cp "$SCRIPT_DIR/gui/$file" "$DESTINAZIONE/gui/$file"
        echo "[OK] Aggiornato gui/$file"
    done
fi
printf '\n[OK] Confronto affiancato + sovrapposto + ingrandimento laterale installati.\n'
echo "[INFO] Nessun file src/, include/, test/ o CMakeLists.txt e' stato modificato."
echo "[INFO] In CLion: seleziona RicercaTraiettoriaGUI e ricompila (Build/Rebuild)."
