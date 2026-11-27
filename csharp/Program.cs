// SOLCAT POLYGLOT TOOLS - C# / .NET 6 edition
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Security.Cryptography;
using System.Text;

namespace SolcatPolyglotTools
{
    // ---------------- Calculator: recursive descent ----------------
    internal sealed class Calculator
    {
        private readonly string _s;
        private int _pos;

        public Calculator(string input) { _s = input; }

        public double Evaluate()
        {
            double v = Expr();
            Skip();
            if (_pos != _s.Length) throw new Exception("unexpected trailing input");
            return v;
        }

        private void Skip()
        {
            while (_pos < _s.Length && char.IsWhiteSpace(_s[_pos])) _pos++;
        }

        private double Factor()
        {
            Skip();
            if (_pos >= _s.Length) throw new Exception("unexpected end of expression");
            char ch = _s[_pos];
            if (ch == '+') { _pos++; return Factor(); }
            if (ch == '-') { _pos++; return -Factor(); }
            if (ch == '(')
            {
                _pos++;
                double v = Expr();
                Skip();
                if (_pos >= _s.Length || _s[_pos] != ')') throw new Exception("missing ')'");
                _pos++;
                return v;
            }
            int start = _pos;
            while (_pos < _s.Length)
            {
                char c = _s[_pos];
                if (char.IsDigit(c) || c == '.') { _pos++; continue; }
                if (c == 'e' || c == 'E')
                {
                    int j = _pos + 1;
                    if (j < _s.Length && (_s[j] == '+' || _s[j] == '-')) j++;
                    if (j < _s.Length && char.IsDigit(_s[j])) { _pos = j + 1; continue; }
                }
                break;
            }
            if (start == _pos) throw new Exception("invalid number");
            return double.Parse(_s.Substring(start, _pos - start), CultureInfo.InvariantCulture);
        }

        private double Term()
        {
            double v = Factor();
            for (;;)
            {
                Skip();
                if (_pos >= _s.Length) break;
                char op = _s[_pos];
                if (op != '*' && op != '/') break;
                _pos++;
                double r = Factor();
                if (op == '*') v *= r;
                else
                {
                    if (r == 0) throw new Exception("division by zero");
                    v /= r;
                }
            }
            return v;
        }

        private double Expr()
        {
            double v = Term();
            for (;;)
            {
                Skip();
                if (_pos >= _s.Length) break;
                char op = _s[_pos];
                if (op != '+' && op != '-') break;
                _pos++;
                double r = Term();
                v += op == '+' ? r : -r;
            }
            return v;
        }
    }

    // ---------------- Converter ----------------
    internal static class Converter
    {
        private static readonly Dictionary<string, double> Length = new()
        {
            ["m"] = 1, ["meter"] = 1, ["cm"] = 0.01, ["mm"] = 0.001, ["km"] = 1000,
            ["in"] = 0.0254, ["ft"] = 0.3048, ["yd"] = 0.9144, ["mi"] = 1609.344
        };
        private static readonly Dictionary<string, double> Weight = new()
        {
            ["kg"] = 1, ["g"] = 0.001, ["mg"] = 1e-6, ["t"] = 1000,
            ["lb"] = 0.45359237, ["oz"] = 0.028349523125
        };

        public static double Convert(double value, string from, string to)
        {
            from = from.ToLowerInvariant();
            to = to.ToLowerInvariant();
            if (IsTemp(from) && IsTemp(to)) return FromC(ToC(value, from), to);
            if (ByTable(Length, value, from, to, out double r)) return r;
            if (ByTable(Weight, value, from, to, out r)) return r;
            throw new Exception($"cannot convert {from} -> {to}");
        }

        private static bool IsTemp(string u) => u == "c" || u == "f" || u == "k";
        private static double ToC(double v, string u) =>
            u == "c" ? v : u == "f" ? (v - 32) * 5.0 / 9.0 : v - 273.15;
        private static double FromC(double v, string u) =>
            u == "c" ? v : u == "f" ? v * 9.0 / 5.0 + 32 : v + 273.15;
        private static bool ByTable(Dictionary<string, double> t, double v,
                                    string from, string to, out double result)
        {
            if (t.TryGetValue(from, out double f) && t.TryGetValue(to, out double x))
            {
                result = v * f / x;
                return true;
            }
            result = 0;
            return false;
        }
    }

    // ---------------- Password ----------------
    internal static class PasswordGenerator
    {
        private const string Chars =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";

        public static string Generate(int length)
        {
            var sb = new StringBuilder(length);
            for (int i = 0; i < length; i++)
            {
                int idx = RandomNumberGenerator.GetInt32(Chars.Length);
                sb.Append(Chars[idx]);
            }
            return sb.ToString();
        }
    }

