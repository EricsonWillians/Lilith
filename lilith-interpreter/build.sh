#!/usr/bin/env bash
# =============================================================================
# Lilith Interpreter — Professional Build Script
# =============================================================================
# SPDX-License-Identifier: MIT
#
# A comprehensive, production-grade build orchestrator for the Lilith
# interpreter.  It wraps CMake with sensible defaults, cross-compiler
# support, sanitizer integration, static-analysis hooks, and a polished
# CLI experience.
#
# Usage:
#   ./build.sh [options] <command>
#
# Commands:
#   build       Configure and build the project (default)
#   clean       Remove the build directory and generated artifacts
#   test        Build and run the unit-test suite
#   examples    Build and validate all example programs
#   install     Install binaries to the system or a prefix
#   format      Check source formatting with clang-format
#   tidy        Run clang-tidy static analysis
#   dist        Create a source-distribution tarball
#   all         Full CI pipeline: clean → build → test → examples
#
# Options:
#   -c, --compiler <cc>      C compiler to use (default: $CC or cc)
#   -t, --type <type>        CMake build type:
#                            Debug | Release | RelWithDebInfo | MinSizeRel
#                            (default: Release)
#   -j, --jobs <n>           Parallel build jobs (default: $(nproc))
#   -B, --build-dir <dir>    Build directory (default: build)
#   -s, --sanitize <san>     Enable sanitizer:
#                            address | memory | undefined | thread
#   --static                 Link statically
#   --werror                 Treat warnings as errors
#   --coverage               Enable coverage instrumentation (GCC/Clang)
#   -v, --verbose            Verbose CMake / make output
#   -n, --dry-run            Print commands without executing
#   -h, --help               Show this help message
#
# Examples:
#   ./build.sh                           # Release build, all targets
#   ./build.sh -t Debug -j4              # Debug build with 4 jobs
#   ./build.sh -c clang -s address test  # Clang + ASan, then test
#   ./build.sh all                       # Full CI pipeline
# =============================================================================

set -euo pipefail
shopt -s nullglob

# ---------------------------------------------------------------------------
# Constants & Defaults
# ---------------------------------------------------------------------------
readonly SCRIPT_NAME="$(basename "$0")"
readonly PROJECT_NAME="LilithInterpreter"
readonly PROJECT_VERSION="1.0.0"
readonly SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly LOCKFILE="${SOURCE_DIR}/.build.lock"

# Defaults (may be overridden by CLI)
BUILD_TYPE="Release"
BUILD_DIR="${SOURCE_DIR}/build"
JOBS="$(nproc 2>/dev/null || echo 4)"
COMPILER="${CC:-cc}"
SANITIZER=""
STATIC_LINK="OFF"
WERROR="OFF"
COVERAGE="OFF"
VERBOSE="OFF"
DRY_RUN="OFF"
COMMAND="build"
INSTALL_PREFIX="/usr/local"

# ---------------------------------------------------------------------------
# ANSI Colours (disabled when not a TTY)
# ---------------------------------------------------------------------------
if [[ -t 1 ]]; then
    C_RESET=$'\033[0m'
    C_BOLD=$'\033[1m'
    C_DIM=$'\033[2m'
    C_RED=$'\033[31m'
    C_GREEN=$'\033[32m'
    C_YELLOW=$'\033[33m'
    C_BLUE=$'\033[34m'
    C_MAGENTA=$'\033[35m'
    C_CYAN=$'\033[36m'
else
    C_RESET=""; C_BOLD=""; C_DIM=""; C_RED=""; C_GREEN=""
    C_YELLOW=""; C_BLUE=""; C_MAGENTA=""; C_CYAN=""
fi

# ---------------------------------------------------------------------------
# Logging Primitives
# ---------------------------------------------------------------------------
log()  { printf "%s[%s]%s %s\n" "$C_DIM" "$(date '+%H:%M:%S')" "$C_RESET" "$*"; }
info() { printf "%s[INFO]%s  %s\n" "$C_BLUE" "$C_RESET" "$*"; }
ok()   { printf "%s[OK]%s    %s\n" "$C_GREEN" "$C_RESET" "$*"; }
warn() { printf "%s[WARN]%s  %s\n" "$C_YELLOW" "$C_RESET" "$*" >&2; }
err()  { printf "%s[ERROR]%s %s\n" "$C_RED" "$C_RESET" "$*" >&2; }

die() {
    err "$*"
    exit 1
}

