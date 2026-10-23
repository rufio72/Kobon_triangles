#!/bin/sh
# Censimento degenere n = 8 (priorità), poi 3..7, 9, 10: tipi degeneri e
# sezione README in ../k<n>, verifica del tetto a T+1 (per n = 9 e 10 con
# tetto di tempo LIMITE_TETTO secondi, predefinito 12 ore; il tetto gira in
# parallelo al censimento), confronto con la forza bruta per n <= 8,
# riepilogo finale. PARTI > 1 divide n = 9 e 10 in parti parallele
# (--parti): misurato più lento su n = 9, quindi predefinito 1. Messaggi di avanzamento e scarti in tmp/degeneri-n<n>.log.
set -e
QUI=$(cd "$(dirname "$0")" && pwd)
RADICE=$(dirname "$QUI")
LIMITE_TETTO=${LIMITE_TETTO:-43200}
PARTI=${PARTI:-1}
cd "$QUI"
make -s
./census-degeneri --autotest
RIEP=$(mktemp)
ESITO=0
trap 'rm -f "$RIEP"' EXIT
for n in 8 3 4 5 6 7 9 10; do
    echo "=== n = $n ==="
    LIM=0
    K=1
    if [ "$n" -ge 9 ]; then
        LIM=$LIMITE_TETTO
        K=$PARTI
    fi
    USCITA=$(./census-degeneri "$n" --out "$RADICE/k$n" --limite "$LIM" \
             --parti "$K" 2>"$QUI/tmp/degeneri-n$n.log") || ESITO=$?
    printf '%s\n' "$USCITA"
    printf '%s\n' "$USCITA" | grep '^RIEPILOGO' >> "$RIEP" || true
    if [ "$n" -le 8 ]; then
        ./census-degeneri "$n" --brute 2>>"$QUI/tmp/degeneri-n$n.log" ||
            ESITO=$?
    fi
done
echo
echo "riepilogo"
printf '%-3s | %-3s | %-8s | %-8s | %-6s | %s\n' n T semplici degeneri \
    totale "UNSAT T+1"
sort -k2,2n "$RIEP" | while read -r _ n t s d tetto _; do
    printf '%-3s | %-3s | %-8s | %-8s | %-6s | %s\n' "$n" "$t" "$s" "$d" \
        "$((s + d))" "$tetto"
done
exit "$ESITO"
