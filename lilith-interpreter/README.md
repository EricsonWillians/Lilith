# Lilith: The Mirror of Symmetry

Welcome to **Lilith** – a programming language where every symbol is a reflection of beauty, balance, and cosmic order. Inspired by ancient myth and modern design, Lilith challenges conventional programming paradigms by embracing an entirely punctuation–based syntax that mirrors the symmetry found in nature, art, and mathematics.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Philosophy of Lilith](#philosophy-of-lilith)
3. [Language Features](#language-features)
   - [Punctuation-Only Syntax](#punctuation-only-syntax)
   - [Symmetry and Paired Tokens](#symmetry-and-paired-tokens)
   - [Advanced Constructs](#advanced-constructs)
4. [Architecture Overview](#architecture-overview)
5. [Code Examples](#code-examples)
   - [Metaprogramming and Macros](#metaprogramming-and-macros)
   - [Lambda Expressions](#lambda-expressions)
   - [Comprehensions and Collections](#comprehensions-and-collections)
6. [Installation and Usage](#installation-and-usage)
7. [Contributing](#contributing)
8. [Roadmap](#roadmap)
9. [License](#license)
10. [Conclusion](#conclusion)

---

## Introduction

Lilith is not merely a programming language—it is a philosophical journey into the heart of symmetry and balance. Born from the idea that true power and creativity emerge when order and beauty are intertwined, Lilith invites you to write code that is as aesthetically pleasing as it is functionally powerful. Designed for blazing performance and seamless integration into C/C++ projects, Lilith leverages modern hardware with full support for multicore CPUs, GPU acceleration, and asynchronous execution.

---

## Philosophy of Lilith

### Lilith: The Mirror of Symmetry

In myth and folklore, Lilith is portrayed as a powerful, independent archetype—often associated with primordial forces and the untamed aspects of the feminine. In our language, **Lilith** is a celebration of symmetry. Every symbol, every token, every construct is chosen for its balanced, mirrored form.

- **Essence of Symmetry:**  
  Symmetry represents balance, unity, and harmony. In Lilith, every opening token has a corresponding closing token—just as `{[` is always paired with `]}`, and `( (` with `))`. Even the operators, such as `++` and `--`, mirror one another, reflecting an underlying order within the chaos.

- **The Mirror Metaphor:**  
  Imagine a mirror that returns an image in perfect balance; nothing is lost, nothing is extraneous. In Lilith, every program is a self-contained masterpiece where every block, every expression, and every function is a reflection of its complementary part.

- **A Manifesto for Creative Coding:**  
  Lilith challenges you to craft code that is both functional and beautiful—a language where aesthetics and logic coalesce into a harmonious, powerful tool.

---

## Language Features

Lilith’s design is a marriage of sophisticated grammar and high-performance execution. Its features include:

### Punctuation-Only Syntax

- **Pure Expression:**  
  All keywords and operators are composed solely of punctuation. No alphanumeric characters appear in the language’s syntax—only in literals and identifiers. This forces creativity and delivers an unmistakable, otherworldly style.

### Symmetry and Paired Tokens

- **Mirrored Delimiters:**  
  Every opening token has an exact matching closing token. For example:
  - Programs: `{[` ... `]}`
  - Blocks: `[[` ... `]]`
  - Expression grouping: `( (` ... `))`
  - Lambda definitions: `(:<` ... `>:)`
  
- **Enforced Balance:**  
  This enforced symmetry is both a design principle and a source of aesthetic pleasure—a constant reminder that every element must have its mirror.

### Advanced Constructs

- **Collections & Comprehensions:**  
  Lilith supports lists, tuples, dictionaries, and sets with a unified, symmetric syntax. For instance, a list literal is written as:
  ```lilith
  [< 1 ,, 2 ,, 3 >]
  ```
  And a comprehension may appear as:
  ```lilith
  [< x++2
      [:< x [%] [< 1 ,, 2 ,, 3 ,, -4 >] >:]
  >]
  ```

- **Functions & Lambdas:**  
  Define both synchronous and asynchronous functions with type annotations. Lambdas are expressed in a succinct, symmetric form:
  ```lilith
  (:< ((!)) [[
      [=] !# ( ! + 1 )
  ]]>:)
  ```
  Notice how every opening delimiter has its exact mirror.

- **Macros & Metaprogramming:**  
  Macros are first-class citizens in Lilith. They allow compile-time code generation and transformation, reducing boilerplate and extending language capabilities:
  ```lilith
  <%| !DEBUG ((!!)) [[
      @!(( "DEBUG: " ++ !! ))
  ]]|%>
  ```
  This macro expands at compile time, injecting debug-print functionality into your code.

- **Pattern Matching & Exception Handling:**  
  Robust constructs for error handling and data destructuring are available:
  ```lilith
  {?
    [[ ... ]]
    [! (!VAR) [/]
      [[ ... ]]
    !]
    [:~ [[ ... ]] ~:]
  ?}
  ```
  Here, the try/except/finally structure is symmetrically delimited.

- **Asynchronous Constructs:**  
  Non-blocking operations are native in Lilith. Use await expressions to suspend and resume tasks:
  ```lilith
  ~( expression )~
  ```

- **Meticulous Arithmetic:**  
  Even arithmetic operators are designed with symmetry in mind, such as the paired operators `++` and `--`, and the palindromic unary operator `:-:`.

---

## Architecture Overview

Lilith is built for high performance and modularity. Its architecture includes:

1. **Lexer & Parser:**  
   A high-performance tokenizer and syntax analyzer that convert source code into an elegant Abstract Syntax Tree (AST).

2. **Abstract Syntax Tree (AST):**  
   A modular, lazily evaluated tree that represents every aspect of your program—optimized for parallel execution and transformation.

3. **Runtime Engine:**  
   A blazing-fast interpreter or bytecode VM that executes code without a Global Interpreter Lock, thanks to lock-free structures and fine-grained concurrency.

4. **Concurrency & Scheduling:**  
   A custom thread pool with a work-stealing scheduler dynamically distributes independent tasks across CPU cores, with optional GPU offloading for compute-heavy operations.

5. **Asynchronous I/O & Event Loop:**  
   A non-blocking event loop seamlessly handles asynchronous operations, ensuring high throughput even during heavy I/O.

6. **Macro System & Metaprogramming:**  
   Compile-time macro expansion transforms your source code, allowing you to generate boilerplate automatically and extend language capabilities.

7. **Seamless C/C++ Integration:**  
   A clean C API makes embedding Lilith in C/C++ projects straightforward, enabling seamless communication between host applications and scripts.

---

## Code Examples

### 1. Metaprogramming: Debug-Print Macro

A macro defined to inline a debug print statement, fully symmetric in its design:

```lilith
<%| !DEBUG ((!!)) [[
    @!(( "DEBUG: " ++ !! ))
]]|%>
```

**Usage:**

```lilith
!DEBUG(( "Value of X is " ++ X ))
```

*Explanation:*  
- The macro `!DEBUG` is defined with a parameter block `((!!))` and a body delimited by `[[` and `]]`.
- It expands to a call of the built-in print function `@!` that prepends `"DEBUG: "` to its argument.
- Every opening token has a matching closing token, ensuring perfect symmetry.

---

### 2. Compile-Time Factorial Macro

A recursive macro that computes factorial at compile time, showcasing balanced conditionals and recursion:

```lilith
<%| !FAC ((!!)) [[
    [ ? ((!! == 0))
        [[ [=] !# 1 ]]
        :|:
        [[ [=] !# (!! * !FAC((!! - 1))) ]]
    ?]
]]|%>
```

**Usage:**

```lilith
[=] FACT_RESULT ( !FAC((5)) )
```

*Explanation:*  
- The macro `!FAC` takes one parameter `!!` and uses a conditional structure to decide between a base case and a recursive case.
- Notice the symmetric pairing: the macro is enclosed between `<%|` and `|%>`, the parameter block by `((` and `))`, and the body by `[[` and `]]`.
- When invoked with `5`, the macro expands recursively at compile time to yield `120`.

---

### 3. Lambda Expression: Anonymous Function

A succinct lambda function for adding 1, expressed in perfect symmetry:

```lilith
(:< ((!)) [[
    [=] !# ( ! + 1 )
]]>:)
```

*Explanation:*  
- The lambda begins with `(:<` and ends with `>:)`.
- Its parameter list is enclosed in `((` and `))`, and its body is enclosed in `[[` and `]]`.
- Every delimiter has a matching partner, reinforcing Lilith’s commitment to balance.

---

### 4. Comprehensions and Collections

A symmetric list comprehension that squares positive numbers from a list:

```lilith
[< 
  (| !SQUARE ((!!)) [[
      [=] !# ( !! ** 2 )
  ]]|)
  [:< !$ [%] [< 1 ,, -2 ,, 3 ,, -4 ,, 5 >] >:]
>]
```

*Explanation:*  
- A lambda `!SQUARE` is defined using the function syntax with symmetric delimiters.
- The list literal is enclosed between `[<` and `>]`, and the for-clause of the comprehension is enclosed between `[:<` and `>:]` (with the in-operator `[ % ]` inside).
- The overall structure is a cascade of balanced tokens—each opening has its matching closing.

---

## Installation and Usage

### Building the Interpreter

Lilith uses CMake for its build system. To compile Lilith:

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/lilith.git
   cd lilith/lilith-interpreter
   ```

2. **Create a Build Directory and Configure:**
   ```bash
   mkdir build && cd build
   cmake ..
   ```

3. **Compile with Maximum Parallelism:**
   ```bash
   make -j$(nproc)
   ```

### Running Lilith

After building, you will find the executable `lilith` in the build directory. Launch the interpreter in REPL mode or run a script file:
```bash
./lilith
# or
./lilith path/to/script.lilith
```

### Running Tests

Lilith includes a suite of tests to verify each module:
```bash
ctest --output-on-failure
# or run the test executable directly:
./lilith_tests
```

---

## Contributing

We welcome contributions from anyone who shares our passion for symmetry and high-performance design.
- **Report Issues:** Please open issues for bugs, feature requests, or design discussions.
- **Submit Pull Requests:** Ensure your changes follow our coding style and include tests for new features.
- **Join the Community:** Participate in our forums and chat channels to help shape the future of Lilith.

---

## Roadmap

1. **JIT Compilation:** Explore just-in-time compilation for enhanced runtime performance.
2. **Enhanced GPU Integration:** Expand GPU offloading capabilities for compute-intensive tasks.
3. **Advanced Static Analysis & Typing:** Integrate gradual typing and static analysis to catch errors early.
4. **Plugin Ecosystem:** Enable third-party extensions to further enrich Lilith’s capabilities.
5. **Improved Debugging Tools:** Enhance logging, profiling, and debugging facilities for production environments.

---

## License

Lilith is open source and distributed under the [GNU GENERAL PUBLIC LICENSE](LICENSE). We encourage you to use, modify, and share Lilith in your projects.