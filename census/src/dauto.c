/* Autotest del censimento degenere e riscontro con soluzioni razionali
 * esatte (GMP), tramite il pacchetto geom/canon copiato da k18. */
#include "dauto.h"

#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "dcnf.h"
#include "dorbita.h"
#include "json_min.h"

void dauto_sha(const TabellaD *t, char out[65])
{
    static events_t ev, form;
    dtab_a_eventi(t, &ev);
    canonical_form(&ev, &form);
    canonical_sha256(&form, out);
}

/* Rette da un file con "lines_frac"; restituisce k (-1 se errore). */
static int carica_rette(const char *json, zline *L, jnode **radice)
{
    char err[256];
    jnode *j = json_load_file(json, err, sizeof err);
    if (!j) {
        fprintf(stderr, "errore: %s: %s\n", json, err);
        return -1;
    }
    jnode *lf = json_get(j, "lines_frac");
    if (!lf || lf->t != J_ARR || lf->n < 1 || lf->n > GEOM_MAXK) {
        fprintf(stderr, "errore: %s senza lines_frac valido\n", json);
        json_free(j);
        return -1;
    }
    int k = (int)lf->n;
    for (int i = 0; i < k; i++) {
        jnode *r = json_at(lf, (size_t)i);
        const char *s[3];
        for (int c = 0; c < 3; c++) {
            jnode *x = json_at(r, (size_t)c);
            s[c] = x && x->t == J_STR ? x->s : "";
        }
        zline_init(&L[i]);
        if (zline_from_strings(&L[i], s) != 0) {
            fprintf(stderr, "errore: %s: retta %d non valida\n", json, i);
            json_free(j);
            return -1;
        }
    }
    *radice = j;
    return k;
}

/* Lista di tuple dichiarate (array di array di interi) in tuple_list. */
static void tuple_dichiarate(const jnode *a, tuple_list *l)
{
    l->n = 0;
    l->t = NULL;
    if (!a || a->t != J_ARR)
        return;
    l->t = calloc(a->n + 1, sizeof *l->t);
    for (size_t i = 0; i < a->n; i++) {
        jnode *x = json_at(a, i);
        tuple_t *t = &l->t[l->n++];
        t->n = 0;
        for (size_t q = 0; x && q < x->n; q++) {
            long v = 0;
            json_long(json_at(x, q), &v);
            t->v[t->n++] = (int)v;
        }
    }
}

/* Presentazione in convenzione da rette vere: wiring per x crescente
 * (dall'alto in basso a x -> -inf: pendenza crescente, a parità di
 * pendenza intercetta decrescente), ogni retta orientata per x crescente.
 * -1 se c'è una retta verticale o troppe rette. */
