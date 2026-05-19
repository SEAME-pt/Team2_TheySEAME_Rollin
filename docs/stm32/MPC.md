# Model Predictive Control

Model predictive control (MPC) is an advanced method of process control that is used to control a process while satisfying a set of constraints. They rely on dynamic models of the process (e.g. bycicle model), most often linear empirical models obtained by system identification.
The main advantage of MPC is the fact that it allows the current timeslot to be optimized, while keeping future timeslots in account. 
This is achieved by optimizing a finite time-horizon, but only implementing the current timeslot and then optimizing again, repeatedly, thus differing from a linear–quadratic regulator (LQR). 

## Scope
- Lane Perception is done by Hailo and it retrives binary masks of lanes
- Based on masks we need to calculate/measure the vehicle state [$e_{y}$, $\Psi_{error}$, $v$], where $e_{y}$ is the car/lane center offset, $\Psi_{error}$ is the actual angle minus the desired angle and $v$ is the current velocity
- The MPC will compute the new steering angle and acceleration based on this state
- The acceleration can be achieved using the PID

## Goals
- Enhanced constraints: max steering, max speed (e.g., speed limit)
- Softer turns: model takes into account the Center of Mass of the vehicle and rotates around it and also modifies the speed to enhance curve description

## Working Principle
Each run the MPC will take the input state [$e_{y}$, $\Psi_{error}$, $v$] where:
- $e_{y}$ is the car/lane center offset
- $\Psi_{error}$ is the actual angle minus the desired angle
- $v$ is the current velocity

And compute:

$$\{\delta_0, a_0, \delta_1, a_1, \ldots, \delta_{N-1}, a_{N-1}\}$$

where,
- $\delta$ is the desired angle
- $a$ is the acceleration
- 
Each step \(N\) is used to compute the cost:

$$J = \sum_{k=0}^{N-1} \left(Q_e e_{y,k}^2+ Q_\psi \psi_{error,k}^2+ Q_v (v_k - v_{ref})^2+ R_\delta \delta_k^2+ R_a a_k^2\right)$$

While also enforcing the constraints.
The output state is [ $\delta$, $a$] = [ $\delta_0$, $a_0$]

## Discrete-Time System Dynamics (Local Frame)
These equations model the evolution of the input parameters over $N$ steps and can be used in the cost calculation.
Using Euler discretization with sampling time \( \Delta t \):

### Lateral error dynamics
$$e_{y,k+1} = e_{y,k} + v_k \sin(\psi_{error,k}) \Delta t$$

### Heading error dynamics
$$\psi_{error,k+1} = \psi_{error,k} + \frac{v_k}{L} \tan(\delta_k)\Delta t$$

### Velocity dynamics
$$v_{k+1} = v_k + a_k \Delta t$$

## General Overview of algorithm
At each cycle, the solver:
1) Starts with a guess for [ $\delta$, $a$]
2) Applies system dynamics → predicts trajectory
3) Computes cost J
4) Adjusts inputs iteratively (gradient/QP search/etc.)
5) Repeats until cost is minimized
