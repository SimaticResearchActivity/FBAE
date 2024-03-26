# boost_subset

This directory contains the subset of [boost](https://www.boost.org) 1.84.0 (December 13th, 2023 23:54 GMT) required to compile programs using `Boost.Asio` without downloading the whole Boost.

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

You can check that all files were correctly modified by invoking command 
`diff -ur . ~/Software/boost_1_84_0/boost | grep -v 'Only in'` : This command
must produce no output.

## History of boost_subset

Version | Date                          | Reason to change
------: | :-------------------------    | :------------------
1.84.0  | December 13th, 2023 23:54 GMT | Version 1.82.0 had problems executing asynchronous Boost.asio under Windows
1.82.0  | April 14th, 2023 03:08 GMT    | Initial version