static int presentazione_geometrica(const zline *L, const events_t *ev,
                                    TabellaD *t)
{
    int k = ev->k, ord[GEOM_MAXK], lab[GEOM_MAXK];
    if (k > N_MAX)
        return -1;
    mpq_t m[GEOM_MAXK], q[GEOM_MAXK];
    for (int i = 0; i < k; i++) {
        if (mpz_sgn(L[i].b) == 0)
            return -1;
        mpq_inits(m[i], q[i], NULL);
        mpq_set_num(m[i], L[i].a);
        mpq_set_den(m[i], L[i].b);
        if (mpz_sgn(L[i].b) < 0)
            mpz_neg(mpq_denref(m[i]), mpq_denref(m[i]));
        else
            mpz_neg(mpq_numref(m[i]), mpq_numref(m[i]));
        mpq_canonicalize(m[i]);             /* m = -a/b */
        mpq_set_num(q[i], L[i].c);
        mpq_set_den(q[i], L[i].b);
        if (mpz_sgn(L[i].b) < 0) {
            mpz_neg(mpq_numref(q[i]), mpq_numref(q[i]));
            mpz_neg(mpq_denref(q[i]), mpq_denref(q[i]));
        }
        mpq_canonicalize(q[i]);             /* q = c/b */
        ord[i] = i;
    }
    for (int a = 1; a < k; a++)
        for (int b = a; b > 0; b--) {
            int x = ord[b], y = ord[b - 1];
            int c = mpq_cmp(m[x], m[y]);
            if (c > 0 || (c == 0 && mpq_cmp(q[x], q[y]) <= 0))
                break;
            ord[b] = y;
            ord[b - 1] = x;
        }
    for (int i = 0; i < k; i++)
        lab[ord[i]] = i;
    dtab_vuota(t, k);
    for (int i = 0; i < k; i++) {
        /* geom ordina per s = -b*x + a*y: x crescente se b < 0 */
        int rov = mpz_sgn(L[i].b) > 0, ne = ev->nev[i];
        t->nev[lab[i]] = (unsigned char)ne;
        for (int e = 0; e < ne; e++)
            for (int r = ev->start[i][e]; r < ev->start[i][e + 1]; r++)
                t->pos[lab[i]][lab[ev->mem[i][r]]] =
                    (unsigned char)(rov ? ne - 1 - e : e);
    }
    for (int i = 0; i < k; i++)
        mpq_clears(m[i], q[i], NULL);
    return dtab_ben_formata(t) ? 0 : -1;
}

int dauto_sha_json(const char *json, char out[65])
{
    zline L[GEOM_MAXK];
    jnode *j;
    int k = carica_rette(json, L, &j);
    if (k < 0)
        return -1;
    static events_t ev, form;
    geom_events(L, k, &ev, NULL, NULL);
    canonical_form(&ev, &form);
    canonical_sha256(&form, out);
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
    json_free(j);
    return 0;
}

int dauto_riscontro(const char *json)
{
    zline L[GEOM_MAXK];
    jnode *j;
    int k = carica_rette(json, L, &j);
    if (k < 0)
        return 1;
    static events_t ev, form;
    tuple_list par, multi;
    geom_events(L, k, &ev, &par, &multi);
    char *rp = tuple_list_repr(&par), *rm = tuple_list_repr(&multi);
    canonical_form(&ev, &form);
    char sha[65];
    canonical_sha256(&form, sha);
    printf("riscontro %s\n  rette %d, triangoli %d, punti multipli %s, "
           "parallele %s\n  canone-eventi %s\n", json, k,
           geom_count_by_adjacency(&ev), rm, rp, sha);
    TabellaD t, can;
    if (presentazione_geometrica(L, &ev, &t) == 0) {
        int o = dorb_canonica(&t, &can);
        if (o > 0) {
            printf("  tabella canonica (orbita %d): ", o);
            dtab_scrivi_compatta(stdout, &can);
            putchar('\n');
        } else {
            printf("  presentazione geometrica senza wiring: ERRORE\n");
        }
    }
    free(rp);
    free(rm);
    tuple_list_free(&par);
    tuple_list_free(&multi);
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
    json_free(j);
    return 0;
}

static int esito(int n, int ok, const char *msg)
{
    printf("autotest %d: %s — %s\n", n, ok ? "OK" : "FALLITO", msg);
    return ok ? 0 : 1;
}

/* Sequenze di eventi della soluzione pubblicata n=8 (0-based, compito 02). */
static const char *FIX_K8[8] = {
    "1|7|3|5|2 6|4", "0|7|6|5|3|4|2", "3|7|5|0 6|4|1", "2|7|0|5 6|1|4",
    "3|1|2|0|6|7|5", "1|3 6|0|2|7|4", "1|3 5|0 2|4|7", "1|0|3|2|5|4|6"};

