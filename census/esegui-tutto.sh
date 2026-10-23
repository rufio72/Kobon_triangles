#!/bin/sh
# Censimento completo n = 3..10: tipi e README in ../k<n>, verifica del
# tetto a T+1, confronto esaustivo per n <= 6, riepilogo finale.
set -e
QUI=$(cd "$(dirname "$0")" && pwd)
RADICE=$(dirname "$QUI")
cd "$QUI"
make -s
RIEP=$(mktemp)
trap 'rm -f "$RIEP"' EXIT
for n in 3 4 5 6 7 8 9 10; do
    echo "=== n = $n ==="
    ./census-kobon "$n" --out "$RADICE/k$n" | tee /dev/stderr | grep '^RIEPILOGO' >> "$RIEP"
    if [ "$n" -le 6 ]; then
        ./census-kobon "$n" --brute
    fi
done
echo
echo "riepilogo"
printf '%-3s | %-3s | %-4s | %-10s | %s\n' n T tipi "UNSAT T+1" "scartate non realizzabili"
while read -r _ n t tipi tetto scartate; do
    printf '%-3s | %-3s | %-4s | %-10s | %s\n' "$n" "$t" "$tipi" "$tetto" "$scartate"
done < "$RIEP"
