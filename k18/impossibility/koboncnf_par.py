# Kobon triangle problem - CNF model generator, PARALLEL-CLASSES fork.
# Base model: Pavlo Savchuk 2025 (kobon-cnf). Fork 2026-08-15:
# supporto a CLASSI DI RETTE PARALLELE (parallel_classes, 0-based).
#
# Differenze dal modello base (koboncnf_ext.py, che resta intatto):
# - ogni retta r ha dominio AV[r] = tutte le altre tranne i partner
#   paralleli; la riga ha L[r] = N-1-|par(r)| colonne;
# - tutte le quantificazioni (At/G/X/prima/ultima/transitivita'/
#   triangoli) girano sui domini per-riga;
# - triple con una coppia parallela dentro: escluse dagli assiomi di
#   consistenza e dai vincoli-triangolo; in piu' un segmento delimitato
#   da due parallele adiacenti su una trasversale NON puo' mai essere
#   lato di un triangolo => clausola G(r,i,j) -> Mk(r,i,j) (o vietato
#   se non ci sono slot di mancanti);
# - niente opzioni mirrored/rotational (assert).
#
# NOTA DI PERIMETRO: la rimozione degli assiomi sulle triple parallele
# e' conservativa per gli UNSAT (il modello puo' solo SOVRA-generare:
# se dice impossibile, e' impossibile). Un eventuale SAT va riverificato
# a valle (conteggio indipendente + tentativo di raddrizzamento).

from collections import Counter

cnf_header = """c ==================================================================
c Kobon CNF, parallel-classes fork. N={N} classes={P} missing={T}
c =================================================================="""

hr = "\nc\nc ------------------------------------------------------------------"


def domains(N, parallel_classes):
    par = [set() for _ in range(N)]
    for cl in parallel_classes:
        for a in cl:
            for b in cl:
                if a != b:
                    par[a].add(b)
    AV = [[i for i in range(N) if i != r and i not in par[r]]
          for r in range(N)]
    L = [len(AV[r]) for r in range(N)]
    return par, AV, L


def var_ids(N, parallel_classes, missing_counts):
    """Numerazione IDENTICA a quella di generate(): per ogni (r, i in
    AV[r]): coppie G,X su j in AV[r]\\{i}, poi A su k in range(L[r]).
    Poi le M dei mancanti. Ritorna (G, X, A, M, n_vars)."""
    par, AV, L = domains(N, parallel_classes)
    G, X, A, M = {}, {}, {}, {}
    u = 0
    for r in range(N):
        for i in AV[r]:
            for j in AV[r]:
                if j == i:
                    continue
                u += 1; G[(r, i, j)] = u
                u += 1; X[(r, i, j)] = u
            for k in range(L[r]):
                u += 1; A[(r, i, k)] = u
    for r, c in missing_counts:
        for k in range(c):
            for i in AV[r]:
                for j in AV[r]:
                    if i == j:
                        continue
                    u += 1; M[(k, r, i, j)] = u
    return G, X, A, M, u


