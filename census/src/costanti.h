/* Costanti del censimento: limiti e massimi noti di triangoli di Kobon. */
#ifndef COSTANTI_H
#define COSTANTI_H

/* Numero massimo di rette gestito (fase 2: n = 11 e 12). */
#define N_MAX 12

/* Taglia massima ammessa di un'orbita sotto il gruppo di isomorfismo. */
#define ORBITA_MAX (8 * N_MAX)

/* Massimi noti di triangoli (OEIS A006066), indice = numero di rette. */
static const int MAXT[N_MAX + 1] = {0, 0, 0, 1, 2, 5, 7, 11, 15, 21, 25, 32, 38};

/* Segmenti finiti che non sono lato di un triangolo vuoto, dati n rette e
 * t triangoli: ogni retta ha n-2 segmenti finiti, ogni triangolo ne usa 3
 * e un segmento non è mai lato di due triangoli. Negativo = impossibile. */
static inline int segmenti_mancanti(int n, int t)
{
    return n * (n - 2) - 3 * t;
}

#endif