    // ---------------- 2048 game logic ----------------
    internal sealed class Game2048
    {
        private int[,] _board;

        public Game2048(int[,] board) { _board = (int[,])board.Clone(); }

        public void Move(string dir)
        {
            var next = (int[,])_board.Clone();
            for (int i = 0; i < 4; i++)
            {
                var line = new int[4];
                int[] collapsed;
                if (dir == "left")
                {
                    for (int j = 0; j < 4; j++) line[j] = next[i, j];
                    collapsed = Collapse(line);
                    for (int j = 0; j < 4; j++) next[i, j] = collapsed[j];
                }
                else if (dir == "right")
                {
                    for (int j = 0; j < 4; j++) line[j] = next[i, 3 - j];
                    collapsed = Collapse(line);
                    for (int j = 0; j < 4; j++) next[i, 3 - j] = collapsed[j];
                }
                else if (dir == "up")
                {
                    for (int j = 0; j < 4; j++) line[j] = next[j, i];
                    collapsed = Collapse(line);
                    for (int j = 0; j < 4; j++) next[j, i] = collapsed[j];
                }
                else if (dir == "down")
                {
                    for (int j = 0; j < 4; j++) line[j] = next[3 - j, i];
                    collapsed = Collapse(line);
                    for (int j = 0; j < 4; j++) next[3 - j, i] = collapsed[j];
                }
                else throw new Exception("direction must be left/right/up/down");
            }
            _board = next;
        }

        public void Print(string label)
        {
            if (label.Length > 0) Console.WriteLine(label);
            for (int i = 0; i < 4; i++)
                Console.WriteLine("  {0,4} {1,4} {2,4} {3,4}",
                    _board[i, 0], _board[i, 1], _board[i, 2], _board[i, 3]);
        }

        private static int[] Collapse(int[] line)
        {
            var vals = new List<int>();
            foreach (int v in line) if (v != 0) vals.Add(v);
            var outArr = new int[4];
            int k = 0;
            for (int i = 0; i < vals.Count; i++)
            {
                if (i + 1 < vals.Count && vals[i] == vals[i + 1])
                {
                    outArr[k++] = vals[i] * 2;
                    i++;
                }
                else outArr[k++] = vals[i];
            }
            return outArr;
        }
    }

    internal static class Program
    {
        private const string Easter =
            "  S O L C A T   P O L Y G L O T   T O O L S\n" +
            "       \\  ^__^\n" +
            "        \\ (oo)\\_______\n" +
            "          (__)\\       )\\/\\\n" +
            "              ||----w |\n" +
            "              ||     ||\n";

        private static void Usage()
        {
            Console.WriteLine("SOLCAT POLYGLOT TOOLS (C#)");
            Console.WriteLine("Usage:");
            Console.WriteLine("  tool calc <expression>         e.g. calc \"1+2*(3-4)\"");
            Console.WriteLine("  tool convert <val> <from> <to> e.g. convert 100 cm m");
            Console.WriteLine("  tool password [length]         e.g. password 16");
            Console.WriteLine("  tool 2048 [left|right|up|down]");
            Console.WriteLine("  tool easter");
        }

        private static int Main(string[] args)
        {
            try
            {
                if (args.Length == 0) { Usage(); return 0; }
                string cmd = args[0];
                if (cmd == "easter" || cmd == "solcat") { Console.Write(Easter); return 0; }
                if (args.Length < 2) { Usage(); return 0; }

                switch (cmd)
                {
                    case "calc":
                        if (args.Length < 2) throw new Exception("calc needs an expression");
                        Console.WriteLine(new Calculator(args[1]).Evaluate().ToString("0.#########", CultureInfo.InvariantCulture));
                        break;
                    case "convert":
                        if (args.Length < 4) throw new Exception("convert needs <value> <from> <to>");
                        double v = double.Parse(args[1], CultureInfo.InvariantCulture);
                        Console.WriteLine(Converter.Convert(v, args[2], args[3]).ToString("F6", CultureInfo.InvariantCulture) + " " + args[3]);
                        break;
                    case "password":
                        int n = args.Length > 1 ? int.Parse(args[1]) : 16;
                        Console.WriteLine(PasswordGenerator.Generate(n));
                        break;
                    case "2048":
                        var b = new int[4, 4] { { 2, 0, 2, 0 }, { 4, 2, 4, 2 }, { 0, 4, 0, 4 }, { 2, 2, 2, 2 } };
                        var game = new Game2048(b);
                        string dir = args.Length > 1 ? args[1] : "left";
                        game.Print("Before:");
                        game.Move(dir);
                        game.Print("After (" + dir + "):");
                        break;
                    default:
                        Console.WriteLine("unknown command: " + cmd);
                        Usage();
                        break;
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("error: " + e.Message);
                return 1;
            }
            return 0;
        }
    }
}