static int eventi_come_fixture(const events_t *ev)
{
    char buf[256];
    for (int i = 0; i < 8; i++) {
        int w = 0;
        for (int e = 0; e < ev->nev[i]; e++) {
            if (e)
                buf[w++] = '|';
            for (int q = ev->start[i][e]; q < ev->start[i][e + 1]; q++)
                w += sprintf(buf + w, q > ev->start[i][e] ? " %d" : "%d",
                             ev->mem[i][q]);
        }
        buf[w] = '\0';
        if (strcmp(buf, FIX_K8[i]) != 0) {
            printf("  retta %d: eventi %s, fixture %s\n", i, buf, FIX_K8[i]);
            return 0;
        }
    }
    return 1;
}

/* Autotest 1 e 2: soluzione pubblicata n=8. */
static int test_k8(const char *radice)
{
    char p[2048], msg[512];
    snprintf(p, sizeof p, "%s/dati/k8-pubblicata.json", radice);
    zline L[GEOM_MAXK];
    jnode *j;
    int k = carica_rette(p, L, &j), fall = 0;
    if (k != 8)
        return esito(1, 0, "dati/k8-pubblicata.json non leggibile") + 1;
    static events_t ev;
    tuple_list par, multi;
    geom_events(L, k, &ev, &par, &multi);
    char *rm = tuple_list_repr(&multi);
    int tri = geom_count_by_adjacency(&ev);
    int ok = tri == 15 && strcmp(rm, "[(0, 2, 6), (3, 5, 6)]") == 0 &&
             par.n == 0 && eventi_come_fixture(&ev);
    snprintf(msg, sizeof msg, "k8 pubblicata: %d triangoli, punti multipli "
             "%s, %d coppie parallele, eventi %s", tri, rm, par.n,
             eventi_come_fixture(&ev) ? "= fixture" : "DIVERSI");
    fall += esito(1, ok, msg);

    TabellaD t;
    TabellaD orb[DORB_MAX];
    int cnt = 0, buone = 0;
    if (presentazione_geometrica(L, &ev, &t) == 0)
        cnt = dorb_orbita(&t, orb);
    for (int q = 0; q < cnt; q++)
        if (dcnf_controlla(&orb[q], 15, NULL) == 0)
            buone++;
    if (cnt > 0 && buone == 0)
        dcnf_controlla(&orb[0], 15, stdout);
    snprintf(msg, sizeof msg, "--controlla sulla k8 pubblicata: %d "
             "presentazioni valide, %d soddisfano tutte le clausole",
             cnt, buone);
    fall += esito(2, buone > 0, msg);
    free(rm);
    tuple_list_free(&par);
    tuple_list_free(&multi);
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
    json_free(j);
    return fall;
}

static const char *FIX3 = "2 3 ; 1 3 ; 1 2";
static const char *FIX5 = "2 4 3 5 ; 1 4 5 3 ; 4 1 5 2 ; 3 1 2 5 ; 1 3 2 4";

