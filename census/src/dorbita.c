/* Wiring generalizzato e orbita per via geometrica.
 *
 * Dalla tabella si ricostruisce il wiring diagram (permutazione iniziale
 * identità); il suo bordo, letto lungo un cerchio che racchiude tutti gli
 * incroci, dà le 2n scelte dell'estremo di partenza e i 2 versi di
 * percorrenza della costruzione del paper (arXiv:2507.07951, par. 2.1).
 * Ogni scelta numera le rette e le orienta: è una presentazione. */
#include "dorbita.h"

#include <string.h>

int dorb_wiring(const TabellaD *t, int *finale)
{
    int n = t->n, perm[N_MAX], cur[N_MAX], sz[N_MAX][N_MAX];
    int mancanti = 0;
    for (int r = 0; r < n; r++) {
        perm[r] = r;
        cur[r] = 0;
        mancanti += t->nev[r];
        for (int e = 0; e < N_MAX; e++)
            sz[r][e] = 0;
        for (int j = 0; j < n; j++)
            if (j != r && t->pos[r][j] != DT_NO)
                sz[r][t->pos[r][j]]++;
    }
    while (mancanti > 0) {
        int progresso = 0;
        for (int p = 0; p < n;) {
            int u = perm[p];
            if (cur[u] >= t->nev[u]) {
                p++;
                continue;
            }
            int m = sz[u][cur[u]] + 1, ok = p + m <= n;
            /* il blocco perm[p..p+m-1] deve essere l'evento di u più u */
            for (int q = 1; ok && q < m; q++)
                ok = t->pos[u][perm[p + q]] == cur[u];
            /* e per ogni sua retta v il prossimo evento è il resto del blocco */
            for (int q = 0; ok && q < m; q++) {
                int v = perm[p + q];
                ok = cur[v] < t->nev[v] && sz[v][cur[v]] == m - 1;
                for (int w = 0; ok && w < m; w++)
                    if (w != q)
                        ok = t->pos[v][perm[p + w]] == cur[v];
            }
            if (!ok) {
                p++;
                continue;
            }
            for (int q = 0; q < m; q++)
                cur[perm[p + q]]++;
            for (int a = p, b = p + m - 1; a < b; a++, b--) {
                int x = perm[a];
                perm[a] = perm[b];
                perm[b] = x;
            }
            mancanti -= m;
            progresso = 1;
            p += m;
        }
        if (!progresso)
            return 0;
    }
    if (finale)
        memcpy(finale, perm, (size_t)n * sizeof *perm);
    return 1;
}

void dorb_presentazione(const TabellaD *t, const int *finale, int s, int d,
                        TabellaD *out)
{
    int n = t->n, lab[N_MAX], inv[N_MAX], prossima = 0;
    for (int i = 0; i < n; i++)
        lab[i] = -1;
    for (int passo = 0; passo < 2 * n; passo++) {
        int k = ((s + d * passo) % (2 * n) + 2 * n) % (2 * n);
        int L = k < n ? k : finale[2 * n - 1 - k];
        if (lab[L] < 0) {
            lab[L] = prossima++;
            inv[L] = k >= n;    /* primo estremo = uscita: verso rovesciato */
        }
    }
    dtab_vuota(out, n);
    for (int L = 0; L < n; L++) {
        int a = lab[L], ne = t->nev[L];
        out->nev[a] = (unsigned char)ne;
        for (int j = 0; j < n; j++) {
            int e = t->pos[L][j];
            if (j == L || e == DT_NO)
                continue;
            out->pos[a][lab[j]] = (unsigned char)(inv[L] ? ne - 1 - e : e);
        }
    }
}

int dorb_valida(const TabellaD *t)
{
    return dtab_classi_consecutive(t) && dorb_wiring(t, NULL);
}

int dorb_orbita(const TabellaD *t, TabellaD *orb)
{
    int n = t->n, finale[N_MAX], cnt = 0;
    if (!dorb_wiring(t, finale))
        return 0;
    for (int s = 0; s < 2 * n; s++)
        for (int d = 1; d >= -1; d -= 2) {
            TabellaD p;
            dorb_presentazione(t, finale, s, d, &p);
            if (!dorb_valida(&p))
                continue;
            int nuova = 1;
            for (int q = 0; q < cnt && nuova; q++)
                nuova = dtab_confronta(&orb[q], &p) != 0;
            if (nuova)
                orb[cnt++] = p;
        }
    return cnt;
}

int dorb_canonica(const TabellaD *t, TabellaD *out)
{
    TabellaD orb[DORB_MAX];
    int cnt = dorb_orbita(t, orb);
    if (cnt == 0)
        return 0;
    int best = 0;
    for (int q = 1; q < cnt; q++)
        if (dtab_confronta(&orb[q], &orb[best]) < 0)
            best = q;
    *out = orb[best];
    return cnt;
}
