# Perun Programming Language

**Update \[2021-01-01\]:** The rewrite has been abandoned and this repository is now archived.
The original version of Perun with its source code is private, but available upon request.

---

The Perun C++ compiler is __under heavy development__ as it is being rewritten from a prototype.

## Building

### Dependencies

* CMake >= 3.0
* C++14-compatible compiler (such as GCC >= 5.0 or Clang >= 3.6)

### Instructions

#### Linux & Make

```
mkdir build
cd build
cmake ..
make
cd ..
build/perun $YOUR_FILE.per
```

#### Windows

##### JetBrains CLion or Visual Studio 2017+

Just open the top-level folder as a project
and build it. You can then run the produced binary
from the command line or using the dedicated button.

_Note: VS2017 has not been tested yet but it should support
CMake as its build/project files_
