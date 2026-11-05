#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Se lo ZIP e' estratto dentro la radice del progetto, il progetto e' nella stessa cartella.
# Se e' estratto in una sottocartella, indicare la cartella progetto al primo argomento.
DESTINAZIONE="${1:-$SCRIPT_DIR}"
if [[ ! -f "$DESTINAZIONE/CMakeLists.txt" ]]; then
  echo "[ERRORE] Non trovo CMakeLists.txt nella cartella: $DESTINAZIONE"
  echo "Estrai il contenuto dello ZIP nella radice del progetto oppure lancia:"
  echo "    bash installa_gui.sh /home/helgrind/CLionProjects/RicercaTraiettoriaIdealeOMP"
  exit 1
fi

if [[ "$SCRIPT_DIR" != "$DESTINAZIONE" ]]; then
  mkdir -p "$DESTINAZIONE/gui"
  cp -R "$SCRIPT_DIR/gui/." "$DESTINAZIONE/gui/"
fi

if grep -q 'APEX LAB · GUI QT 6' "$DESTINAZIONE/CMakeLists.txt"; then
  echo "[OK] CMake gia' configurato: non aggiungo una seconda copia."
else
  printf '\n\n' >> "$DESTINAZIONE/CMakeLists.txt"
  cat "$SCRIPT_DIR/CMake_AGGIUNTA_GUI.txt" >> "$DESTINAZIONE/CMakeLists.txt"
  echo "[OK] Blocco Qt6 aggiunto a CMakeLists.txt."
fi

echo "[OK] File GUI pronti in $DESTINAZIONE/gui"
echo "[INFO] In CLion: Reload CMake Project, quindi seleziona il target RicercaTraiettoriaGUI."
echo "[INFO] La GUI usa il tuo motore C++ e NON modifica main.cpp o i test."
