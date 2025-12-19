# Remote Control

The sub-module adds Car control via a remote control

---
### Module Type:
- [ ] Module
- [x] Sub-Module

### Interacts with:
- RPI Data Processing

## Specification Items (Requirements)

```
`dsn~remote-control-read-inputs~1`

The Remote Control shall read inputs from the gamepad, updating it's state
Then it fowards it's state to the RPI Data Processing sub-module

Needs: impl, test

Covers:
- `arch~remote~control~support~1`

Status: draft
```

## Further documentation:
Available at:
- [Event Interface in the linux kernel](https://docs.kernel.org/input/input.html#event-interface)