def generate(line_count, cnf_filename, parallel_classes=(),
             missing_triangles=(), generate_table=False,
             force_missing_use=False):
    N = line_count
    MT = list(Counter([(r - 1) for r in missing_triangles]).items())
    par, AV, L = domains(N, parallel_classes)
    G, X, A, M, unknowns = var_ids(N, parallel_classes, MT)
    result = {"status": "OK"}

    # ---------------------- Generate Table ---------------------------
    if generate_table:
        vv = {}
        with open(cnf_filename) as file:
            for l in file:
                if l.startswith("s"):
                    if "UNSATISFIABLE" in l:
                        result["status"] = "UNSATISFIABLE"
                        result["table"] = []
                        return result
                    elif "SATISFIABLE" not in l:
                        result["status"] = "ERROR"
                        result["msg"] = "Expecting `s SATISFIABLE`."
                        return result
                if not l.startswith("v"):
                    continue
                for t in l.split(" "):
                    if t in ("v", "", "\n"):
                        continue
                    v = int(t)
                    vv[abs(v)] = v > 0
        tab = []
        for r in range(N):
            row = []
            first = r
            for i in AV[r]:
                if vv[A[(r, i, 0)]]:
                    first = i
                    break
            if first == r:
                result["status"] = "ERROR"
                result["msg"] = "Invalid boolean values."
                return result
            row.append(first + 1)
            while first != r:
                j = first
                first = r
                for i in AV[r]:
                    if i == j:
                        continue
                    if vv[G[(r, j, i)]]:
                        first = i
                        row.append(i + 1)
                        break
            tab.append(row)
        result["table"] = tab
        return result

    # ------------------------ Generate CNF ---------------------------
    clauses = 0
    with open(cnf_filename, "w") as f:

        f.write(hr)
        f.write("\nc Each row has all its crossing lines:")
        for r in range(N):
            for i in AV[r]:
                f.write("\n")
                for k in range(L[r]):
                    f.write("{0} ".format(A[(r, i, k)]))
                f.write("0")
                clauses += 1

        f.write(hr)
        f.write("\nc Rows don't have duplicate entries:")
        for r in range(N):
            for i in AV[r]:
                for k in range(L[r]):
                    for j in range(L[r]):
                        if j == k:
                            continue
                        f.write("\n-{0} -{1} 0".format(A[(r, i, k)],
                                                       A[(r, i, j)]))
                        clauses += 1

        f.write(hr)
        f.write("\nc Connection between A and G:")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    if j == i:
                        continue
                    for k in range(L[r] - 1):
                        f.write("\n-{0} -{1} {2} 0".format(
                            A[(r, i, k)], A[(r, j, k + 1)], G[(r, i, j)]))
                        f.write("\n-{0} -{1} {2} 0".format(
                            A[(r, i, k)], G[(r, i, j)], A[(r, j, k + 1)]))
                        f.write("\n-{0} -{1} {2} 0".format(
                            G[(r, i, j)], A[(r, j, k + 1)], A[(r, i, k)]))
                        clauses += 3

        f.write(hr)
        f.write("\nc Only one line can be immediately after another:")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    if j == i:
                        continue
                    for k in AV[r]:
                        if k == i or k == j:
                            continue
                        f.write("\n-{0} -{1} 0".format(G[(r, i, j)],
                                                       G[(r, i, k)]))
                        clauses += 1

        f.write(hr)
        f.write("\nc Relation between `first` and `immediately after`:")
        for r in range(N):
            for i in AV[r]:
                line = ""
                for j in AV[r]:
                    if j == i:
                        continue
                    f.write("\n-{0} -{1} 0".format(A[(r, i, 0)],
                                                   G[(r, j, i)]))
                    clauses += 1
                    line += "{0} ".format(G[(r, j, i)])
                if line:
                    f.write("\n{0} {1}0".format(A[(r, i, 0)], line))
                    clauses += 1

        f.write(hr)
        f.write("\nc Relation between `last` and `immediately after`:")
        for r in range(N):
            for i in AV[r]:
                line = ""
                for j in AV[r]:
                    if j == i:
                        continue
                    f.write("\n-{0} -{1} 0".format(A[(r, i, L[r] - 1)],
                                                   G[(r, i, j)]))
                    clauses += 1
                    line += "{0} ".format(G[(r, i, j)])
                if line:
                    f.write("\n{0} {1}0".format(A[(r, i, L[r] - 1)], line))
                    clauses += 1

        f.write(hr)
        f.write("\nc Missing slots: exactly one segment per used slot,")
        f.write("\nc and only existing (adjacent) segments:")
        for r, c in MT:
            for k in range(c):
                l = ""
                for i in AV[r]:
                    for j in AV[r]:
                        if i == j:
                            continue
                        f.write("\n-{0} {1} 0".format(M[(k, r, i, j)],
                                                      G[(r, i, j)]))
                        clauses += 1
                        l += "{0} ".format(M[(k, r, i, j)])
                        for i2 in AV[r]:
                            for j2 in AV[r]:
                                if i2 == j2 or (i2 == i and j2 == j):
                                    continue
                                f.write("\n-{0} -{1} 0".format(
                                    M[(k, r, i, j)], M[(k, r, i2, j2)]))
                                clauses += 1
                if l and force_missing_use:
                    f.write("\n" + l + "0")
                    clauses += 1

        f.write(hr)
        f.write("\nc X total:")
        for r in range(N):
            for ii in range(len(AV[r])):
                for jj in range(ii + 1, len(AV[r])):
                    i, j = AV[r][ii], AV[r][jj]
                    f.write("\n{0} {1} 0".format(X[(r, i, j)], X[(r, j, i)]))
                    f.write("\n-{0} -{1} 0".format(X[(r, i, j)],
                                                   X[(r, j, i)]))
                    clauses += 2

        f.write(hr)
        f.write("\nc X transitive:")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    for k in AV[r]:
                        if i == j or i == k or j == k:
                            continue
                        f.write("\n-{0} -{1} {2} 0".format(
                            X[(r, i, j)], X[(r, j, k)], X[(r, i, k)]))
                        clauses += 1

        f.write(hr)
        f.write("\nc X vs first/last:")
        for r in range(N):
            for i in AV[r]:
                l_first = ""
                l_last = ""
                for j in AV[r]:
                    if i == j:
                        continue
                    f.write("\n-{0} {1} 0".format(A[(r, i, 0)],
                                                  X[(r, i, j)]))
                    f.write("\n-{0} {1} 0".format(A[(r, i, L[r] - 1)],
                                                  X[(r, j, i)]))
                    clauses += 2
                    l_first += "-{0} ".format(X[(r, i, j)])
                    l_last += "-{0} ".format(X[(r, j, i)])
                if l_first:
                    f.write("\n" + l_first + "{0} 0".format(A[(r, i, 0)]))
                    f.write("\n" + l_last + "{0} 0".format(
                        A[(r, i, L[r] - 1)]))
                    clauses += 2

        f.write(hr)
        f.write("\nc G implies X:")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    if i == j:
                        continue
                    f.write("\n-{0} {1} 0".format(G[(r, i, j)],
                                                  X[(r, i, j)]))
                    clauses += 1

        # ------------------ Triangles (6-case tables) -----------------
        f.write(hr)
        f.write("\nc Every finite segment bounded by CROSSING lines is")
        f.write("\nc an edge of a triangle unless missing; segments")
        f.write("\nc bounded by a PARALLEL pair can never close: those")
        f.write("\nc are forced missing (or forbidden with no slots).")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    if i == j:
                        continue
                    m_ij = ""
                    m_ji = ""
                    if (r + 1) in missing_triangles:
                        c = dict(MT)[r]
                        for k in range(c):
                            m_ij += "{0} ".format(M[(k, r, i, j)])
                            m_ji += "{0} ".format(M[(k, r, j, i)])
                    if j in par[i]:
                        # coppia parallela adiacente su r: mai triangolo
                        if i < j:
                            f.write("\n{1}-{0} 0".format(G[(r, i, j)], m_ij))
                            f.write("\n{1}-{0} 0".format(G[(r, j, i)], m_ji))
                            clauses += 2
                        continue
                    clauses += 6
                    if (r < i) and (i < j):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, r, j)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, r, i)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(i, r, j)], G[(j, r, i)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, j, r)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, i, r)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(i, j, r)], G[(j, i, r)], m_ji))
                    elif (r < j) and (j < i):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, r, i)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, r, j)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(j, r, i)], G[(i, r, j)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, i, r)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, j, r)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(j, i, r)], G[(i, j, r)], m_ij))
                    elif (i < r) and (r < j):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, r, j)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, i, r)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(i, r, j)], G[(j, i, r)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, j, r)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, r, i)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(i, j, r)], G[(j, r, i)], m_ji))
                    elif (j < r) and (r < i):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, r, i)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, j, r)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(j, r, i)], G[(i, j, r)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, i, r)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, r, j)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(j, i, r)], G[(i, r, j)], m_ij))
                    elif (i < j) and (j < r):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, j, r)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, i, r)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(i, j, r)], G[(j, i, r)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, r, j)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, r, i)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(i, r, j)], G[(j, r, i)], m_ji))
                    elif (j < i) and (i < r):
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(j, i, r)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, j, i)], G[(i, j, r)], m_ji))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, j, i)], G[(j, i, r)], G[(i, j, r)], m_ji))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(j, r, i)], m_ij))
                        f.write("\n{2}-{0} {1} 0".format(G[(r, i, j)], G[(i, r, j)], m_ij))
                        f.write("\n{0} -{1} -{2} {3}0".format(G[(r, i, j)], G[(j, r, i)], G[(i, r, j)], m_ij))

        f.write(hr)
        f.write("\nc X-consistency on crossing triples (parallel pairs")
        f.write("\nc inside a triple: no constraint):")
        for r in range(N):
            for i in AV[r]:
                for j in AV[r]:
                    if i == j or j in par[i]:
                        continue
                    clauses += 6
                    if (r < i) and (i < j):
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, r, j)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, r, i)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(i, r, j)], X[(j, r, i)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, j, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, i, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(i, j, r)], X[(j, i, r)]))
                    elif (r < j) and (j < i):
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, r, i)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, r, j)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(j, r, i)], X[(i, r, j)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, i, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, j, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(j, i, r)], X[(i, j, r)]))
                    elif (i < r) and (r < j):
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, r, j)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, i, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(i, r, j)], X[(j, i, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, j, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, r, i)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(i, j, r)], X[(j, r, i)]))
                    elif (j < r) and (r < i):
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, r, i)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, j, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(j, r, i)], X[(i, j, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, i, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, r, j)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(j, i, r)], X[(i, r, j)]))
                    elif (i < j) and (j < r):
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, j, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, i, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(i, j, r)], X[(j, i, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, r, j)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, r, i)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(i, r, j)], X[(j, r, i)]))
                    elif (j < i) and (i < r):
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(j, i, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, j, i)], X[(i, j, r)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, j, i)], X[(j, i, r)], X[(i, j, r)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(j, r, i)]))
                        f.write("\n-{0} {1} 0".format(X[(r, i, j)], X[(i, r, j)]))
                        f.write("\n{0} -{1} -{2} 0".format(X[(r, i, j)], X[(j, r, i)], X[(i, r, j)]))

    # header con p cnf in testa
    import shutil, tempfile
    with tempfile.NamedTemporaryFile("w", delete=False) as temp:
        temp_name = temp.name
        temp.write(cnf_header.format(N=N, P=list(parallel_classes), T=MT))
        temp.write("\np cnf {0} {1}".format(unknowns, clauses))
        with open(cnf_filename) as orig:
            shutil.copyfileobj(orig, temp)
    shutil.move(temp_name, cnf_filename)
    return result
