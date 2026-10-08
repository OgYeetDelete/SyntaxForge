// SyntaxForge Phase 2 - lexer, parser (AST), semantic analysis, TAC, optimizer, interpreter
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <cctype>
#include <iomanip>
#include <cmath>
using namespace std;

struct Token { string lex, type; int line, col; };
struct CompileError { string phase, msg; int line, col; };

// ---------- Lexer ----------
vector<Token> lexer(const string& s, vector<CompileError>& errs) {
    vector<Token> t; int line = 1, col = 1;
    for (size_t i = 0; i < s.size();) {
        char c = s[i]; int c0 = col;
        if (c == '\n') { line++; col = 1; i++; continue; }
        if (isspace((unsigned char)c)) { i++; col++; continue; }
        if (isalpha((unsigned char)c) || c == '_') {
            string id; while (i < s.size() && (isalnum((unsigned char)s[i]) || s[i]=='_')) { id += s[i++]; col++; }
            t.push_back({id, "IDENTIFIER", line, c0});
        } else if (isdigit((unsigned char)c)) {
            string n; bool dot = false;
            while (i < s.size() && (isdigit((unsigned char)s[i]) || (s[i]=='.' && !dot))) { if (s[i]=='.') dot = true; n += s[i++]; col++; }
            t.push_back({n, dot ? "FLOAT" : "INTEGER", line, c0});
        } else if (string("+-*/").find(c) != string::npos) { t.push_back({string(1,c), "OPERATOR", line, c0}); i++; col++; }
        else if (c == '=') { t.push_back({"=", "ASSIGNMENT", line, c0}); i++; col++; }
        else if (c == ';') { t.push_back({";", "DELIMITER", line, c0}); i++; col++; }
        else if (c == '(' || c == ')') { t.push_back({string(1,c), "PARENTHESIS", line, c0}); i++; col++; }
        else { errs.push_back({"LEXICAL", string("invalid character '") + c + "'", line, c0}); i++; col++; }
    }
    t.push_back({"", "EOF", line, col});
    return t;
}

// ---------- AST ----------
struct Node { string kind, val; shared_ptr<Node> l, r; int line = 0, col = 0; };
typedef shared_ptr<Node> NP;
NP mk(string k, string v, NP l = nullptr, NP r = nullptr, int ln = 0, int cl = 0) { return make_shared<Node>(Node{k, v, l, r, ln, cl}); }

// ---------- Parser (recursive descent, precedence climbing) ----------
struct Parser {
    vector<Token> t; size_t p = 0; vector<CompileError>& errs;
    Parser(vector<Token> tk, vector<CompileError>& e) : t(tk), errs(e) {}
    Token& cur() { return t[p]; }
    void err(const string& m) { errs.push_back({"SYNTAX", m, cur().line, cur().col}); }
    void sync() { while (cur().type != "EOF" && cur().lex != ";") p++; if (cur().lex == ";") p++; }
    vector<NP> program() {
        vector<NP> stmts;
        while (cur().type != "EOF") {
            size_t before = errs.size();
            NP s = statement();
            if (s && errs.size() == before) stmts.push_back(s); else if (errs.size() == before) {} else sync();
        }
        return stmts;
    }
    NP statement() {
        if (cur().type != "IDENTIFIER") { err("expected identifier at start of statement, found '" + cur().lex + "'"); return nullptr; }
        Token id = cur(); p++;
        if (cur().type != "ASSIGNMENT") { err("expected '=' after '" + id.lex + "'"); return nullptr; }
        p++;
        NP e = expr(0); if (!e) return nullptr;
        if (cur().lex != ";") { errs.push_back({"SYNTAX", "missing ';' at end of statement", t[p>0?p-1:0].line, t[p>0?p-1:0].col + (int)t[p>0?p-1:0].lex.size()}); return nullptr; }
        p++;
        return mk("Assign", id.lex, nullptr, e, id.line, id.col);
    }
    int prec(const string& o) { return (o == "+" || o == "-") ? 1 : 2; }
    NP expr(int minp) {
        NP lhs = unary(); if (!lhs) return nullptr;
        while (cur().type == "OPERATOR" && prec(cur().lex) > minp) {
            Token op = cur(); p++;
            NP rhs = expr(prec(op.lex)); if (!rhs) return nullptr;   // left-assoc
            lhs = mk("Binary", op.lex, lhs, rhs, op.line, op.col);
        }
        return lhs;
    }
    NP unary() {
        if (cur().type == "OPERATOR" && cur().lex == "-") { Token o = cur(); p++; NP e = unary(); return e ? mk("Unary", "-", nullptr, e, o.line, o.col) : nullptr; }
        return primary();
    }
    NP primary() {
        Token k = cur();
        if (k.type == "INTEGER" || k.type == "FLOAT") { p++; return mk("Num", k.lex, nullptr, nullptr, k.line, k.col); }
        if (k.type == "IDENTIFIER") { p++; return mk("Var", k.lex, nullptr, nullptr, k.line, k.col); }
        if (k.lex == "(") {
            p++; NP e = expr(0); if (!e) return nullptr;
            if (cur().lex != ")") { err("unbalanced parenthesis: expected ')'"); return nullptr; }
            p++; return e;
        }
        err("unexpected token '" + (k.type == "EOF" ? string("end of input") : k.lex) + "' in expression"); return nullptr;
    }
};

