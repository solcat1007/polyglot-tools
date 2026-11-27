/* SOLCAT POLYGLOT TOOLS - C edition */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

/* ---------------- Calc: recursive descent parser ---------------- */
typedef struct { const char *s; const char *p; } Parser;

static void skip(Parser *ps) { while (*ps->p && (*ps->p == ' ' || *ps->p == '\t')) ps->p++; }

static double factor(Parser *ps);
static double term(Parser *ps);
static double expr(Parser *ps);

static double factor(Parser *ps) {
    skip(ps);
    if (!*ps->p) { fprintf(stderr, "error: unexpected end of expression\n"); exit(1); }
    char ch = *ps->p;
    if (ch == '+') { ps->p++; return factor(ps); }
    if (ch == '-') { ps->p++; return -factor(ps); }
    if (ch == '(') {
        ps->p++;
        double v = expr(ps);
        skip(ps);
        if (*ps->p != ')') { fprintf(stderr, "error: missing ')'\n"); exit(1); }
        ps->p++;
        return v;
    }
    char *end;
    double v = strtod(ps->p, &end);
    if (end == ps->p) { fprintf(stderr, "error: invalid number\n"); exit(1); }
    ps->p = end;
    return v;
}

static double term(Parser *ps) {
    double v = factor(ps);
    for (;;) {
        skip(ps);
        char op = *ps->p;
        if (op != '*' && op != '/') break;
        ps->p++;
        double r = factor(ps);
        if (op == '*') v *= r;
        else {
            if (r == 0) { fprintf(stderr, "error: division by zero\n"); exit(1); }
            v /= r;
        }
    }
    return v;
}

static double expr(Parser *ps) {
    double v = term(ps);
    for (;;) {
        skip(ps);
        char op = *ps->p;
        if (op != '+' && op != '-') break;
        ps->p++;
        double r = term(ps);
        if (op == '+') v += r; else v -= r;
    }
    return v;
}

static double calc(const char *s) {
    Parser ps = { s, s };
    double v = expr(&ps);
    skip(&ps);
    if (*ps.p) { fprintf(stderr, "error: trailing input\n"); exit(1); }
    return v;
}

/* ---------------- Convert ---------------- */
typedef struct { const char *name; double factor; } Unit;
static const Unit LENGTH[] = {
    {"m",1},{"meter",1},{"cm",0.01},{"mm",0.001},{"km",1000},
    {"in",0.0254},{"ft",0.3048},{"yd",0.9144},{"mi",1609.344}
};
static const Unit WEIGHT[] = {
    {"kg",1},{"g",0.001},{"mg",1e-6},{"t",1000},
    {"lb",0.45359237},{"oz",0.028349523125}
};
static double lookup(const Unit *u, size_t n, const char *name, int *found) {
    for (size_t i = 0; i < n; i++) if (strcmp(u[i].name, name) == 0) { *found = 1; return u[i].factor; }
    *found = 0; return 0;
}
static int is_temp(const char *u) {
    return strcmp(u,"c")==0 || strcmp(u,"f")==0 || strcmp(u,"k")==0;
}
static double to_c(double v, const char *u) {
    if (!strcmp(u,"c")) return v;
    if (!strcmp(u,"f")) return (v-32)*5.0/9.0;
    if (!strcmp(u,"k")) return v-273.15;
    fprintf(stderr,"error: bad temperature unit\n"); exit(1);
}
static double from_c(double v, const char *u) {
    if (!strcmp(u,"c")) return v;
    if (!strcmp(u,"f")) return v*9.0/5.0+32;
    if (!strcmp(u,"k")) return v+273.15;
    fprintf(stderr,"error: bad temperature unit\n"); exit(1);
}
static double convert(double v, const char *from, const char *to) {
    char a[32], b[32];
    for (int i=0; from[i]; i++) a[i] = tolower((unsigned char)from[i]); a[strlen(from)]=0;
    for (int i=0; to[i]; i++) b[i] = tolower((unsigned char)to[i]); b[strlen(to)]=0;
    if (is_temp(a) && is_temp(b)) return from_c(to_c(v,a),b);
    int f1,f2;
    double x = lookup(LENGTH, sizeof(LENGTH)/sizeof(Unit), a, &f1);
    double y = lookup(LENGTH, sizeof(LENGTH)/sizeof(Unit), b, &f2);
    if (f1 && f2) return v*x/y;
    x = lookup(WEIGHT, sizeof(WEIGHT)/sizeof(Unit), a, &f1);
    y = lookup(WEIGHT, sizeof(WEIGHT)/sizeof(Unit), b, &f2);
    if (f1 && f2) return v*x/y;
    fprintf(stderr, "error: cannot convert %s -> %s\n", a, b); exit(1);
}

