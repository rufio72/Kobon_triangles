/* census-degeneri: censimento dei tipi di arrangiamenti di n pseudorette
 * con punti multipli e/o classi di parallele al massimo noto di triangoli.
 *
 * Uso: census-degeneri <n> [--out DIR] [--unsat] [--brute] [--con-semplici]
 *                          [--limite S] [--parti K] [--parziale] [--kissat PATH]
 *                          [--tmp DIR]
 *      census-degeneri --autotest | --controlla FILE | --riscontro JSON |
 *                      --canone FILE
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "costanti.h"
#include "dauto.h"
#include "dbruto.h"
#include "dcnf.h"
#include "dorbita.h"
#include "drisultati.h"
#include "dtab.h"

#ifndef CARTELLA_TMP
#define CARTELLA_TMP "tmp"
#endif
#ifndef CARTELLA_CENSUS
#define CARTELLA_CENSUS "."
#endif

typedef struct {
    int n;
    const char *out, *kissat, *tmp, *controlla, *riscontro, *canone;
    int unsat, brute, con_semplici, autotest;
    long limite;
    int parti;          /* processi paralleli, uno per parte (1 = nessuna) */
    int parziale;       /* solo i tipi del file di stato, che resta */
} Opzioni;

typedef struct {
    TabellaD *tipi;
    int ntipi, cap, scartate, chiamate;
    double secondi;
} Censimento;

