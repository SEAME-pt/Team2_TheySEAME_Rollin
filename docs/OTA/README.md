# OTA Plan

This document describes the OTA workflow

## Workflow

This graph shows the current workflow for the OTA upates

```mermaid
graph TD
    Request Github API-->Parse Release Tags-->Compare requested Tags with Current ones-->If there's no updates, wait for the next cycle
    Compare requested Tags with Current ones-->If there's an updated version-->Download updates-->Run Tests-->Replace the current version
```

Some clarifications.

- The release tag in github must contain a version number of the app, like v1.0.0
- The replace step stores the old version in case there's crucial error with the latest
