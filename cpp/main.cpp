// SOLCAT POLYGLOT TOOLS - C++17 edition (classes + STL)
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------- Calculator: recursive descent ----------------
class Calculator {
public:
    explicit Calculator(std::string input) : s_(std::move(input)) {}

    double evaluate() {
        double v = expr();
        skip();
        if (pos_ != s_.size()) throw std::runtime_error("unexpected trailing input");
        return v;
    }

private:
    std::string s_;
    size_t pos_ = 0;

    void skip() {
        while (pos_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[pos_]))) pos_++;
    }

    double factor() {
        skip();
        if (pos_ >= s_.size()) throw std::runtime_error("unexpected end of expression");
        char ch = s_[pos_];
        if (ch == '+') { pos_++; return factor(); }
        if (ch == '-') { pos_++; return -factor(); }
        if (ch == '(') {
            pos_++;
            double v = expr();
            skip();
            if (pos_ >= s_.size() || s_[pos_] != ')') throw std::runtime_error("missing ')'");
            pos_++;
            return v;
        }
        size_t start = pos_;
        while (pos_ < s_.size()) {
            char c = s_[pos_];
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') { pos_++; continue; }
            if (c == 'e' || c == 'E') {
                size_t j = pos_ + 1;
                if (j < s_.size() && (s_[j] == '+' || s_[j] == '-')) j++;
                if (j < s_.size() && std::isdigit(static_cast<unsigned char>(s_[j]))) {
                    pos_ = j + 1;
                    continue;
                }
            }
            break;
        }
        if (start == pos_) throw std::runtime_error("invalid number");
        try {
            return std::stod(s_.substr(start, pos_ - start));
        } catch (...) {
            throw std::runtime_error("invalid number");
        }
    }

    double term() {
        double v = factor();
        for (;;) {
            skip();
            if (pos_ >= s_.size()) break;
            char op = s_[pos_];
            if (op != '*' && op != '/') break;
            pos_++;
            double r = factor();
            if (op == '*') v *= r;
            else {
                if (r == 0) throw std::runtime_error("division by zero");
                v /= r;
            }
        }
        return v;
    }

    double expr() {
        double v = term();
        for (;;) {
            skip();
            if (pos_ >= s_.size()) break;
            char op = s_[pos_];
            if (op != '+' && op != '-') break;
            pos_++;
            double r = term();
            v += (op == '+') ? r : -r;
        }
        return v;
    }
};