/* ---------------- Password ---------------- */
static const char PWCHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";
static void password(int n) {
    char buf[257];
    for (int i=0; i<n && i<256; i++) buf[i] = PWCHARS[rand() % (sizeof(PWCHARS)-1)];
    buf[n<256?n:256] = 0;
    puts(buf);
}

/* ---------------- 2048 ---------------- */
static int board[4][4] = {{2,0,2,0},{4,2,4,2},{0,4,0,4},{2,2,2,2}};
static void print_board(const char *label, int b[4][4]) {
    if (label[0]) printf("%s\n", label);
    for (int i=0;i<4;i++) printf("  %4d %4d %4d %4d\n", b[i][0],b[i][1],b[i][2],b[i][3]);
}
static void collapse(int line[4], int out[4]) {
    int tmp[4], n=0, k=0;
    for (int i=0;i<4;i++) if (line[i]) tmp[n++]=line[i];
    for (int i=0;i<n;i++) {
        if (i+1<n && tmp[i]==tmp[i+1]) { out[k++]=tmp[i]*2; i++; }
        else out[k++]=tmp[i];
    }
    while (k<4) out[k++]=0;
}
static void move2048(const char *dir) {
    int b[4][4];
    memcpy(b, board, sizeof(b));
    for (int i=0;i<4;i++) {
        int line[4], out[4];
        if (!strcmp(dir,"left")) {
            for (int j=0;j<4;j++) line[j]=b[i][j];
            collapse(line,out);
            for (int j=0;j<4;j++) b[i][j]=out[j];
        } else if (!strcmp(dir,"right")) {
            for (int j=0;j<4;j++) line[j]=b[i][3-j];
            collapse(line,out);
            for (int j=0;j<4;j++) b[i][3-j]=out[j];
        } else if (!strcmp(dir,"up")) {
            for (int j=0;j<4;j++) line[j]=b[j][i];
            collapse(line,out);
            for (int j=0;j<4;j++) b[j][i]=out[j];
        } else if (!strcmp(dir,"down")) {
            for (int j=0;j<4;j++) line[j]=b[3-j][i];
            collapse(line,out);
            for (int j=0;j<4;j++) b[3-j][i]=out[j];
        } else { fprintf(stderr,"error: direction must be left/right/up/down\n"); exit(1); }
    }
    print_board("Before:", board);
    printf("After (%s):\n", dir); print_board("", b);
}

/* ---------------- Easter egg ---------------- */
static const char EASTER[] =
"  S O L C A T   P O L Y G L O T   T O O L S\n"
"       \\  ^__^\n"
"        \\ (oo)\\_______\n"
"          (__)\\       )\\/\\\n"
"              ||----w |\n"
"              ||     ||\n";

static void usage(void) {
    puts("SOLCAT POLYGLOT TOOLS (C)");
    puts("Usage:");
    puts("  tool calc <expression>         e.g. calc \"1+2*(3-4)\"");
    puts("  tool convert <val> <from> <to> e.g. convert 100 cm m");
    puts("  tool password [length]         e.g. password 16");
    puts("  tool 2048 [left|right|up|down]");
    puts("  tool easter");
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    if (argc < 2) { usage(); return 0; }
    const char *cmd = argv[1];
    if (!strcmp(cmd,"easter") || !strcmp(cmd,"solcat")) { fputs(EASTER, stdout); return 0; }
    if (!strcmp(cmd,"calc")) {
        if (argc < 3) { puts("error: calc needs an expression"); return 1; }
        printf("%.6g\n", calc(argv[2]));
    } else if (!strcmp(cmd,"convert")) {
        if (argc < 5) { puts("error: convert needs <value> <from> <to>"); return 1; }
        printf("%.6f %s\n", convert(atof(argv[2]), argv[3], argv[4]), argv[4]);
    } else if (!strcmp(cmd,"password")) {
        password(argc > 2 ? atoi(argv[2]) : 16);
    } else if (!strcmp(cmd,"2048")) {
        move2048(argc > 2 ? argv[2] : "left");
    } else {
        printf("unknown command: %s\n", cmd); usage();
    }
    (void)fabs; /* keep math link on all platforms */
    return 0;
}
