# SEA:ME — GoogleTest Complete Beginner Guide

# 📘 What is GoogleTest?

**GoogleTest** (or **GTest**) is Google’s official C++ unit testing framework. It helps us ensure our code behaves correctly.

### ✔ **Unit Test**

A small, automated test that verifies a single part (“unit”) of your code — such as a function or class method.

### ✔ Why We Need It

GoogleTest helps us:

* **catch bugs early**,
* **prove** that code works as intended,
* **refactor safely**,
* **maintain a stable project**, especially long-term ones like SEA:ME or PiRacer.

### ✔ Key Features

* **Automatic Test Discovery:** GTest automatically finds tests; no manual listing.
* **Huge Assertion Library:** Check equality, booleans, ranges, floating point, strings, exceptions, and more.
* **Portable:** Works on macOS, Linux, Windows.
* **Clean Integration with CMake:** Works well with modern C++ projects.

---

# 🛠️ Project Setup (Recommended)

Here’s a clean directory structure for a PiRacer or SEA:ME project:

```
pi_racer/
├── src/            # Your main project C++ code
├── include/        # Headers
├── tests/          # All test files go here
├── lib/
│   └── googletest/ # GTest source (git submodule or downloaded)
└── CMakeLists.txt  # Root CMake file
```

---

# 🔧 Integrating GoogleTest with CMake

## Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(PiRacerTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add GoogleTest (assuming lib/googletest exists)
add_subdirectory(lib/googletest)

# Enable testing
enable_testing()

# Add tests folder
add_subdirectory(tests)
```

---

## tests/CMakeLists.txt

```cmake
add_executable(piracer_unit_tests
    test_file.cpp   # Add all your test .cpp files here
)

# Link with GoogleTest and your library
# gtest_main already provides the main() entry point

target_link_libraries(piracer_unit_tests
    PRIVATE
    GTest::gtest_main
    MyPiRacerCodeLibrary
)

include(GoogleTest)
gtest_discover_tests(piracer_unit_tests)
```

> Replace **MyPiRacerCodeLibrary** with the library containing your project code.

---

# 🏗️ Building and Running Tests

### Configure and Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run All Tests

```bash
ctest
```

Or run the executable directly:

```bash
./piracer_unit_tests
```

---

# 🧪 Writing Tests: TEST, TEST_F, TEST_P

All tests begin with:

```cpp
#include <gtest/gtest.h>
```

---

## 1️⃣ **Simple Tests — TEST()**

Use when no shared data or setup is needed.

```cpp
TEST(SuiteName, TestName) {
    EXPECT_EQ(2 + 2, 4);
}
```

**Example:**

```cpp
TEST(SensorTest, ReadsDefaultValue) {
    EXPECT_EQ(sensor.read(), 0);
}
```

---

## 2️⃣ **Fixture Tests — TEST_F()** (Most Common)

Use when multiple tests require the same object or setup.

### Steps:

1. Create a class inheriting from `::testing::Test`.
2. Add optional `SetUp()` and `TearDown()` functions.
3. Use `TEST_F()` to define tests using that fixture.

### Example Fixture

```cpp
class MotorTest : public ::testing::Test {
protected:
    void SetUp() override {
        motor = new MotorController();
    }
    void TearDown() override {
        delete motor;
    }
    MotorController* motor;
};
```

### Using the Fixture

```cpp
TEST_F(MotorTest, InitialSpeedIsZero) {
    EXPECT_EQ(0, motor->get_speed());
}
```

---

## 3️⃣ **Parameterized Tests — TEST_P()**

Run the same test logic with different values.

Example:

```cpp
class InputTest : public ::testing::TestWithParam<int> {};

TEST_P(InputTest, RangeCheck) {
    int value = GetParam();
    EXPECT_GE(value, 0);
    EXPECT_LE(value, 100);
}

INSTANTIATE_TEST_SUITE_P(
    ValidInputs,
    InputTest,
    ::testing::Values(0, 25, 50, 100)
);
```

---

# 🔍 Assertions: Plain-English Explanation

Assertions verify that conditions are true.
You use two categories:

| Type          | Prefix    | Behavior                   |
| ------------- | --------- | -------------------------- |
| **Fatal**     | `ASSERT_` | Stops the test immediately |
| **Non-Fatal** | `EXPECT_` | Logs failure but continues |

---

## Boolean Assertions

| Macro             | Meaning                          |
| ----------------- | -------------------------------- |
| `EXPECT_TRUE(x)`  | Expect x to be true              |
| `EXPECT_FALSE(x)` | Expect x to be false             |
| `ASSERT_TRUE(x)`  | Assert x is true (stop on fail)  |
| `ASSERT_FALSE(x)` | Assert x is false (stop on fail) |

---

## Comparison Assertions

| Macro             | Meaning                   |
| ----------------- | ------------------------- |
| `EXPECT_EQ(a, b)` | Expect **equal** (a == b) |
| `EXPECT_NE(a, b)` | Expect **not equal**      |
| `EXPECT_LT(a, b)` | Expect a < b              |
| `EXPECT_LE(a, b)` | Expect a <= b             |
| `EXPECT_GT(a, b)` | Expect a > b              |
| `EXPECT_GE(a, b)` | Expect a >= b             |

Every one has an `ASSERT_` version.

---

## String Assertions (C Strings)

| Macro                    | Meaning                        |
| ------------------------ | ------------------------------ |
| `EXPECT_STREQ(a, b)`     | Expect same text               |
| `EXPECT_STRNE(a, b)`     | Expect different text          |
| `EXPECT_STRCASEEQ(a, b)` | Expect equal ignoring case     |
| `EXPECT_STRCASENE(a, b)` | Expect not equal ignoring case |

---

## Floating‑Point Assertions

| Macro                      | Meaning                       |
| -------------------------- | ----------------------------- |
| `EXPECT_FLOAT_EQ(a, b)`    | Floats equal (with tolerance) |
| `EXPECT_DOUBLE_EQ(a, b)`   | Doubles equal                 |
| `EXPECT_NEAR(v, exp, eps)` | v is within ±eps of exp       |

---

## Exception Assertions

| Macro                        | Meaning               |
| ---------------------------- | --------------------- |
| `EXPECT_THROW(stmt, ExType)` | stmt throws ExType    |
| `EXPECT_ANY_THROW(stmt)`     | stmt throws something |
| `EXPECT_NO_THROW(stmt)`      | stmt throws nothing   |

---

## Death Tests

Used for code that *should crash*.

```cpp
EXPECT_DEATH(code, regex);
```

---

## Predicate Assertions (Custom Conditions)

```cpp
EXPECT_PRED1(pred, a);   // pred(a) is true
EXPECT_PRED2(pred, a, b);
```

---

# 📝 Example Test File (tests/motor_test.cpp)

```cpp
#include <gtest/gtest.h>
#include "../src/motor_controller.h"

int add(int a, int b) { return a + b; }

TEST(BasicMath, AdditionWorks) {
    EXPECT_EQ(5, add(2, 3));
    ASSERT_NE(0, add(-1, 1));
}

class MotorTest : public ::testing::Test {
protected:
    void SetUp() override {
        motor_ = new MotorController();
    }
    void TearDown() override {
        delete motor_;
    }
    MotorController* motor_;
};

TEST_F(MotorTest, InitialSpeedIsZero) {
    EXPECT_EQ(0, motor_->get_speed());
}

TEST_F(MotorTest, CanSetSpeed) {
    motor_->set_speed(50);
    ASSERT_EQ(50, motor_->get_speed());
}
```

---