// ---------------- Converter ----------------
class Converter {
public:
    static double convert(double value, std::string from, std::string to) {
        std::transform(from.begin(), from.end(), from.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::transform(to.begin(), to.end(), to.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (isTemp(from) && isTemp(to)) return fromC(toC(value, from), to);
        if (auto r = byTable(LENGTH, value, from, to)) return *r;
        if (auto r = byTable(WEIGHT, value, from, to)) return *r;
        throw std::runtime_error("cannot convert " + from + " -> " + to);
    }

private:
    static const std::map<std::string, double> LENGTH;
    static const std::map<std::string, double> WEIGHT;

    static bool isTemp(const std::string& u) { return u == "c" || u == "f" || u == "k"; }
    static double toC(double v, const std::string& u) {
        if (u == "c") return v;
        if (u == "f") return (v - 32) * 5.0 / 9.0;
        return v - 273.15;
    }
    static double fromC(double v, const std::string& u) {
        if (u == "c") return v;
        if (u == "f") return v * 9.0 / 5.0 + 32;
        return v + 273.15;
    }
    static std::optional<double> byTable(const std::map<std::string, double>& t,
                                         double v, const std::string& from, const std::string& to) {
        auto f = t.find(from), x = t.find(to);
        if (f == t.end() || x == t.end()) return std::nullopt;
        return v * f->second / x->second;
    }
};

const std::map<std::string, double> Converter::LENGTH = {
    {"m",1},{"meter",1},{"cm",0.01},{"mm",0.001},{"km",1000},
    {"in",0.0254},{"ft",0.3048},{"yd",0.9144},{"mi",1609.344}
};
const std::map<std::string, double> Converter::WEIGHT = {
    {"kg",1},{"g",0.001},{"mg",1e-6},{"t",1000},
    {"lb",0.45359237},{"oz",0.028349523125}
};

// ---------------- Password ----------------
class PasswordGenerator {
public:
    explicit PasswordGenerator(size_t length) : length_(length) {}
    std::string generate() const {
        static const std::string chars =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
        std::string out;
        out.reserve(length_);
        for (size_t i = 0; i < length_; i++) out.push_back(chars[dist(gen)]);
        return out;
    }
private:
    size_t length_;
};

// ---------------- 2048 game logic ----------------
class Game2048 {
public:
    using Board = std::array<std::array<int, 4>, 4>;

    explicit Game2048(Board b) : board_(std::move(b)) {}

    void move(const std::string& dir) {
        Board next = board_;
        for (int i = 0; i < 4; i++) {
            std::array<int, 4> line{}, out{};
            if (dir == "left") {
                for (int j = 0; j < 4; j++) line[j] = next[i][j];
                out = collapse(line);
                for (int j = 0; j < 4; j++) next[i][j] = out[j];
            } else if (dir == "right") {
                for (int j = 0; j < 4; j++) line[j] = next[i][3-j];
                out = collapse(line);
                for (int j = 0; j < 4; j++) next[i][3-j] = out[j];
            } else if (dir == "up") {
                for (int j = 0; j < 4; j++) line[j] = next[j][i];
                out = collapse(line);
                for (int j = 0; j < 4; j++) next[j][i] = out[j];
            } else if (dir == "down") {
                for (int j = 0; j < 4; j++) line[j] = next[3-j][i];
                out = collapse(line);
                for (int j = 0; j < 4; j++) next[3-j][i] = out[j];
            } else {
                throw std::runtime_error("direction must be left/right/up/down");
            }
        }
        board_ = next;
    }

    void print(const std::string& label) const {
        if (!label.empty()) std::cout << label << "\n";
        for (const auto& row : board_) {
            std::cout << " ";
            for (int v : row) std::cout << std::setw(4) << v;
            std::cout << "\n";
        }
    }

private:
    Board board_;

    static std::array<int, 4> collapse(const std::array<int, 4>& line) {
        std::vector<int> vals;
        for (int v : line) if (v) vals.push_back(v);
        std::array<int, 4> out{};
        size_t k = 0;
        for (size_t i = 0; i < vals.size(); i++) {
            if (i + 1 < vals.size() && vals[i] == vals[i+1]) {
                out[k++] = vals[i] * 2;
                i++;
            } else out[k++] = vals[i];
        }
        return out;
    }
};

// ---------------- Easter egg ----------------
static const char* EASTER =
    "  S O L C A T   P O L Y G L O T   T O O L S\n"
    "       \\  ^__^\n"
    "        \\ (oo)\\_______\n"
    "          (__)\\       )\\/\\\n"
    "              ||----w |\n"
    "              ||     ||\n";

static void usage() {
    std::cout << "SOLCAT POLYGLOT TOOLS (C++)\n"
              << "Usage:\n"
              << "  tool calc <expression>         e.g. calc \"1+2*(3-4)\"\n"
              << "  tool convert <val> <from> <to> e.g. convert 100 cm m\n"
              << "  tool password [length]         e.g. password 16\n"
              << "  tool 2048 [left|right|up|down]\n"
              << "  tool easter\n";
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) { usage(); return 0; }
        std::string cmd = argv[1];
        if (cmd == "easter" || cmd == "solcat") { std::cout << EASTER; return 0; }

        if (cmd == "calc") {
            if (argc < 3) throw std::runtime_error("calc needs an expression");
            std::cout << std::setprecision(9) << Calculator(argv[2]).evaluate() << "\n";
        } else if (cmd == "convert") {
            if (argc < 5) throw std::runtime_error("convert needs <value> <from> <to>");
            std::cout << std::fixed << std::setprecision(6)
                      << Converter::convert(std::stod(argv[2]), argv[3], argv[4])
                      << " " << argv[4] << "\n";
        } else if (cmd == "password") {
            size_t n = argc > 2 ? static_cast<size_t>(std::stoul(argv[2])) : 16;
            std::cout << PasswordGenerator(n).generate() << "\n";
        } else if (cmd == "2048") {
            Game2048::Board b = {{{2,0,2,0},{4,2,4,2},{0,4,0,4},{2,2,2,2}}};
            Game2048 game(b);
            game.print("Before:");
            std::string dir = argc > 2 ? argv[2] : "left";
            game.move(dir);
            game.print("After (" + dir + "):");
        } else {
            std::cout << "unknown command: " << cmd << "\n";
            usage();
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