static double adesso(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void uso(void)
{
    fprintf(stderr,
        "uso: census-degeneri <n> [--out DIR] [--unsat] [--brute] "
        "[--con-semplici]\n"
        "                        [--limite SECONDI] [--parti K] "
        "[--kissat PATH] [--tmp DIR]\n"
        "     census-degeneri --autotest | --controlla FILE | "
        "--riscontro JSON | --canone FILE\n"
        "  n tra 3 e %d; senza opzioni: tipi degeneri a T = massimo noto;\n"
        "  --out DIR: anche verifica del tetto a T+1, degenereN.txt e "
        "sezione README in DIR;\n"
        "  --unsat: solo verifica del tetto a T+1 (--limite: tetto di tempo "
        "di kissat);\n"
        "  --brute: DFS esaustiva (n <= %d) e confronto con il SAT;\n"
        "  --con-semplici: censimento senza il vincolo di degenerazione;\n"
        "  --parti K: K processi paralleli, la parte p < K-1 con esattamente "
        "p terne\n    per un punto comune, l'ultima con almeno K-1.\n",
        N_MAX, DBRUTO_N_MAX);
    exit(1);
}

static void aggiungi_tipo(Censimento *c, const TabellaD *t)
{
    for (int i = 0; i < c->ntipi; i++)
        if (dtab_confronta(&c->tipi[i], t) == 0) {
            fprintf(stderr, "errore: tipo già registrato ritrovato\n");
            exit(2);
        }
    if (c->ntipi == c->cap) {
        c->cap = c->cap ? 2 * c->cap : 16;
        c->tipi = realloc(c->tipi, (size_t)c->cap * sizeof *c->tipi);
    }
    c->tipi[c->ntipi++] = *t;
}

/* Registra una tabella (valida o scartata) e blocca la sua orbita. */
static void registra(Censimento *c, DCnf *f, const TabellaD *t, int valida)
{
    if (!valida) {
        c->scartate++;
        dcnf_blocca(f, t);
        return;
    }
    TabellaD orb[DORB_MAX];
    int cnt = dorb_orbita(t, orb), best = 0;
    for (int q = 0; q < cnt; q++) {
        dcnf_blocca(f, &orb[q]);
        if (dtab_confronta(&orb[q], &orb[best]) < 0)
            best = q;
    }
    aggiungi_tipo(c, &orb[best]);
}

static void riprendi(Censimento *c, DCnf *f, const char *stato)
{
    FILE *fs = fopen(stato, "r");
    if (!fs)
        return;
    char riga[4096];
    int lette = 0;
    while (fgets(riga, sizeof riga, fs)) {
        TabellaD t;
        if ((riga[0] != 'V' && riga[0] != 'X') ||
            dtab_leggi(riga + 1, &t) != 0 || t.n != f->n) {
            fprintf(stderr, "errore: riga non valida in %s\n", stato);
            exit(2);
        }
        registra(c, f, &t, riga[0] == 'V');
        lette++;
    }
    fclose(fs);
    fprintf(stderr, "ripresa da %s: %d tabelle già note\n", stato, lette);
}

static void salva_stato(const char *stato, const TabellaD *t, int valida)
{
    FILE *fs = fopen(stato, "a");
    if (!fs) {
        fprintf(stderr, "errore: impossibile scrivere %s\n", stato);
        exit(2);
    }
    fputs(valida ? "V " : "X ", fs);
    dtab_scrivi_compatta(fs, t);
    fputc('\n', fs);
    fclose(fs);
}

static int confronta_q(const void *a, const void *b)
{
    return dtab_confronta(a, b);
}

/* Vincolo della parte p di K (K <= 1: nessuno). */
static void limita_parte(DCnf *f, int p, int K)
{
    if (K > 1)
        dcnf_parte(f, p, p < K - 1 ? p : -1);
}

/* Ciclo di enumerazione: SAT, estrazione, validazione, conteggio, blocco
 * dell'orbita, fino a UNSAT. Solo la parte p di K se K > 1. */
static void censisci_parte(const Opzioni *o, int modo, int p, int K,
                           Censimento *c)
{
    int n = o->n, T = MAXT[n];
    char tag[32];
    snprintf(tag, sizeof tag, "%s", modo == DCNF_CENSIMENTO ? "deg" : "degs");
    if (K > 1)
        snprintf(tag + strlen(tag), sizeof tag - strlen(tag), "-p%d", p);
    char pcnf[1024], pout[1024], pstato[1024];
    snprintf(pcnf, sizeof pcnf, "%s/%s-n%d.cnf", o->tmp, tag, n);
    snprintf(pout, sizeof pout, "%s/%s-n%d.out", o->tmp, tag, n);
    snprintf(pstato, sizeof pstato, "%s/stato-%s-n%d.txt", o->tmp, tag, n);
    memset(c, 0, sizeof *c);
    double t0 = adesso();
    DCnf f;
    dcnf_costruisci(&f, n, T, modo);
    limita_parte(&f, p, K);
    riprendi(c, &f, pstato);
    while (!o->parziale) {
        if (dcnf_scrivi(&f, pcnf) != 0) {
            fprintf(stderr, "errore: impossibile scrivere %s\n", pcnf);
            exit(2);
        }
        signed char *val = malloc((size_t)f.nvar + 1);
        int esito = dcnf_risolvi(o->kissat, pcnf, pout, f.nvar, val, 0);
        c->chiamate++;
        if (esito == 20) {
            free(val);
            break;
        }
        if (esito != 10)
            exit(2);
        TabellaD t;
        if (dcnf_estrai(n, val, &t) != 0) {
            fprintf(stderr, "errore: modello SAT incoerente\n");
            exit(2);
        }
        free(val);
        int valida = dorb_valida(&t);
        if (valida && dtab_triangoli(&t) != T) {
            fprintf(stderr, "errore: tabella con %d triangoli invece di %d "
                    "(modello incoerente): ", dtab_triangoli(&t), T);
            dtab_scrivi_compatta(stderr, &t);
            fputc('\n', stderr);
            exit(2);
        }
        if (!valida) {
            fprintf(stderr, "scartata (wiring non ricostruibile): ");
            dtab_scrivi_compatta(stderr, &t);
            fputc('\n', stderr);
        }
        salva_stato(pstato, &t, valida);
        registra(c, &f, &t, valida);
        if (c->chiamate % 50 == 0)
            fprintf(stderr, "n=%d %s: %d chiamate, %d tipi, %.0f s\n", n,
                    tag, c->chiamate, c->ntipi, adesso() - t0);
    }
    dcnf_libera(&f);
    if (!o->parziale)
        remove(pstato);
    qsort(c->tipi, (size_t)c->ntipi, sizeof *c->tipi, confronta_q);
    c->secondi = adesso() - t0;
}

/* Attende i K figli; 0 se tutti escono con codice 0 (o ammesso). */
static int attendi(int K, const pid_t *pid, int *codice)
{
    int errori = 0;
    for (int p = 0; p < K; p++) {
        int st;
        if (waitpid(pid[p], &st, 0) < 0 || !WIFEXITED(st)) {
            codice[p] = 2;
            errori++;
        } else {
            codice[p] = WEXITSTATUS(st);
        }
    }
    return errori;
}

static pid_t biforca(void)
{
    fflush(NULL);
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "errore: fork non riuscita\n");
        exit(2);
    }
    return pid;
}

