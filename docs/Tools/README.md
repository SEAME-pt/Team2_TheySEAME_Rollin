# Tools

This is a list of tools used and how to install them

## LLVM tools

This project uses LLVM tools 22.0.0 version. From the tools it uses ```clang-format``` and ```clang-tidy```

- ```clang-format```: Ensures standard style
- ```clang-tidy```: Linter tool

Install ```clang-format``` and ```clang-tidy```

```bash
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 22
sudo apt-get update
sudo apt-get install clang-format-22 clang-tidy-22
```

## Doxygen

This project uses [Doxygen](https://www.doxygen.nl/manual/index.html) documentation

### Installation

To install **Doxygen** either follow the doxygen [docs](https://www.doxygen.nl/manual/install.html)
or install it via apt ```sudo apt install doxygen```

For **Doxygen** to generate diagrams and graphs it is also needed to install **Graphviz**.
Here is the installation [docs](https://www.graphviz.org/download/) for **Graphviz**

### Build documentation

To build the docs, run the doxygen command with the path to the Doxyfile
```bash
doxygen <path-to-Doxyfile>
```
