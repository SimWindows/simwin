# SimWindows

**SimWindows** is a 1D semiconductor device simulator. It models the coupled optical, electrical, and thermal behavior of semiconductor structures, enabling engineers and researchers to define device configurations, run physics simulations, and analyze results.

Originally developed by David W. Winston (Copyright 2013), released under the **GPL-3.0** license.

---

## What It Does

- Define layered semiconductor device structures with configurable materials
- Solve coupled electrical, thermal, and carrier transport equations in 1D
- Model optical cavity modes and photon dynamics
- Analyze device performance (I-V curves, band diagrams, carrier distributions, etc.)
- Execute batch simulation macros for parameter sweeps
- Visualize results via built-in plotting

---

## Tech Stack

| Layer | Technology |
|---|---|
| Language | C++ (UI), C (numerics / formula parser) |
| GUI Framework | OWL (Object Windows Library) — Borland C++ legacy |
| Formula Parsing | FORMULC 2.2 (by Harald Helfgott) |
| Threading | Windows threads (simulation + macro execution) |
| Build System | Borland C++ IDE (original; no modern build scripts) |

---

## Directory Structure

```
simwin11/
├── OWL/           # Windows GUI layer (MDI windows, dialogs, plotting)
│   ├── INCLUDE/   # UI header files
│   ├── simwin.cpp # Application entry point (OwlMain)
│   ├── simclien.cpp
│   ├── simdial.cpp / simcdial.cpp
│   ├── simedit.cpp
│   ├── simplot.cpp
│   ├── macclass.cpp
│   └── winfunc.cpp
│
├── NUMERIC/       # Core physics simulation engine (~23,000 LOC)
│   ├── INCLUDE/   # 28 header files, 122+ classes
│   ├── devclass.cpp   # TDevice — top-level simulator
│   ├── envclass.cpp   # TEnvironment — simulation environment
│   ├── elcclass.cpp   # TElectrical — electrical solver
│   ├── thrclass.cpp   # TThermal — thermal solver
│   ├── carclass.cpp   # TCarrier — carrier transport
│   ├── solclass.cpp   # TSolution — solution storage
│   ├── grdclass.cpp   # TGrid — spatial discretization
│   ├── matclass.cpp   # TMaterial — material properties
│   └── ...            # ~15 more specialized physics classes
│
├── Formulc/       # Embedded math formula parser/evaluator
├── RESOURCE/      # Windows bitmaps and .rc resource definitions
└── PROJECT/       # Material parameter database (MATERIAL.PRM)
```

---

## Main Entry Points

| File | Symbol | Role |
|---|---|---|
| `OWL/simwin.cpp:329` | `OwlMain()` | Application entry point |
| `NUMERIC/devclass.cpp` | `TDevice` | Top-level simulation orchestrator |
| `NUMERIC/envclass.cpp` | `TEnvironment` | Core simulation environment |
| `NUMERIC/elcclass.cpp` | `TElectrical` | Electrical property solver |
| `NUMERIC/thrclass.cpp` | `TThermal` | Thermal property solver |
| `NUMERIC/carclass.cpp` | `TCarrier` | Carrier transport solver |

---

## Code Statistics

- ~32,000 lines across 36 source files
- 122+ C++ classes in the NUMERIC module alone
- Clear separation: UI (OWL) ↔ Physics engine (NUMERIC) ↔ Utilities (Formulc)

---

## Status

This repository contains the original source code targeting Windows with the Borland OWL framework. Active work is underway to refactor and port the codebase to compile and run on modern operating systems. See [CLAUDE.md](CLAUDE.md) for the modernization plan.

---

## License

- SimWindows: GNU General Public License v3.0 — David W. Winston (2013)
- Formulc: Copyright Harald Helfgott (1995)
