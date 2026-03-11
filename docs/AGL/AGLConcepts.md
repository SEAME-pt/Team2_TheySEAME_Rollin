# AGL Concepts

This is just concepts about AGL

## What's a Recipe

A recipe is a **.bb** file. This file contains information about the recipe, as well as, how to build it.
The name of the recipe is the filename without the **.bb**, so **python.bb** exposes a recipe called **python**

An example of a recipe called **myapp.bb**

```bb
SUMMARY = "Compile C file"
DESCRIPTION = "Compile C"
LICENSE = "CLOSED"

SRC_URI = "file://main.c"

S = ${WORKDIR}

do_compile() {
    ${CC} main.c -o myapp
}

do_install() {
    install -d ${D}${bindir}
    install -m 0775 myapp ${D}${bindir}
}
```

### Recipe Tasks

Recipes can have different tasks, which are executed at different times.
The Tasks always have **do_** prepended to their name

The example above shows 2 different tasks, **do_install** and **do_compile**

[Here](https://docs.yoctoproject.org/ref-manual/tasks.html) is a lists of all the tasks

## What's a layer

A layer is a collection of recipes and the layer config files.
It's directory name, normally, starts with **meta-** and it follows the directory structure represented below

In this case, we have a layer, called **meta-mylayer**, which has 2 recipes (rpi-cmdline.bb and rpi-config_git.bb)

```bash
meta-mylayer
├── COPYING.MIT
├── README.md
├── conf
│   └── layer.conf
└── recipes-raspberry
    ├── boot-files
    │   ├── files
    │   │   ├── cmdline.txt
    │   │   └── config.txt
    │   ├── rpi-cmdline.bb
    │   └── rpi-config_git.bb
```
