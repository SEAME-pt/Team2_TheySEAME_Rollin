# GenAI Integration in Project Development

## Overview
This document explains how Generative AI (GenAI) tools are integrated into the development workflow of this project.  
The purpose of GenAI is to accelerate the development process by handling repetitive tasks, not to substitute the developer's critical thinking and decision making in any way!

---

## 1. Purpose of GenAI in This Project
Generative AI is used as a **development assistant** to speed up learning, code prototyping, and documentation writing.  
It improves efficiency in areas where repetitive or boilerplate tasks are common, and provides quick guidance or examples for embedded development.

AI is **not** responsible for any final design or deployment decisions — those are verified and tested manually by the development team.

---

## 2. Where AI Is Being Used

| Area | Description | AI’s Role |
|------|--------------|-----------|
| **Firmware Development** | STM32 embedded C code for I2C, UART, and sensor communication. | AI helps generate template code. |
| **Requirements Definition** | AI can be used to expand Requirements and brainstorm potential needs of a system. | AI helps generate requirements. |
| **Debugging Assistance** | Troubleshooting I2C failures, GPIO configuration, or LED logic. | AI suggests diagnostic steps, wiring verification, and HAL debugging approaches. |
| **Documentation** | Technical reports and code explanations. | AI helps format and write structured `.md` files. |
| **Testing** | Generate artificial data for testing. |... |
| **Git Workflow** | Speed up git processes like PR and Issues creation, and obtain feedback on code submits. | AI helps name Pull Requests and Commit titles. AI reviews code and provides feedback on unsafe operations and styling. |

### Firmware development
GenAI is used to:
- Draft boilerplate HAL/LL driver code (I2C, UART, GPIO)
- Create example FreeRTOS task structures
- Suggest modular approaches and architecture
- Provide examples of interrupt handling and queues
Developers must validate and test all generated code on hardware.

### Requirements Definition
GenAI can:
- Suggest additional requirements or corner cases
- Translate vague goals into structured requirements
- Provide alternative strategies or subsystems
Developers decide what becomes an official project requirement.

### Debugging assistance
GenAI can help troubleshoot:
- Pin misconfiguration
- Clock setup
- Interrupt priority issues
- Memory allocation or timing conflicts

GenAI is used by:
- Asking targeted questions
- Providing code snippets (not full source)
- Requesting potential causes or debug procedures
Developers must confirm solutions using real hardware tests.

### Documentation
GenAI can:
- Generate readable .md files
- Create module summaries
- Format technical sections
- Write change logs
  - Help structure system design documents
All documentation is reviewed and edited by developers.

### Git Workflow
We may use AI in our Git and workflow to help speed up processes, testing, and review the code better.

#### Naming commit messages, issues and pull-requests
Github automatically handles this with Copilot.

#### Review code
We can run a python script that passes a DIFF of the pull request to an AI Model like GPT or Gemini.
Example Git Action:
```
on: pull_request
jobs:
  ai_review:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v4
    - run: |
        DIFF=$(git diff origin/main)
        python review.py "$DIFF
```
#### Testing
AI can:
- Suggest unit test scenarios
- Help define Test Cases
- Generate mock data for simulation
- Write simple test skeletons

---

## 3. Responsible Use of AI

The following principles are applied when using GenAI:

1. **AI in coding** - only used for brainstorming, template generation and resolving small, localized, issues directly in code (Copilot).
2. **Never provide the entire codebase** - We won't provide Copilot the entire codebase and never ask to resolve issues in the entire program.
3. **Criticalities** - AI may help in brainstorming ideas and solutions, but the critical decision making and thinking is to be made by the developers.

---

## 4. Example Prompts used
Below are examples of safe and useful prompt categories.

✅ **Useful:**
- “Suggest a FreeRTOS task architecture for IMU + motor interface.”
- “Help write a Markdown section describing the UART driver.”
- “Summarize this PR diff into a commit message.”

❌ **Not allowed:**
- “Here is the entire repo; please fix everything.”
- “Write the whole firmware for me.”
