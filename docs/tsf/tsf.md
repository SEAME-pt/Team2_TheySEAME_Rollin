# Trustable Software Framework (TSF) for Student Automotive Projects  
> Using YAML-based Requirements & Module Documentation

## Overview

This document describes how to apply a simplified **Trustable Software Framework (TSF)** in an **automotive project**.  
The approach focuses on:
- Lightweight tooling
- Version-controlled artifacts
- Clear traceability
- Automotive-inspired workflow (ISO 26262 concepts)

The TSF workflow ensures that requirements are well-defined, implemented in modules, and verified using associated test cases.

This process keeps the spirit of professional safety-critical development—without requiring expensive commercial tools.

---

## Goals

- Ensure traceability between requirements → design → code → tests
- Enforce disciplined software development
- Support incremental development
- Enable automated verification via CI
- Maintain required artifacts in version control

---

# Requirement Types

A TSF-aligned project categorizes requirements into four levels of abstraction:

| Type | Purpose | Example |
|------|---------|---------|
| Client (Stakeholder) | High-level customer needs | The vehicle must not skid under braking |
| System | What the total system must achieve | The braking system shall modulate pressure |
| Software | Requirements allocated to software | The ABS module shall sample wheel speed @ ≥1 kHz |
| Test Case | How requirements are verified | Verify sample rate ≥1 kHz |

Each requirement is uniquely tracked, traceable, and versioned.

---

# Project Folder Structure

```
├── requirements/
│   ├── client/
│   │   ├── REQ-CL-001.yaml
│   │   └── ...
│   ├── system/
│   │   ├── REQ-SYS-001.yaml
│   │   └── ...
│   ├── software/
│   │   ├── REQ-SW-001.yaml
│   │   └── ...
│   └── testcases/
│       ├── TC-ABS-001.yaml
│       └── ...
│
├── modules/
│   ├── abs_controller/
│   │   ├── module.yaml
│   │   ├── src/
│   │   └── tests/
│   ├── wheel_speed_sensor/
│   └── ...
│
├── docs/
│   ├── architecture.md
│   └── design/
│
├── src/
├── tests/
└── .github/workflows/
```

---

# Requirements in YAML

All requirements live in version control.  
Each requirement exists as a YAML file containing:

- ID
- Type: client/system/software/test
- Description
- Priority level
- Link to parent Requirement
- Link to sub-requirements
- Links to implementing modules
- Links to test cases
- Change history

### Example: Client Requirement

`requirements/client/REQ-CL-001.yaml`
```yaml
id: REQ-CL-001
type: client
title: Vehicle must remain steerable during braking
description: >
  The vehicle must maintain steering capability during braking events.
status: approved
system_reqs:
  - REQ-SYS-001
version: 1.0
```

#### Example: System Requirement

`requirements/system/REQ-SYS-001.yaml`

``ỳaml
id: REQ-SYS-001
type: system
title: Anti-lock braking functionality
description: >
  The braking system shall prevent wheel lock and maintain traction.
status: approved
software_reqs:
  - REQ-SW-001
version: 1.0
```

Example: Software Requirement

`requirements/software/REQ-SW-001.yaml`

``ỳaml
id: REQ-SW-001
type: software
title: ABS slip-based pressure modulation
description: >
  The ABS module shall reduce brake pressure when slip exceeds calibrated thresholds.
modules:
  - abs_controller
test_cases:
  - TC-ABS-001
status: implemented
version: 1.1
```

#### Example: Test Case

`requirements/testcases/TC-ABS-001.yaml`

```yaml
id: TC-ABS-001
type: test_case
title: Validate ABS slip-based pressure reduction
description: >
  Verifies that pressure is reduced when wheel slip exceeds threshold.
requirements:
  - REQ-SW-001
method: unit_test
status: passed
version: 1.0
```

### Software Modules

A software module is a cohesive functional unit with:
- A defined responsibility
- Controlled interfaces
- Tests
- Requirement allocation

Each module has a module.yaml describing:
- Name
- Function
- Requirements it fulfills
- Inputs/outputs
- Test coverage
- Status

#### Example Module Definition

modules/abs_controller/module.yaml
``` yaml
name: abs_controller
description: >
  Calculates wheel slip and adjusts brake pressure.
software_reqs:
  - REQ-SW-001
source:
  - src/abs_controller.c
tests:
  - tests/test_abs_controller.py
version: 1.0
status: implemented
```

### Traceability
Traceability must cover:
Client → System → Software → Test Case
              ↓
            Module
            
The YAML structure provides mechanical traceability:
- Client → System
- System → Software
- Software → Module + Test Case
- Module → Code + Tests

Traceability can be validated automatically in CI:
- Check all software requirements map to code
- Check test cases reference valid requirements
- Summarize coverage

---

### Workflow
1. Define Requirements
- Start with client-level requirements.
- Break them down into system and software requirements.
- Store them as YAML files under /requirements.

2. Allocate to Modules

- Assign software requirements to software modules in:
  /modules/.../module.yaml

3. Implement

- Write code in that module directory.
- Ensure commit messages reference requirement IDs.

Example commit:

```feat(abs): REQ-SW-001 pressure modulation```

4. Test

- Unit tests reference:
- Requirement ID
- Test case ID

5. Review
- Pull requests must:
- Reference requirement IDs
- Document impacted modules
- Describe tests performed

6. CI Checks
- GitHub Actions may:
- Validate YAML consistency
- Ensure mappings are correct
- Run tests linked to REQ IDs
- Generate a traceability report

7. Verification

- Each requirement tracked as:
- draft → approved → implemented → verified

#### Example Pull Request Format
- Implements: REQ-SW-001
- Module: abs_controller
- Tests: TC-ABS-001
