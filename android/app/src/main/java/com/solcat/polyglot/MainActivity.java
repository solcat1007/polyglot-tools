package com.solcat.polyglot;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.util.Locale;

/**
 * SOLCAT POLYGLOT TOOLS - Android (Material 3) 计算器 UI。
 * 复用与 CLI 各语言版本一致的递归下降表达式求值算法。
 */
public class MainActivity extends AppCompatActivity {

    private TextView display;
    private String expression = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        display = findViewById(R.id.display);

        int[] ids = {
                R.id.btn_clear, R.id.btn_back, R.id.btn_lparen, R.id.btn_rparen,
                R.id.btn_7, R.id.btn_8, R.id.btn_9, R.id.btn_div,
                R.id.btn_4, R.id.btn_5, R.id.btn_6, R.id.btn_mul,
                R.id.btn_1, R.id.btn_2, R.id.btn_3, R.id.btn_minus,
                R.id.btn_0, R.id.btn_dot, R.id.btn_eq, R.id.btn_plus
        };
        View.OnClickListener listener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onKey(((Button) v).getText().toString());
            }
        };
        for (int id : ids) findViewById(id).setOnClickListener(listener);
    }

    private void onKey(String key) {
        if ("C".equals(key)) {
            expression = "";
            display.setText("0");
            return;
        }
        if ("⌫".equals(key)) {
            if (!expression.isEmpty()) expression = expression.substring(0, expression.length() - 1);
            display.setText(expression.isEmpty() ? "0" : expression);
            return;
        }
        if ("=".equals(key)) {
            if ("solcat".equalsIgnoreCase(expression.trim())) {
                // 彩蛋：与 CLI 版本一致的署名
                display.setText(R.string.easter_egg);
                Toast.makeText(this, "SOLCAT POLYGLOT TOOLS", Toast.LENGTH_LONG).show();
                expression = "";
                return;
            }
            try {
                double result = new Parser(expression).evaluate();
                expression = format(result);
                display.setText(expression);
            } catch (IllegalArgumentException e) {
                Toast.makeText(this, "Error: " + e.getMessage(), Toast.LENGTH_SHORT).show();
            }
            return;
        }
        String op = key;
        if ("÷".equals(op)) op = "/";
        else if ("×".equals(op)) op = "*";
        else if ("−".equals(op)) op = "-";
        expression += op;
        display.setText(expression);
    }

    private static String format(double v) {
        String s = String.format(Locale.ROOT, "%.6f", v)
                .replaceAll("0+$", "").replaceAll("\\.$", "");
        return s.equals("-0") ? "0" : s;
    }

    /** 递归下降解析器：四则运算 + 括号 + 一元正负号（与其他语言实现一致）。 */
    static class Parser {
        private final String s;
        private int pos;

        Parser(String s) { this.s = s; }

        double evaluate() {
            double v = expr();
            skip();
            if (pos != s.length()) throw new IllegalArgumentException("unexpected input");
            return v;
        }

        private void skip() {
            while (pos < s.length() && Character.isWhitespace(s.charAt(pos))) pos++;
        }

        private double factor() {
            skip();
            if (pos >= s.length()) throw new IllegalArgumentException("incomplete expression");
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
                break;
            }
            if (start == pos) throw new IllegalArgumentException("invalid number");
            return Double.parseDouble(s.substring(start, pos));
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
}
