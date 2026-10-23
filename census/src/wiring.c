/* Ricostruzione greedy del wiring diagram ed estrazione della tabella. */
#include "wiring.h"
#include "cnf.h"

int wiring_valida(const Tabella *t)
{
    int n = t->n;
    int perm[N_MAX], cur[N_MAX + 1];
    for (int i = 0; i < n; i++) {
        perm[i] = i + 1;
        cur[i + 1] = 0;
    }
    int totale = n * (n - 1) / 2, fatti = 0;
    while (fatti < totale) {
        int trovato = 0;
        for (int p = 0; p + 1 < n; p++) {
            int u = perm[p], v = perm[p + 1];
            if (cur[u] < n - 1 && t->r[u - 1][cur[u]] == v &&
                cur[v] < n - 1 && t->r[v - 1][cur[v]] == u) {
                perm[p] = v;
                perm[p + 1] = u;
                cur[u]++;
                cur[v]++;
                fatti++;
                trovato = 1;
            }
        }
        if (!trovato)
            return 0;
    }
    /* Ogni coppia si è incrociata una volta: la permutazione è invertita. */
    for (int i = 0; i < n; i++)
        if (perm[i] != n - i)
            return 0;
    return 1;
}

int wiring_valida_orbita(const Tabella *t)
{
    Tabella orb[ORBITA_MAX];
    int cnt = tab_orbita(t, orb);
    for (int i = 0; i < cnt; i++)
        if (wiring_valida(&orb[i]))
            return 1;
    return 0;
}

int wiring_estrai(int n, const signed char *val, Tabella *t)
{
    tab_vuota(t, n);
    for (int r = 0; r < n; r++) {
        /* Il primo della riga è l'unico i con A(r,i,0) vero, poi si
         * seguono le relazioni G(r,.,.). */
        int primo = -1;
        for (int i = 0; i < n && primo < 0; i++)
            if (i != r && val[var_A(n, r, i, 0)] > 0)
                primo = i;
        if (primo < 0)
            return -1;
        int k = 0, x = primo;
        t->r[r][k++] = (unsigned char)(x + 1);
        while (k < n - 1) {
            int succ = -1;
            for (int j = 0; j < n && succ < 0; j++)
                if (j != r && j != x && val[var_G(n, r, x, j)] > 0)
                    succ = j;
            if (succ < 0)
                return -1;
            t->r[r][k++] = (unsigned char)(succ + 1);
            x = succ;
        }
    }
    return tab_ben_formata(t) ? 0 : -1;
}
