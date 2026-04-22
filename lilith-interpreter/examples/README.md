# Lilith Example Programs

This directory contains example `.lilith` files demonstrating every language feature that the current interpreter supports. The Lilith interpreter now **parses and executes** code, including expressions, control flow, functions, classes, collections, pattern matching, and exception handling.

---

## Building the Interpreter

From the project root (`lilith-interpreter/`):

```bash
./build.sh build
```

Or manually:

```bash
cd build
cmake ..
make
```

This produces the `lilith` executable inside `build/`.

---

## Running a Single Example

```bash
./build/lilith examples/01_hello_world.lilith
```

Output:
```
=== 01_hello_world ===
Hello, World!
Program finished successfully.
```

---

## Running All Examples

From the project root:

```bash
for f in examples/*.lilith; do
    echo "=== $f ==="
    ./build/lilith "$f"
done
```

Most examples produce extensive runtime output so you can observe the interpreter executing step-by-step.

---

## Example Index

| File | Feature Category | Status |
|------|------------------|--------|
| `01_hello_world.lilith` | Program structure (`{[ … ]}`), print expression | Working |
| `02_variables.lilith` | Integer, string, boolean (`#true` / `#false`), `nil`, reassignment, tuple destructuring, string stdlib | Working |
| `03_expressions.lilith` | Arithmetic (`++` `--` `**` `//` `%%`), unary (`:-:`), comparison (`==` `!=` `<<` `>>`), grouping `((…))`, ternary conditional, math stdlib | Working |
| `04_control_flow.lilith` | `if` / `else`, `while` loops, `break`, `continue` | Working |
| `05_functions.lilith` | Function definition `(| … |)`, parameters, `return`, nested calls | Working |
| `06_async.lilith` | Async functions `(| ~ … |)`, `await` `~(…)~`, `yield` `)-? … ?-(`, HTTP fetch, JSON decode | Working |
| `07_collections.lilith` | List `[<…>]`, tuple `(<…>)`, dict `{<…>}`, set `[{…}]`, comprehensions, index `a[i]`, member `obj.prop`, list/string/json stdlib | Working |
| `08_classes.lilith` | Class definition `{| … |}`, inheritance `([: … :])`, methods, `self` | Working |
| `09_pattern_matching.lilith` | `match` statement `(-< … >-)`, literal and tuple patterns | Working |
| `10_exceptions.lilith` | `try` / `except` / `finally` blocks | Working; catches expected errors from `risky_operation`, `open`, `close` |
| `11_hpc.lilith` | Parallel `<|…|>`, GPU `<%…%>`, tensor `[#…#]`, stream `<~…~>`, memory `[^…^]` | Parses; fails at runtime on undefined HPC natives |
| `12_imports.lilith` | Module imports `<{ … }>` | Working (parsed as no-op) |
| `13_lambdas.lilith` | Lambda expressions `(:< … >:)` | Working; lambdas currently return `nil` (no implicit return) |
| `14_advanced.lilith` | Nested functions, nested HPC inside loops, async + HPC combined, complex comprehensions | Partial; fails at runtime on undefined HPC natives |

---

## Standard Library Reference

The interpreter ships with a growing native standard library. All functions use the `..` namespace separator.

### I/O & HTTP
| Function | Description |
|----------|-------------|
| `http..get(url)` | Blocking HTTP/1.0 GET. Returns response body string or `nil`. |
| `io..read(path)` | Reads entire file into a string. |
| `io..write(path, contents)` | Writes string to file. Returns `true`/`false`. |

### Math
| Function | Description |
|----------|-------------|
| `math..abs(n)` | Absolute value. |
| `math..floor(n)` | Floor. |
| `math..ceil(n)` | Ceiling. |
| `math..sqrt(n)` | Square root. |
| `math..pow(base, exp)` | Power. |
| `math..sin(n)` | Sine (radians). |
| `math..cos(n)` | Cosine (radians). |
| `math..tan(n)` | Tangent (radians). |
| `math..pi()` | Returns π. |
| `math..e()` | Returns e. |
| `math..random()` | Random float [0,1). |
| `math..random(min, max)` | Random integer [min, max). |

