/* Esecuzione di kissat e lettura delle righe "s" e "v". */
#include "sat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int sat_risolvi(const char *kissat, const char *cnf_path, const char *out_path,
                int nvar, signed char *val)
{
    char cmd[4096];
    snprintf(cmd, sizeof cmd, "'%s' -q '%s' > '%s' 2>&1", kissat, cnf_path,
             out_path);
    int st = system(cmd);
    if (st == -1 || !WIFEXITED(st)) {
        fprintf(stderr, "errore: impossibile eseguire kissat\n");
        return -1;
    }
    int codice = WEXITSTATUS(st);
    if (codice != 10 && codice != 20) {
        fprintf(stderr, "errore: kissat è uscito con codice %d (vedi %s)\n",
                codice, out_path);
        return -1;
    }
    FILE *f = fopen(out_path, "r");
    if (!f) {
        fprintf(stderr, "errore: non riesco a leggere %s\n", out_path);
        return -1;
    }
    int stato = 0;
    char riga[1 << 16];
    memset(val, 0, (size_t)nvar + 1);
    while (fgets(riga, sizeof riga, f)) {
        if (riga[0] == 's') {
            if (strstr(riga, "UNSATISFIABLE"))
                stato = 20;
            else if (strstr(riga, "SATISFIABLE"))
                stato = 10;
        } else if (riga[0] == 'v') {
            char *p = riga + 1, *fine;
            for (;;) {
                long v = strtol(p, &fine, 10);
                if (fine == p || v == 0)
                    break;
                long a = v < 0 ? -v : v;
                if (a <= nvar)
                    val[a] = v > 0 ? 1 : -1;
                p = fine;
            }
        }
    }
    fclose(f);
    if (stato != codice) {
        fprintf(stderr, "errore: uscita di kissat incoerente (%d/%d)\n",
                codice, stato);
        return -1;
    }
    return stato;
}
