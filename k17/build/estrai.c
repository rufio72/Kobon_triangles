/* Estrazione dei 21 arrangiamenti ottimali a 17 rette dal pacchetto k18.
 *
 * Unico file del pacchetto che conosce il k18. Legge
 * esperimenti/risultati_sotto17.json del k18 (per ogni arrangiamento a 18
 * rette e 93 triangoli, il conteggio dopo aver tolto ciascuna retta), prende
 * le coppie (arrangiamento, retta) con 85 triangoli nell'ordine del file,
 * toglie la retta, ricalcola eventi, conteggio, degenerazioni residue e forma
 * canonica con i moduli geom e canon, raggruppa in classi e scrive
 * k17_solutions.json nella radice del pacchetto.
 *
 * Il k18 si legge soltanto: questo programma non scrive nulla lì.
 *
 * Il file scritto è identico byte per byte a quello di json.dump(...,
 * indent=1) di Python seguito da un a capo: stesse chiavi nello stesso
 * ordine, stringhe "p/q" copiate identiche dalla fonte, lines_float con
 * arrotondamento corretto al double più vicino e resa come repr() di Python.
 *
 * Uso:  ./build/estrai [--k18 PERCORSO] [--uscita FILE]
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "geom.h"
#include "json_min.h"

#define K18 "/home/rufio72/git/kobon_triangles_k18"
#define CONTEGGIO 85
#define ATTESI 21
#define FONTI 2357
#define RETTE 18

static void muori(const char *fmt, const char *a, int b, int c)
{
    fprintf(stderr, fmt, a, b, c);
    fprintf(stderr, "\n");
    exit(1);
}

static jnode *carica(const char *percorso)
{
    char err[1024];
    jnode *n = json_load_file(percorso, err, sizeof err);
    if (!n)
        muori("%s", err, 0, 0);
    return n;
}

/* ---- valori in virgola mobile (solo per lines_float, mai nei predicati) */

/* Double più vicino al razionale v (pari in caso di parità), come
 * float(Fraction) di Python; mpq_get_d invece tronca verso zero. */
static double q_a_double(const mpq_t v)
{
    double d = mpq_get_d(v);
    mpq_t t, m;
    int s = mpq_sgn(v);
    if (s == 0)
        return 0.0;
    mpq_inits(t, m, NULL);
    mpq_set_d(t, d);
    if (!mpq_equal(t, v)) {
        double d2 = nextafter(d, s > 0 ? HUGE_VAL : -HUGE_VAL);
        mpq_set_d(m, d2);
        mpq_add(m, m, t);
        mpq_div_2exp(m, m, 1);              /* punto medio esatto */
        int c = mpq_cmp(v, m) * s;          /* > 0: oltre il medio */
        if (c > 0) {
            d = d2;
        } else if (c == 0) {
            uint64_t bits;
            memcpy(&bits, &d, sizeof bits);
            if (bits & 1)
                d = d2;                     /* mantissa pari */
        }
    }
    mpq_clears(t, m, NULL);
    return d;
}

/* Resa di un double identica a repr() di Python: cifre più brevi che
 * rileggono lo stesso valore; notazione fissa per esponenti in [-4, 16),
 * scientifica altrimenti. */
static void repr_double(double x, char *out)
{
    char buf[64], cifre[32];
    int prec, esp;
    if (x == 0) {
        strcpy(out, signbit(x) ? "-0.0" : "0.0");
        return;
    }
    for (prec = 0; prec < 17; prec++) {
        snprintf(buf, sizeof buf, "%.*e", prec, x);
        if (strtod(buf, NULL) == x)
            break;
    }
    snprintf(buf, sizeof buf, "%.*e", prec, x);
    /* buf: [-]d.ddde[+-]XX */
    char *p = buf, *o = out;
    if (*p == '-')
        *o++ = *p++;
    int n = 0;
    for (; *p != 'e'; p++)
        if (*p != '.')
            cifre[n++] = *p;
    esp = atoi(p + 1);
    while (n > 1 && cifre[n - 1] == '0')
        n--;
    if (esp >= -4 && esp < 16) {
        if (esp < 0) {
            o += sprintf(o, "0.");
            for (int i = 0; i < -esp - 1; i++)
                *o++ = '0';
            memcpy(o, cifre, (size_t)n);
            o += n;
        } else {
            for (int i = 0; i <= esp; i++)
                *o++ = i < n ? cifre[i] : '0';
            *o++ = '.';
            if (n > esp + 1) {
                memcpy(o, cifre + esp + 1, (size_t)(n - esp - 1));
                o += n - esp - 1;
            } else {
                *o++ = '0';
            }
        }
    } else {
        *o++ = cifre[0];
        if (n > 1) {
            *o++ = '.';
            memcpy(o, cifre + 1, (size_t)(n - 1));
            o += n - 1;
        }
        o += sprintf(o, "e%c%02d", esp < 0 ? '-' : '+', esp < 0 ? -esp : esp);
    }
    *o = '\0';
}

