# Trustable Software Framework (TSF) for Student Automotive Projects  

## Overview

This document describes how to apply a simplified **Trustable Software Framework (TSF)** in an **automotive project**.  
The approach focuses on:
- Version-controlled specification items
- Clear traceability with Open Fast Trace
- Automotive-inspired workflow (ISO 26262 concepts)

The TSF workflow ensures that requirements are well-defined, implemented by components and type, and verified using associated test cases.

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
| Client/Feature | High-level features the software must have. Typically required by the Client, or deemed as necessary by the development team. | The vehicle must not skid under braking |
| Architecture | Define performance and data interface behaviour, assigning responsibilities to each software component. | The braking system shall modulate pressure |
| Design/Software | Describe the internal structure and behaviour of the software so that it achieve the higher-level requirements. | The ABS module shall sample wheel speed @ ≥1 kHz |
| Test Case | Define tests that validate the code against each requirement. Can be Unit, Integration or System Tests | Verify sample rate ≥1 kHz |

---

# Project Folder Structure

```
├── docs/
│   ├── modules/
│   │   ├── cluster.md
│   │   ├── cluster_tests.md
│   │   └── microcontroller.md
│   │   ├── microcontroller_tests.md
│   │   └── ...
│   ├── features.md
│   ├─  architecture.md
├── apps/
│  ├── cluster/
│  ├── microcontroller/
│  ├── ...
├── tests/
│  ├── cluster_latency_test.py
│  ├── ...
└── .github/workflows/
```

---

# Specification Items

"Specification Item" is a general term used to denominate all normative pieces of specifications and markers to their coverage in the implementation.

Examples:

- Feature definitions
- Requirements in a system requirement specification
- Markers in implementation and tests that signal coverage

A **Requirement** is denoted as a Specification Item, but a Specification Item is not necessarily a requirement.

## Specification Item ID

The identifier (ID) of a [specification item](#specification-item) is a project-globally unique key which is used to refer to a specification item.

The specification item ID consists of the following parts:
* [Artifact type](#specification-item-artifact-type)
* [Name](#specification-item-name)
* [Revision](#specification-item-revision)

All parts are integral to the ID. The name alone is neither unique nor complete. In OFT's native document formats the ID is represented as a character string where the three parts are separated by the tilde (`~`) symbol.

Examples:
```
    feat~show-signs-obstacles~1
    arch~cluster-show-sign-obstacles~1
    dsn~cluster-show-stop-sign~1
    test~cluster-show-stop-sign-test~4
```
The following sections explain the each of the three parts in detail.

### Specification Item Artifact Type

The artifact type serves two purposes:

1. identifying the source document type
2. identifying the position in the tracing hierarchy

Artifact types are represented by character strings consisting out of ASCII letters. No other characters are allowed.
We use the following denominations:

* `feat` - high-level feature
* `arch` - architectural requirement
* `dsn` - design requirement
* `impl` - implementation
* `test` - test case: unit, system or integration test

### Specification Item Name

The name part of the ID must be a character string consisting of Unicode letters and/or numbers separated by hyphen (`-`) . Whitespaces are not allowed.

If the requirement is a feature (feat), the name should describe roughly what it does:
```
    detect-traffic-sign
    software-updates
```

If the requirement is an architectural or design requirement, the name should start with the module name and then a brief description of what it requires.
```
    adas-detect-traffic-signs
    adas-detect-obstacles
    cluster-show-speed
```

#### Specification Item Revision

The revision number of a specification item is a positive integer number started at one.

The revision is intended to obsolete existing coverage links in case the content of a specification item semantically changed. Incrementing the revision voids all existing links to this item so that authors linking to the item know they have to check for changes and adapt the covering items.

Examples:

If you change a requirement that lists all browsers that an HTML export needs to be compatible with, you made a semantic change and should raise the revision number.

If on the other hand you only added a missing period at the end of a sentence, the requirement content did not really change and there is no need to invalidate existing coverage.

### Examples:
#### Client Requirement/Feature

`requirements/features.md`
```
`feat~report-speed-battery~1` <--- Id
The vehicle shall determine and report its ground speed and remaining battery percentage. <---- Description

Needs: arch <--- The type of Specification Item that needs to trace to
Status: approved
```

#### Architecture Requirement

`requirements/architecture.md`

```
`arch~instrument-cluster-speed~1`
The Cluster shall display speed data from the speedometer with a resolution of at least 0.1 m/s.

Component: Instrument Cluster/Speed Data            <--- Architecture component linked to this arch Specification Item

Needs: dsn

Covers:
- feat~report-speed-battery~1              <--- This Specification Item has to cover a requirement that Needs an arch type (feature)

Status: proposed
```

#### Design Requirement

`requirements/components/cluster.md`

```
`dsg~speed-information-over-can-bus~1`
Speed shall be sent via can bus with a minimum frequency of X Hz.

Needs: impl, test

Covers:
- arch~instrument-cluster-speed~1

Status: draft
```

#### Test Case

`requirements/components/cluster.md`

```
`test~speed-information-over-can-bus-over-10hz~1`
Verify that the instrument cluster displays the vehicle speed.

Covers:
- dsg~speed-information-over-can-bus~1

Script:
- tests/cluster_latency_test.py
```

### Software Modules

A software module is a cohesive functional unit with:
- A defined responsibility
- Controlled interfaces
- Tests
- Requirements allocation

Each module has a module_tests.md with all the unit, system and integration test Specification Items for traceability. There shall also be tests in a separate tests/ directory for each module.

### Test Levels Overview

This document summarizes the three primary levels of testing used in the project.

| **Level**          | **Purpose**                               | **What It Tests**              | **Example in This Project**                                      |
|--------------------|---------------------------------------------|---------------------------------|-------------------------------------------------------------------|
| **Unit Test**       | Test one function or small module           | Code in isolation                | Check `calculate_speed()` outputs the correct value               |
| **Integration Test**| Test multiple components working together   | Interfaces between modules       | RPi Instrument Cluster reads speed sent by STM32 via CAN         |
| **System Test**     | Test the entire vehicle                     | End-to-end user-visible behavior | Car displays speed with ≤0.5s latency while driving               |


### Traceability
Traceability must cover:
```
Feature/Client → Architecture/System → Software/Design → Test Case
                                     ↓
                                  Module
```

Traceability can be validated automatically in CI:
- Check all software requirements map to code
- Check test cases reference valid requirements and steps/scripts to execute
- Summarize coverage

---

### Workflow
```
Developer → Git Push → GitHub Action
                ↓
        Run traceability check
                ↓
        Run unit + module tests
                ↓
        Upload report
                ↓
       Block/Approve PR
```
