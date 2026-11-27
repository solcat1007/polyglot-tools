// SOLCAT POLYGLOT TOOLS - Rust edition (std only)
use std::collections::HashMap;
use std::env;
use std::time::{SystemTime, UNIX_EPOCH};

// ---------------- Calculator: recursive descent ----------------
struct Parser<'a> {
    s: &'a [u8],
    pos: usize,
}

impl<'a> Parser<'a> {
    fn new(s: &'a str) -> Self {
        Parser { s: s.as_bytes(), pos: 0 }
    }

    fn skip(&mut self) {
        while self.pos < self.s.len() && (self.s[self.pos] == b' ' || self.s[self.pos] == b'\t') {
            self.pos += 1;
        }
    }

    fn evaluate(&mut self) -> Result<f64, String> {
        let v = self.expr()?;
        self.skip();
        if self.pos != self.s.len() {
            return Err("unexpected trailing input".into());
        }
        Ok(v)
    }

    fn factor(&mut self) -> Result<f64, String> {
        self.skip();
        if self.pos >= self.s.len() {
            return Err("unexpected end of expression".into());
        }
        match self.s[self.pos] {
            b'+' => { self.pos += 1; self.factor() }
            b'-' => { self.pos += 1; Ok(-self.factor()?) }
            b'(' => {
                self.pos += 1;
                let v = self.expr()?;
                self.skip();
                if self.pos >= self.s.len() || self.s[self.pos] != b')' {
                    return Err("missing ')'".into());
                }
                self.pos += 1;
                Ok(v)
            }
            _ => {
                let start = self.pos;
                while self.pos < self.s.len() {
                    let c = self.s[self.pos];
                    if c.is_ascii_digit() || c == b'.' {
                        self.pos += 1;
                        continue;
                    }
                    if c == b'e' || c == b'E' {
                        let mut j = self.pos + 1;
                        if j < self.s.len() && (self.s[j] == b'+' || self.s[j] == b'-') {
                            j += 1;
                        }
                        if j < self.s.len() && self.s[j].is_ascii_digit() {
                            self.pos = j + 1;
                            continue;
                        }
                    }
                    break;
                }
                if start == self.pos {
                    return Err("invalid number".into());
                }
                let text = std::str::from_utf8(&self.s[start..self.pos]).unwrap();
                text.parse::<f64>().map_err(|_| "invalid number".to_string())
            }
        }
    }

    fn term(&mut self) -> Result<f64, String> {
        let mut v = self.factor()?;
        loop {
            self.skip();
            if self.pos >= self.s.len() { break; }
            let op = self.s[self.pos];
            if op != b'*' && op != b'/' { break; }
            self.pos += 1;
            let r = self.factor()?;
            if op == b'*' { v *= r; }
            else {
                if r == 0.0 { return Err("division by zero".into()); }
                v /= r;
            }
        }
        Ok(v)
    }

    fn expr(&mut self) -> Result<f64, String> {
        let mut v = self.term()?;
        loop {
            self.skip();
            if self.pos >= self.s.len() { break; }
            let op = self.s[self.pos];
            if op != b'+' && op != b'-' { break; }
            self.pos += 1;
            let r = self.term()?;
            v += if op == b'+' { r } else { -r };
        }
        Ok(v)
    }
}

fn calc(s: &str) -> Result<f64, String> {
    Parser::new(s).evaluate()
}

// ---------------- Converter ----------------
fn table(pairs: &[(&str, f64)]) -> HashMap<String, f64> {
    pairs.iter().map(|(k, v)| (k.to_string(), *v)).collect()
}

fn convert(value: f64, from: &str, to: &str) -> Result<f64, String> {
    let from = from.to_lowercase();
    let to = to.to_lowercase();
    let is_temp = |u: &str| u == "c" || u == "f" || u == "k";
    let to_c = |v: f64, u: &str| match u {
        "c" => v, "f" => (v - 32.0) * 5.0 / 9.0, "k" => v - 273.15,
        _ => unreachable!(),
    };
    let from_c = |v: f64, u: &str| match u {
        "c" => v, "f" => v * 9.0 / 5.0 + 32.0, "k" => v + 273.15,
        _ => unreachable!(),
    };
    if is_temp(&from) && is_temp(&to) {
        return Ok(from_c(to_c(value, &from), &to));
    }
    let length = table(&[
        ("m", 1.0), ("meter", 1.0), ("cm", 0.01), ("mm", 0.001), ("km", 1000.0),
        ("in", 0.0254), ("ft", 0.3048), ("yd", 0.9144), ("mi", 1609.344),
    ]);
    let weight = table(&[
        ("kg", 1.0), ("g", 0.001), ("mg", 1e-6), ("t", 1000.0),
        ("lb", 0.45359237), ("oz", 0.028349523125),
    ]);
    for t in [&length, &weight] {
        if let (Some(f), Some(x)) = (t.get(&from), t.get(&to)) {
            return Ok(value * f / x);
        }
    }
    Err(format!("cannot convert {} -> {}", from, to))
}

