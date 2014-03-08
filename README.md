NetPort
=======

A portable C++ network abstraction library, design should strive to be compatible and functional.

The permissive Expat (aka MIT) license also makes it free to use in just about everything.

configure with CMake and build:

```
cmake .
make
```

Linux and Windows are tested and supported.

Including in other CMake projects
------

Download NetPort (or add as git submodule) to location in your project, `netport` for example.

Add these rules into CMake, replace `targetname` with the name of the executable/library using NetPort:

```CMake
add_subdirectory(netport)
include_directories(netport/include)

target_link_libraries(targetname netport)
```

