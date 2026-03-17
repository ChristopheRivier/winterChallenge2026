#!/usr/bin/env bash
# Test unitaire : scénario où le bot peut perdre (my_id=0, grille 38x21, 4 snakebots)
# Vérifie que le bot s'exécute sans crash et produit une sortie valide par tour.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BOT="${PROJECT_DIR}/bot"
INPUT="${SCRIPT_DIR}/test_losing_input.txt"

if [[ ! -f "$BOT" ]]; then
  echo "Compilation du bot..."
  (cd "$PROJECT_DIR" && g++ -std=c++17 -O2 -o bot src/main.cpp src/Game.cpp)
fi

if [[ ! -f "$INPUT" ]]; then
  echo "Fichier d'entrée manquant: $INPUT"
  exit 1
fi

# Lancer le bot (stderr séparé pour ne pas polluer la sortie)
OUTPUT=$(mktemp)
STDERR=$(mktemp)
trap "rm -f '$OUTPUT' '$STDERR'" EXIT

"$BOT" --max-turns 1 < "$INPUT" > "$OUTPUT" 2> "$STDERR"
EXIT_CODE=$?
if [[ "$EXIT_CODE" -ne 0 ]]; then
  echo "ÉCHEC: le bot a quitté avec le code $EXIT_CODE"
  cat "$STDERR"
  exit 1
fi

# On attend 1 tour dans l'input (init + tour1), donc 1 ligne de sortie
LINES=$(grep -c . "$OUTPUT" 2>/dev/null || echo 0)
if [[ "$LINES" -lt 1 ]]; then
  echo "ÉCHEC: attendu au moins 1 ligne de sortie, obtenu $LINES"
  echo "--- stdout ---"
  cat "$OUTPUT"
  echo "--- stderr ---"
  cat "$STDERR"
  exit 1
fi

# Vérifier que chaque ligne est non vide et ressemble à des actions (chiffres + UP/DOWN/LEFT/RIGHT ou WAIT)
LINE_NUM=0
while IFS= read -r line; do
  LINE_NUM=$((LINE_NUM + 1))
  if [[ -z "$line" ]]; then
    echo "ÉCHEC: ligne $LINE_NUM vide"
    exit 1
  fi
  if [[ "$line" != "WAIT" ]] && [[ ! "$line" =~ [0-9].*(UP|DOWN|LEFT|RIGHT) ]]; then
    echo "ÉCHEC: ligne $LINE_NUM format invalide (attendu id UP/DOWN/LEFT/RIGHT ou WAIT): $line"
    exit 1
  fi
done < "$OUTPUT"

echo "OK: test scénario perdant — le bot a produit $LINES ligne(s) de sortie valide(s) et s’est terminé correctement."