/* ---- scrittore JSON con la resa di json.dump(indent=1) */

typedef struct {
    FILE *f;
    int prof;                 /* profondità corrente */
    int conta[64];            /* elementi già scritti a ogni livello */
    int dopo_chiave;
} jout;

static void jo_rientro(jout *o, int n)
{
    fputc('\n', o->f);
    for (int i = 0; i < n; i++)
        fputc(' ', o->f);
}

/* Separatore e rientro prima di un valore (non dopo una chiave). */
static void jo_pre(jout *o)
{
    if (o->dopo_chiave) {
        o->dopo_chiave = 0;
        return;
    }
    if (o->prof > 0) {
        if (o->conta[o->prof]++)
            fputc(',', o->f);
        jo_rientro(o, o->prof);
    }
}

static void jo_stringa(jout *o, const char *s, size_t len)
{
    jo_pre(o);
    fputc('"', o->f);
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        unsigned cp;
        switch (c) {
        case '"': fputs("\\\"", o->f); continue;
        case '\\': fputs("\\\\", o->f); continue;
        case '\n': fputs("\\n", o->f); continue;
        case '\r': fputs("\\r", o->f); continue;
        case '\t': fputs("\\t", o->f); continue;
        case '\b': fputs("\\b", o->f); continue;
        case '\f': fputs("\\f", o->f); continue;
        default: break;
        }
        if (c >= 0x20 && c <= 0x7e) {
            fputc(c, o->f);
            continue;
        }
        /* come ensure_ascii: \uXXXX, coppie surrogate oltre 0xFFFF */
        if (c < 0x80) {
            cp = c;
        } else {
            int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : 1;
            cp = c & (0x3F >> extra);
            for (int k = 0; k < extra && i + 1 < len; k++)
                cp = (cp << 6) | ((unsigned char)s[++i] & 0x3F);
        }
        if (cp >= 0x10000) {
            cp -= 0x10000;
            fprintf(o->f, "\\u%04x\\u%04x", 0xD800 + (cp >> 10), 0xDC00 + (cp & 0x3FF));
        } else {
            fprintf(o->f, "\\u%04x", cp);
        }
    }
    fputc('"', o->f);
}

static void jo_chiave(jout *o, const char *k)
{
    jo_stringa(o, k, strlen(k));
    fputs(": ", o->f);
    o->dopo_chiave = 1;
}

static void jo_testo(jout *o, const char *t)
{
    jo_pre(o);
    fputs(t, o->f);
}

static void jo_intero(jout *o, long v)
{
    jo_pre(o);
    fprintf(o->f, "%ld", v);
}

static void jo_apri(jout *o, char c)
{
    jo_pre(o);
    fputc(c, o->f);
    o->conta[++o->prof] = 0;
}

static void jo_chiudi(jout *o, char c)
{
    if (o->conta[o->prof])
        jo_rientro(o, o->prof - 1);
    o->prof--;
    fputc(c, o->f);
}

/* Copia di un nodo letto dalla fonte, come lo riscriverebbe Python. */
static void jo_nodo(jout *o, const jnode *n)
{
    char buf[64];
    switch (n->t) {
    case J_NULL: jo_testo(o, "null"); break;
    case J_TRUE: jo_testo(o, "true"); break;
    case J_FALSE: jo_testo(o, "false"); break;
    case J_STR: jo_stringa(o, n->s, n->slen); break;
    case J_NUM:
        if (n->is_int) {
            jo_testo(o, strcmp(n->s, "-0") == 0 ? "0" : n->s);
        } else {
            repr_double(strtod(n->s, NULL), buf);
            jo_testo(o, buf);
        }
        break;
    case J_ARR:
    case J_OBJ:
        jo_apri(o, n->t == J_ARR ? '[' : '{');
        for (size_t i = 0; i < n->n; i++) {
            if (n->t == J_OBJ)
                jo_chiave(o, n->keys[i]);
            jo_nodo(o, n->items[i]);
        }
        jo_chiudi(o, n->t == J_ARR ? ']' : '}');
        break;
    }
}