/* Autotest 3 e 4: canoniche semplici del compito 01. */
static int test_semplici(void)
{
    const char *fix[2] = {FIX3, FIX5};
    int Tfix[2] = {1, 5}, fall = 0, ok3 = 1, ok4 = 1;
    char msg[1024], parte[256];
    msg[0] = '\0';
    for (int f = 0; f < 2; f++) {
        TabellaD t;
        if (dtab_leggi(fix[f], &t) != 0)
            return esito(3, 0, "fixture illeggibile") + 1;
        long v = dcnf_controlla(&t, Tfix[f], NULL);
        int tri = dtab_triangoli(&t);
        ok3 = ok3 && v == 0 && tri == Tfix[f];
        snprintf(parte, sizeof parte, "%sn=%d: %d triangoli, %ld clausole "
                 "violate", f ? "; " : "", t.n, tri, v);
        strcat(msg, parte);
        if (v)
            dcnf_controlla(&t, Tfix[f], stdout);
    }
    fall += esito(3, ok3, msg);
    msg[0] = '\0';
    for (int f = 0; f < 2; f++) {
        TabellaD t, can, orb[DORB_MAX], id;
        Tabella s, orbs[ORBITA_MAX];
        dtab_leggi(fix[f], &t);
        int finale[N_MAX];
        dorb_wiring(&t, finale);
        dorb_presentazione(&t, finale, 0, 1, &id);
        int cnt = dorb_orbita(&t, orb);
        dorb_canonica(&t, &can);
        /* stessa orbita di tab_orbita (compito 01) */
        int n = t.n;
        tab_vuota(&s, n);
        for (int r = 0; r < n; r++)
            for (int j = 0; j < n; j++)
                if (t.pos[r][j] != DT_NO)
                    s.r[r][t.pos[r][j]] = (unsigned char)(j + 1);
        int cs = tab_orbita(&s, orbs), stessa = cs == cnt;
        for (int a = 0; a < cs && stessa; a++) {
            TabellaD d;
            dtab_da_tabella(&orbs[a], &d);
            int trovata = 0;
            for (int b = 0; b < cnt && !trovata; b++)
                trovata = dtab_confronta(&d, &orb[b]) == 0;
            stessa = trovata;
        }
        int ok = cnt == 2 && stessa && dtab_confronta(&can, &t) == 0 &&
                 dtab_confronta(&id, &t) == 0;
        ok4 = ok4 && ok;
        snprintf(parte, sizeof parte, "%sn=%d: orbita %d (fase 1: %d, %s), "
                 "canonica %s, identità %s", f ? "; " : "", n, cnt, cs,
                 stessa ? "stessa" : "DIVERSA",
                 dtab_confronta(&can, &t) == 0 ? "= fixture" : "DIVERSA",
                 dtab_confronta(&id, &t) == 0 ? "ok" : "DIVERSA");
        strcat(msg, parte);
    }
    fall += esito(4, ok4, msg);
    return fall;
}

/* Autotest 5: k18 sol1, 18 rette con 4 punti tripli. */
static int test_k18(const char *radice)
{
    char p[2048], msg[512];
    snprintf(p, sizeof p, "%s/../k18/sol1/lines_rational.json", radice);
    zline L[GEOM_MAXK];
    jnode *j;
    int k = carica_rette(p, L, &j);
    if (k < 0)
        return esito(5, 0, "k18/sol1 non leggibile");
    static events_t ev;
    tuple_list par, multi, dpar, dmul;
    geom_events(L, k, &ev, &par, &multi);
    tuple_dichiarate(json_get(j, "declared_triple_points"), &dmul);
    tuple_dichiarate(json_get(j, "declared_parallel_pairs"), &dpar);
    long cnt = -1;
    json_long(json_get(j, "count"), &cnt);
    int tri = geom_count_by_adjacency(&ev);
    int stessi = multi.n == dmul.n && par.n == dpar.n;
    for (int q = 0; q < multi.n && stessi; q++)
        stessi = tuple_cmp(&multi.t[q], &dmul.t[q]) == 0;
    for (int q = 0; q < par.n && stessi; q++)
        stessi = tuple_cmp(&par.t[q], &dpar.t[q]) == 0;
    char *rm = tuple_list_repr(&multi);
    snprintf(msg, sizeof msg, "k18 sol1: %d rette, %d triangoli (count %ld), "
             "%d punti multipli %s (%s ai dichiarati), %d parallele", k, tri,
             cnt, multi.n, rm, stessi ? "uguali" : "DIVERSI", par.n);
    int r = esito(5, k == 18 && tri == 93 && cnt == 93 && multi.n == 4 &&
                  stessi, msg);
    free(rm);
    tuple_list_free(&par);
    tuple_list_free(&multi);
    tuple_list_free(&dpar);
    tuple_list_free(&dmul);
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
    json_free(j);
    return r;
}

int dauto_autotest(const char *radice)
{
    int fall = test_k8(radice) + test_semplici() + test_k18(radice);
    printf("autotest: %s\n", fall ? "FALLITO" : "tutti OK");
    return fall ? 1 : 0;
}
