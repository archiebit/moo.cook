# Cook - Build Automation Tool


## About
I developed this tool for my own projects, but you can use it too.

Idea and design are simple. File with the name `recipe.ini` contains multiple instructions on how to build (cook) different files.

For example: `recipe.ini` for the cook tool itself.
```ini
# This is a comment.
# Configuration section contains root-defined variables.
# Variables can be substituted if you put brackets around it.
# Just look at the 'name' variable.
#
# Configuration has special use variables:
#     name : Configuration name
#     list : Build targets

[configuration]
name = "win-x64-dev"
tmp  = "out/<name>/tmp"
out  = "out/<name>/out"
bin  = "out/<name>/out/bin"
src  = "src"
list = "cook.exe"




# This recipe is a child of the config from the above.
# Tool will search configuration build target through its recipes.
# This recipe fits 'cook.exe'
#
# As you can see, single quots encode file path.
#
# Recipe has special use variables:
#     pattern : Wildcard template for the target search
#     output  : Result of the recipe (Useful with 'prepare' command)
#     command : Command to execute

[recipe]
pattern = "*.exe"
output  = "'<bin>/{file}'"
src     = prepare "<src>/{file}/*.cc"
args    = "-o {output} {src}"
command = "clang++ {args}"




# Do you see a 'prepare' command from the above recipe?
# It will search for all wildcarded files.
# Then cook tool finds recipe for all those files.
# Output of this recipe will be written in the 'src' variable of its 'parent' recipe (above).
[recipe]
pattern = "<src>/(file)/*.cc"
output  = "'<tmp>/(file)/{filename}.o'"
inc     = "-I 'inc'"
args    = "-c -Wall -Wextra -std=c++23 -ggdb"
command = "clang++ -o {output} '<src>/(file)/{file}' {args} {inc}"
```


## Variable substitution
There are multiple ways to substitute a variable:
- `{...}` - Search starts from the current recipe's variable set
- `(...)` - Search starts from the parent's variable set
- `<...>` - Search starts from the configuration's variable set

Example:
```ini
[recipe]
pattern = "<src>/(file)/*.cc"
```

`<src>`  - Is the configuration variable with value `src`.  
`(file)` - Is the parent variable (which has `*.exe` pattern).
The value is `cook.exe`.

Final look is `src/cook.exe/*.cc`.

> [!IMPORTANT]
> If the recipe doesn't contain the variable being searched for, the search moves to its parent.
> This procedure can go to the configuration at the end.


## Special recipe variables
When cook tool finds a recipe for its target, its creates special
variables for the recipe. For example we take the `*.exe`
recipe with `cook.exe` target.

Those are:
- `filename` - Just a file name `cook`
- `filetype` - Just a file extension `.exe`
- `file`     - The whole target `cook.exe`


## How to use?
Add cook tool to the `PATH` enviroment variable.

Parse `recipe.ini` and build __win-x64-dev__ configuration.
```
cook win-x64-dev
```

Parse other file and build __linux-x86__ configuration.
```
cook other.ini linux-x86
```