# WLP4 Compiler

A compiler for **WLP4** (Waterloo Language Plus Pointers Plus Procedures), a small subset of C++ used in CS 241 (Foundations of Sequential Programs) at the University of Waterloo. The compiler turns WLP4 source code into MIPS assembly, which can then be assembled and run on the CS 241 MIPS emulator.

WLP4 supports `int` and `int*` types, procedures with parameters, pointers, dynamic allocation via `new[]` and `delete[]`, the usual control flow (`if`/`else`, `while`), and `println` for output. Every program has an entry point called `wain` that takes exactly two parameters.

## Pipeline

The compiler is a classic three-stage toolchain. Each stage is a standalone program that reads from stdin and writes to stdout, so they can be composed with shell pipes:

```
source.wlp4 → [scanner] → tokens → [parser] → parse tree → [code generator] → MIPS assembly
```

A typical end-to-end run looks like:

```bash
./scanner < program.wlp4 | ./parser | ./codegen > program.asm
```

The resulting `.asm` file links against the CS 241 MIPS runtime (`init`, `new`, `delete`, `print`) and can be assembled with the standard course assembler.

## Stages

### 1. Scanner

Reads WLP4 source from stdin and emits one token per line in the form `KIND lexeme`. Implemented as a DFA using simplified maximal munch — at each step it consumes the longest prefix of the input that ends in an accepting state, emits a token, and resumes from there. Whitespace and comments are dropped; reserved words (`int`, `wain`, `if`, `while`, `return`, `new`, `delete`, `NULL`, `println`) are recognized as distinct token kinds rather than identifiers.

### 2. Parser

An LR(1) shift-reduce parser. On startup it loads `wlp4.lr1`, the standard CS 241 parsing table format containing the grammar's terminals, non-terminals, production rules, and `(state, symbol, action, target)` transitions. It then reads the scanner's tokens from stdin and runs the standard LR algorithm with two parallel stacks — one for parser states and one for partial parse-tree nodes. After each input symbol it applies all available reductions, building tree nodes by popping children off the stack and attaching them to a fresh node labeled with the rule's left-hand side, before shifting the next symbol.

The output is a preorder dump of the parse tree in the `.wlp4i` format the code generator expects: each non-terminal line prints the rule (`expr expr PLUS term`) and each terminal line prints `KIND lexeme`. The augmented start rule `start BOF procedures EOF` is added by hand on output. On failure the parser prints `ERROR at <position>` to stderr and exits.

### 3. Code generator

Walks the parse tree in three passes and emits MIPS assembly to stdout.

**Pass 1 — symbol table construction.** Per-procedure tables map identifiers to types (`int` or `int*`) and record the ordered parameter list for each procedure. Catches redeclared identifiers, redeclared procedures, calls to undeclared procedures, and references to undeclared variables.

**Pass 2 — type checking.** Enforces the WLP4 typing rules. Some highlights:

| Construct | Rule |
|---|---|
| `&lvalue` | `lvalue` must be `int`; result is `int*` |
| `*factor` | `factor` must be `int*`; result is `int` |
| `expr + expr` | `int + int → int`, `int* + int → int*`, `int + int* → int*`; `int* + int*` is rejected |
| `expr - expr` | `int - int → int`, `int* - int → int*`, `int* - int* → int` |
| `new int[expr]` | `expr` must be `int`; result is `int*` |
| `delete[] expr` | `expr` must be `int*` |
| `println(expr)` | `expr` must be `int` |
| Comparisons | both sides must have matching type |
| Assignment | lhs and rhs must match |
| Function calls | argument types must match the declared parameter list |

**Pass 3 — code generation.** Emits MIPS using the standard CS 241 calling convention.

Register conventions:

- `$30` — stack pointer
- `$29` — frame pointer
- `$31` — return address
- `$4` — constant 4 (for stack arithmetic and pointer scaling)
- `$11` — constant 1 (used as the WLP4 NULL representation)
- `$3` — expression result
- `$5` — scratch register for binary operations

Each procedure sets `$29 = $30 - 4` on entry. Locals and parameters are addressed as offsets from `$29`. For `wain`, locals sit at negative offsets pushed onto the stack; for other procedures, parameters live at positive offsets `4 * (num_params - i)`.

The calling convention pushes `$29` and `$31`, then arguments left to right, then `jalr`s to the procedure label `F<name>`. The caller pops everything back after return; the callee restores `$30 = $29 + 4` and `jr $31`s.

Pointer arithmetic scales integers by 4 using `mult`/`mflo` before adding. Pointer subtraction divides the byte difference by 4. Pointer comparisons use unsigned `sltu` rather than signed `slt`. `new int[n]` calls the runtime `new` and falls back to `$11` (NULL) on allocation failure; `delete[]` is skipped when the pointer is NULL. Control flow uses a monotonically increasing label counter to generate unique labels (`else1`, `endif1`, `loop1`, `endWhile1`, …).

## Repository layout

```
.
├── scanner.cc      # Stage 1: WLP4 source → tokens
├── parser.cc       # Stage 2: tokens → parse tree (.wlp4i)
├── codegen.cc      # Stage 3: parse tree → MIPS assembly
├── wlp4.lr1        # LR(1) parsing table consumed by the parser
└── README.md
```

(File names may differ — adjust as needed.)

## Build

Each stage is a single translation unit and can be built independently with C++14:

```bash
g++ -std=c++14 -O2 -o scanner scanner.cc
g++ -std=c++14 -O2 -o parser  parser.cc
g++ -std=c++14 -O2 -o codegen codegen.cc
```

The parser expects `wlp4.lr1` in its working directory.

## Running

End-to-end compilation of a WLP4 source file:

```bash
./scanner < program.wlp4 | ./parser | ./codegen > program.asm
```

Each stage can also be run on its own to inspect the output of an earlier stage — useful for debugging:

```bash
./scanner < program.wlp4              # see the token stream
./scanner < program.wlp4 | ./parser   # see the parse tree
```

## Errors

Each stage writes diagnostics to stderr.

- **Scanner:** rejects malformed tokens (e.g. integer literals out of range, illegal characters).
- **Parser:** prints `ERROR at <position>` when no shift action is available for the current `(state, symbol)` pair.
- **Code generator:** reports redeclarations, undeclared identifiers, type mismatches, and signature mismatches with `ERROR:`-prefixed messages.

## Notes

- The compiler targets the CS 241 MIPS environment and assumes the runtime procedures `init`, `new`, `delete`, and `print` are available at link time.
- `wain`'s first parameter may be `int` or `int*`. When it is `int*`, the runtime `init` treats it as an array pointer with the length passed in `$2`.
- The pipeline matches the assignment sequence in CS 241: the scanner corresponds to A9, the parser to A8, and the code generator to A10.