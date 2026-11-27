// SOLCAT POLYGLOT TOOLS - Go edition
// Subcommands: calc | convert | password | 2048 | easter
package main

import (
	"crypto/rand"
	"flag"
	"fmt"
	"math"
	"math/big"
	"os"
	"strconv"
	"strings"
)

// ---------------- Calc: recursive descent parser ----------------
type parser struct {
	s   string
	pos int
}

func (p *parser) skip() {
	for p.pos < len(p.s) && (p.s[p.pos] == ' ' || p.s[p.pos] == '\t') {
		p.pos++
	}
}

// expr := term {('+'|'-') term}
func (p *parser) expr() float64 {
	v := p.term()
	for {
		p.skip()
		if p.pos >= len(p.s) {
			break
		}
		if p.s[p.pos] == '+' || p.s[p.pos] == '-' {
			op := p.s[p.pos]
			p.pos++
			r := p.term()
			if op == '+' {
				v += r
			} else {
				v -= r
			}
		} else {
			break
		}
	}
	return v
}

// term := factor {('*'|'/') factor}
func (p *parser) term() float64 {
	v := p.factor()
	for {
		p.skip()
		if p.pos >= len(p.s) {
			break
		}
		if p.s[p.pos] == '*' || p.s[p.pos] == '/' {
			op := p.s[p.pos]
			p.pos++
			r := p.factor()
			if op == '*' {
				v *= r
			} else {
				if r == 0 {
					panic("division by zero")
				}
				v /= r
			}
		} else {
			break
		}
	}
	return v
}

// factor := ('+'|'-') factor | '(' expr ')' | number
func (p *parser) factor() float64 {
	p.skip()
	if p.pos >= len(p.s) {
		panic("unexpected end of expression")
	}
	ch := p.s[p.pos]
	if ch == '+' {
		p.pos++
		return p.factor()
	}
	if ch == '-' {
		p.pos++
		return -p.factor()
	}
	if ch == '(' {
		p.pos++
		v := p.expr()
		p.skip()
		if p.pos >= len(p.s) || p.s[p.pos] != ')' {
			panic("missing ')'")
		}
		p.pos++
		return v
	}
	start := p.pos
	for p.pos < len(p.s) {
		c := p.s[p.pos]
		if (c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-' {
			// stop on exponent sign only when preceded by e/E
			if (c == '+' || c == '-') && !(p.s[p.pos-1] == 'e' || p.s[p.pos-1] == 'E') {
				break
			}
			p.pos++
		} else {
			break
		}
	}
	if start == p.pos {
		panic("invalid character in expression")
	}
	v, err := strconv.ParseFloat(p.s[start:p.pos], 64)
	if err != nil {
		panic(err.Error())
	}
	return v
}

func calc(expr string) float64 {
	p := &parser{s: expr}
	v := p.expr()
	p.skip()
	if p.pos != len(p.s) {
		panic("unexpected trailing input")
	}
	return v
}

// ---------------- Convert: length / weight / temperature ----------------
var lengthFactors = map[string]float64{
	"m": 1, "meter": 1, "cm": 0.01, "mm": 0.001, "km": 1000,
	"in": 0.0254, "ft": 0.3048, "yd": 0.9144, "mi": 1609.344,
}
var weightFactors = map[string]float64{
	"kg": 1, "g": 0.001, "mg": 1e-6, "t": 1000,
	"lb": 0.45359237, "oz": 0.028349523125,
}
var tempUnits = map[string]bool{"c": true, "f": true, "k": true}

func toCelsius(v float64, unit string) float64 {
	switch strings.ToLower(unit) {
	case "c":
		return v
	case "f":
		return (v - 32) * 5.0 / 9.0
	case "k":
		return v - 273.15
	}
	panic("unknown temperature unit: " + unit)
}

func fromCelsius(v float64, unit string) float64 {
	switch strings.ToLower(unit) {
	case "c":
		return v
	case "f":
		return v*9.0/5.0 + 32
	case "k":
		return v + 273.15
	}
	panic("unknown temperature unit: " + unit)
}

func convert(v float64, from, to string) float64 {
	from, to = strings.ToLower(from), strings.ToLower(to)
	if tempUnits[from] && tempUnits[to] {
		return fromCelsius(toCelsius(v, from), to)
	}
	if f, ok := lengthFactors[from]; ok {
		if t, ok := lengthFactors[to]; ok {
			return v * f / t
		}
	}
	if f, ok := weightFactors[from]; ok {
		if t, ok := weightFactors[to]; ok {
			return v * f / t
		}
	}
	panic(fmt.Sprintf("cannot convert %s -> %s", from, to))
}

// ---------------- Password: crypto/rand ----------------
const pwChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+"

func password(n int) string {
	var sb strings.Builder
	for i := 0; i < n; i++ {
		idx, err := rand.Int(rand.Reader, big.NewInt(int64(len(pwChars))))
		if err != nil {
			panic(err.Error())
		}
		sb.WriteByte(pwChars[idx.Int64()])
	}
	return sb.String()
}

