# AGL

Link to Yocto [docs](https://docs.yoctoproject.org/)
Link to bitbake [docs](https://docs.yoctoproject.org/bitbake/2.12/index.html)
Link to dnf [docs](https://dnf.readthedocs.io/en/latest/)

## BitBake Useful Commands

### Bitbake command

Build a specific recipe

```bash
bitbake <recipe_name>
```

To run a specific task in a recipe

```bash
bitbake <recipe_name> -c <task>
```

### Bitbake-layers command

Create a layer

```bash
bitbake-layers create-layer <layer_name>
```

Add a layer to the bblayers file

```bash
bitbake-layers add-layer <layer_name>
```

Remove it from bblayers file

```bash
bitbake-layers remove-layer <layer_name>
```

To see which layers an image has

```bash
bitbake-layers show-layers
```

To show the recipes of all layers in an image

```bash
bitbake-layers show-recipes
```

To show the recipes of a particular layer of the image

```bash
bitbake-layers show-recipes -l <layer_name>
```

To search for a specific recipe

```bash
bitbake-layers show-recipes <recipe_name>
```

## Dnf command

To install a package

```bash
dnf install <package-name>
```

To uninstall a package

```bash
dnf remove <package-name>
```

To update the cache

```bash
dnf makecache
```
