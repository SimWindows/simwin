# CLAUDE.md — SimWindows Modernization

This file guides Claude Code sessions working on the SimWindows refactoring project.

## Goal

Port SimWindows from its original Borland C++ / Windows OWL environment to compile and run on a **modern OS** (Linux and/or macOS, with Windows as a secondary target). The physics simulation core (NUMERIC) should be fully preserved — only the platform and UI layers need replacement.

---

## Project Context

- **Language:** C++ (Borland-era, pre-C++11 idioms) and C
- **Original GUI:** OWL (Object Windows Library) — Windows-only, unavailable on modern toolchains
- **Original Build:** Borland C++ IDE — no Makefile, no CMake
- **Physics engine:** `NUMERIC/` — self-contained, no external dependencies, portable C++
- **Formula parser:** `Formulc/` — plain C, already portable

---

## Modernization Strategy

### Phase 1 — Build System
- [ ] Add a `CMakeLists.txt` at the root
- [ ] Get `NUMERIC/` and `Formulc/` compiling cleanly with a modern compiler (GCC/Clang, C++17)
- [ ] Fix any Borland-specific extensions (`__cdecl`, `pascal` calling conventions, non-standard headers)
- [ ] Enable warnings (`-Wall -Wextra`) and address them systematically
- [ ] Target: build a static library from `NUMERIC/` + `Formulc/`

### Phase 2 — Isolate the Physics Core
- [ ] Identify all Windows-specific API calls inside `NUMERIC/` (threading, file I/O, etc.) and abstract them behind platform-agnostic wrappers
- [ ] Replace Windows threading (`CreateThread`, etc.) with `std::thread`
- [ ] Replace Windows file paths / IO with `std::filesystem`
- [ ] Ensure `TDevice`, `TEnvironment`, and solvers can be instantiated and run without any GUI dependency

### Phase 3 — Replace the GUI Layer
- [ ] Remove `OWL/` from the build (archive but do not delete)
- [ ] Choose a cross-platform GUI toolkit (candidates: **Qt6**, **wxWidgets**, **Dear ImGui + SDL**)
- [ ] Implement a minimal new UI that covers: device editor, simulation runner, results plotter
- [ ] Wire new UI to the `NUMERIC/` library via a clean API

### Phase 4 — Validation
- [ ] Reproduce a known-good simulation result (e.g., a simple p-n junction IV curve) and compare against reference output
- [ ] Add a CLI/headless mode for running simulations without a GUI (useful for scripting and CI)
- [ ] Write a basic test harness around `TDevice` solve cycles

---

## Key Files to Understand First

| File | Why |
|---|---|
| `NUMERIC/INCLUDE/devclass.h` | Top-level device API — this is the public interface to preserve |
| `NUMERIC/devclass.cpp` | Device orchestration logic |
| `NUMERIC/envclass.cpp` | Largest file; environment + material setup |
| `OWL/simwin.cpp` | Entry point; shows how the UI drives `TDevice` |
| `OWL/INCLUDE/simapp.h` | Main application class definition |
| `PROJECT/MATERIAL.PRM` | Material parameter file format — must remain compatible |

---

## Coding Conventions (for new code)

- C++17 minimum
- `snake_case` for new free functions and variables; preserve existing class/member names where possible to minimize diff noise
- No platform-specific headers (`<windows.h>`, `<owl/...>`) in `NUMERIC/`
- Prefer `std::` containers and algorithms over manual memory management
- Keep new GUI code strictly separated from physics code (no NUMERIC headers in UI files except through a defined API)

---

## Branch

All work happens on: `claude/explain-codebase-mlwqoo9kc8sn4eci-EGKSL`

---

## Notes / Gotchas

- Many `NUMERIC/` files use `#include <owl/...>` headers for things like `TWindow*` pointers passed in for progress callbacks — these need to be replaced with a callback interface (e.g., `std::function<void(int)>`)
- Borland uses non-standard `int16`, `uint32` typedefs — replace with `<cstdint>` equivalents
- `FORMULC` uses `setjmp`/`longjmp` for error handling — leave this as-is unless it causes issues
- The `.PRM` material file parser lives in `NUMERIC/` and uses custom tokenization — treat as a black box initially