/* Censimento completo: diretto, o in K processi paralleli (uno per parte)
 * che scrivono i loro tipi in tmp e vengono poi riuniti. */
static void censisci(const Opzioni *o, int modo, Censimento *c)
{
    int K = o->parti, n = o->n;
    if (K <= 1) {
        censisci_parte(o, modo, 0, 1, c);
        return;
    }
    double t0 = adesso();
    pid_t pid[64];
    int codice[64];
    char pris[1024];
    for (int p = 0; p < K; p++) {
        pid[p] = biforca();
        if (pid[p] > 0)
            continue;
        Censimento cp;
        censisci_parte(o, modo, p, K, &cp);
        snprintf(pris, sizeof pris, "%s/ris-%d-n%d-p%d.txt", o->tmp, modo, n,
                 p);
        FILE *f = fopen(pris, "w");
        if (!f)
            _exit(2);
        fprintf(f, "S %d %d\n", cp.scartate, cp.chiamate);
        for (int i = 0; i < cp.ntipi; i++) {
            fputs("V ", f);
            dtab_scrivi_compatta(f, &cp.tipi[i]);
            fputc('\n', f);
        }
        fclose(f);
        fprintf(stderr, "n=%d parte %d di %d: %d tipi, %d chiamate, %.0f s\n",
                n, p, K, cp.ntipi, cp.chiamate, cp.secondi);
        _exit(0);
    }
    if (attendi(K, pid, codice)) {
        fprintf(stderr, "errore: un processo di una parte è fallito\n");
        exit(2);
    }
    memset(c, 0, sizeof *c);
    for (int p = 0; p < K; p++) {
        if (codice[p] != 0)
            exit(2);
        snprintf(pris, sizeof pris, "%s/ris-%d-n%d-p%d.txt", o->tmp, modo, n,
                 p);
        FILE *f = fopen(pris, "r");
        char riga[4096];
        while (f && fgets(riga, sizeof riga, f)) {
            TabellaD t;
            int sc, ch;
            if (riga[0] == 'S' && sscanf(riga + 1, "%d %d", &sc, &ch) == 2) {
                c->scartate += sc;
                c->chiamate += ch;
            } else if (riga[0] == 'V' && dtab_leggi(riga + 1, &t) == 0) {
                aggiungi_tipo(c, &t);   /* un doppione fra parti è errore */
            } else {
                fprintf(stderr, "errore: riga non valida in %s\n", pris);
                exit(2);
            }
        }
        if (!f)
            exit(2);
        fclose(f);
        remove(pris);
    }
    qsort(c->tipi, (size_t)c->ntipi, sizeof *c->tipi, confronta_q);
    c->secondi = adesso() - t0;
}

/* Tetto sulla parte p di K: 20 UNSAT, 10 SAT, 0 tempo scaduto, -1 errore.
 * Un modello SAT viene validato e stampato in evidenza. */
static int tetto_parte(const Opzioni *o, int p, int K)
{
    int n = o->n, T1 = MAXT[n] + 1;
    char pcnf[1024], pout[1024], tag[16] = "";
    if (K > 1)
        snprintf(tag, sizeof tag, "-p%d", p);
    snprintf(pcnf, sizeof pcnf, "%s/tetto-deg-n%d%s.cnf", o->tmp, n, tag);
    snprintf(pout, sizeof pout, "%s/tetto-deg-n%d%s.out", o->tmp, n, tag);
    DCnf f;
    dcnf_costruisci(&f, n, T1, DCNF_UNSAT);
    limita_parte(&f, p, K);
    if (dcnf_scrivi(&f, pcnf) != 0)
        return -1;
    signed char *val = malloc((size_t)f.nvar + 1);
    int e = dcnf_risolvi(o->kissat, pcnf, pout, f.nvar, val, o->limite);
    if (e == 10) {
        TabellaD t;
        int ok = dcnf_estrai(n, val, &t) == 0;
        int valida = ok && dorb_valida(&t);
        printf("n=%d, almeno %d triangoli%s: SAT!  %s, %d triangoli: ", n, T1,
               tag, valida ? "tabella valida" : "tabella NON valida",
               ok ? dtab_triangoli(&t) : -1);
        if (ok)
            dtab_scrivi_compatta(stdout, &t);
        putchar('\n');
        fflush(stdout);
    }
    free(val);
    dcnf_libera(&f);
    return e;
}

