/* census-kobon: censimento dei tipi di arrangiamenti semplici di n
 * pseudorette con il massimo noto di triangoli di Kobon.
 *
 * Uso: census-kobon <n> [--out DIR] [--unsat] [--brute] [--kissat PATH]
 *                       [--tmp DIR]
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "bruto.h"
#include "cnf.h"
#include "costanti.h"
#include "risultati.h"
#include "sat.h"
#include "tabella.h"
#include "wiring.h"

#ifndef CARTELLA_TMP
#define CARTELLA_TMP "tmp"
#endif

typedef struct {
    int n;
    const char *out, *kissat, *tmp;
    int unsat, brute;
} Opzioni;

typedef struct {
    Tabella *tipi;
    int ntipi, cap;
    int scartate;
    int chiamate;
    double secondi;
} Censimento;

/* Esiti della verifica a T+1. */
enum { TETTO_ARITMETICO, TETTO_UNSAT, TETTO_SAT, TETTO_ERRORE };

static double adesso(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void crea_cartella(const char *p)
{
    if (mkdir(p, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "errore: impossibile creare %s\n", p);
        exit(2);
    }
}

static void uso(void)
{
    fprintf(stderr,
            "uso: census-kobon <n> [--out DIR] [--unsat] [--brute] "
            "[--kissat PATH] [--tmp DIR]\n"
            "  n tra 3 e %d; --unsat: solo verifica del tetto a T+1;\n"
            "  --brute: enumerazione esaustiva (n <= %d) e confronto;\n"
            "  --out DIR: scrive tipi, README e verifica T+1 in DIR.\n",
            N_MAX, BRUTO_N_MAX);
    exit(1);
}

static void aggiungi_tipo(Censimento *c, const Tabella *t)
{
    for (int i = 0; i < c->ntipi; i++)
        if (tab_confronta(&c->tipi[i], t) == 0) {
            fprintf(stderr, "errore: tipo già registrato ritrovato\n");
            exit(2);
        }
    if (c->ntipi == c->cap) {
        c->cap = c->cap ? 2 * c->cap : 16;
        c->tipi = realloc(c->tipi, (size_t)c->cap * sizeof *c->tipi);
    }
    c->tipi[c->ntipi++] = *t;
}

static void blocca_orbita(Cnf *f, const Tabella *t)
{
    Tabella orb[ORBITA_MAX];
    int cnt = tab_orbita(t, orb);
    for (int i = 0; i < cnt; i++)
        cnf_blocca(f, &orb[i]);
}

/* Registra una tabella trovata (valida o scartata) e blocca la sua orbita. */
static void registra(Censimento *c, Cnf *f, const Tabella *t, int valida)
{
    if (valida) {
        Tabella can;
        tab_canonica(t, &can);
        aggiungi_tipo(c, &can);
    } else {
        c->scartate++;
    }
    blocca_orbita(f, t);
}

/* Rilegge il file di stato di un censimento interrotto. */
static void riprendi(Censimento *c, Cnf *f, const char *stato, int n)
{
    FILE *fs = fopen(stato, "r");
    if (!fs)
        return;
    char riga[4096];
    int lette = 0;
    while (fgets(riga, sizeof riga, fs)) {
        Tabella t;
        if ((riga[0] != 'V' && riga[0] != 'X') ||
            tab_leggi(riga + 1, n, &t) != 0) {
            fprintf(stderr, "errore: riga non valida in %s\n", stato);
            exit(2);
        }
        registra(c, f, &t, riga[0] == 'V');
        lette++;
    }
    fclose(fs);
    fprintf(stderr, "ripresa da %s: %d tabelle già note\n", stato, lette);
}

static void salva_stato(const char *stato, const Tabella *t, int valida)
{
    FILE *fs = fopen(stato, "a");
    if (!fs) {
        fprintf(stderr, "errore: impossibile scrivere %s\n", stato);
        exit(2);
    }
    fputc(valida ? 'V' : 'X', fs);
    for (int r = 0; r < t->n; r++)
        for (int k = 0; k < t->n - 1; k++)
            fprintf(fs, " %d", t->r[r][k]);
    fputc('\n', fs);
    fclose(fs);
}

static int confronta_q(const void *a, const void *b)
{
    return tab_confronta(a, b);
}

/* Ciclo di enumerazione: SAT, estrazione, validazione, blocco dell'orbita,
 * fino a UNSAT. */
static void censisci(const Opzioni *o, Censimento *c)
{
    int n = o->n, T = MAXT[n], m = segmenti_mancanti(n, T);
    char pcnf[1024], pout[1024], pstato[1024];
    snprintf(pcnf, sizeof pcnf, "%s/census-n%d.cnf", o->tmp, n);
    snprintf(pout, sizeof pout, "%s/census-n%d.out", o->tmp, n);
    snprintf(pstato, sizeof pstato, "%s/stato-n%d.txt", o->tmp, n);
    memset(c, 0, sizeof *c);
    double t0 = adesso();

    Cnf f;
    cnf_kobon(&f, n, m);
    riprendi(c, &f, pstato, n);
    signed char *val = malloc((size_t)f.nvar + 1);
    for (;;) {
        if (cnf_scrivi(&f, pcnf) != 0) {
            fprintf(stderr, "errore: impossibile scrivere %s\n", pcnf);
            exit(2);
        }
        int esito = sat_risolvi(o->kissat, pcnf, pout, f.nvar, val);
        c->chiamate++;
        if (esito == 20)
            break;
        if (esito != 10)
            exit(2);
        Tabella t;
        if (wiring_estrai(n, val, &t) != 0) {
            fprintf(stderr, "errore: modello SAT incoerente\n");
            exit(2);
        }
        int valida = wiring_valida_orbita(&t);
        if (valida && tab_triangoli(&t) != T) {
            fprintf(stderr, "errore: tabella con %d triangoli invece di %d\n",
                    tab_triangoli(&t), T);
            tab_scrivi(stderr, &t);
            exit(2);
        }
        if (!valida) {
            fprintf(stderr, "scartata (non realizzabile come pseudorette): ");
            tab_stampa_lista(stderr, &t);
            fputc('\n', stderr);
        }
        salva_stato(pstato, &t, valida);
        registra(c, &f, &t, valida);
    }
    free(val);
    cnf_libera(&f);
    remove(pstato);   /* enumerazione completa: lo stato non serve più */
    qsort(c->tipi, (size_t)c->ntipi, sizeof *c->tipi, confronta_q);
    c->secondi = adesso() - t0;
}

/* Verifica del tetto: nessun arrangiamento con T+1 triangoli. */
static int verifica_tetto(const Opzioni *o, double *secondi)
{
    int n = o->n, m1 = segmenti_mancanti(n, MAXT[n] + 1);
    *secondi = 0;
    if (m1 < 0)
        return TETTO_ARITMETICO;
    char pcnf[1024], pout[1024];
    snprintf(pcnf, sizeof pcnf, "%s/tetto-n%d.cnf", o->tmp, n);
    snprintf(pout, sizeof pout, "%s/tetto-n%d.out", o->tmp, n);
    double t0 = adesso();
    Cnf f;
    cnf_kobon(&f, n, m1);
    if (cnf_scrivi(&f, pcnf) != 0)
        return TETTO_ERRORE;
    signed char *val = malloc((size_t)f.nvar + 1);
    int esito = sat_risolvi(o->kissat, pcnf, pout, f.nvar, val);
    free(val);
    cnf_libera(&f);
    *secondi = adesso() - t0;
    if (esito == 20)
        return TETTO_UNSAT;
    return esito == 10 ? TETTO_SAT : TETTO_ERRORE;
}

static const char *nome_tetto(int e)
{
    switch (e) {
    case TETTO_ARITMETICO: return "aritmetico";
    case TETTO_UNSAT: return "UNSAT";
    case TETTO_SAT: return "SAT!";
    default: return "errore";
    }
}

static void stampa_tetto(const Opzioni *o, int e, double s)
{
    int n = o->n, T = MAXT[n];
    if (e == TETTO_ARITMETICO)
        printf("n=%d, %d triangoli: impossibile per conteggio dei segmenti "
               "(%d*%d - 3*%d < 0)\n", n, T + 1, n, n - 2, T + 1);
    else if (e == TETTO_UNSAT)
        printf("n=%d, %d triangoli: UNSAT (al più %d segmenti mancanti), "
               "%.2f s\n", n, T + 1, segmenti_mancanti(n, T + 1), s);
    else if (e == TETTO_SAT)
        printf("n=%d, %d triangoli: SAT, il modello contraddice il massimo "
               "noto: ERRORE\n", n, T + 1);
    else
        printf("n=%d: errore nella verifica del tetto\n", n);
}

static void stampa_tipi(const Censimento *c)
{
    for (int i = 0; i < c->ntipi; i++) {
        printf("tipo %d: ", i + 1);
        tab_stampa_lista(stdout, &c->tipi[i]);
        static Tabella orb[ORBITA_MAX];
        printf("  (orbita %d)\n", tab_orbita(&c->tipi[i], orb));
    }
}

static int modo_brute(const Opzioni *o)
{
    int n = o->n, T = MAXT[n];
    if (n > BRUTO_N_MAX) {
        fprintf(stderr, "errore: --brute ammesso solo per n <= %d\n",
                BRUTO_N_MAX);
        return 1;
    }
    double t0 = adesso();
    EsitoBruto e;
    bruto_esegui(n, T, &e);
    printf("brute n=%d: parole %lld, tabelle distinte %d, tabelle con T max "
           "%d, classi %d, non valide %d, %.2f s\n", n, e.parole, e.distinte,
           e.con_tmax, e.classi, e.non_valide, adesso() - t0);
    Censimento c;
    censisci(o, &c);
    printf("SAT   n=%d: classi %d, scartate %d, chiamate %d, %.2f s\n", n,
           c.ntipi, c.scartate, c.chiamate, c.secondi);
    int ok = e.classi == c.ntipi;
    for (int i = 0; ok && i < c.ntipi; i++)
        ok = tab_confronta(&e.canon[i], &c.tipi[i]) == 0;
    printf("confronto brute/SAT: %s\n", ok ? "COINCIDONO" : "DIVERSI");
    bruto_libera(&e);
    free(c.tipi);
    return ok ? 0 : 3;
}

int main(int argc, char **argv)
{
    Opzioni o = {0, NULL, "kissat", CARTELLA_TMP, 0, 0};
    if (argc < 2)
        uso();
    o.n = atoi(argv[1]);
    if (o.n < 3 || o.n > N_MAX)
        uso();
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--unsat")) o.unsat = 1;
        else if (!strcmp(argv[i], "--brute")) o.brute = 1;
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) o.out = argv[++i];
        else if (!strcmp(argv[i], "--kissat") && i + 1 < argc)
            o.kissat = argv[++i];
        else if (!strcmp(argv[i], "--tmp") && i + 1 < argc) o.tmp = argv[++i];
        else uso();
    }
    crea_cartella(o.tmp);
    int n = o.n, T = MAXT[n];

    if (o.unsat) {
        double s;
        int e = verifica_tetto(&o, &s);
        stampa_tetto(&o, e, s);
        return (e == TETTO_ARITMETICO || e == TETTO_UNSAT) ? 0 : 3;
    }
    if (o.brute)
        return modo_brute(&o);

    Censimento c;
    censisci(&o, &c);
    printf("n=%d, T=%d, segmenti mancanti ammessi %d: %d tipi, %d tabelle "
           "scartate, %d chiamate SAT, %.2f s\n", n, T,
           segmenti_mancanti(n, T), c.ntipi, c.scartate, c.chiamate,
           c.secondi);
    stampa_tipi(&c);
    int esito = 0;
    if (o.out) {
        double s;
        int e = verifica_tetto(&o, &s);
        stampa_tetto(&o, e, s);
        scrivi_risultati(o.out, n, T, c.tipi, c.ntipi, c.scartate,
                         nome_tetto(e));
        printf("RIEPILOGO %d %d %d %s %d\n", n, T, c.ntipi, nome_tetto(e),
               c.scartate);
        if (e != TETTO_ARITMETICO && e != TETTO_UNSAT)
            esito = 3;
    }
    free(c.tipi);
    return esito;
}