// ---------------- 2048 game logic ----------------
var board = [4][4]int{
	{2, 0, 2, 0},
	{4, 2, 4, 2},
	{0, 4, 0, 4},
	{2, 2, 2, 2},
}

func collapse(line []int) []int {
	out := make([]int, 0, 4)
	for _, v := range line {
		if v != 0 {
			out = append(out, v)
		}
	}
	merged := make([]int, 0, 4)
	for i := 0; i < len(out); i++ {
		if i+1 < len(out) && out[i] == out[i+1] {
			merged = append(merged, out[i]*2)
			i++
		} else {
			merged = append(merged, out[i])
		}
	}
	for len(merged) < 4 {
		merged = append(merged, 0)
	}
	return merged
}

func move2048(dir string) {
	var b [4][4]int
	for i := 0; i < 4; i++ {
		for j := 0; j < 4; j++ {
			b[i][j] = board[i][j]
		}
	}
	for i := 0; i < 4; i++ {
		var line [4]int
		switch dir {
		case "left":
			for j := 0; j < 4; j++ {
				line[j] = b[i][j]
			}
			c := collapse(line[:])
			for j := 0; j < 4; j++ {
				b[i][j] = c[j]
			}
		case "right":
			for j := 0; j < 4; j++ {
				line[j] = b[i][3-j]
			}
			c := collapse(line[:])
			for j := 0; j < 4; j++ {
				b[i][3-j] = c[j]
			}
		case "up":
			for j := 0; j < 4; j++ {
				line[j] = b[j][i]
			}
			c := collapse(line[:])
			for j := 0; j < 4; j++ {
				b[j][i] = c[j]
			}
		case "down":
			for j := 0; j < 4; j++ {
				line[j] = b[3-j][i]
			}
			c := collapse(line[:])
			for j := 0; j < 4; j++ {
				b[3-j][i] = c[j]
			}
		default:
			panic("direction must be left/right/up/down")
		}
	}
	printBoard("Before:", board)
	printBoard("After ("+dir+"):", b)
}

func printBoard(label string, b [4][4]int) {
	fmt.Println(label)
	for i := 0; i < 4; i++ {
		fmt.Printf("  %4d %4d %4d %4d\n", b[i][0], b[i][1], b[i][2], b[i][3])
	}
}

// ---------------- Easter egg ----------------
const easterArt = `  S O L C A T   P O L Y G L O T   T O O L S
       \  ^__^
        \ (oo)\_______
          (__)\       )\/\
              ||----w |
              ||     ||`

func usage() {
	fmt.Println("SOLCAT POLYGLOT TOOLS (Go)")
	fmt.Println("Usage:")
	fmt.Println("  tool calc <expression>        e.g. calc \"1+2*(3-4)\"")
	fmt.Println("  tool convert <val> <from> <to> e.g. convert 100 cm m")
	fmt.Println("  tool password [length]        e.g. password 16")
	fmt.Println("  tool 2048 [left|right|up|down]")
	fmt.Println("  tool easter")
}

func main() {
	if len(os.Args) < 2 {
		usage()
		return
	}
	cmd := os.Args[1]
	if cmd == "easter" || cmd == "solcat" {
		fmt.Println(easterArt)
		return
	}
	args := os.Args[2:]

	switch cmd {
	case "calc":
		fs := flag.NewFlagSet("calc", flag.ExitOnError)
		fs.Parse(args)
		if fs.NArg() < 1 {
			fmt.Println("error: calc needs an expression")
			return
		}
		fmt.Printf("%.6g\n", calc(fs.Arg(0)))
	case "convert":
		fs := flag.NewFlagSet("convert", flag.ExitOnError)
		fs.Parse(args)
		if fs.NArg() < 3 {
			fmt.Println("error: convert needs <value> <from> <to>")
			return
		}
		v, err := strconv.ParseFloat(fs.Arg(0), 64)
		if err != nil {
			fmt.Println("error:", err)
			return
		}
		fmt.Printf("%.6f %s\n", convert(v, fs.Arg(1), fs.Arg(2)), fs.Arg(2))
	case "password":
		fs := flag.NewFlagSet("password", flag.ExitOnError)
		n := fs.Int("n", 16, "password length")
		fs.Parse(args)
		fmt.Println(password(*n))
	case "2048":
		fs := flag.NewFlagSet("2048", flag.ExitOnError)
		dir := fs.String("dir", "left", "move direction")
		fs.Parse(args)
		if fs.NArg() > 0 {
			*dir = fs.Arg(0)
		}
		move2048(*dir)
	case "solcat":
		fmt.Println(easterArt)
	default:
		fmt.Printf("unknown command: %s\n", cmd)
		usage()
	}
	_ = math.Abs // keep math import meaningful for parity with other editions
}
