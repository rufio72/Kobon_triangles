/* isocheck — verifica indipendente di non-isomorfismo degli arrangiamenti k18.
 *
 * Uso:
 *   ./isocheck [radice_k18] [--self-test]
 *   ./isocheck --k17 <k17_solutions.json>
 *
 * Isomorfismo = rietichettatura delle rette + inversione del verso di
 * percorrenza di singole rette. Per ogni arrangiamento si ricava la
 * struttura degli eventi (in aritmetica esatta da "lines_frac", oppure
 * direttamente dal campo "events" per i C3 non raddrizzati), se ne
 * calcola la forma canonica completa (canon.c del pacchetto k17, copiato
 * senza modifiche) e si contano le forme distinte su semplici, degeneri,
 * C3 e sul totale. Ogni gruppo di forme uguali con due o più membri
 * produce una riga "DUP: <id1> <id2> ...".
 *
 * Controlli per voce: 93 triangoli per adiacenza, punti multipli e
 * coppie parallele calcolati uguali ai dichiarati, e per i C3 con
 * entrambe le rappresentazioni forma da rette = forma da eventi.
 *
 * Con --self-test: (i) su tutte le voci, forma canonica invariata dopo
 * rietichettatura casuale e inversione casuale di righe applicate agli
 * eventi; (ii) su un campione di 66 voci, forma invariata dopo
 * permutazione delle rette, cambi di segno e riflessione x -> -x.
 *
 * Con --k17: coerenza col pacchetto k17 (85 triangoli, degenerazioni,
 * SHA dichiarati, 8 classi dichiarate).
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "geom.h"
#include "json_min.h"

#define PROVE 5                 /* prove per voce nei self-test */
#define TRIANGOLI_K18 93

enum { CAT_SEMPLICE, CAT_DEGENERE, CAT_C3 };

typedef struct {
    char id[24];
    int cat;
    int ord;                    /* ordine di caricamento */
    char sha[65];
} voce_t;

static voce_t *voci;
static int nvoci, capvoci;
static int esito_ok = 1;
static int do_self;

/* contatori dei self-test */
static int st_ev_tot, st_ev_ok, st_geo_tot, st_geo_ok;
static int st_ev_deg, st_geo_deg;   /* casi degeneri esercitati */

static void muori(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(2);
}

/* Riga di errore per una voce; rende l'esito complessivo FAIL. */
static void fallisci(const char *id, const char *fmt, ...)
{
    va_list ap;
    printf("%s: FAIL — ", id);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    esito_ok = 0;
}

/* PRNG deterministico (xorshift64), come in classify.c del k17. */
static uint64_t rng_stato = 0x9E3779B97F4A7C15ULL;

static uint64_t rng_next(void)
{
    uint64_t x = rng_stato;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return rng_stato = x;
}

static int rng_sotto(int n)
{
    return (int)(rng_next() % (uint64_t)n);
}

static void permutazione(int *p, int k)
{
    for (int i = 0; i < k; i++)
        p[i] = i;
    for (int i = k - 1; i > 0; i--) {         /* Fisher-Yates */
        int j = rng_sotto(i + 1), x = p[i];
        p[i] = p[j];
        p[j] = x;
    }
}