void printAST(const NP& n, const string& pre, bool last, bool root) {
    if (!n) return;
    string label = n->kind == "Assign" ? "Assign(" + n->val + ")" : n->kind == "Binary" ? "Binary(" + n->val + ")" : n->kind == "Unary" ? "Unary(-)" : n->kind == "Num" ? "Num(" + n->val + ")" : "Var(" + n->val + ")";
    cout << pre << (root ? "" : (last ? "`-- " : "|-- ")) << label << "\n";
    string np = pre + (root ? "" : (last ? "    " : "|   "));
    if (n->l && n->r) { printAST(n->l, np, false, false); printAST(n->r, np, true, false); }
    else if (n->r) printAST(n->r, np, true, false);
}

// ---------- Symbol table + semantic analysis ----------
struct Sym { string type; int line; double value; bool known; };
map<string, Sym> symtab;

string semType(const NP& n, vector<CompileError>& errs) {
    if (n->kind == "Num") return n->val.find('.') != string::npos ? "float" : "int";
    if (n->kind == "Var") {
        auto it = symtab.find(n->val);
        if (it == symtab.end()) { errs.push_back({"SEMANTIC", "variable '" + n->val + "' used before definition", n->line, n->col}); return "error"; }
        return it->second.type;
    }
    if (n->kind == "Unary") return semType(n->r, errs);
    string a = semType(n->l, errs), b = semType(n->r, errs);
    if (a == "error" || b == "error") return "error";
    if (n->val == "/" && n->r->kind == "Num" && stod(n->r->val) == 0) { errs.push_back({"SEMANTIC", "division by constant zero", n->line, n->col}); return "error"; }
    return (a == "float" || b == "float") ? "float" : "int";
}

// ---------- TAC ----------
struct Quad { string op, a1, a2, res; };
int tmp = 0;
string newT() { return "t" + to_string(++tmp); }
string gen(const NP& n, vector<Quad>& q) {
    if (n->kind == "Num" || n->kind == "Var") return n->val;
    if (n->kind == "Unary") { string a = gen(n->r, q), t = newT(); q.push_back({"neg", a, "", t}); return t; }
    string a = gen(n->l, q), b = gen(n->r, q), t = newT();
    q.push_back({n->val, a, b, t}); return t;
}
bool isNum(const string& s) { return !s.empty() && (isdigit((unsigned char)s[0]) || (s[0]=='-' && s.size()>1)); }
string fmt(double v) { ostringstream o; if (v == floor(v) && fabs(v) < 1e15) o << (long long)v; else o << v; return o.str(); }
void printTAC(const vector<Quad>& q) {
    int i = 0;
    for (auto& x : q) {
        cout << setw(2) << ++i << ": ";
        if (x.op == "=") cout << x.res << " = " << x.a1 << "\n";
        else if (x.op == "neg") cout << x.res << " = -" << x.a1 << "\n";
        else cout << x.res << " = " << x.a1 << " " << x.op << " " << x.a2 << "\n";
    }
}
void printQuads(const vector<Quad>& q) {
    cout << left << setw(6) << "OP" << setw(8) << "ARG1" << setw(8) << "ARG2" << "RESULT\n";
    for (auto& x : q) cout << left << setw(6) << x.op << setw(8) << (x.a1.empty()?"-":x.a1) << setw(8) << (x.a2.empty()?"-":x.a2) << x.res << "\n";
}