/* Verifica del tetto: nessun arrangiamento con almeno T+1 triangoli,
 * diretta o in K parti parallele. Restituisce il testo dell'esito (inglese,
 * per il README) in esito; codice 0 UNSAT, 4 tempo scaduto, 3 altrimenti. */
static int verifica_tetto(const Opzioni *o, char *esito, size_t len)
{
    int n = o->n, T1 = MAXT[n] + 1, K = o->parti > 1 ? o->parti : 1;
    double t0 = adesso();
    int e;
    if (K == 1) {
        e = tetto_parte(o, 0, 1);
    } else {
        pid_t pid[64];
        int codice[64], sat = 0, scad = 0, err = 0;
        for (int p = 0; p < K; p++) {
            pid[p] = biforca();
            if (pid[p] == 0) {
                int r = tetto_parte(o, p, K);
                _exit(r == 20 ? 0 : r == 0 ? 4 : r == 10 ? 3 : 2);
            }
        }
        err = attendi(K, pid, codice);
        for (int p = 0; p < K; p++) {
            sat += codice[p] == 3;
            scad += codice[p] == 4;
            err += codice[p] == 2;
        }
        e = sat ? 10 : err ? -1 : scad ? 0 : 20;
    }
    double s = adesso() - t0;
    char parti[96] = "";
    if (K > 1)
        snprintf(parti, sizeof parti, ", %d parallel parts by number of "
                 "concurrent triples", K);
    if (e == 20) {
        snprintf(esito, len, "**UNSAT** (kissat%s, %.1f s)", parti, s);
        printf("n=%d, almeno %d triangoli: UNSAT, %.2f s\n", n, T1, s);
        return 0;
    }
    if (e == 0) {
        snprintf(esito, len, "open (runtime: no answer within %ld s%s)",
                 o->limite, parti);
        printf("n=%d, almeno %d triangoli: tempo scaduto (%ld s), tetto "
               "aperto\n", n, T1, o->limite);
        return 4;
    }
    if (e == 10)
        snprintf(esito, len, "SAT at pseudoline level (see census output)");
    else
        snprintf(esito, len, "error");
    return 3;
}

static void stampa_tipo(int i, const TabellaD *t)
{
    char sha[65];
    TabellaD orb[DORB_MAX];
    dauto_sha(t, sha);
    printf("tipo %d: ", i + 1);
    dtab_scrivi_compatta(stdout, t);
    printf("\n  orbita %d, triangoli %d, punti multipli ", dorb_orbita(t, orb),
           dtab_triangoli(t));
    dtab_scrivi_multipli(stdout, t);
    fputs(", parallele ", stdout);
    dtab_scrivi_parallele(stdout, t);
    printf(", %s\n  canone-eventi %s\n", dtab_semplice(t) ? "semplice" :
           "degenere", sha);
}