static int int_cmp(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

/* ---------- caricamento ---------- */

static jnode *carica(const char *radice, const char *rel)
{
    char path[4096], err[512];
    snprintf(path, sizeof path, "%s/%s", radice, rel);
    jnode *d = json_load_file(path, err, sizeof err);
    if (!d)
        muori("%s: %s", path, err);
    return d;
}

/* Rette da "lines_frac"; restituisce k oppure -1 se malformate. */
static int carica_rette(const jnode *lf, zline *L)
{
    if (!lf || lf->t != J_ARR || lf->n > GEOM_MAXK)
        return -1;
    for (size_t i = 0; i < lf->n; i++) {
        jnode *t = lf->items[i];
        const char *s[3];
        if (t->t != J_ARR || t->n != 3)
            return -1;
        for (int c = 0; c < 3; c++) {
            if (t->items[c]->t != J_STR)
                return -1;
            s[c] = t->items[c]->s;
        }
        zline_init(&L[i]);
        if (zline_from_strings(&L[i], s)) {
            for (size_t j = 0; j <= i; j++)
                zline_clear(&L[j]);
            return -1;
        }
    }
    return (int)lf->n;
}

static void libera_rette(zline *L, int k)
{
    for (int i = 0; i < k; i++)
        zline_clear(&L[i]);
}

/* Struttura eventi dal campo "events": ogni riga è una lista di eventi,
 * ogni evento una lista di indici (ordinati qui in modo crescente).
 * Controlla che nessuna retta compaia due volte o su se stessa e che
 * la relazione di incrocio sia simmetrica. 0 se valida. */
static int eventi_da_json(const jnode *E, events_t *ev)
{
    static int vista[GEOM_MAXK][GEOM_MAXK];
    if (!E || E->t != J_ARR || E->n == 0 || E->n > GEOM_MAXK)
        return -1;
    int k = (int)E->n;
    ev->k = k;
    memset(vista, 0, sizeof vista);
    for (int i = 0; i < k; i++) {
        jnode *riga = E->items[i];
        if (riga->t != J_ARR || riga->n >= GEOM_MAXK)
            return -1;
        int q = 0;
        ev->nev[i] = (int)riga->n;
        for (size_t e = 0; e < riga->n; e++) {
            jnode *g = riga->items[e];
            if (g->t != J_ARR || g->n == 0)
                return -1;
            ev->start[i][e] = q;
            for (size_t m = 0; m < g->n; m++) {
                long v;
                if (!json_long(g->items[m], &v) || v < 0 || v >= k || v == i ||
                    vista[i][v] || q >= GEOM_MAXK)
                    return -1;
                vista[i][v] = 1;
                ev->mem[i][q++] = (int)v;
            }
            qsort(&ev->mem[i][ev->start[i][e]], g->n, sizeof(int), int_cmp);
        }
        ev->start[i][riga->n] = q;
    }
    for (int i = 0; i < k; i++)
        for (int j = 0; j < k; j++)
            if (vista[i][j] != vista[j][i])
                return -1;
    return 0;
}

/* Lista dichiarata di tuple di interi, ordinata; -1 se malformata. */
static int tuple_cmp_void(const void *x, const void *y)
{
    return tuple_cmp(x, y);
}

static int carica_tuple(const jnode *a, tuple_list *out)
{
    out->n = 0;
    out->t = NULL;
    if (!a || a->t != J_ARR)
        return -1;
    out->t = calloc(a->n ? a->n : 1, sizeof *out->t);
    if (!out->t)
        muori("memoria esaurita");
    for (size_t i = 0; i < a->n; i++) {
        jnode *p = a->items[i];
        if (p->t != J_ARR || p->n > GEOM_MAXK)
            return -1;
        out->t[i].n = (int)p->n;
        for (size_t j = 0; j < p->n; j++) {
            long v;
            if (!json_long(p->items[j], &v))
                return -1;
            out->t[i].v[j] = (int)v;
        }
        out->n++;
    }
    qsort(out->t, (size_t)out->n, sizeof *out->t, tuple_cmp_void);
    return 0;
}

static int tuple_uguali(const tuple_list *x, const tuple_list *y)
{
    if (x->n != y->n)
        return 0;
    for (int i = 0; i < x->n; i++)
        if (tuple_cmp(&x->t[i], &y->t[i]))
            return 0;
    return 1;
}

/* Confronta le degenerazioni calcolate con le liste dichiarate. */
static void confronta_liste(const char *id, const tuple_list *par,
                            const tuple_list *multi, const jnode *dpar_j,
                            const jnode *dmul_j)
{
    tuple_list dpar, dmul;
    if (carica_tuple(dpar_j, &dpar) || !tuple_uguali(par, &dpar)) {
        char *r = tuple_list_repr(par);
        fallisci(id, "coppie parallele calcolate %s diverse dalle dichiarate", r);
        free(r);
    }
    if (carica_tuple(dmul_j, &dmul) || !tuple_uguali(multi, &dmul)) {
        char *r = tuple_list_repr(multi);
        fallisci(id, "punti multipli calcolati %s diversi dai dichiarati", r);
        free(r);
    }
    tuple_list_free(&dpar);
    tuple_list_free(&dmul);
}

/* ---------- self-test ---------- */

/* (i) Rietichettatura + inversione di righe applicate agli eventi. */
static void self_test_eventi(const events_t *ev, const events_t *rif, int degenere)
{
    static events_t nw, form;
    int p[GEOM_MAXK], inv[GEOM_MAXK], k = ev->k, ok = 1;
    for (int t = 0; t < PROVE && ok; t++) {
        permutazione(p, k);             /* la retta i diventa p[i] */
        for (int i = 0; i < k; i++)
            inv[i] = (int)(rng_next() >> 63);
        nw.k = k;
        for (int i = 0; i < k; i++) {
            int r = p[i], n = ev->nev[i], q = 0;
            nw.nev[r] = n;
            for (int e = 0; e < n; e++) {
                int src = inv[r] ? n - 1 - e : e;
                int a = ev->start[i][src], b = ev->start[i][src + 1];
                nw.start[r][e] = q;
                for (int m = a; m < b; m++)
                    nw.mem[r][q++] = p[ev->mem[i][m]];
                qsort(&nw.mem[r][nw.start[r][e]], (size_t)(b - a), sizeof(int),
                      int_cmp);
            }
            nw.start[r][n] = q;
        }
        canonical_form(&nw, &form);
        ok = canon_cmp(&form, rif) == 0;
    }
    st_ev_tot++;
    st_ev_ok += ok;
    st_ev_deg += degenere && ok;
}

/* (ii) Permutazione delle rette, cambi di segno, riflessione x -> -x. */
static void self_test_geometria(const zline *L, int k, const events_t *rif,
                                int degenere)
{
    static events_t ev, form;
    zline nw[GEOM_MAXK];
    int p[GEOM_MAXK], ok = 1;
    for (int i = 0; i < k; i++)
        zline_init(&nw[i]);
    for (int t = 0; t < PROVE && ok; t++) {
        permutazione(p, k);
        for (int i = 0; i < k; i++) {
            const zline *o = &L[p[i]];
            mpz_set(nw[i].a, o->a);
            mpz_set(nw[i].b, o->b);
            mpz_set(nw[i].c, o->c);
            if (rng_next() >> 63) {
                mpz_neg(nw[i].a, nw[i].a);
                mpz_neg(nw[i].b, nw[i].b);
                mpz_neg(nw[i].c, nw[i].c);
            }
            mpz_neg(nw[i].a, nw[i].a);        /* riflessione x -> -x */
        }
        geom_events(nw, k, &ev, NULL, NULL);
        canonical_form(&ev, &form);
        ok = canon_cmp(&form, rif) == 0;
    }
    libera_rette(nw, k);
    st_geo_tot++;
    st_geo_ok += ok;
    st_geo_deg += degenere && ok;
}

/* ---------- elaborazione di una voce ---------- */

static void registra(const char *id, int cat, const events_t *form)
{
    if (nvoci == capvoci) {
        capvoci = capvoci ? 2 * capvoci : 1024;
        voci = realloc(voci, (size_t)capvoci * sizeof *voci);
        if (!voci)
            muori("memoria esaurita");
    }
    voce_t *v = &voci[nvoci++];
    snprintf(v->id, sizeof v->id, "%s", id);
    v->cat = cat;
    v->ord = nvoci - 1;
    canonical_sha256(form, v->sha);
}

/* Voce con "lines_frac": eventi esatti, triangoli, degenerazioni.
 * Se dpar_j/dmul_j sono liste si confrontano come liste; altrimenti
 * (voci C3) si confrontano i conteggi npar/nmul (se >= 0).
 * La forma canonica va in form; geo_test chiede il self-test (ii).
 * Restituisce 0, oppure -1 se le rette sono malformate. */
static int elabora_rette(const char *id, const jnode *lf, const jnode *dpar_j,
                         const jnode *dmul_j, long npar, long nmul, int geo_test,
                         int degenere, events_t *form)
{
    static events_t ev;
    zline L[GEOM_MAXK];
    tuple_list par, multi;
    int k = carica_rette(lf, L);
    if (k < 0) {
        fallisci(id, "lines_frac malformato");
        return -1;
    }
    geom_events(L, k, &ev, &par, &multi);
    int cnt = geom_count_by_adjacency(&ev);
    if (cnt != TRIANGOLI_K18)
        fallisci(id, "%d triangoli invece di %d", cnt, TRIANGOLI_K18);
    if (dpar_j || dmul_j)
        confronta_liste(id, &par, &multi, dpar_j, dmul_j);
    if (npar >= 0 && par.n != npar)
        fallisci(id, "%d coppie parallele calcolate, %ld dichiarate", par.n, npar);
    if (nmul >= 0 && multi.n != nmul)
        fallisci(id, "%d punti multipli calcolati, %ld dichiarati", multi.n, nmul);
    tuple_list_free(&par);
    tuple_list_free(&multi);
    canonical_form(&ev, form);
    if (do_self) {
        self_test_eventi(&ev, form, degenere);
        if (geo_test)
            self_test_geometria(L, k, form, degenere);
    }
    libera_rette(L, k);
    return 0;
}

/* Voce C3 dal campo "events": triangoli e conteggi di degenerazione. */
static int elabora_eventi(const char *id, const jnode *E, long ntrip, long npar,
                          int self, events_t *form)
{
    static events_t ev;
    if (eventi_da_json(E, &ev)) {
        fallisci(id, "campo events malformato o non simmetrico");
        return -1;
    }
    int cnt = geom_count_by_adjacency(&ev), gruppi = 0, mancanti = 0, k = ev.k;
    if (cnt != TRIANGOLI_K18)
        fallisci(id, "%d triangoli (da events) invece di %d", cnt, TRIANGOLI_K18);
    for (int i = 0; i < k; i++) {
        int presenti = 0;
        for (int e = 0; e < ev.nev[i]; e++) {
            int l = ev.start[i][e + 1] - ev.start[i][e];
            gruppi += l >= 2;
            presenti += l;
        }
        mancanti += k - 1 - presenti;
    }
    mancanti /= 2;              /* simmetrico: ogni coppia contata due volte */
    if (gruppi != 3 * ntrip)
        fallisci(id, "%d eventi multipli (da events), attesi 3 x %ld", gruppi, ntrip);
    if (mancanti != npar)
        fallisci(id, "%d coppie parallele (da events), %ld dichiarate", mancanti, npar);
    canonical_form(&ev, form);
    if (self && do_self)
        self_test_eventi(&ev, form, ntrip > 0 || npar > 0);
    return 0;
}

/* ---------- sorgenti k18 ---------- */

static void sorgente_all(const char *radice)
{
    static events_t form;
    jnode *d = carica(radice, "all_solutions.json");
    jnode *S = json_get(d, "solutions");
    if (!S || S->t != J_ARR)
        muori("all_solutions.json: manca il campo solutions");
    int camp_sem = 0, camp_deg = 0;
    for (size_t a = 0; a < S->n; a++) {
        jnode *s = S->items[a];
        long nid;
        char id[24];
        if (!json_long(json_get(s, "id"), &nid))
            muori("all_solutions.json: id non intero");
        snprintf(id, sizeof id, "a%ld", nid);
        jnode *tp = json_get(s, "triple_points"), *pp = json_get(s, "parallel_pairs");
        int deg = !(tp && tp->t == J_ARR && tp->n == 0 && pp && pp->t == J_ARR &&
                    pp->n == 0);
        int geo = deg ? camp_deg < 20 : camp_sem < 20;
        if (elabora_rette(id, json_get(s, "lines_frac"), pp, tp, -1, -1, geo, deg,
                          &form) == 0) {
            camp_deg += deg && geo;
            camp_sem += !deg && geo;
            registra(id, deg ? CAT_DEGENERE : CAT_SEMPLICE, &form);
        }
    }
    json_free(d);
}

static void sorgente_sol(const char *radice)
{
    static events_t form;
    for (int n = 1; n <= 20; n++) {
        char rel[64], id[24];
        snprintf(rel, sizeof rel, "sol%d/lines_rational.json", n);
        snprintf(id, sizeof id, "sol%d", n);
        jnode *d = carica(radice, rel);
        jnode *tp = json_get(d, "declared_triple_points");
        jnode *pp = json_get(d, "declared_parallel_pairs");
        int deg = !(tp && tp->t == J_ARR && tp->n == 0 && pp && pp->t == J_ARR &&
                    pp->n == 0);
        if (!deg)
            fallisci(id, "attesa degenere, dichiarata semplice");
        if (elabora_rette(id, json_get(d, "lines_frac"), pp, tp, -1, -1, 1, deg,
                          &form) == 0)
            registra(id, CAT_DEGENERE, &form);
        json_free(d);
    }
}

static void sorgente_c3(const char *radice)
{
    static events_t fr, fe;
    jnode *d = carica(radice, "c3_symmetric/arrangements.json");
    jnode *S = json_get(d, "solutions");
    if (!S || S->t != J_ARR)
        muori("c3_symmetric/arrangements.json: manca il campo solutions");
    int camp_sem = 0, camp_deg = 0;
    for (size_t a = 0; a < S->n; a++) {
        jnode *s = S->items[a], *jid = json_get(s, "id");
        jnode *st = json_get(s, "straightened"), *lf = json_get(s, "lines_frac");
        long ntrip, npar;
        if (!jid || jid->t != J_STR || !json_long(json_get(s, "triple_points"), &ntrip) ||
            !json_long(json_get(s, "parallel_pairs"), &npar) || !st)
            muori("c3: voce %zu malformata", a);
        const char *id = jid->s;
        int deg = ntrip > 0 || npar > 0, raddr = st->t == J_TRUE;
        if (raddr != (lf != NULL))
            fallisci(id, "straightened e presenza di lines_frac non concordano");
        /* il self-test (i) si fa una volta sola: sugli eventi solo se
         * la voce non ha rette */
        if (elabora_eventi(id, json_get(s, "events"), ntrip, npar, lf == NULL, &fe))
            continue;
        if (lf) {
            int geo = deg ? camp_deg < 2 : camp_sem < 3;
            if (elabora_rette(id, lf, NULL, NULL, npar, ntrip, geo, deg, &fr))
                continue;
            camp_deg += deg && geo;
            camp_sem += !deg && geo;
            if (canon_cmp(&fr, &fe) != 0)
                fallisci(id, "forma da lines_frac diversa dalla forma da events");
            registra(id, CAT_C3, &fr);
        } else {
            registra(id, CAT_C3, &fe);
        }
    }
    json_free(d);
}

static void sorgente_sat(const char *radice)
{
    static events_t form;
    jnode *d = carica(radice, "c3_symmetric_93_lines.json");
    if (elabora_rette("c3_sat", json_get(d, "lines_frac"), NULL, NULL, 0, 0, 1, 0,
                      &form) == 0)
        registra("c3_sat", CAT_C3, &form);
    json_free(d);
}

/* ---------- conteggio delle forme distinte ---------- */

static int voce_cmp(const void *x, const void *y)
{
    const voce_t *a = x, *b = y;
    int c = strcmp(a->sha, b->sha);
    return c ? c : (a->ord > b->ord) - (a->ord < b->ord);
}

/* Forme distinte fra le voci della categoria cat (-1 = tutte); le
 * voci devono essere già ordinate per SHA. Con stampa, righe DUP. */
static int distinte(int cat, int *quante, int stampa)
{
    int d = 0, n = 0;
    for (int i = 0; i < nvoci;) {
        int j = i, membri = 0;
        while (j < nvoci && strcmp(voci[j].sha, voci[i].sha) == 0)
            j++;
        for (int m = i; m < j; m++)
            membri += cat < 0 || voci[m].cat == cat;
        n += membri;
        d += membri > 0;
        if (stampa && membri >= 2) {
            printf("DUP:");
            for (int m = i; m < j; m++)
                printf(" %s", voci[m].id);
            printf("\n");
        }
        i = j;
    }
    *quante = n;
    return d;
}

static int modo_k18(const char *radice)
{
    sorgente_all(radice);
    sorgente_sol(radice);
    sorgente_c3(radice);
    sorgente_sat(radice);

    /* ordinamento stabile per SHA: a parità, ordine di caricamento */
    qsort(voci, (size_t)nvoci, sizeof *voci, voce_cmp);

    int nt, ns, nd, nc;
    int dt = distinte(-1, &nt, 1);
    int ds = distinte(CAT_SEMPLICE, &ns, 0);
    int dd = distinte(CAT_DEGENERE, &nd, 0);
    int dc = distinte(CAT_C3, &nc, 0);
    if (dt != nt)
        esito_ok = 0;

    if (do_self) {
        int ok1 = st_ev_ok == st_ev_tot && st_ev_tot == nvoci && st_ev_deg > 0;
        int ok2 = st_geo_ok == st_geo_tot && st_geo_tot > 0 && st_geo_deg > 0;
        printf("SELF-TEST casi degeneri invarianti: eventi %d, geometria %d\n",
               st_ev_deg, st_geo_deg);
        printf("SELF-TEST eventi: invariante su %d/%d — %s\n",
               st_ev_ok, nvoci, ok1 ? "PASS" : "FAIL");
        printf("SELF-TEST geometria: invariante su %d/%d — %s\n",
               st_geo_ok, st_geo_tot, ok2 ? "PASS" : "FAIL");
        esito_ok &= ok1 && ok2;
    }
    printf("SEMPLICI: %d arrangiamenti, %d forme distinte\n", ns, ds);
    printf("DEGENERI: %d arrangiamenti, %d forme distinte\n", nd, dd);
    printf("C3: %d arrangiamenti, %d forme distinte\n", nc, dc);
    printf("TOTALE: %d arrangiamenti, %d forme distinte — %s\n", nt, dt,
           esito_ok ? "PASS" : "FAIL");
    free(voci);
    return esito_ok ? 0 : 1;
}

/* ---------- modalità --k17 ---------- */

static int modo_k17(const char *file)
{
    static events_t ev, form;
    char err[512];
    jnode *d = json_load_file(file, err, sizeof err);
    if (!d)
        muori("%s: %s", file, err);
    jnode *A = json_get(d, "arrangements"), *C = json_get(d, "classes");
    long tri, ncl;
    if (!A || A->t != J_ARR || !C || C->t != J_ARR ||
        !json_long(json_get(d, "triangles"), &tri) ||
        !json_long(json_get(d, "class_count"), &ncl))
        muori("%s: campi mancanti", file);
    int n = (int)A->n;
    for (int a = 0; a < n; a++) {
        jnode *s = A->items[a], *jid = json_get(s, "id");
        jnode *dsha = json_get(s, "canonical_sha256");
        if (!jid || jid->t != J_STR)
            muori("k17: id mancante");
        zline L[GEOM_MAXK];
        tuple_list par, multi;
        int k = carica_rette(json_get(s, "lines_frac"), L);
        if (k < 0) {
            fallisci(jid->s, "lines_frac malformato");
            continue;
        }
        geom_events(L, k, &ev, &par, &multi);
        libera_rette(L, k);
        int cnt = geom_count_by_adjacency(&ev);
        if (cnt != tri)
            fallisci(jid->s, "%d triangoli invece di %ld", cnt, tri);
        confronta_liste(jid->s, &par, &multi, json_get(s, "parallel_pairs"),
                        json_get(s, "triple_points"));
        tuple_list_free(&par);
        tuple_list_free(&multi);
        canonical_form(&ev, &form);
        registra(jid->s, 0, &form);
        if (!dsha || dsha->t != J_STR || strcmp(dsha->s, voci[nvoci - 1].sha))
            fallisci(jid->s, "canonical_sha256 diverso dal dichiarato");
    }

    /* classi nell'ordine del primo membro (ordine del file) */
    int *classe = malloc((size_t)(nvoci ? nvoci : 1) * sizeof *classe), nc = 0;
    if (!classe)
        muori("memoria esaurita");
    for (int i = 0; i < nvoci; i++) {
        classe[i] = -1;
        for (int j = 0; j < i && classe[i] < 0; j++)
            if (strcmp(voci[j].sha, voci[i].sha) == 0)
                classe[i] = classe[j];
        if (classe[i] < 0)
            classe[i] = nc++;
    }
    for (int c = 0; c < nc; c++) {
        printf("classe %d:", c + 1);
        for (int i = 0; i < nvoci; i++)
            if (classe[i] == c)
                printf(" %s", voci[i].id);
        printf("\n");
    }

    /* confronto con le classi dichiarate (per numero di classe) */
    int uguali = (size_t)nc == C->n && nc == ncl;
    for (int c = 0; uguali && c < nc; c++) {
        jnode *dc = NULL;
        for (size_t x = 0; x < C->n; x++) {
            long v;
            if (json_long(json_get(C->items[x], "class"), &v) && v == c + 1)
                dc = C->items[x];
        }
        jnode *m = dc ? json_get(dc, "members") : NULL;
        jnode *cs = dc ? json_get(dc, "canonical_sha256") : NULL;
        uguali = m && m->t == J_ARR;
        size_t p = 0;
        for (int i = 0; uguali && i < nvoci; i++) {
            if (classe[i] != c)
                continue;
            uguali = p < m->n && m->items[p]->t == J_STR &&
                     strcmp(m->items[p]->s, voci[i].id) == 0 && cs &&
                     cs->t == J_STR && strcmp(cs->s, voci[i].sha) == 0;
            p++;
        }
        uguali = uguali && p == m->n;
    }
    if (!uguali) {
        printf("le classi dichiarate differiscono da quelle calcolate\n");
        esito_ok = 0;
    }
    printf("CLASSI: %d distinte su %d arrangiamenti — %s\n", nc, n,
           esito_ok ? "PASS" : "FAIL");
    free(classe);
    free(voci);
    json_free(d);
    return esito_ok ? 0 : 1;
}

int main(int argc, char **argv)
{
    const char *radice = NULL, *k17 = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--self-test") == 0)
            do_self = 1;
        else if (strcmp(argv[i], "--k17") == 0 && i + 1 < argc)
            k17 = argv[++i];
        else if (argv[i][0] != '-' && !radice)
            radice = argv[i];
        else
            muori("uso: isocheck [radice_k18] [--self-test] | isocheck --k17 <file>");
    }
    if (k17)
        return modo_k17(k17);
    return modo_k18(radice ? radice : "../..");
}
