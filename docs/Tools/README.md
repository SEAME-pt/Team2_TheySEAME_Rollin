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
