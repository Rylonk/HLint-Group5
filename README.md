# HLInt — a simple interpreter for the hypothetical language "HL"

Written in C++17 (no libraries beyond the standard one), so it builds with
g++, MinGW, clang, Visual Studio, or Code::Blocks / Dev-C++ (set the standard to C++17).

## Build and run

    make                      # or:  g++ -std=c++17 -O2 -o HLInt src/*.cpp
    ./HLInt samples/PROG1.HL  # Windows:  HLInt.exe samples\PROG1.HL
    make test                 # runs 31 automated checks

Run it with no argument and it asks for the file name instead.

## What it does (follows "The Process" in the specification)

| Step | Action | Result |
|------|--------|--------|
| 1 | Opens `PROG1.HL` / `PROG2.HL` / `PROG3.HL` (or any HL file) | |
| 2 | Removes all spaces | `NOSPACES.TXT` |
| 3 | Lists every reserved word and symbol, in order, with its line number | `RES_SYM.TXT` |
| 4 | Checks the syntax | prints `ERROR` or `NO ERROR(S) FOUND` on screen |
| 5 | If no errors, runs the program | program output on screen |

On `ERROR`, the details (line, column, message) go to the error stream, so the
screen shows the required `ERROR` line followed by the explanation. Every
mistake in the file is reported in a single run, not just the first.
Exit code: 0 = ok, 1 = syntax error(s), 2 = file could not be opened.

## Architecture

    source.HL
       |
       +--> FileIO::removeSpaces ------------------> NOSPACES.TXT
       |
       v
    Lexer  (Lexer.cpp)      characters -> tokens
       |  +-----------------------------------------> RES_SYM.TXT
       v
    Parser (Parser.cpp)     tokens -> syntax + semantic check -> AST (Ast.h)
       |
       |  errors?  yes -> print "ERROR" + details, stop
       v  no
    Interpreter (Interpreter.cpp)   walks the AST and executes it

| File | Responsibility |
|------|----------------|
| `src/Config.h` | Every judgement call in one place (see below) |
| `src/Token.h` | Token types: reserved words, symbols, literals |
| `src/Lexer.h/.cpp` | Turns text into tokens; reports bad characters and unterminated strings |
| `src/Ast.h` | Expression and statement node types, plus `Value` (int or double) |
| `src/Parser.h/.cpp` | Recursive-descent parser; grammar is documented at the top of `Parser.h` |
| `src/Interpreter.h/.cpp` | Executes the AST; doubles keep 2-decimal precision |
| `src/FileIO.h/.cpp` | File reading/writing, space removal, `RES_SYM.TXT` report |
| `src/main.cpp` | Wires the five steps together |

## The HL language as implemented

    x: integer;            declaration (types: integer, double)
    y: double;
    x := 5;                assignment  (plain '=' is accepted too, as the spec uses both)
    y := 4 + 2.56;         expressions: only + and -, on numbers and variables
    output << "hello";     print a string
    output << x + y;       print a value (doubles print with 2 decimals)
    if (x < 5)             one-way if: > < == !=, ONE statement, no else
        output << x;

Rules enforced by the checker:

- every variable must be declared (once) before use
- a double cannot be assigned to an integer variable (int to double is fine)
- integer literals are a single digit (0-9); double literals have at most 2 decimals
- a declaration is not allowed as the body of an `if`
- reserved words are case-insensitive (`If`, `Output` work, as in the spec's examples); variable names are case-sensitive

## Decisions the spec left open (change them in `src/Config.h`)

| Flag | Default | Meaning |
|------|---------|---------|
| `CASE_INSENSITIVE_KEYWORDS` | true | `If` / `if` / `IF` are all the same keyword |
| `ALLOW_PLAIN_EQUALS` | true | accept `x = 3;` as well as `x := 3;` |
| `ENFORCE_LITERAL_LIMITS` | true | single-digit integers, max 2 decimals |
| `KEEP_NEWLINES_IN_NOSPACES` | true | keep line breaks in NOSPACES.TXT |
| `STRIP_SPACES_IN_STRINGS` | true | spaces inside "..." are removed from NOSPACES.TXT too (the interpreter still prints strings correctly) |

Other choices: variables start at 0; each `output` ends with a new line.
Ask your instructor about the ones that matter for grading.

## Tests

`tests/cases/*.hl` are small HL programs (13 valid, 16 invalid) and `*.expected`
holds the exact screen output for each. `make test` runs them all and also
checks the contents of `NOSPACES.TXT` and `RES_SYM.TXT` for PROG3.


## Video Presentation
https://drive.google.com/file/d/1qH-7HszCeKEbynDRY0c5zmQ0z10AHaBu/view?usp=sharing
- The file was too big so it cannot be uploaded here
