# Module 2 Action Plan: Advanced Driver-Assistance Systems (ADAS)

## Executive Summary

Module 2 focuses on developing a functional ADAS system including Lane Keep Assistant, Cruise Control, Object Detection/Avoidance, and Traffic Sign Recognition. The module combines theoretical understanding with practical implementation using simulation environments (CARLA), AI/ML models, and automotive-grade software practices.

**Core Objective:** Deploy autonomous driving features with proper testing, documentation, and OTA update capabilities

---

## Technologies Involved

### 1. **Sensing & Perception**
- Cameras, [Radar](#radar), [Lidar](#lidar) sensors
- Computer Vision algorithms
- Sensor fusion techniques

### 2. **Simulation Environment**
- CARLA (open-source autonomous driving simulator)
- 3D realistic environment simulation
- Multi-scenario testing capabilities

### 3. **Control Systems**
- Model Predictive Control ([MPC](#mpc))
- [PID](#pid) Controllers
- Steering and acceleration control algorithms

### 4. **AI/ML/Deep Learning**
- Object detection models ([YOLO](#yolo), [R-CNN](#r-cnn), etc.)
- Lane detection algorithms
- Traffic sign recognition ([CNN](#cnn)s)
- Deep Learning frameworks (TensorFlow, PyTorch)

### 5. **Generative AI**
- [GAN](#gan)s (Generative Adversarial Networks)
- [VAE](#vae)s (Variational Autoencoders)
- Diffusion models
- Transformer-based architectures
- Data augmentation and synthetic scenario generation

### 6. **Software Engineering**
- Qt Framework (UI development)
- OTA (Over-The-Air) updates
- Unit testing frameworks
- Version control (Git)
- TSF (Technical Safety Framework) requirements

### 7. **Communication & Protocols**
- Vehicle-to-Everything ([V2X](#v2x)) communication concepts
- CAN bus understanding (automotive networking)

---

## Module Goals Breakdown

### Goal 1: ADAS Feature Familiarization
**Priority:** Foundation
- Study key ADAS features:
  - Automatic Emergency Braking ([AEB](#aeb))
  - Lane Departure Warning ([LDW](#ldw))
  - Adaptive Cruise Control ([ACC](#acc))
  - Blind-spot Detection
  - Traffic Sign Recognition ([TSR](#tsr))
  - Parking Assistance ([PA](#pa))

**Deliverable:** Summary document with each feature's working principle

---

### Goal 2: Core Concepts Understanding
**Priority:** Foundation
- **Perception:** Sensor data acquisition and environment interpretation
- **Control:** Steering, acceleration, braking algorithms
- **Localization & Path Planning:** Position estimation and trajectory generation
- **Simulation:** Testing in virtual environments

**Deliverable:** Concept map showing relationships between components

---

### Goal 3: CARLA Setup
**Priority:** Critical
- Install and configure CARLA simulator
- Set up development environment
- Create test scenarios
- Integrate sensors (virtual)
- Validate installation with basic vehicle control

**Deliverable:** Working CARLA environment with documentation

---

### Goal 4: Model Selection & Architecture
**Priority:** Critical
- Research open-source autonomous driving models
- Evaluate obstacle detection/avoidance algorithms
- Compare model performance, accuracy, computational requirements
- Document selection criteria

**Deliverable:** Architecture Decision Record ([ADR](#adr))

---

### Goal 5: GenAI Integration
**Priority:** Advanced
- Explore GenAI tools for ADAS
- Implement data augmentation techniques
- Generate synthetic training scenarios
- Enhance model robustness

**Deliverable:** GenAI pipeline for data enhancement

---

### Goal 6: OTA Update Mechanism
**Priority:** Intermediate
- Design OTA update architecture
- Implement secure update mechanism
- Test update deployment
- Document security considerations

**Deliverable:** Functional OTA system with security documentation

---

## Expected Deliverables (End of Module)

### Core Features (Must-Have)

1. **Control Model**
   - Autonomous acceleration and steering
   - [MPC](#mpc) or equivalent control algorithm
   - Smooth trajectory following

2. **Lane Keep Assistant ([LKA](#lka))**
   - Keep vehicle centered in lane
   - Complete full lap autonomously
   - Handle curves and straight sections

3. **Basic Cruise Control ([CC](#cc))**
   - Maintain target speed
   - Speed adjustment capability
   - Integration with control model

4. **Emergency Brake Assist ([EBA](#eba))**
   - Object detection capability
   - Collision prediction
   - Automatic braking intervention

5. **Traffic Sign Recognition ([TSR](#tsr))**
   - Detect speed limit signs
   - Adjust vehicle behavior accordingly
   - Real-time recognition

### Documentation & Engineering

6. **TSF Requirements Document**
   - Define module requirements using TSF framework
   - Safety considerations
   - Performance metrics

7. **Course Book Pull Request**
   - List all requirements
   - Documentation updates
   - Module integration notes

8. **Architecture Decision Record ([ADR](#adr))**
   - Model selection justification
   - Comparative analysis
   - Technical trade-offs

9. **Qt UI Update**
   - Display ADAS status
   - Show speed, lane position, detected objects
   - User controls for features

10. **Testing Infrastructure**
    - Unit tests for all components
    - Coverage report generation
    - CI/CD integration

### Bonus Features

11. **Adaptive Cruise Control ([ACC](#acc))**
    - Maintain safe distance from lead vehicle
    - Speed adjustment based on traffic

12. **Truck Platooning**
    - Multiple vehicle coordination
    - [V2V](#v2v) communication simulation

---

## Priority Matrix

### Phase 1: Foundation - HIGH PRIORITY
```
┌─────────────────────────────────────────┐
│ 1. ADAS Concepts Study                  │
│ 2. CARLA Installation & Setup           │
│ 3. Core Technologies Research           │
│ 4. TSF Requirements                     │
└─────────────────────────────────────────┘
```

### Phase 2: Core Development - HIGH PRIORITY
```
┌─────────────────────────────────────────┐
│ 1. Model Selection & ADR                │
│ 2. Control Model Implementation (MPC)   │
│ 3. Lane Detection Algorithm             │
│ 4. Basic Object Detection               │
└─────────────────────────────────────────┘
```

### Phase 3: Feature Implementation - MEDIUM PRIORITY
```
┌─────────────────────────────────────────┐
│ 1. Lane Keep Assistant (LKA)            │
│ 2. Cruise Control (CC)                  │
│ 3. Emergency Brake Assist (EBA)         │
│ 4. Traffic Sign Recognition (TSR)       │
└─────────────────────────────────────────┘
```

### Phase 4: Integration & Testing - HIGH PRIORITY
```
┌─────────────────────────────────────────┐
│ 1. Qt UI Development                    │
│ 2. Unit Testing & Coverage              │
│ 3. System Integration Testing           │
└─────────────────────────────────────────┘
```

### Phase 5: Advanced Features - LOW PRIORITY
```
┌─────────────────────────────────────────┐
│ 1. GenAI Integration                    │
│ 2. OTA Update Mechanism                 │
│ 3. Bonus Features (ACC, Platooning)     │
└─────────────────────────────────────────┘
```

---

## Recommended Task Division

### **Bruno: CARLA Simulation Lead & testing** 🎮
**Responsibilities:**
- CARLA installation and configuration
- Create test scenarios and maps
- Sensor setup in simulation
- Integration testing environment
- Performance monitoring
- Unit test development
- Testing infrastructure setup

**Key Deliverables:**
- Working CARLA environment
- Test scenarios library
- Integration support
- Test suite

---

### **Rui/others: Control Systems Engineer** 🎯
**Responsibilities:**
- Implement control model ([MPC](#mpc) - Model Predictive Control)
- Autonomous steering and acceleration
- Vehicle dynamics modeling
- Control algorithm tuning
- Performance optimization

**Key Deliverables:**
- Control model implementation
- Tuning documentation
- Performance metrics

---

### **Hugo/others: Computer Vision & AI Specialist** 👁️
**Responsibilities:**
- Lane detection algorithm
- Object detection model selection
- Traffic sign recognition
- Model training and validation
- GenAI integration

**Key Deliverables:**
- Lane detection system
- Object detection model
- [TSR](#tsr) model
- [ADR](#adr) document

---

### **Marco/others: Software Integration Engineer** 🔧
**Responsibilities:**
- System architecture design
- Component integration
- Qt UI development
- OTA update mechanism
- Code quality and documentation

**Key Deliverables:**
- Qt cluster UI
- OTA system
- Integration documentation

---

### **Barbot: Research, Documentation Lead & Quality Assurance** 📚
**Responsibilities:**
- ADAS feature research
- Literature review
- [ADR](#adr) preparation
- Course book pull request
- Technical documentation
- Coverage report generation
- Bug tracking and resolution
- TSF requirements documentation

**Key Deliverables:**
- Research summaries
- [ADR](#adr) document
- Coverage reports
- TSF documentation
- Course book updates
- Final documentation

---

## Success Criteria

### Minimum Viable Product
- ✅ Control model operational in CARLA
- ✅ Lane Keep Assistant completes one lap
- ✅ Basic Cruise Control functional
- ✅ Emergency braking on obstacle detection
- ✅ Traffic signs detected and speed adjusted
- ✅ Basic Qt UI showing system status
- ✅ Unit tests with >70% coverage

### Stretch Goals
- ⭐ Adaptive Cruise Control implemented
- ⭐ Truck platooning demonstration
- ⭐ GenAI synthetic data pipeline
- ⭐ OTA update fully functional
- ⭐ Unit test coverage >90%

---

## Glossary of Acronyms

| Acronym | Full Name | Description |
|---------|-----------|-------------|
| <a id="acc"></a>**ACC** | Adaptive Cruise Control | Automatically adjusts vehicle speed to maintain a safe following distance from the vehicle ahead |
| <a id="adr"></a>**ADR** | Architecture Decision Record | A document that captures an important architectural decision made, along with its context and consequences |
| <a id="aeb"></a>**AEB** | Automatic Emergency Braking | System that automatically applies the brakes when an imminent collision is detected |
| <a id="cc"></a>**CC** | Cruise Control | System that automatically controls vehicle speed at a set constant value |
| <a id="cnn"></a>**CNN** | Convolutional Neural Network | Deep learning architecture primarily used for image recognition and computer vision tasks |
| <a id="eba"></a>**EBA** | Emergency Brake Assist | System that detects objects in the vehicle's path and applies braking to avoid or mitigate collisions |
| <a id="gan"></a>**GAN** | Generative Adversarial Network | Machine learning framework where two neural networks compete to generate realistic synthetic data |
| <a id="ldw"></a>**LDW** | Lane Departure Warning | System that warns the driver when the vehicle begins to move out of its lane unintentionally |
| <a id="lidar"></a>**Lidar** | Light Detection and Ranging | Sensor technology that uses laser pulses to measure distances and build 3D maps of the environment |
| <a id="lka"></a>**LKA** | Lane Keep Assistant | Active system that applies steering corrections to keep the vehicle centered within its lane |
| <a id="mpc"></a>**MPC** | Model Predictive Control | Advanced control algorithm that uses a model of the system to predict and optimize future behavior |
| <a id="pa"></a>**PA** | Parking Assistance | System that assists the driver in maneuvering the vehicle into a parking space |
| <a id="pid"></a>**PID** | Proportional-Integral-Derivative | Classic feedback control algorithm that calculates a correction based on error, its integral, and its derivative |
| <a id="r-cnn"></a>**R-CNN** | Region-based Convolutional Neural Network | Object detection model that proposes candidate regions before classifying them with a CNN |
| <a id="radar"></a>**Radar** | Radio Detection and Ranging | Sensor that uses radio waves to detect the range, speed, and angle of objects |
| <a id="tsr"></a>**TSR** | Traffic Sign Recognition | Computer vision system that identifies and interprets road signs in real time |
| <a id="v2v"></a>**V2V** | Vehicle-to-Vehicle | Wireless communication between vehicles to share speed, position, and hazard information |
| <a id="v2x"></a>**V2X** | Vehicle-to-Everything | Communication framework between a vehicle and any entity that may affect or be affected by it |
| <a id="vae"></a>**VAE** | Variational Autoencoder | Generative model that learns a compressed latent representation of data to generate new samples |
| <a id="yolo"></a>**YOLO** | You Only Look Once | Real-time object detection algorithm that processes the entire image in a single pass of the neural network |

---

*Document Version: 1.0 (Module 2 planning)*  
*Last Updated: 11/02/26*  
*Next Review: After Sprint 1 Retrospective*

