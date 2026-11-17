# Symbolic Maths Engine

A command-line program written in C++ that parses a mathematical expression in one variable, `x`, and then differentiates it symbolically, integrates it symbolically, or approximates a definite integral numerically.

**Programming language:** C++ (C++17)

**Purpose and main functionality:**

A command-line mathematical engine that parses mathematical expressions in one variable (`x`) and performs symbolic differentiation, symbolic integration and numerical integration using Simpson's rule.

**Scale and structure:**

The project is organised into three source files (`main.cpp`, `parser.cpp` and `expression.cpp`) and a shared header file (`classes.h`). It contains several classes, structures and recursive functions responsible for parsing and manipulating mathematical expressions.

**Object-oriented programming:**

The project uses classes to organise the parser and mathematical expression functionality, with encapsulation separating different responsibilities. Expression trees are managed using `unique_ptr`, providing automatic memory management and ownership of child nodes.

**Data structures and algorithms:**

The central data structure is a binary expression tree, allowing mathematical expressions to be represented and manipulated recursively. The project also implements a recursive-descent parser, symbolic differentiation algorithms, rule-based symbolic integration and Simpson's numerical integration method.

An automated testing suite uses numerical central differences to independently verify symbolic differentiation and checks symbolic antiderivatives by differentiating them.

## Features

- **Expression parser**: reads expressions such as `x^3 + 2*x` or `sin(3*x + 1)` into an expression tree, with correct operator precedence and brackets.
- **Symbolic differentiation**: sum, difference, product, quotient and chain rules, the power rule, and the general power rule for `u^v`.
- **Symbolic integration**: sums and differences, constant multiples, powers of `x` (including `x^-1`, which gives `ln(abs(x))`), and `sin`, `cos` and `exp` of a linear argument such as `sin(3*x + 1)`.
- **Numerical integration**: Simpson's rule for definite integrals.
- **Simplifier**: folds constants and removes identities such as `x + 0`, `x * 1`, `x * 0` and `x ^ 1`.
- **Built-in tests**: an automatic test suite you can run from the menu.

## Building and running

Any C++17 compiler will do. With g++:

```
git clone https://github.com/qila-09/Symbolic-Maths-Engine.git
cd Symbolic-Maths-Engine
g++ -std=c++17 source/main.cpp source/parser.cpp source/expression.cpp -o engine
.\engine.exe
```

## Using the program

The program shows a menu:

```
1. Differentiate an expression
2. Find a definite integral (Simpson's rule)
3. Find a symbolic integral
4. Run automatic tests
5. Exit
```

Example session:

| Menu option | Input | Output |
|---|---|---|
| 1 | `x^3 + 2*x` | `Derivative: 3 * x ^ 2 + 2` |
| 3 | `sin(3*x+1)` | `Integral: -1 * cos(3 * x + 1) / 3 + C` |
| 2 | `x^3+2*x`, limits `0` and `2` | `Approximate integral: 8` |

### Accepted syntax

Expressions can use:

- the variable `x` and the constant `pi`
- numbers, including decimals such as `2.5`
- the operators `+`, `-`, `*`, `/` and `^`, plus unary `+` and `-`
- brackets `( )`
- the functions `sin()`, `cos()`, `exp()`, `ln()` and `abs()`

The grammar is:

```
expression = term { ("+" | "-") term }
term       = unary { ("*" | "/") unary }
unary      = "+" unary | "-" unary | power
power      = primary [ "^" unary ]
primary    = number | "x" | "pi" | function "(" expression ")" | "(" expression ")"
```

`^` is right-associative, so `2^3^2` means `2^(3^2)`, and `-x^2` means `-(x^2)`. Multiplication must be written explicitly, so use `2*x`, not `2x`.

## How it works

Every expression is stored as a binary tree (`Expression` in `classes.h`). Each node has a type (`number`, `x`, an operator, or a function name), a value for number nodes, and left and right children. Trees are owned through `unique_ptr`, so memory is freed automatically.

The pipeline for each menu option is:

1. **Parse**: `Parser` is a recursive-descent parser with one method per grammar rule, which is how the precedence rules are enforced.
2. **Simplify**: the tree is simplified bottom-up.
3. **Operate**: the tree is differentiated, integrated or evaluated.
4. **Simplify and print**: the result is simplified again and converted back into text, adding brackets only where precedence requires them.

### Integration

The integrator is rule-based. For `sin`, `cos` and `exp` it checks whether the argument has the form `a*x + b` using `getLinearForm`, and divides the result by the gradient `a`. If no rule matches, it reports an error rather than returning a wrong answer.

### Numerical integration

Simpson's rule splits the range into 1000 intervals by default and weights the sample points 1, 4, 2, 4, ..., 4, 1. It is exact for polynomials up to degree 3.

## Testing

Menu option 4 runs 11 tests:

- **Derivatives (5 tests)**: the symbolic derivative is compared with a numerical central-difference estimate at `x = 0.7, 1.3, 2.1`, using a tolerance of `1e-4`. The numerical estimate does not use the differentiator, so the check is independent.
- **Integrals (5 tests)**: each antiderivative is differentiated and compared with the original function at the same three points.
- **Simpson's rule (1 test)**: the integral of `x^3 + 2*x` from 0 to 2 must equal 8.

## Project structure

| File | Contents |
|---|---|
| `classes.h` | Shared includes and constants, the `Expression` struct, the `Parser` class and all function declarations |
| `parser.cpp` | `Parser` implementation |
| `expression.cpp` | Tree construction, simplification, differentiation, evaluation, printing, integration and Simpson's rule |
| `main.cpp` | The test suite and the menu loop |

## Limitations

- Only one variable, `x`, is supported.
- There is no implicit multiplication (`2x`), no constant `e` (use `exp(1)`) and no scientific notation.
- The integrator has no integration by parts or substitution beyond linear arguments, and it cannot integrate quotients such as `1/x` (write `x^-1` instead).
- Simplification is a single bottom-up pass, so results are correct but not always in their neatest form (for example `-1 * cos(3 * x + 1) / 3`).
- Simpson's rule uses a fixed number of intervals and does not adapt to rapidly changing functions.