// ---------------- Password (xorshift PRNG, std only) ----------------
fn password(n: usize) -> String {
    const CHARS: &[u8] = b"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+";
    let mut seed = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_nanos() as u64)
        .unwrap_or(123456789);
    (0..n).map(|_| {
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
        CHARS[(seed as usize) % CHARS.len()] as char
    }).collect()
}

// ---------------- 2048 game logic ----------------
type Board = [[i32; 4]; 4];

fn collapse(line: [i32; 4]) -> [i32; 4] {
    let vals: Vec<i32> = line.iter().copied().filter(|&v| v != 0).collect();
    let mut out = [0i32; 4];
    let mut k = 0;
    let mut i = 0;
    while i < vals.len() {
        if i + 1 < vals.len() && vals[i] == vals[i + 1] {
            out[k] = vals[i] * 2;
            k += 1;
            i += 2;
        } else {
            out[k] = vals[i];
            k += 1;
            i += 1;
        }
    }
    out
}

fn move_board(board: &Board, dir: &str) -> Result<Board, String> {
    let mut next = *board;
    for i in 0..4 {
        let mut line = [0i32; 4];
        let collapsed;
        match dir {
            "left" => {
                for j in 0..4 { line[j] = next[i][j]; }
                collapsed = collapse(line);
                for j in 0..4 { next[i][j] = collapsed[j]; }
            }
            "right" => {
                for j in 0..4 { line[j] = next[i][3 - j]; }
                collapsed = collapse(line);
                for j in 0..4 { next[i][3 - j] = collapsed[j]; }
            }
            "up" => {
                for j in 0..4 { line[j] = next[j][i]; }
                collapsed = collapse(line);
                for j in 0..4 { next[j][i] = collapsed[j]; }
            }
            "down" => {
                for j in 0..4 { line[j] = next[3 - j][i]; }
                collapsed = collapse(line);
                for j in 0..4 { next[3 - j][i] = collapsed[j]; }
            }
            _ => return Err("direction must be left/right/up/down".into()),
        }
    }
    Ok(next)
}

fn print_board(label: &str, b: &Board) {
    if !label.is_empty() { println!("{}", label); }
    for row in b {
        println!("  {:4} {:4} {:4} {:4}", row[0], row[1], row[2], row[3]);
    }
}

// ---------------- Easter egg ----------------
const EASTER: &str = r#"  S O L C A T   P O L Y G L O T   T O O L S
       \  ^__^
        \ (oo)\_______
          (__)\       )\/\
              ||----w |
              ||     ||
"#;

fn usage() {
    println!("SOLCAT POLYGLOT TOOLS (Rust)");
    println!("Usage:");
    println!("  tool calc <expression>         e.g. calc \"1+2*(3-4)\"");
    println!("  tool convert <val> <from> <to> e.g. convert 100 cm m");
    println!("  tool password [length]         e.g. password 16");
    println!("  tool 2048 [left|right|up|down]");
    println!("  tool easter");
}

fn run() -> Result<(), String> {
    let args: Vec<String> = env::args().skip(1).collect();
    if args.is_empty() { usage(); return Ok(()); }
    let cmd = &args[0];
    if cmd == "easter" || cmd == "solcat" {
        print!("{}", EASTER);
        return Ok(());
    }
    match cmd.as_str() {
        "calc" => {
            if args.len() < 2 { return Err("calc needs an expression".into()); }
            println!("{}", calc(&args[1])?);
        }
        "convert" => {
            if args.len() < 4 { return Err("convert needs <value> <from> <to>".into()); }
            let v: f64 = args[1].parse().map_err(|_| "invalid value".to_string())?;
            println!("{:.6} {}", convert(v, &args[2], &args[3])?, args[3]);
        }
        "password" => {
            let n: usize = if args.len() > 1 {
                args[1].parse().map_err(|_| "invalid length".to_string())?
            } else { 16 };
            println!("{}", password(n));
        }
        "2048" => {
            let board: Board = [[2, 0, 2, 0], [4, 2, 4, 2], [0, 4, 0, 4], [2, 2, 2, 2]];
            let dir = args.get(1).map(String::as_str).unwrap_or("left");
            print_board("Before:", &board);
            let next = move_board(&board, dir)?;
            print_board(&format!("After ({}):", dir), &next);
        }
        _ => {
            println!("unknown command: {}", cmd);
            usage();
        }
    }
    Ok(())
}

fn main() {
    if let Err(e) = run() {
        eprintln!("error: {}", e);
        std::process::exit(1);
    }
}
