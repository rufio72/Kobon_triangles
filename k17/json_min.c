/* Minimal JSON parser (DOM) for the fixed data formats of this package.
 *
 * Recursive descent over a single buffer, linear in the input size.
 * Strings are decoded to UTF-8 (including \u escapes and surrogate
 * pairs); numbers keep their raw text, plus a long value for integers.
 */
#include "json_min.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *p, *end;
    char *err;
    size_t errlen;
    int depth;
} parser;

static void set_err(parser *ps, const char *msg)
{
    if (ps->err && ps->err[0] == '\0')
        snprintf(ps->err, ps->errlen, "%s", msg);
}

static void skip_ws(parser *ps)
{
    while (ps->p < ps->end && (*ps->p == ' ' || *ps->p == '\t' ||
                               *ps->p == '\n' || *ps->p == '\r'))
        ps->p++;
}

static jnode *new_node(jtype t)
{
    jnode *n = calloc(1, sizeof *n);
    if (n)
        n->t = t;
    return n;
}

static int hex4(parser *ps, unsigned *out)
{
    unsigned v = 0;
    if (ps->end - ps->p < 4)
        return -1;
    for (int i = 0; i < 4; i++) {
        char c = *ps->p++;
        v <<= 4;
        if (c >= '0' && c <= '9') v |= (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') v |= (unsigned)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') v |= (unsigned)(c - 'A' + 10);
        else return -1;
    }
    *out = v;
    return 0;
}

