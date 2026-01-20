# Doxygen

This project uses [Doxygen](https://www.doxygen.nl/manual/index.html) documentation

## Installation

To install **Doxygen** either follow the doxygen [docs](https://www.doxygen.nl/manual/install.html)
or install it via apt ```sudo apt install doxygen```

For **Doxygen** to generate diagrams and graphs it is also needed to install **Graphviz**.
Here is the installation [docs](https://www.graphviz.org/download/) for **Graphviz**

## Integrate Doxygen comment in code

This is a template for the Doxygen comment to put in the code.
It's preffered to put this in the header files (.h/.hpp) but it's not mandatory

- A **@brief** command is a small title for the function
- The **@param** command lists the parameters the function takes
- Then a description about what the function does and important details that need to be mentioned
- After the description, put the OFT requirements fulfiled by the function
- Then the **@return** command lists the possible returns a function has

Other **@commands** can be used if it makes sense, otherwise try to stick to this template

```c
/**
 * @brief Read a CAN frame from the Bus
 *
 * @param frame reference to a frame struct
 *
 * This function reads a CAN frame from the Bus and puts it into the
 * the can_frame reference
 *
 * [impl->dsn~comms-can-rpi-receiveMsg~2]
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
```

## Build documentation

Create a **Doxyfile** if you don't have one. If you leave the parameter empty, it will generate a file with the default name (Doxyfile)
```bash
doxygen -g <Doxyfile-name>
```

To build the docs, run the doxygen command with the path to the Doxyfile. It will generate a **html** and **latex** directories
```bash
doxygen <path-to-Doxyfile>
```

Then open the **index.html**, inside the **html** directory, with your browser

If you change any configuration inside the **Doxyfile** you need to build the docs again
