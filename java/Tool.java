// SOLCAT POLYGLOT TOOLS - Java edition
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class Tool {

    // ---------------- Calculator: recursive descent ----------------
    static class Parser {
        private final String s;
        private int pos;

        Parser(String s) { this.s = s; }

        double evaluate() {
            double v = expr();
            skip();
            if (pos != s.length()) throw new IllegalArgumentException("unexpected trailing input");
            return v;
        }

        private void skip() {
            while (pos < s.length() && Character.isWhitespace(s.charAt(pos))) pos++;
        }

        private double factor() {
            skip();
            if (pos >= s.length()) throw new IllegalArgumentException("unexpected end of expression");
            char ch = s.charAt(pos);
            if (ch == '+') { pos++; return factor(); }
            if (ch == '-') { pos++; return -factor(); }
            if (ch == '(') {
                pos++;
                double v = expr();
                skip();
                if (pos >= s.length() || s.charAt(pos) != ')')
                    throw new IllegalArgumentException("missing ')'");
                pos++;
                return v;
            }
            int start = pos;
            while (pos < s.length()) {
                char c = s.charAt(pos);
                if (Character.isDigit(c) || c == '.') { pos++; continue; }
                if (c == 'e' || c == 'E') {
                    int j = pos + 1;
                    if (j < s.length() && (s.charAt(j) == '+' || s.charAt(j) == '-')) j++;
                    if (j < s.length() && Character.isDigit(s.charAt(j))) { pos = j + 1; continue; }
                }
                break;
            }
            if (start == pos) throw new IllegalArgumentException("invalid number");
            try {
                return Double.parseDouble(s.substring(start, pos));
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("invalid number");
            }
        }

        private double term() {
            double v = factor();
            for (;;) {
                skip();
                if (pos >= s.length()) break;
                char op = s.charAt(pos);
                if (op != '*' && op != '/') break;
                pos++;
                double r = factor();
                if (op == '*') v *= r;
                else {
                    if (r == 0) throw new IllegalArgumentException("division by zero");
                    v /= r;
                }
            }
            return v;
        }

        private double expr() {
            double v = term();
            for (;;) {
                skip();
                if (pos >= s.length()) break;
                char op = s.charAt(pos);
                if (op != '+' && op != '-') break;
                pos++;
                double r = term();
                v += op == '+' ? r : -r;
            }
            return v;
        }
    }

    static double calc(String s) { return new Parser(s).evaluate(); }

    // 与其他语言版本一致的紧凑数字格式（去掉末尾多余的 0）
    static String fmt(double v) {
        String s = String.format(Locale.ROOT, "%.6f", v)
                .replaceAll("0+$", "").replaceAll("\\.$", "");
        return s.equals("-0") ? "0" : s;
    }

    // ---------------- Converter ----------------
    private static final Map<String, Double> LENGTH = new HashMap<>();
    private static final Map<String, Double> WEIGHT = new HashMap<>();
    static {
        LENGTH.put("m", 1.0); LENGTH.put("meter", 1.0); LENGTH.put("cm", 0.01);
        LENGTH.put("mm", 0.001); LENGTH.put("km", 1000.0); LENGTH.put("in", 0.0254);
        LENGTH.put("ft", 0.3048); LENGTH.put("yd", 0.9144); LENGTH.put("mi", 1609.344);
        WEIGHT.put("kg", 1.0); WEIGHT.put("g", 0.001); WEIGHT.put("mg", 1e-6);
        WEIGHT.put("t", 1000.0); WEIGHT.put("lb", 0.45359237); WEIGHT.put("oz", 0.028349523125);
    }

    static double convert(double value, String from, String to) {
        from = from.toLowerCase(Locale.ROOT);
        to = to.toLowerCase(Locale.ROOT);
        boolean tempFrom = isTemp(from), tempTo = isTemp(to);
        if (tempFrom && tempTo) return fromC(toC(value, from), to);
        Double f = LENGTH.get(from), x = LENGTH.get(to);
        if (f != null && x != null) return value * f / x;
        f = WEIGHT.get(from); x = WEIGHT.get(to);
        if (f != null && x != null) return value * f / x;
        throw new IllegalArgumentException("cannot convert " + from + " -> " + to);
    }

    private static boolean isTemp(String u) { return u.equals("c") || u.equals("f") || u.equals("k"); }
    private static double toC(double v, String u) {
        if (u.equals("c")) return v;
        if (u.equals("f")) return (v - 32) * 5.0 / 9.0;
        return v - 273.15;
    }
    private static double fromC(double v, String u) {
        if (u.equals("c")) return v;
        if (u.equals("f")) return v * 9.0 / 5.0 + 32;
        return v + 273.15;
    }

    // ---------------- Password ----------------
    private static final String CHARS =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";

    static String password(int n) {
        SecureRandom random = new SecureRandom();
        StringBuilder sb = new StringBuilder(n);
        for (int i = 0; i < n; i++) sb.append(CHARS.charAt(random.nextInt(CHARS.length())));
        return sb.toString();
    }

    // ---------------- 2048 game logic ----------------
    static int[] collapse(int[] line) {
        List<Integer> vals = new ArrayList<>();
        for (int v : line) if (v != 0) vals.add(v);
        int[] out = new int[4];
        int k = 0;
        for (int i = 0; i < vals.size(); i++) {
            if (i + 1 < vals.size() && vals.get(i).equals(vals.get(i + 1))) {
                out[k++] = vals.get(i) * 2;
                i++;
            } else out[k++] = vals.get(i);
        }
        return out;
    }

    static int[][] move2048(int[][] board, String dir) {
        if (!dir.equals("left") && !dir.equals("right") &&
            !dir.equals("up") && !dir.equals("down"))
            throw new IllegalArgumentException("direction must be left/right/up/down");
        int[][] next = new int[4][4];
        for (int i = 0; i < 4; i++) System.arraycopy(board[i], 0, next[i], 0, 4);
        for (int i = 0; i < 4; i++) {
            int[] line = new int[4];
            int[] collapsed;
            if (dir.equals("left")) {
                for (int j = 0; j < 4; j++) line[j] = next[i][j];
                collapsed = collapse(line);
                for (int j = 0; j < 4; j++) next[i][j] = collapsed[j];
            } else if (dir.equals("right")) {
                for (int j = 0; j < 4; j++) line[j] = next[i][3 - j];
                collapsed = collapse(line);
                for (int j = 0; j < 4; j++) next[i][3 - j] = collapsed[j];
            } else if (dir.equals("up")) {
                for (int j = 0; j < 4; j++) line[j] = next[j][i];
                collapsed = collapse(line);
                for (int j = 0; j < 4; j++) next[j][i] = collapsed[j];
            } else {
                for (int j = 0; j < 4; j++) line[j] = next[3 - j][i];
                collapsed = collapse(line);
                for (int j = 0; j < 4; j++) next[3 - j][i] = collapsed[j];
            }
        }
        return next;
    }

    static void printBoard(String label, int[][] b) {
        if (!label.isEmpty()) System.out.println(label);
        for (int[] row : b) System.out.printf("  %4d %4d %4d %4d%n", row[0], row[1], row[2], row[3]);
    }

    // ---------------- Easter egg ----------------
    static final String EASTER =
        "  S O L C A T   P O L Y G L O T   T O O L S\n" +
        "       \\  ^__^\n" +
        "        \\ (oo)\\_______\n" +
        "          (__)\\       )\\/\\\n" +
        "              ||----w |\n" +
        "              ||     ||";

    static void usage() {
        System.out.println("SOLCAT POLYGLOT TOOLS (Java)");
        System.out.println("Usage:");
        System.out.println("  Tool calc <expression>         e.g. calc \"1+2*(3-4)\"");
        System.out.println("  Tool convert <val> <from> <to> e.g. convert 100 cm m");
        System.out.println("  Tool password [length]         e.g. password 16");
        System.out.println("  Tool 2048 [left|right|up|down]");
        System.out.println("  Tool easter");
    }

    public static void main(String[] args) {
        try {
            if (args.length == 0) { usage(); return; }
            String cmd = args[0];
            if (cmd.equals("easter") || cmd.equals("solcat")) {
                System.out.println(EASTER);
                return;
            }
            switch (cmd) {
                case "calc":
                    if (args.length < 2) throw new IllegalArgumentException("calc needs an expression");
                    System.out.println(fmt(calc(args[1])));
                    break;
                case "convert":
                    if (args.length < 4)
                        throw new IllegalArgumentException("convert needs <value> <from> <to>");
                    System.out.printf("%.6f %s%n", convert(Double.parseDouble(args[1]), args[2], args[3]), args[3]);
                    break;
                case "password":
                    int n = args.length > 1 ? Integer.parseInt(args[1]) : 16;
                    System.out.println(password(n));
                    break;
                case "2048":
                    int[][] board = {{2, 0, 2, 0}, {4, 2, 4, 2}, {0, 4, 0, 4}, {2, 2, 2, 2}};
                    String dir = args.length > 1 ? args[1] : "left";
                    printBoard("Before:", board);
                    printBoard("After (" + dir + "):", move2048(board, dir));
                    break;
                default:
                    System.out.println("unknown command: " + cmd);
                    usage();
            }
        } catch (Exception e) {
            System.err.println("error: " + e.getMessage());
            System.exit(1);
        }
    }
}
