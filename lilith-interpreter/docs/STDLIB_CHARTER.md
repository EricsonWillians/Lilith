# Lilith Standard Library Naming Charter

Version: 1.0  
Date: 2026-04-22

---

## Preamble

This charter governs the naming, organization, and expansion of the Lilith standard library. It exists to ensure that the library feels like a single, intentional system rather than an accumulation of convenient utilities.

---

## I. The Three Layers

The public API is organized into three layers:

### 1. Sacred Core

A minimal set of global functions that are so fundamental they do not belong to any domain.

* `@!` — the canonical print operator
* `print` — alias for `@!`
* `input` — read from stdin

**Rule:** The sacred core never grows. A new function enters the sacred core only if it is impossible to place it in a domain without violating conceptual coherence.

### 2. Namespaced Modules

All other public natives are exposed as `domain..verb`.

**Rule:** Every native function MUST belong to exactly one domain. No function may appear under two domains in the public taxonomy.

### 3. Convenience Aliases

Selected namespaced functions MAY be aliased into the global namespace for ergonomic code.

**Rule:** An alias is a convenience, not an authority. Documentation always points to the namespaced form as canonical. Aliases are not guaranteed to exist on all implementations.

Current aliases:

| Alias | Canonical |
|-------|-----------|
| `len` | `seq..len` |
| `type` | `meta..type` |
| `str` | `str..from` |
| `num` | `num..from` |
| `clock` | `time..clock` |

---

## II. Domain Registry

The following domains are canonical. New domains require a charter amendment.

| Domain | Concern | Functions |
|--------|---------|-----------|
| `core` | Sacred globals | `@!`, `print`, `input` |
| `http` | Network requests | `http..get` |
| `io` | File system | `io..read`, `io..write` |
| `sys` | Process control | `sys..exit` |
| `math` | Mathematics | `math..abs`, `math..floor`, `math..ceil`, `math..sqrt`, `math..pow`, `math..sin`, `math..cos`, `math..tan`, `math..pi`, `math..e`, `math..rand` |
| `str` | Strings | `str..from`, `str..trim`, `str..contains`, `str..starts`, `str..ends`, `str..replace`, `str..slice`, `str..split`, `str..join` |
| `list` | Lists | `list..push`, `list..pop`, `list..find`, `list..sort` |
| `json` | JSON | `json..encode`, `json..decode` |
| `env` | Environment variables | `env..get`, `env..set` |
| `os` | OS services | `os..time`, `os..sleep` |
| `meta` | Reflection | `meta..type` |
| `seq` | Generic sequences | `seq..len` |
| `time` | Timekeeping | `time..clock` |
| `num` | Numeric conversion | `num..from` |

---

## III. Naming Rules

### 3.1 Verb Form

A function name MUST be a short, concrete verb or verb phrase.

* Good: `get`, `read`, `push`, `sort`, `encode`
* Bad: `file_handler`, `data_processor`, `perform_calculation`

### 3.2 Minimal Underscores

Prefer single words. When two words are necessary, use an underscore only if omission creates ambiguity.

* `str..starts` — clear without `_with`
* `str..ends` — clear without `_with`
* `str..slice` — clearer than `sub`

### 3.3 No Domain Redundancy

A concept lives in exactly one domain. Length is `seq..len`, not `str..len` or `list..len`. Randomness is `math..rand`, not `rand` or `random`.

### 3.4 Conversion Functions

Functions that convert a value into the domain's type use `from` as the verb.

* `str..from(value)` — convert anything to string
* `num..from(value)` — convert string to number

### 3.5 Constants

Constants exposed as functions use the constant name as the verb.

* `math..pi()` — returns π
* `math..e()` — returns e

---

## IV. Type System (Gradual Typing)

Lilith supports optional runtime-checked type annotations. They are not parse-time constraints; they are validated when a function is called or returns.

### 4.1 Annotation Syntax

* Parameter type: `param (:) type_name`
* Return type: `-> type_name`

Example:
```lilith
(| add ((a (:) number,, b (:) number)) -> number [[ ... ]] |)
```

### 4.2 Annotation Vocabulary

Annotation type names MUST match the output of `meta..type` exactly. Both use the same source of truth.

Supported types: `any`, `nil`, `bool`, `number`, `string`, `list`, `tuple`, `dict`, `function`, `class`, `instance`.

`native` is visible in `meta..type` output but is not encouraged as a user-facing annotation.

### 4.3 Coarse Runtime Categories

Annotations describe broad runtime categories, not nominal types.

* `class` means "any class object," not "class Foo."
* `instance` means "any instance object," not "instance of Foo."

### 4.4 Forward Compatibility

Unknown type names are silently accepted. This preserves evolution space for user-defined types and future richer type forms.

### 4.5 Zero Overhead Principle

Unannotated code incurs no type-checking overhead. Checks only run when `param_types` or `return_type` is non-NULL.

---

## V. C Layer Separation

The C implementation uses its own naming convention (`native_math_abs`, `native_str_trim`, etc.). This is internal and does not appear to Lilith programmers.

**Rule:** The C name and the Lilith name are independent. The C name describes what the function does in C. The Lilith name describes what the user invokes.

---

## VI. Amendment Process

1. Propose a new domain or function on the project tracker.
2. Demonstrate that the proposed name follows Sections II and III.
3. Demonstrate that no existing domain already owns the concept.
4. Obtain consensus from a maintainer.
5. Update this charter and the canonical domain registry.

---

---

*This charter is a living document. It gains authority through adherence, not rigidity.*
