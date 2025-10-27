# GitFlow

This project uses GitFlow as it's branching strategy. This doc serves as a guideline to our GitFlow implementation

---

## Branch Strategy

- main
    Stable, releasable code only. Protected; PRs required.
- develop
    Sprint integration branch. Protected; all features merge here first.
- feature/
    Short-lived branches for tasks or TSF requirements.
    Examples:
    - feature/qt-static-ui
    - feature/hw-pwm-servo-calibration
    - feature/net-remote-control-ws
- fix/
    Bugfix branches targeting develop (or main if urgent).
- release/
    Stabilization branch before tagging and shipping.

---

## Workflow

1. Create a feature branch
    git checkout -b feature/qt-static-ui

2. Commit using Conventional Commits
    feat: add Qt status panel

3. Push and open a Pull Request into develop
    Fill template sections: Linked Requirements, Architecture Impact, Tests, Safety & Limits, Verification.

4. Reviews and CI must pass
    Required checks: build-app, test, lint, docs-traceability.

5. Merge with “Squash and merge”
    Keep history clean and focused.

6. Release and tag
    - Create release/, stabilize.
    - Merge to main, tag (e.g., v0.3.0-qt-ui).
    - Back-merge main → develop.

---

## Release Cadence

- v0.1.0-hw-bringup — Week 1
    Hardware setup, GPIO/I2C scaffolding, servo/motor smoke tests.
- v0.2.0-process-ci — Week 2
    GitHub Actions CI, contribution guide, PR templates, project board.
- v0.3.0-qt-ui — Week 3
    Basic Qt app on device, auto-start service, logging.
- v0.4.0-tsf-remote — Week 4
    TSF requirements + traceability; remote control with safety limits; course book PR.

---
## Naming + Conventions

- Branches  
    feature/short-description  
- Pull Requests
    Small and cohesive; one requirement or tightly related change set.

---
## Protections and CI

- Branch protections
    main and develop require PRs, 1+ review, and green checks.

- Required checks
    
    - build-app: compile Qt app
    - test: run unit/integration tests
    - lint: code + markdown link checks
---