/* Nodo facoltativo: lista vuota se assente (come .get(chiave, [])). */
static void jo_nodo_o_vuoto(jout *o, const jnode *n)
{
    if (n) {
        jo_nodo(o, n);
    } else {
        jo_apri(o, '[');
        jo_chiudi(o, ']');
    }
}

/* ---- fonti del k18 */

typedef struct {
    char *nome;
    jnode *frac, *triple, *parallele;
} fonte;

/* Nome -> fonte; l'ultima occorrenza vince, come in un dizionario. */
static fonte *trova(fonte *f, int n, const char *nome)
{
    for (int i = n - 1; i >= 0; i--)
        if (strcmp(f[i].nome, nome) == 0)
            return &f[i];
    return NULL;
}

/* Stessa logica di carica_tutti() in esperimenti/sotto17.py del k18,
 * ma conserva le stringhe "p/q" originali per copiarle identiche. */
static fonte *carica_tutti(const char *k18, int *nfonti, jnode **tenere)
{
    char percorso[4096];
    snprintf(percorso, sizeof percorso, "%s/all_solutions.json", k18);
    jnode *dati = carica(percorso);
    tenere[0] = dati;
    jnode *sol = json_get(dati, "solutions");
    if (!sol || sol->t != J_ARR)
        muori("%s: manca \"solutions\"", percorso, 0, 0);
    fonte *f = calloc(sol->n + 20, sizeof *f);
    int n = 0;
    if (!f)
        muori("%smemoria esaurita", "", 0, 0);
    for (size_t i = 0; i < sol->n; i++) {
        jnode *s = sol->items[i], *id = json_get(s, "id");
        if (!id || (id->t != J_NUM && id->t != J_STR))
            muori("%s: soluzione senza id", percorso, 0, 0);
        f[n].nome = malloc(id->slen + 5);
        sprintf(f[n].nome, "all_%s", id->s);
        f[n].frac = json_get(s, "lines_frac");
        f[n].triple = json_get(s, "triple_points");
        f[n].parallele = json_get(s, "parallel_pairs");
        n++;
    }
    for (int k = 1; k <= 20; k++) {
        snprintf(percorso, sizeof percorso, "%s/sol%d/lines_rational.json", k18, k);
        jnode *d = carica(percorso);
        tenere[k] = d;
        f[n].nome = malloc(16);
        sprintf(f[n].nome, "sol%d", k);
        f[n].frac = json_get(d, "lines_frac");
        f[n].triple = json_get(d, "declared_triple_points");
        f[n].parallele = json_get(d, "declared_parallel_pairs");
        n++;
    }
    *nfonti = n;
    return f;
}

static int distinti(fonte *f, int n)
{
    int d = 0;
    for (int i = 0; i < n; i++)
        if (trova(f, i, f[i].nome) == NULL)
            d++;
    return d;
}

/* ---- programma principale */

typedef struct {
    char id[16];
    int classe, rimossa;
    const char *nome;
    fonte *fo;
    char sha[65];
    double fl[GEOM_MAXK][3];
} voce;

static const char *NOTA =
    "Arrangements of 17 straight lines forming 85 Kobon "
    "triangles (Tamura's bound, hence optimal), each obtained "
    "by removing one line from an 18-line, 93-triangle "
    "arrangement of k18_final. Lines are exact rationals "
    "(a, b, c) meaning a*x + b*y = c, copied verbatim from the "
    "source; lines_float is for plotting only. Arrangements "
    "with equal canonical_sha256 are isomorphic (relabeling of "
    "lines and reversal of individual lines).";

static const char *FONTE =
    "k18_final (A. Maiorana, 2026): 2357 arrangements of 18 "
    "lines with 93 triangles";

