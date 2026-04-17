# Lexical Analyzer

A simple lexical analyzer (tokenizer) for C/C++ source code, written in C++.

It reads a source file, scans it character by character, and classifies each
token into a category such as keyword, identifier, operator, literal, etc. It
also detects a few common errors like unterminated strings, unterminated
multi-line comments and malformed numeric literals.

## Token categories

| Type          | Examples                                |
|---------------|-----------------------------------------|
| KEYWORD       | `int`, `float`, `if`, `return`, `class` |
| IDENTIFIER    | `pi`, `radius`, `main`                  |
| OPERATOR      | `+`, `==`, `<<`, `->`, `<<=`            |
| SYMBOL        | `(` `)` `{` `}` `;` `,` `.`             |
| INTEGER       | `10`, `42`                              |
| FLOAT         | `3.14`, `0.5`                           |
| STRING        | `"hello"`                               |
| CHARACTER     | `'a'`, `'\n'`                           |
| PREPROCESSOR  | `#include`, `#define`                   |
| COMMENT       | `// ...`, `/* ... */`                   |
| UNKNOWN       | anything that fails the rules above     |

## Build

Requires a C++ compiler with C++11 support (g++, clang, or MSVC).

```bash
g++ -std=c++11 -O2 -o lexer lexer.cpp
```

## Run

```bash
./lexer <source-file>
```

If no file is given, it falls back to `test_code.cpp` and creates a small
sample file if that one does not exist.

### Example

Input (`test_code.cpp`):

```cpp
#include <iostream>
int main() {
    float pi = 3.14;
    int radius = 10;
    // Calculate area
    float area = pi * radius * radius;
    return 0;
}
```

Output:

```
TYPE           VALUE                    LINE
--------------------------------------------------
PREPROCESSOR   #include                 1
OPERATOR       <                        1
IDENTIFIER     iostream                 1
OPERATOR       >                        1
KEYWORD        int                      2
IDENTIFIER     main                     2
SYMBOL         (                        2
SYMBOL         )                        2
SYMBOL         {                        2
KEYWORD        float                    3
IDENTIFIER     pi                       3
OPERATOR       =                        3
FLOAT          3.14                     3
SYMBOL         ;                        3
...
```

## How it works

The scanner is a hand-written state machine inside `LexicalAnalyzer::getNextToken()`.
For each call it:

1. Skips whitespace and tracks line numbers.
2. Looks at the current character and dispatches to a handler for comments,
   preprocessor directives, strings, chars, identifiers/keywords, numbers,
   operators, or symbols.
3. Uses longest-match for multi-character operators (3-char, then 2-char,
   then 1-char).
4. Returns a `Token { type, value, line }`.

Keywords, operators and symbols are kept in `std::set` containers so the
lists are easy to extend.

## Project layout

```
.
├── lexer.cpp        # the analyzer
├── test_code.cpp    # sample input file
└── README.md
```

## License

MIT
