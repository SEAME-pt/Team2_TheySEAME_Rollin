# OTA Plan

This document describes the OTA workflow

## Workflow

This graph shows the current workflow for the OTA upates

```mermaid
flowchart TD
    A[Request Github API] --> B[Parse Release Tags] --> C[Compare requested Tags with Current ones] --> D{Is there any updates?};
    D -- No --> E[Do not update];
    D -- Yes --> F[Download updates] --> G[Run Tests] --> H[Replace the current version];
```

## Clarifications.

- The release tag in github must contain a version number of the app, like v1.0.0
- The replace step stores the old version in case there's crucial error with the latest