# ---------------------------------------------------------------------------
# Banner
# ---------------------------------------------------------------------------
banner() {
    cat <<EOF
${C_BOLD}${C_CYAN}
 ╔══════════════════════════════════════════════════════════════════════╗
 ║  ${PROJECT_NAME} v${PROJECT_VERSION} Build System                               ║
 ╚══════════════════════════════════════════════════════════════════════╝${C_RESET}
EOF
}

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
show_help() {
    sed -n '/^# Usage:$/,/^# ====/p' "$0" | sed 's/^# //; s/^#$//'
    echo
    echo "Project directory: ${SOURCE_DIR}"
}

# ---------------------------------------------------------------------------
# Argument Parsing
# ---------------------------------------------------------------------------
parse_args() {
    local args
    if ! args=$(getopt -n "$SCRIPT_NAME" \
        -o c:t:j:B:s:vndh \
        -l compiler:,type:,jobs:,build-dir:,sanitize:,static,werror,coverage,verbose,dry-run,help \
        -- "$@"); then
        show_help
        exit 1
    fi

    eval set -- "$args"
    while true; do
        case "$1" in
            -c|--compiler)  COMPILER="$2"; shift 2 ;;
            -t|--type)      BUILD_TYPE="$2"; shift 2 ;;
            -j|--jobs)      JOBS="$2"; shift 2 ;;
            -B|--build-dir) BUILD_DIR="$2"; shift 2 ;;
            -s|--sanitize)  SANITIZER="$2"; shift 2 ;;
            --static)       STATIC_LINK="ON"; shift ;;
            --werror)       WERROR="ON"; shift ;;
            --coverage)     COVERAGE="ON"; shift ;;
            -v|--verbose)   VERBOSE="ON"; shift ;;
            -n|--dry-run)   DRY_RUN="ON"; shift ;;
            -h|--help)      show_help; exit 0 ;;
            --) shift; break ;;
            *) die "Internal error parsing arguments" ;;
        esac
    done

    if [[ $# -gt 0 ]]; then
        COMMAND="$1"
        shift
    fi

    # Remaining positional args (e.g. install prefix)
    if [[ $# -gt 0 ]]; then
        case "$COMMAND" in
            install) INSTALL_PREFIX="$1" ;;
        esac
    fi
}

# ---------------------------------------------------------------------------
# Preconditions
# ---------------------------------------------------------------------------
check_prerequisites() {
    local missing=()

    if ! command -v cmake &>/dev/null; then
        missing+=("cmake")
    fi
    if ! command -v make &>/dev/null; then
        missing+=("make")
    fi
    if ! command -v "$COMPILER" &>/dev/null; then
        missing+=("$COMPILER")
    fi

    if [[ ${#missing[@]} -gt 0 ]]; then
        die "Missing required tools: ${missing[*]}"
    fi

    # Validate sanitizer choice
    if [[ -n "$SANITIZER" ]]; then
        case "$SANITIZER" in
            address|memory|undefined|thread) ;;
            *) die "Invalid sanitizer '$SANITIZER'. Choose: address, memory, undefined, thread" ;;
        esac
    fi

    # Validate build type
    case "$BUILD_TYPE" in
        Debug|Release|RelWithDebInfo|MinSizeRel) ;;
        *) die "Invalid build type '$BUILD_TYPE'. Choose: Debug, Release, RelWithDebInfo, MinSizeRel" ;;
    esac
}

# ---------------------------------------------------------------------------
# Locking
# ---------------------------------------------------------------------------
acquire_lock() {
    if [[ -f "$LOCKFILE" ]]; then
        local pid
        pid="$(cat "$LOCKFILE" 2>/dev/null || echo "?")"
        if kill -0 "$pid" 2>/dev/null; then
            die "Another build is already running (PID $pid). Remove $LOCKFILE if stale."
        else
            warn "Removing stale lockfile (PID $pid no longer exists)"
            rm -f "$LOCKFILE"
        fi
    fi
    echo $$ > "$LOCKFILE"
}

release_lock() {
    rm -f "$LOCKFILE"
}

# ---------------------------------------------------------------------------
# Command Runner
# ---------------------------------------------------------------------------
run() {
    if [[ "$DRY_RUN" == "ON" ]]; then
        printf "%s[DRY]%s   %s\n" "$C_MAGENTA" "$C_RESET" "$*"
        return 0
    fi
    if [[ "$VERBOSE" == "ON" ]]; then
        log "Executing: $*"
    fi
    "$@"
}

# ---------------------------------------------------------------------------
# CMake Configuration
# ---------------------------------------------------------------------------
configure() {
    info "Configuring ${PROJECT_NAME} (${BUILD_TYPE})"
    info "  Compiler : $(command -v "$COMPILER")"
    info "  Build dir: ${BUILD_DIR}"
    info "  Jobs     : ${JOBS}"
    [[ -n "$SANITIZER" ]]  && info "  Sanitizer: ${SANITIZER}"
    [[ "$STATIC_LINK" == "ON" ]] && info "  Linking  : static"
    [[ "$WERROR" == "ON" ]]      && info "  Warnings : treated as errors"
    [[ "$COVERAGE" == "ON" ]]    && info "  Coverage : enabled"

    local cmake_opts=()
    cmake_opts+=("-S" "$SOURCE_DIR")
    cmake_opts+=("-B" "$BUILD_DIR")
    cmake_opts+=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
    cmake_opts+=("-DCMAKE_C_COMPILER=${COMPILER}")

    if [[ "$STATIC_LINK" == "ON" ]]; then
        cmake_opts+=("-DCMAKE_EXE_LINKER_FLAGS=-static")
    fi

    if [[ "$WERROR" == "ON" ]]; then
        cmake_opts+=("-DLILITH_WERROR=ON")
    fi

    if [[ -n "$SANITIZER" ]]; then
        cmake_opts+=("-DCMAKE_C_FLAGS=-fsanitize=${SANITIZER} -fno-omit-frame-pointer")
        cmake_opts+=("-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=${SANITIZER}")
    fi

    if [[ "$COVERAGE" == "ON" ]]; then
        cmake_opts+=("-DCMAKE_C_FLAGS=--coverage")
        cmake_opts+=("-DCMAKE_EXE_LINKER_FLAGS=--coverage")
    fi

    if [[ "$VERBOSE" == "ON" ]]; then
        cmake_opts+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
    fi

    run cmake "${cmake_opts[@]}"
    ok "Configuration complete"
}

# ---------------------------------------------------------------------------
# Compilation
# ---------------------------------------------------------------------------
build_project() {
    info "Building ${PROJECT_NAME} with ${JOBS} parallel job(s)"
    local make_opts=("-C" "$BUILD_DIR" "-j${JOBS}")
    [[ "$VERBOSE" == "ON" ]] && make_opts+=("VERBOSE=1")
    run make "${make_opts[@]}"
    ok "Build complete"
}

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------
run_tests() {
    info "Running unit-test suite"
    local ctest_opts=("--output-on-failure" "-j${JOBS}")
    [[ "$VERBOSE" == "ON" ]] && ctest_opts+=("-V")
    run ctest "${ctest_opts[@]}" --test-dir "$BUILD_DIR"
    ok "All tests passed"
}

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
run_examples() {
    info "Validating example programs"
    local example_dir="${SOURCE_DIR}/examples"
    local total=0 passed=0 failed=0

    if [[ ! -d "$example_dir" ]]; then
        warn "No examples directory found at ${example_dir}"
        return 0
    fi

    for f in "$example_dir"/*.lilith; do
        total=$((total + 1))
        local basename
        basename="$(basename "$f")"
        if [[ "$VERBOSE" == "ON" ]]; then
            printf "  %-40s … " "$basename"
        fi
        if "$BUILD_DIR/lilith" "$f" >/dev/null 2>&1; then
            passed=$((passed + 1))
            [[ "$VERBOSE" == "ON" ]] && printf "%sPASS%s\n" "$C_GREEN" "$C_RESET"
        else
            failed=$((failed + 1))
            [[ "$VERBOSE" == "ON" ]] && printf "%sFAIL%s\n" "$C_RED" "$C_RESET"
        fi
    done

    if [[ $failed -gt 0 ]]; then
        err "Examples: ${passed}/${total} passed, ${failed} failed"
        return 1
    fi
    ok "Examples: ${passed}/${total} passed"
}

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------
do_clean() {
    info "Removing build directory: ${BUILD_DIR}"
    if [[ -d "$BUILD_DIR" ]]; then
        run rm -rf "$BUILD_DIR"
    fi
    # Remove lockfile if it exists
    rm -f "$LOCKFILE"
    ok "Clean complete"
}

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
do_install() {
    info "Installing to prefix: ${INSTALL_PREFIX}"
    run cmake --install "$BUILD_DIR" --prefix "$INSTALL_PREFIX"
    ok "Install complete"
}

# ---------------------------------------------------------------------------
# Format Check
# ---------------------------------------------------------------------------
do_format() {
    if ! command -v clang-format &>/dev/null; then
        die "clang-format not found. Install clang-format to use this command."
    fi

    info "Checking source formatting"
    local files=()
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$SOURCE_DIR/src" "$SOURCE_DIR/tests" -maxdepth 2 -name "*.c" -o -name "*.h" -print0)

    if [[ ${#files[@]} -eq 0 ]]; then
        warn "No source files found"
        return 0
    fi

    local fail=0
    for f in "${files[@]}"; do
        if ! diff -q <(clang-format "$f") "$f" >/dev/null 2>&1; then
            err "Formatting mismatch: $f"
            fail=1
        fi
    done

    if [[ $fail -eq 1 ]]; then
        die "Format check failed. Run 'clang-format -i' on the files above."
    fi
    ok "Format check passed"
}

# ---------------------------------------------------------------------------
# Clang-Tidy
# ---------------------------------------------------------------------------
do_tidy() {
    if ! command -v clang-tidy &>/dev/null; then
        die "clang-tidy not found. Install clang-tidy to use this command."
    fi

    info "Running clang-tidy static analysis"
    local files=()
    while IFS= read -r -d '' f; do
        files+=("$f")
    done < <(find "$SOURCE_DIR/src" -maxdepth 2 -name "*.c" -print0)

    if [[ ${#files[@]} -eq 0 ]]; then
        warn "No source files found"
        return 0
    fi

    # Generate compile_commands.json if missing
    if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
        run cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        build_project
    fi

    for f in "${files[@]}"; do
        run clang-tidy -p "$BUILD_DIR" "$f"
    done
    ok "Static analysis complete"
}

# ---------------------------------------------------------------------------
# Distribution
# ---------------------------------------------------------------------------
do_dist() {
    local dist_name="${PROJECT_NAME}-${PROJECT_VERSION}"
    local dist_dir="${SOURCE_DIR}/${dist_name}"
    local tarball="${SOURCE_DIR}/${dist_name}.tar.gz"

    info "Creating source distribution: ${tarball}"

    # Staging directory
    rm -rf "$dist_dir" "$tarball"
    mkdir -p "$dist_dir"

    # Copy project tree, excluding VCS and build artifacts
    rsync -a \
        --exclude='.git' \
        --exclude='build' \
        --exclude='*.tar.gz' \
        --exclude='.build.lock' \
        "$SOURCE_DIR/" "$dist_dir/"

    run tar -czf "$tarball" -C "$SOURCE_DIR" "$dist_name"
    rm -rf "$dist_dir"

    local size
    size="$(du -h "$tarball" | cut -f1)"
    ok "Distribution created (${size}): ${tarball}"
}

# ---------------------------------------------------------------------------
# Summary Report
# ---------------------------------------------------------------------------
summary() {
    local elapsed="$1"
    echo
    printf "%s%s══════════════════════════════════════════════════════════════════════%s\n" "$C_BOLD" "$C_CYAN" "$C_RESET"
    printf "%s%s  BUILD SUMMARY%s\n" "$C_BOLD" "$C_GREEN" "$C_RESET"
    printf "%s%s══════════════════════════════════════════════════════════════════════%s\n" "$C_BOLD" "$C_CYAN" "$C_RESET"
    printf "  %-20s %s\n"   "Project"      "$PROJECT_NAME"
    printf "  %-20s %s\n"   "Version"      "$PROJECT_VERSION"
    printf "  %-20s %s\n"   "Build type"   "$BUILD_TYPE"
    printf "  %-20s %s\n"   "Compiler"     "$COMPILER"
    printf "  %-20s %s\n"   "Build dir"    "$BUILD_DIR"
    printf "  %-20s %ss\n"  "Elapsed"      "$elapsed"
    printf "  %-20s %s\n"   "Command"      "$COMMAND"
    [[ -n "$SANITIZER" ]] && printf "  %-20s %s\n" "Sanitizer" "$SANITIZER"
    printf "%s%s══════════════════════════════════════════════════════════════════════%s\n" "$C_BOLD" "$C_CYAN" "$C_RESET"
    echo
}

# ---------------------------------------------------------------------------
# Main Dispatch
# ---------------------------------------------------------------------------
main() {
    parse_args "$@"
    check_prerequisites

    # Trap to release lock on exit
    trap release_lock EXIT
    acquire_lock

    local start_time end_time elapsed
    start_time="$(date +%s)"

    banner

    case "$COMMAND" in
        build)
            configure
            build_project
            ;;
        clean)
            do_clean
            ;;
        test)
            configure
            build_project
            run_tests
            ;;
        examples)
            configure
            build_project
            run_examples
            ;;
        install)
            if [[ ! -d "$BUILD_DIR" ]]; then
                configure
                build_project
            fi
            do_install
            ;;
        format)
            do_format
            ;;
        tidy)
            do_tidy
            ;;
        dist)
            do_dist
            ;;
        all)
            do_clean
            configure
            build_project
            run_tests
            run_examples
            ;;
        *)
            die "Unknown command: $COMMAND. Run '$SCRIPT_NAME --help' for usage."
            ;;
    esac

    end_time="$(date +%s)"
    elapsed=$((end_time - start_time))

    ok "Command '$COMMAND' completed successfully"
    summary "$elapsed"
}

main "$@"