static size_t put_utf8(char *o, unsigned cp)
{
    if (cp < 0x80) { o[0] = (char)cp; return 1; }
    if (cp < 0x800) {
        o[0] = (char)(0xC0 | (cp >> 6));
        o[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        o[0] = (char)(0xE0 | (cp >> 12));
        o[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        o[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    o[0] = (char)(0xF0 | (cp >> 18));
    o[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
    o[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
    o[3] = (char)(0x80 | (cp & 0x3F));
    return 4;
}

/* Parse a string literal (ps->p on the opening quote). */
static char *parse_string(parser *ps, size_t *outlen)
{
    const char *q = ++ps->p;
    while (q < ps->end && *q != '"') {
        if (*q == '\\')
            q++;
        q++;
    }
    if (q >= ps->end) {
        set_err(ps, "unterminated string");
        return NULL;
    }
    /* Decoded text is never longer than the escaped text. */
    char *out = malloc((size_t)(q - ps->p) + 1);
    if (!out) {
        set_err(ps, "out of memory");
        return NULL;
    }
    size_t n = 0;
    while (*ps->p != '"') {
        unsigned char c = (unsigned char)*ps->p++;
        if (c < 0x20) {
            set_err(ps, "control character in string");
            free(out);
            return NULL;
        }
        if (c != '\\') {
            out[n++] = (char)c;
            continue;
        }
        char e = *ps->p++;
        switch (e) {
        case '"': out[n++] = '"'; break;
        case '\\': out[n++] = '\\'; break;
        case '/': out[n++] = '/'; break;
        case 'b': out[n++] = '\b'; break;
        case 'f': out[n++] = '\f'; break;
        case 'n': out[n++] = '\n'; break;
        case 'r': out[n++] = '\r'; break;
        case 't': out[n++] = '\t'; break;
        case 'u': {
            unsigned cp, lo;
            if (hex4(ps, &cp)) goto bad;
            if (cp >= 0xD800 && cp < 0xDC00 && ps->end - ps->p >= 6 &&
                ps->p[0] == '\\' && ps->p[1] == 'u') {
                const char *save = ps->p;
                ps->p += 2;
                if (hex4(ps, &lo)) goto bad;
                if (lo >= 0xDC00 && lo < 0xE000)
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                else
                    ps->p = save;
            }
            n += put_utf8(out + n, cp);
            break;
        }
        default:
            goto bad;
        }
    }
    ps->p++;                       /* closing quote */
    out[n] = '\0';
    *outlen = n;
    return out;
bad:
    set_err(ps, "bad escape in string");
    free(out);
    return NULL;
}

static jnode *parse_value(parser *ps);

static int push_child(jnode *n, jnode *child, char *key, size_t *cap)
{
    if (n->n == *cap) {
        size_t nc = *cap ? 2 * *cap : 4;
        jnode **it = realloc(n->items, nc * sizeof *it);
        if (!it)
            return -1;
        n->items = it;
        if (n->t == J_OBJ) {
            char **ks = realloc(n->keys, nc * sizeof *ks);
            if (!ks)
                return -1;
            n->keys = ks;
        }
        *cap = nc;
    }
    n->items[n->n] = child;
    if (n->t == J_OBJ)
        n->keys[n->n] = key;
    n->n++;
    return 0;
}

static jnode *parse_container(parser *ps, jtype t)
{
    char close = t == J_ARR ? ']' : '}';
    jnode *n = new_node(t);
    size_t cap = 0;
    if (!n) {
        set_err(ps, "out of memory");
        return NULL;
    }
    if (++ps->depth > 200) {
        set_err(ps, "nesting too deep");
        goto fail;
    }
    ps->p++;
    skip_ws(ps);
    if (ps->p < ps->end && *ps->p == close) {
        ps->p++;
        ps->depth--;
        return n;
    }
    for (;;) {
        char *key = NULL;
        skip_ws(ps);
        if (t == J_OBJ) {
            size_t kl;
            if (ps->p >= ps->end || *ps->p != '"') {
                set_err(ps, "expected key");
                goto fail;
            }
            key = parse_string(ps, &kl);
            if (!key)
                goto fail;
            skip_ws(ps);
            if (ps->p >= ps->end || *ps->p != ':') {
                set_err(ps, "expected ':'");
                free(key);
                goto fail;
            }
            ps->p++;
        }
        jnode *child = parse_value(ps);
        if (!child || push_child(n, child, key, &cap)) {
            if (child)
                json_free(child);
            free(key);
            set_err(ps, "out of memory");
            goto fail;
        }
        skip_ws(ps);
        if (ps->p < ps->end && *ps->p == ',') {
            ps->p++;
            continue;
        }
        if (ps->p < ps->end && *ps->p == close) {
            ps->p++;
            break;
        }
        set_err(ps, t == J_ARR ? "expected ',' or ']'" : "expected ',' or '}'");
        goto fail;
    }
    ps->depth--;
    return n;
fail:
    json_free(n);
    return NULL;
}

static jnode *parse_number(parser *ps)
{
    const char *s = ps->p;
    int is_int = 1;
    if (ps->p < ps->end && *ps->p == '-')
        ps->p++;
    if (ps->p >= ps->end || *ps->p < '0' || *ps->p > '9') {
        set_err(ps, "bad number");
        return NULL;
    }
    while (ps->p < ps->end) {
        char c = *ps->p;
        if (c == '.' || c == 'e' || c == 'E')
            is_int = 0;
        else if (!((c >= '0' && c <= '9') || c == '+' || c == '-'))
            break;
        ps->p++;
    }
    jnode *n = new_node(J_NUM);
    if (!n || !(n->s = malloc((size_t)(ps->p - s) + 1))) {
        free(n);
        set_err(ps, "out of memory");
        return NULL;
    }
    n->slen = (size_t)(ps->p - s);
    memcpy(n->s, s, n->slen);
    n->s[n->slen] = '\0';
    n->is_int = is_int;
    if (is_int) {
        char *e;
        errno = 0;
        n->ival = strtol(n->s, &e, 10);
        n->fits = errno == 0 && *e == '\0';
    }
    return n;
}

static int match(parser *ps, const char *word)
{
    size_t l = strlen(word);
    if ((size_t)(ps->end - ps->p) >= l && memcmp(ps->p, word, l) == 0) {
        ps->p += l;
        return 1;
    }
    return 0;
}

static jnode *parse_value(parser *ps)
{
    skip_ws(ps);
    if (ps->p >= ps->end) {
        set_err(ps, "unexpected end of input");
        return NULL;
    }
    switch (*ps->p) {
    case '{': return parse_container(ps, J_OBJ);
    case '[': return parse_container(ps, J_ARR);
    case '"': {
        size_t l;
        char *s = parse_string(ps, &l);
        if (!s)
            return NULL;
        jnode *n = new_node(J_STR);
        if (!n) {
            free(s);
            set_err(ps, "out of memory");
            return NULL;
        }
        n->s = s;
        n->slen = l;
        return n;
    }
    case 't': if (match(ps, "true")) return new_node(J_TRUE); break;
    case 'f': if (match(ps, "false")) return new_node(J_FALSE); break;
    case 'n': if (match(ps, "null")) return new_node(J_NULL); break;
    default: return parse_number(ps);
    }
    set_err(ps, "unexpected token");
    return NULL;
}

jnode *json_parse(const char *buf, size_t len, char *err, size_t errlen)
{
    parser ps = { buf, buf + len, err, errlen, 0 };
    if (err && errlen)
        err[0] = '\0';
    jnode *n = parse_value(&ps);
    if (!n)
        return NULL;
    skip_ws(&ps);
    if (ps.p != ps.end) {
        set_err(&ps, "trailing data after JSON value");
        json_free(n);
        return NULL;
    }
    return n;
}

jnode *json_load_file(const char *path, char *err, size_t errlen)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(err, errlen, "cannot open %s: %s", path, strerror(errno));
        return NULL;
    }
    size_t cap = 1 << 16, len = 0;
    char *buf = malloc(cap);
    while (buf) {
        if (len == cap) {
            char *nb = realloc(buf, cap * 2);
            if (!nb) {
                free(buf);
                buf = NULL;
                break;
            }
            buf = nb;
            cap *= 2;
        }
        size_t r = fread(buf + len, 1, cap - len, f);
        if (r == 0)
            break;
        len += r;
    }
    int bad = ferror(f);
    fclose(f);
    if (!buf || bad) {
        free(buf);
        snprintf(err, errlen, "cannot read %s", path);
        return NULL;
    }
    jnode *n = json_parse(buf, len, err, errlen);
    free(buf);
    if (!n) {
        char tmp[256];
        snprintf(tmp, sizeof tmp, "%s", err);
        snprintf(err, errlen, "%s: %s", path, tmp);
    }
    return n;
}

void json_free(jnode *n)
{
    if (!n)
        return;
    for (size_t i = 0; i < n->n; i++) {
        json_free(n->items[i]);
        if (n->keys)
            free(n->keys[i]);
    }
    free(n->items);
    free(n->keys);
    free(n->s);
    free(n);
}

jnode *json_get(const jnode *obj, const char *key)
{
    if (!obj || obj->t != J_OBJ)
        return NULL;
    for (size_t i = obj->n; i-- > 0;)
        if (strcmp(obj->keys[i], key) == 0)
            return obj->items[i];
    return NULL;
}

jnode *json_at(const jnode *arr, size_t i)
{
    if (!arr || arr->t != J_ARR || i >= arr->n)
        return NULL;
    return arr->items[i];
}

int json_long(const jnode *n, long *v)
{
    if (!n || n->t != J_NUM || !n->is_int || !n->fits)
        return 0;
    *v = n->ival;
    return 1;
}