static int modo_brute(const Opzioni *o)
{
    int n = o->n, T = MAXT[n];
    if (n > DBRUTO_N_MAX) {
        fprintf(stderr, "errore: --brute ammesso solo per n <= %d\n",
                DBRUTO_N_MAX);
        return 1;
    }
    double t0 = adesso();
    EsitoDBruto e;
    dbruto_esegui(n, T, &e);
    printf("brute n=%d: stati %lld, foglie %lld, tabelle con T=%d: %d, "
           "classi %d (semplici %d, degeneri %d), %.2f s\n", n, e.stati,
           e.foglie, T, e.con_t, e.classi, e.semplici, e.classi - e.semplici,
           adesso() - t0);
    Censimento c;
    censisci(o, DCNF_CENSIMENTO, &c);
    printf("SAT   n=%d: degeneri %d, scartate %d, chiamate %d, %.2f s\n", n,
           c.ntipi, c.scartate, c.chiamate, c.secondi);
    int k = 0, ok = 1;
    for (int i = 0; i < e.classi && ok; i++) {
        if (dtab_semplice(&e.canon[i]))
            continue;
        ok = k < c.ntipi && dtab_confronta(&e.canon[i], &c.tipi[k]) == 0;
        k++;
    }
    ok = ok && k == c.ntipi;
    int oks = e.semplici == SEMPLICI_FASE1[n];
    printf("totale a T: brute %d = semplici fase 1 %d + degeneri SAT %d: %s\n",
           e.classi, SEMPLICI_FASE1[n], c.ntipi,
           ok && oks ? "COINCIDONO" : "DIVERSI");
    if (o->con_semplici) {
        Censimento cs;
        censisci(o, DCNF_CON_SEMPLICI, &cs);
        int uguali = cs.ntipi == e.classi;
        for (int i = 0; i < cs.ntipi && uguali; i++)
            uguali = dtab_confronta(&cs.tipi[i], &e.canon[i]) == 0;
        printf("SAT con semplici n=%d: %d tipi, confronto con brute: %s\n",
               n, cs.ntipi, uguali ? "COINCIDONO" : "DIVERSI");
        ok = ok && uguali;
        free(cs.tipi);
    }
    dbruto_libera(&e);
    free(c.tipi);
    return ok && oks ? 0 : 3;
}

static int modo_controlla(const char *file)
{
    TabellaD t;
    if (dtab_leggi_file(file, &t) != 0) {
        fprintf(stderr, "errore: tabella non valida in %s\n", file);
        return 1;
    }
    int T = dtab_triangoli(&t);
    printf("tabella n=%d, %d triangoli, wiring %s\n", t.n, T,
           dorb_valida(&t) ? "valido" : "NON valido");
    return dcnf_controlla(&t, T, stdout) == 0 ? 0 : 3;
}

static int modo_canone(const char *file)
{
    TabellaD t, can;
    if (dtab_leggi_file(file, &t) != 0) {
        fprintf(stderr, "errore: tabella non valida in %s\n", file);
        return 1;
    }
    if (dorb_canonica(&t, &can) == 0) {
        printf("wiring non ricostruibile: nessuna forma canonica\n");
        return 3;
    }
    stampa_tipo(0, &can);
    return 0;
}

