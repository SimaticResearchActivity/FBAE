# boost_subset_for_asio
Subset of Boost (https://www.boost.org) to compile boost.asio files

This directory contains the subset of [boost](https://www.boost.org) required to compile programs using `Boost.Asio` without downloading the whole Boost.

## Procedure to create initial version

1. Download `boost` from its website into a directory called *original `boost`
   directory* afterwards.
2. In `boost` directory under the current directory containing this `README.md`
   file (called *`boost` subset directory* hereafter), copy `asio` directory from original `boost` directory.
3. In `/tmp` directory, create a `CPP` program which contains 
   `#include <boost/asio.hpp>`
4. Compile this program. If there are compilation errors because of missing    
   include files, copy missing files/directory form original `boost` directory 
   to `boost` subset directory.
5. Proceed until there are no more compilation errors.
6. Afterwards try to compile your `CPP` program on other targeted platforms (e.g.
   Windows, macOS) to check that your `boost` subset directory is indeed 
   complete.

## Procedure to evolve to a new boost version

1. Download new `boost` version form its website into a directory called
   `absolute_path_to_new_boost` hereafter.
2. Apply the following procedure:

```shell
cd boost_subset_directory # where boost_subset_directory` is `boost` directory under the current directory containing this `README.md` file
diff -ur . ~/Software/boost_1_84_0/boost | grep -v 'Only in' > /tmp/my.patch
patch -p1 < /tmp/my.patch
```

3. Check that all files were correctly modified by invoking command 
   `diff -ur . ~/Software/boost_1_84_0/boost | grep -v 'Only in'` : This command
   must produce no output.
4. Compile a `CPP` program which contains `#include <boost/asio.hpp>`
5. If there are compilation errors because of missing    
   include files, copy missing files/directory form original `boost` directory 
   to `boost` subset directory.
