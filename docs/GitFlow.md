# GitFlow

This project uses GitFlow as it's branching strategy. This doc serves as a guideline to our GitFlow implementation

## Branch Strategy

- ```main```: Stable, releasable code only. Protected; PRs required.
- ```develop```: Sprint integration branch. Protected; all features merge here first.
- ```feature```: Short-lived branches for tasks or TSF requirements.
- ```docs```: Branches for writing documentation only.
- ```fix```: Bugfix branches targeting develop (or main if urgent).
- ```release```: Stabilization branch before tagging and shipping.

## Workflow Example

1. Create a feature branch
    git checkout -b ```feature/qt-static-ui```

2. Commit using Conventional Commits ```Add Qt status panel```

3. Push and open a Pull Request into develop
    Fill template sections: Description, Type of Change.

4. Merge with “Squash and merge”
    Keep history clean and focused.

5. Release and tag
    - Create release/, stabilize.
    - Merge to main, tag (e.g., v0.3.0-qt-ui).
    - Back-merge main → develop.

## Release Cadence

A new Release is done when ```develop``` is stable and has enough material for a release.

Release Example:
- v0.1.0-hw-bringup — Week 1
    Hardware setup, GPIO/I2C scaffolding, servo/motor smoke tests.
- v0.2.0-qt-ui — Week 3
    Basic Qt app on device, auto-start service, logging.

## Naming + Conventions

- ```Branches```: anyTypeofBranch/short-description
- ```Pull Requests```: Small and cohesive; one requirement or tightly related change set.

## Protections and CI

- ```Branch protections```: ```main``` and ```develop``` require PRs and 1+ review.