static void scrivi(const char *uscita, voce *v, int nv, char sha[][65], int ncl)
{
    FILE *f = fopen(uscita, "w");
    if (!f)
        muori("non riesco a scrivere %s", uscita, 0, 0);
    jout o = { f, 0, { 0 }, 0 };
    jo_apri(&o, '{');
    jo_chiave(&o, "k"); jo_intero(&o, 17);
    jo_chiave(&o, "triangles"); jo_intero(&o, CONTEGGIO);
    jo_chiave(&o, "tamura_bound"); jo_intero(&o, 17 * 15 / 3);
    jo_chiave(&o, "note"); jo_stringa(&o, NOTA, strlen(NOTA));
    jo_chiave(&o, "source"); jo_stringa(&o, FONTE, strlen(FONTE));
    jo_chiave(&o, "count"); jo_intero(&o, nv);
    jo_chiave(&o, "class_count"); jo_intero(&o, ncl);
    jo_chiave(&o, "arrangements");
    jo_apri(&o, '[');
    for (int a = 0; a < nv; a++) {
        voce *x = &v[a];
        jnode *frac = x->fo->frac;
        char buf[64];
        jo_apri(&o, '{');
        jo_chiave(&o, "id"); jo_stringa(&o, x->id, strlen(x->id));
        jo_chiave(&o, "class"); jo_intero(&o, x->classe);
        jo_chiave(&o, "source_arrangement"); jo_stringa(&o, x->nome, strlen(x->nome));
        jo_chiave(&o, "removed_line"); jo_intero(&o, x->rimossa);
        jo_chiave(&o, "source_line_indices");
        jo_apri(&o, '[');
        for (int i = 0; i < RETTE; i++)
            if (i != x->rimossa)
                jo_intero(&o, i);
        jo_chiudi(&o, ']');
        jo_chiave(&o, "source_triple_points"); jo_nodo_o_vuoto(&o, x->fo->triple);
        jo_chiave(&o, "source_parallel_pairs"); jo_nodo_o_vuoto(&o, x->fo->parallele);
        jo_chiave(&o, "lines_frac");
        jo_apri(&o, '[');
        for (int i = 0; i < RETTE; i++)
            if (i != x->rimossa)
                jo_nodo(&o, frac->items[i]);
        jo_chiudi(&o, ']');
        jo_chiave(&o, "lines_float");
        jo_apri(&o, '[');
        for (int i = 0; i < RETTE - 1; i++) {
            jo_apri(&o, '[');
            for (int c = 0; c < 3; c++) {
                repr_double(x->fl[i][c], buf);
                jo_testo(&o, buf);
            }
            jo_chiudi(&o, ']');
        }
        jo_chiudi(&o, ']');
        /* nessuna degenerazione residua (controllato prima) */
        jo_chiave(&o, "triple_points"); jo_apri(&o, '['); jo_chiudi(&o, ']');
        jo_chiave(&o, "parallel_pairs"); jo_apri(&o, '['); jo_chiudi(&o, ']');
        jo_chiave(&o, "canonical_sha256"); jo_stringa(&o, x->sha, 64);
        jo_chiudi(&o, '}');
    }
    jo_chiudi(&o, ']');
    jo_chiave(&o, "classes");
    jo_apri(&o, '[');
    for (int c = 1; c <= ncl; c++) {
        int primo = -1;
        jo_apri(&o, '{');
        jo_chiave(&o, "class"); jo_intero(&o, c);
        for (int a = 0; a < nv && primo < 0; a++)
            if (v[a].classe == c)
                primo = a;
        jo_chiave(&o, "representative");
        jo_stringa(&o, v[primo].id, strlen(v[primo].id));
        jo_chiave(&o, "members");
        jo_apri(&o, '[');
        for (int a = 0; a < nv; a++)
            if (v[a].classe == c)
                jo_stringa(&o, v[a].id, strlen(v[a].id));
        jo_chiudi(&o, ']');
        jo_chiave(&o, "canonical_sha256"); jo_stringa(&o, sha[c - 1], 64);
        jo_chiudi(&o, '}');
    }
    jo_chiudi(&o, ']');
    jo_chiudi(&o, '}');
    fputc('\n', f);
    if (fclose(f) != 0)
        muori("errore di scrittura su %s", uscita, 0, 0);
}

