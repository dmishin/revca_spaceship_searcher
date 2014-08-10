Margolus Binary CA Spaceship Detector
-------------------------------------

The bulk_analyzer program is a tool for automatic discovery of spaceships in the Margolus binary cellular automata, particularly the "Single Rotation".

See [online simulator](dmishin.github.io/js-revca/index.html) of such automata and [blog post](http://dmishin.blogspot.com/2013/10/reversible-cellular-automata.html) with explanation. 

Regardin the "Single Rotation" rule, see [another blog post](http://dmishin.blogspot.com/2013/11/the-single-rotation-rule-remarkably.html).

Features
========
The purpose of the program is to search spaceships in the given cellular automaton, acting on the field with Margolus neighborhood.
Spaceships can be either searched either among the patterns in the given file, or by brute-forcing.

### Searching spaceships in the pattern collection
To search spaceships in the file, use command-line option -s or --source. File is a newline-separated list of patterns, each pattern having format:

[[x1,y1],[x2,y2],...,[xn,yn]]

where xi, yi are coordinates of the cell.

### Brute-force search
Brute-force searcher sequentially tests all patterns with given cell number, starting from the most "compact" ones. The search is infinite.
Command-line option is -bN or --bruteforce=N, where N is the number of cells in the pattern. 

### Continuing brute-force search
By default, brute-force searcher every time starts search from the beginning. It is possible to continue previous search. To do this, use command-line option -B<index> or --bruteforce-start=<index> where <index> is a start index. It must be a comma-separated list of increasing integers. Current index is periodically printed by the bulk_analyzer, when performing a brute-force search.

### Rules
Only binary automata are supported, i.e. cells can have 2 states: ON and OFF. Moreover, rule must preserve empty field. Rule is given as a list of 16 integers in range 0 to 15. For the rule yo be reversible, this list must be a transposition. The searcher, however, does no requires it.

By default, when rule is not specified, "Single Rotation" rule is used, which have the code: [0,2,8,3,1,5,6,7,4,9,10,11,12,13,14,15].

### Multi-threading
By default, search is done by multiple threads, by the number of processors. This can be changed.

### Other options
See bulk_analyzer --help

### Output file
The result of the search is a library of spaceships in JSON format. It is a list of records, each record represents one spaceship. Is is the same format used by the [online simulator](http://dmishin.github.io/js-revca/index.html). To import the library, open the "Library" pane of the simulator, paste library code to the bottom text area and press "Import from JSON" button. 
Multiple libraries may be merged and processed using the "librarian" script from the [js-revca repository](https://github.com/dmishin/js-revca) (it requires node.js).

Compilation
===========

The program is written in C++11 without any additional external dependencies. All libraries: optionsparser, picojson are in the sources. As the buld system, CMake us used. On linux, the following commands can be used to build the application:

```sh
$ cd revca_spaceship_searcher
$ mkdir build
$ cd build
$ cmake cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

Testing
=======
To use tests, you need put "gtest" source tree to the 'ext/gtest' folder, and enable testing in the CMakeLists.txt.

License
=======
MIT, except for the picojson and optionsparser files.