### String
| Function | Description |
|----------|-------------|
| `str..len(s)` | String length. |
| `str..trim(s)` | Remove leading/trailing whitespace. |
| `str..contains(s, sub)` | Returns `true` if `s` contains `sub`. |
| `str..starts_with(s, prefix)` | Returns `true` if `s` starts with `prefix`. |
| `str..ends_with(s, suffix)` | Returns `true` if `s` ends with `suffix`. |
| `str..replace(s, from, to)` | Replace all occurrences of `from` with `to`. |
| `str..substring(s, start, end)` | Extract substring `[start, end)`. |
| `str..split(s, delim)` | Split string into a list. |
| `str..join(delim, list)` | Join list of strings with delimiter. |

### List
| Function | Description |
|----------|-------------|
| `list..push(list, item)` | Append item. Returns `true`. |
| `list..pop(list)` | Remove and return last item. |
| `list..find(list, item)` | Return index of item or `-1`. |
| `list..sort(list)` | Sort list in-place (numbers or strings). |

### JSON
| Function | Description |
|----------|-------------|
| `json..encode(value)` | Encode value (nil, bool, number, string, list, dict) to JSON string. |
| `json..decode(string)` | Parse JSON string into Lilith values. |

### OS & Environment
| Function | Description |
|----------|-------------|
| `env..get(name)` | Get environment variable or `nil`. |
| `env..set(name, value)` | Set environment variable. Returns `true`/`false`. |
| `os..time()` | Unix timestamp. |
| `os..sleep(seconds)` | Sleep for N seconds. |
| `sys..exit(code)` | Terminate process. |

### Core Built-ins
| Function | Description |
|----------|-------------|
| `@!` / `print` | Print arguments separated by spaces, with newline. |
| `input(prompt?)` | Read a line from stdin. |
| `clock()` | CPU time in seconds. |
| `type(value)` | Return type name as string. |
| `len(value)` | Length of string, list, tuple, or dict. |
| `str(value)` | Convert value to string. |
| `num(value)` | Convert string to number. |

---

## Syntax Quick Reference

| Concept | Syntax |
|---------|--------|
| Program | `{[ stmt1 ][ stmt2 ]}` |
| Assignment | `target [=] expr` |
| Block | `[[ stmts ]]` |
| Expression group | `(( expr ))` |
| Function | `(| name ((params)) [[ body ]] \|)` |
| Async function | `(| ~ name ((params)) [[ body ]] \|)` |
| Class | `{| Name ([: bases :]) [[ methods ]] \|}` |
| If statement | `[?((cond)) [[then]] :|: [[else]] ?]` |
| While loop | `<+((cond)) [[ body ]]+>` |
| Return | `)- expr -(` |
| Yield | `)-? expr ?-(` |
| Break / Continue | `]-!` / `]-?` |
| Await | `~(expr)~` |
| List | `[<a,, b,, c>]` |
| Tuple | `(<a,, b,, c>)` |
| Dict | `{< "k" [:] v >}` |
| Set | `[{a,, b,, c}]` |
| Lambda | `(:<((params)) [[ body ]] >:)` |
| Match | `(-< expr >-) [<pat>] [[ body ]]` |
| Try | `{? [[try]] [! err [/] [[catch]] !] [:~ [[finally]] ~:] ?}` |
| Import | `<{ mod1,, mod2 }>` |
| Parallel | `<| ((spec)) [[ body ]] \|>` |
| GPU | `<% ((spec)) [[ body ]] %>` |
| Tensor | `[# ((spec)) [[ body ]] #]` |
| Stream | `<~ ((spec)) [[ body ]] ~>` |
| Memory | `[^ ((spec)) [[ body ]] ^]` |

---

## Notes & Limitations

* **Numbers** — Only integer literals are supported at this time. Floating-point and negative literal syntax (`-5`) are not yet lexed; use `:-:` for unary negation.
* **String escapes** — Escape sequences inside strings are not processed; write literal characters only.
* **Empty collections** — Empty list/tuple/dict/set literals may not parse correctly; include at least one element.
* **Macros, type annotations, and compile-time features** from the full grammar specification are reserved for future implementation and are not exercised here.
* **HPC blocks** — Parallel, GPU, tensor, stream, and memory blocks are parsed and their bodies are executed, but the surrounding HPC directives are currently no-ops.
* **Lambda returns** — Lambda bodies do not yet implicitly return their last expression; explicit `)- expr -(` is required inside the body to return a value.
* **HTTPS** — `http..get` supports plain HTTP only; HTTPS requires TLS which is not yet implemented.