int main(int argc, char **argv)
{
    const char *k18 = K18, *uscita = NULL;
    char predefinita[4096];
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--k18") == 0 && i + 1 < argc)
            k18 = argv[++i];
        else if (strncmp(argv[i], "--k18=", 6) == 0)
            k18 = argv[i] + 6;
        else if (strcmp(argv[i], "--uscita") == 0 && i + 1 < argc)
            uscita = argv[++i];
        else if (strncmp(argv[i], "--uscita=", 9) == 0)
            uscita = argv[i] + 9;
        else
            muori("uso: %s [--k18 PERCORSO] [--uscita FILE]", argv[0], 0, 0);
    }
    if (!uscita) {
        /* radice del pacchetto: la cartella che contiene build/ */
        const char *barra = strrchr(argv[0], '/');
        if (barra)
            snprintf(predefinita, sizeof predefinita, "%.*s/../k17_solutions.json",
                     (int)(barra - argv[0]), argv[0]);
        else
            snprintf(predefinita, sizeof predefinita, "../k17_solutions.json");
        uscita = predefinita;
    }

    char percorso[4096];
    snprintf(percorso, sizeof percorso, "%s/esperimenti/risultati_sotto17.json", k18);
    jnode *ris = carica(percorso);
    jnode *tenere[21];
    int nfonti;
    fonte *fonti = carica_tutti(k18, &nfonti, tenere);
    jnode *arrs = json_get(ris, "arrangiamenti");
    if (!arrs || arrs->t != J_ARR || distinti(fonti, nfonti) != (int)arrs->n ||
        arrs->n != FONTI)
        muori("%sfonti inattese", "", 0, 0);

    voce *voci = calloc(ATTESI, sizeof *voci);
    int nv = 0;
    for (size_t a = 0; a < arrs->n; a++) {
        jnode *nome = json_get(arrs->items[a], "nome");
        jnode *s17 = json_get(arrs->items[a], "sotto17");
        if (!nome || nome->t != J_STR || !s17 || s17->t != J_ARR)
            muori("%svoce malformata in risultati_sotto17.json", "", 0, 0);
        for (size_t r = 0; r < s17->n; r++) {
            long c;
            if (json_long(s17->items[r], &c) && c == CONTEGGIO) {
                if (nv < ATTESI) {
                    voci[nv].nome = nome->s;
                    voci[nv].rimossa = (int)r;
                }
                nv++;
            }
        }
    }
    if (nv != ATTESI)
        muori("%strovate %d voci, attese %d", "", nv, ATTESI);

    char sha[ATTESI][65];
    int ncl = 0;
    static events_t ev, forma;
    for (int n = 0; n < nv; n++) {
        voce *x = &voci[n];
        snprintf(x->id, sizeof x->id, "k17_%02d", n + 1);
        x->fo = trova(fonti, nfonti, x->nome);
        if (!x->fo)
            muori("fonte assente: %s", x->nome, 0, 0);
        jnode *frac = x->fo->frac;
        if (!frac || frac->t != J_ARR || frac->n != RETTE)
            muori("%s: numero di rette diverso da %d", x->nome, RETTE, 0);
        zline L[GEOM_MAXK];
        mpq_t q[3];
        int k = 0;
        mpq_inits(q[0], q[1], q[2], NULL);
        for (int i = 0; i < RETTE; i++) {
            if (i == x->rimossa)
                continue;
            jnode *t = frac->items[i];
            if (t->t != J_ARR || t->n != 3)
                muori("%s: retta %d malformata", x->nome, i, 0);
            for (int c = 0; c < 3; c++) {
                if (t->items[c]->t != J_STR || geom_parse_rat(q[c], t->items[c]->s))
                    muori("%s: retta %d malformata", x->nome, i, 0);
                x->fl[k][c] = q_a_double(q[c]);
            }
            zline_init(&L[k]);
            if (zline_from_q(&L[k], q[0], q[1], q[2]))
                muori("%s: retta %d degenere", x->nome, i, 0);
            k++;
        }
        mpq_clears(q[0], q[1], q[2], NULL);
        tuple_list par, multi;
        geom_events(L, k, &ev, &par, &multi);
        for (int i = 0; i < k; i++)
            zline_clear(&L[i]);
        int conteggio = geom_count_by_adjacency(&ev);
        if (conteggio != CONTEGGIO)
            muori("%s senza %d: %d", x->nome, x->rimossa, conteggio);
        if (par.n || multi.n)
            muori("%s senza %d: degenerazioni inattese (%d)", x->nome,
                  x->rimossa, par.n + multi.n);
        tuple_list_free(&par);
        tuple_list_free(&multi);
        canonical_form(&ev, &forma);
        canonical_sha256(&forma, x->sha);
        int c = 0;
        while (c < ncl && strcmp(sha[c], x->sha) != 0)
            c++;
        if (c == ncl)
            memcpy(sha[ncl++], x->sha, 65);
        x->classe = c + 1;
    }

    scrivi(uscita, voci, nv, sha, ncl);
    const char *base = strrchr(uscita, '/');
    printf("scritto %s: %d arrangiamenti, %d classi\n", base ? base + 1 : uscita,
           nv, ncl);

    for (int i = 0; i < nfonti; i++)
        free(fonti[i].nome);
    free(fonti);
    free(voci);
    for (int i = 0; i < 21; i++)
        json_free(tenere[i]);
    json_free(ris);
    return 0;
}