int main(int argc, char **argv)
{
    Opzioni o = {0, NULL, "kissat", CARTELLA_TMP, NULL, NULL, NULL,
                 0, 0, 0, 0, 0, 1, 0};
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        int altro = i + 1 < argc;
        if (!strcmp(a, "--unsat")) o.unsat = 1;
        else if (!strcmp(a, "--brute")) o.brute = 1;
        else if (!strcmp(a, "--con-semplici")) o.con_semplici = 1;
        else if (!strcmp(a, "--autotest")) o.autotest = 1;
        else if (!strcmp(a, "--out") && altro) o.out = argv[++i];
        else if (!strcmp(a, "--kissat") && altro) o.kissat = argv[++i];
        else if (!strcmp(a, "--tmp") && altro) o.tmp = argv[++i];
        else if (!strcmp(a, "--controlla") && altro) o.controlla = argv[++i];
        else if (!strcmp(a, "--riscontro") && altro) o.riscontro = argv[++i];
        else if (!strcmp(a, "--canone") && altro) o.canone = argv[++i];
        else if (!strcmp(a, "--limite") && altro) o.limite = atol(argv[++i]);
        else if (!strcmp(a, "--parti") && altro) o.parti = atoi(argv[++i]);
        else if (!strcmp(a, "--parziale")) o.parziale = 1;
        else if (a[0] >= '0' && a[0] <= '9' && o.n == 0) o.n = atoi(a);
        else uso();
    }
    if (o.autotest)
        return dauto_autotest(CARTELLA_CENSUS);
    if (o.controlla)
        return modo_controlla(o.controlla);
    if (o.riscontro)
        return dauto_riscontro(o.riscontro);
    if (o.canone)
        return modo_canone(o.canone);
    if (o.n < 3 || o.n > N_MAX || o.parti < 1 || o.parti > 64)
        uso();
    if (mkdir(o.tmp, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "errore: impossibile creare %s\n", o.tmp);
        return 2;
    }
    int n = o.n, T = MAXT[n];
    char tetto[256];
    if (o.unsat)
        return verifica_tetto(&o, tetto, sizeof tetto);
    if (o.brute)
        return modo_brute(&o);

    /* Con --out il tetto a T+1 gira in un processo parallelo al
     * censimento: il suo esito arriva in un file di tmp. */
    int fai_tetto = o.out && !o.con_semplici && !o.parziale;
    char ptetto[1024];
    snprintf(ptetto, sizeof ptetto, "%s/tetto-esito-n%d.txt", o.tmp, n);
    pid_t figlio = 0;
    if (fai_tetto) {
        remove(ptetto);
        figlio = biforca();
        if (figlio == 0) {
            int r = verifica_tetto(&o, tetto, sizeof tetto);
            FILE *f = fopen(ptetto, "w");
            if (f) {
                fprintf(f, "%d\n%s\n", r, tetto);
                fclose(f);
            }
            fflush(NULL);
            _exit(0);
        }
    }
    Censimento c;
    int modo = o.con_semplici ? DCNF_CON_SEMPLICI : DCNF_CENSIMENTO;
    censisci(&o, modo, &c);
    printf("n=%d, T=%d: %d tipi %s, %d tabelle scartate, %d chiamate SAT, "
           "%.2f s\n", n, T, c.ntipi, o.con_semplici ? "(semplici inclusi)" :
           "degeneri", c.scartate, c.chiamate, c.secondi);
    for (int i = 0; i < c.ntipi; i++)
        stampa_tipo(i, &c.tipi[i]);
    int rc = 0;
    if (o.out && o.parziale) {
        snprintf(tetto, sizeof tetto, "open (runtime: kissat still running "
                 "when the census was closed)");
        rc = 4;
    } else if (fai_tetto) {
        int st;
        waitpid(figlio, &st, 0);
        FILE *f = fopen(ptetto, "r");
        rc = 3;
        snprintf(tetto, sizeof tetto, "error");
        if (f && fscanf(f, "%d\n", &rc) == 1 && fgets(tetto, sizeof tetto, f))
            tetto[strcspn(tetto, "\n")] = '\0';
        if (f)
            fclose(f);
        remove(ptetto);
    }
    if (o.out && !o.con_semplici) {
        char (*sha)[65] = malloc((size_t)(c.ntipi + 1) * sizeof *sha);
        for (int i = 0; i < c.ntipi; i++)
            dauto_sha(&c.tipi[i], sha[i]);
        dris_scrivi_tipi(o.out, n, T, c.tipi, sha, c.ntipi);
        char nota[512], *pnota = NULL, shapub[65];
        if (n == 8 && dauto_sha_json(CARTELLA_CENSUS
                                     "/dati/k8-pubblicata.json", shapub) == 0)
            for (int i = 0; i < c.ntipi && !pnota; i++)
                if (strcmp(sha[i], shapub) == 0) {
                    snprintf(nota, sizeof nota, "`degenere%d.txt` has the "
                             "same event canonical form\n(canonical_sha256 "
                             "%.16s…) as the published solution\n"
                             "(`census/dati/k8-pubblicata.json`).", i + 1,
                             shapub);
                    pnota = nota;
                }
        if (o.parziale) {
            snprintf(nota, sizeof nota, "**Partial result**: the enumeration "
                     "was stopped before completion;\nthe %d types listed are "
                     "those found so far (resumable from\n"
                     "`census/tmp/stato-deg-n%d.txt`).", c.ntipi, n);
            pnota = nota;
        }
        dris_readme(o.out, n, T, c.ntipi, c.scartate, tetto, pnota,
                    o.parziale);
        free(sha);
        const char *breve = strstr(tetto, "UNSAT") ? "UNSAT" :
                            strstr(tetto, "open") ? "aperto" :
                            strstr(tetto, "SAT") ? "SAT!" : "errore";
        printf("RIEPILOGO %d %d %d %d%s %s %d\n", n, T, SEMPLICI_FASE1[n],
               c.ntipi, o.parziale ? "+" : "", breve, c.scartate);
        if (rc == 4)
            rc = 0;     /* tetto aperto per tempo: non è un errore */
    }
    free(c.tipi);
    return rc;
}