// ---------- Optimizer: constant folding + copy propagation + dead temp elimination ----------
int foldCount = 0, copyCount = 0;
vector<Quad> optimize(vector<Quad> q) {
    map<string, string> cp;                     // temp -> value (constant or var)
    vector<Quad> out;
    for (auto x : q) {
        if (cp.count(x.a1)) x.a1 = cp[x.a1];
        if (cp.count(x.a2)) x.a2 = cp[x.a2];
        if (x.op == "neg" && isNum(x.a1)) { cp[x.res] = fmt(-stod(x.a1)); foldCount++; continue; }
        if (x.op != "=" && x.op != "neg" && isNum(x.a1) && isNum(x.a2)) {
            double a = stod(x.a1), b = stod(x.a2), r = 0;
            if (x.op == "+") r = a + b; else if (x.op == "-") r = a - b; else if (x.op == "*") r = a * b; else r = b != 0 ? a / b : 0;
            bool fl = x.a1.find('.') != string::npos || x.a2.find('.') != string::npos;
            string s = (x.op == "/" && !fl) ? fmt(trunc(r)) : fmt(r);
            cp[x.res] = s; foldCount++; continue;
        }
        if (x.op == "=" && x.a1[0] == 't' && cp.count(x.a1)) x.a1 = cp[x.a1];
        out.push_back(x);
    }
    // copy propagation: "tN = expr ; v = tN"  ->  "v = expr"
    vector<Quad> res;
    for (size_t i = 0; i < out.size(); i++) {
        if (i + 1 < out.size() && out[i+1].op == "=" && out[i+1].a1 == out[i].res && out[i].res[0] == 't' && out[i].op != "=") {
            Quad m = out[i]; m.res = out[i+1].res; res.push_back(m); copyCount++; i++;
        } else res.push_back(out[i]);
    }
    return res;
}

// ---------- Interpreter over optimized TAC ----------
map<string, double> env;
bool runTAC(const vector<Quad>& q, vector<CompileError>& errs) {
    auto val = [&](const string& s) { return isNum(s) ? stod(s) : env[s]; };
    for (auto& x : q) {
        double r;
        if (x.op == "=") r = val(x.a1);
        else if (x.op == "neg") r = -val(x.a1);
        else {
            double a = val(x.a1), b = val(x.a2);
            if (x.op == "/" && b == 0) { errs.push_back({"RUNTIME", "division by zero while evaluating " + x.a1 + " / " + x.a2, 0, 0}); return false; }
            r = x.op == "+" ? a + b : x.op == "-" ? a - b : x.op == "*" ? a * b : a / b;
        }
        env[x.res] = r;
    }
    return true;
}

int main(int argc, char** argv) {
    string src, line; while (getline(cin, line)) src += line + "\n";
    cout << "SOURCE\n" << string(52, '-') << "\n" << src;
    vector<CompileError> errs;
    auto toks = lexer(src, errs);
    cout << "\n[1] TOKEN STREAM  (" << toks.size() - 1 << " tokens)\n" << string(52, '-') << "\n";
    for (auto& k : toks) if (k.type != "EOF") cout << left << setw(8) << k.lex << setw(14) << k.type << k.line << ":" << k.col << "\n";
    auto report = [&](const string& stage) {
        if (errs.empty()) return false;
        cout << "\n" << stage << " ERRORS\n" << string(52, '-') << "\n";
        for (auto& e : errs) { cout << "[" << e.phase << "] "; if (e.line) cout << "line " << e.line << ", col " << e.col << ": "; cout << e.msg << "\n"; }
        cout << "\nCompilation halted: " << errs.size() << " error(s).\n"; return true;
    };
    if (report("LEXICAL")) return 1;
    Parser ps(toks, errs); auto prog = ps.program();
    if (report("SYNTAX")) return 1;
    cout << "\n[2] ABSTRACT SYNTAX TREE\n" << string(52, '-') << "\n";
    for (auto& s : prog) printAST(s, "", true, true);
    cout << "\n[3] SEMANTIC ANALYSIS + SYMBOL TABLE\n" << string(52, '-') << "\n";
    for (auto& s : prog) {
        string ty = semType(s->r, errs);
        if (ty != "error") symtab[s->val] = {ty, s->line, 0, false};
    }
    if (!errs.empty()) { report("SEMANTIC"); return 1; }
    cout << left << setw(8) << "NAME" << setw(8) << "TYPE" << "DEFINED@\n";
    for (auto& kv : symtab) cout << left << setw(8) << kv.first << setw(8) << kv.second.type << "line " << kv.second.line << "\n";
    vector<Quad> tac;
    for (auto& s : prog) { string r = gen(s->r, tac); tac.push_back({"=", r, "", s->val}); }
    cout << "\n[4] THREE-ADDRESS CODE  (unoptimized, " << tac.size() << " instr)\n" << string(52, '-') << "\n"; printTAC(tac);
    auto opt = optimize(tac);
    cout << "\n[5] OPTIMIZED TAC  (" << opt.size() << " instr | folded: " << foldCount << ", copies removed: " << copyCount << ")\n" << string(52, '-') << "\n"; printTAC(opt);
    cout << "\nQUADRUPLES\n" << string(52, '-') << "\n"; printQuads(opt);
    cout << "\n[6] EXECUTION RESULT\n" << string(52, '-') << "\n";
    if (!runTAC(opt, errs)) { report("RUNTIME"); return 1; }
    for (auto& kv : symtab) cout << kv.first << " = " << fmt(env[kv.first]) << "\n";
    return 0;
}
