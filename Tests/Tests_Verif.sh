#!/usr/bin/env bash
set -u

# Pour lancer :
#   cd Tests
#   ./Tests_Verif.sh

MINICC="../minicc"

OK_DIR="Verif/OK"
KO_DIR="Verif/KO"

RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
RESET="\033[0m"


total_ok=0
total_ko=0
pass_ok=0
pass_ko=0
fail_ok=0
fail_ko=0

run_one() {
  local file="$1"
  "$MINICC" "$file" -v 2>&1
}

contains_error() {
  grep -Eq '(^|[^A-Za-z])Error([^A-Za-z]|$)'
}

extract_error_line() {
  local out="$1"
  local line


  line="$(printf "%s" "$out" | sed -nE 's/.*[Ee]rror[^0-9]*([0-9]+).*/\1/p' | head -n 1)"
  if [ -z "${line:-}" ]; then
    line="$(printf "%s" "$out" | sed -nE 's/.*[Ll]ine[^0-9]*([0-9]+).*/\1/p' | head -n 1)"
  fi

  if [ -z "${line:-}" ]; then
    printf "?"
  else
    printf "%s" "$line"
  fi
}

echo "== Verif tests =="
echo "Compilateur: $MINICC"
echo

# ----------------------------
# Verif/OK
# ----------------------------
echo "== Verif/OK =="

shopt -v nullglob
ok_files=("$OK_DIR"/*.c)
shopt -u nullglob

if [ ${#ok_files[@]} -eq 0 ]; then
  echo -e "${YELLOW}Aucun fichier .c trouvé dans $OK_DIR${RESET}"
else
  for f in "${ok_files[@]}"; do
    total_ok=$((total_ok+1))
    out="$(run_one "$f")"
    if printf "%s" "$out" | contains_error; then
      fail_ok=$((fail_ok+1))
      echo -e "${RED}[FAIL]${RESET} $f"
      echo "  Sortie:"
      printf "%s\n" "$out" | sed 's/^/  /'
    else
      pass_ok=$((pass_ok+1))
      echo -e "${GREEN}[OK]${RESET}   $f"
    fi
  done
fi

echo

# ----------------------------
# Verif/KO
# ----------------------------
echo "== Verif/KO =="

shopt -v nullglob
ko_files=("$KO_DIR"/*.c)
shopt -u nullglob

if [ ${#ko_files[@]} -eq 0 ]; then
  echo -e "${YELLOW}Aucun fichier .c trouvé dans $KO_DIR${RESET}"
else
  for f in "${ko_files[@]}"; do
    total_ko=$((total_ko+1))
    out="$(run_one "$f")"
    if printf "%s" "$out" | contains_error; then
      pass_ko=$((pass_ko+1))
      line="$(extract_error_line "$out")"
      echo -e "${GREEN}[OK]${RESET}   $f  -> line $line"
      errline="$(printf "%s" "$out" | grep -m1 -E '(^|[^A-Za-z])Error([^A-Za-z]|$)' || true)"
      if [ -n "${errline:-}" ]; then
        echo "  $errline"
      fi
    else
      fail_ko=$((fail_ko+1))
      echo -e "${RED}[FAIL]${RESET} $f"
      echo "  Attendu: un message contenant 'Error'."
      if [ -n "${out:-}" ]; then
        echo "  Sortie obtenue (sans 'Error'):"
        printf "%s\n" "$out" | sed 's/^/  /'
      else
        echo "  Sortie obtenue: (vide)"

      fi
    fi
  done
fi

echo
echo "== Résumé=="
echo "Verif/OK : $pass_ok / $total_ok OK, $fail_ok FAIL"
echo "Verif/KO : $pass_ko / $total_ko OK, $fail_ko FAIL"

# code retour non nul si tout est fail ko
if [ $fail_ok -ne 0 ] || [ $fail_ko -ne 0 ]; then
  exit 1
fi
exit 0
