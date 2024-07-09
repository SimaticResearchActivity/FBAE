# FBAE

Framework for Broadcast Algorithms Evaluation

## Introduction

*FBAE* is a software framework, developped in C++, to evaluate performances (latency and throughput) of Total-Order Broadcast Algorithms when there are no failures. It has been tested on Linux and Windows with compilers: clang, gcc and MSVC. In July 2024, *FBAE* does not compile with Apple clang: macOS users must install gcc or standard clang to compile *FBAE*.

Thanks to *FBAE*, it is possible to evaluate (and thus compare) the performances of different Total-Order broadcast algorithms using different communication protocols (For the moment, only TCP is available, but other communication layers are foreseen).

## Compilation of *FBAE*

### Prerequisites

*FBAE* relies on [log4cxx](https://logging.apache.org/log4cxx/latest_stable/index.html), the library proposed by Apache foundation to do logging. To be able to compile and link a program using this library, you must apply the following installation procedure on the machine where you will compile *FBAE*.

### Linux

As mentionned [here](https://logging.apache.org/log4cxx/latest_stable/build-cmake.html):

```bash
sudo apt-get install libapr1-dev libaprutil1-dev
```

Then, we recommend the following command:

```bash
sudo apt-get install liblog4cxx-dev
```

and you are done. Otherwise, you need to apply the procedure by log4cxx developers, i.e.:

- Download log4cxx 1.2.0 archive from <https://logging.apache.org/log4cxx/latest_stable/download.html>
- Then:

```bash
cd [workDirectory]
# Uncompress archive in a workDirectory
cd [workDirectory]/apache-log4cxx-1.2.0
mkdir build
cd build
cmake -DBUILD_TESTING=off ..
make
# To trigger installation in standard directory, do:
sudo make install
```

### macOS

As mentionned [here](https://logging.apache.org/log4cxx/latest_stable/build-cmake.html), "APR and APR-Util are provided by the platform in Mac OS/X 10.5 and iODBC in 10.4. cmake can be installed by typing "brew install cmake"."

Then, apply linux proposed procedure, i.e. download log4cxx 1.2.0 archive from <https://logging.apache.org/log4cxx/latest_stable/download.html>, etc.

### Windows

- You need command-line utilities(zip, grep, sed). If you have not already installed an environment providing these utilities:
  - Install MSYS2 from <https://www.msys2.org/>
  - Then, in an MSYS2 terminal, run `pacman -S zip`
  - In PATH environment variable, add C:\[pathToMSYS64Directory]\msys64\usr\bin

- Install vcpkg. To do so, open a Powershell. Then

```bash
cd [directoryWhereYouWantToInstallVcpkg; For instance, c:\software]
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg integrate install
```

This last command will display the following message:

```bash
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake"

All MSBuild C++ projects can now #include any installed libraries. Linking will be handled automatically. Installing new libraries will make them instantly available.
```

Note the directory which is suggested (`C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake` in our example), as you will have to specify this directtory in top `CMakeLists.txt` of *FBAE*.

- Now, you can take care of log4cxx:

```bash
cd [directoryContainingVcpkgDirectory]
.\vcpkg\vcpkg install expat apr apr-util log4cxx
```

### Compilation procedure

Once prerequisite are done (see previous section), clone *FBAE* repository and apply *cmake* procedure of [this document](http://www-inf.telecom-sudparis.eu/COURS/JIN/sir/new_site/Supports/Documents/ToolsTeachingUnit/toolsTeachingUnit.html).

Note that, ion case of Windows environment, you may have to adapt top `CMakeLists.txt` to write the path which was displayed by `.\vcpkg\vcpkg integrate install` command mentioned earlier in the line:

```cmake
set(CMAKE_TOOLCHAIN_FILE C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake)
```

## Executing *FBAE*

### Writing JSON site file

First, write a JSON site file defining the sites which will run instances of *FBAE*. To write such a file, inspire yoiurself from the following example, by replacing "localhost" (respectively 4096, 4097 and 4098) by the host names (respectively the ports) you will use for your experiments:

```json
{
    "sites": [
        {
            "tuple_element0": "localhost",
            "tuple_element1": 4096
        },
        {
            "tuple_element0": "localhost",
            "tuple_element1": 4097
        },
        {
            "tuple_element0": "localhost",
            "tuple_element1": 4098
        }
    ]
}
```

This file example defines 3 sites, each running on localhost, the first site listening for connections on port 4096, the second one on port 4097, and the third one on port 4098.

Note: `res` directory of *FBAE* repository contains samples of site files.

### Launch manually *fbae* executable

Once your site file is ready, you can run `fbae` executable according to the following usage:

```txt
fbae -a|--algo <algo_identifier> -c|--comm <communicationLayer_identifier> -f|--frequency <number> -h|--help -m|--maxBatchSize <number_of_messages> -n|--nbMsg <number> -r|--rank <rank_number> -s|--size <size_in_bytes> -S|--site <siteFile_name> -w|--warmupCooldown <number>
Where:
  -a|--algo <algo_identifier>                Broadcast Algorithm
                                                B = BBOBB
                                                S = Sequencer based
  -c|--comm <communicationLayer_identifier>  Communication layer to be used
                                                t = TCP
  -f|--frequency <number>                    [optional] Number of PerfMessage sessionLayer messages which must be sent each second (By default, a PerfMessage is sent when receiving a PerfResponse)
  -h|--help                                  Show help message
  -m|--maxBatchSize <number_of_messages>          [optional] Maximum size of batch of messages (if specified algorithm allows batch of messages; By default, maxBatchSize is unlimited)
  -n|--nbMsg <number>                        Number of messages to be sent
  -r|--rank <rank_number>                    Rank of process in site file (if 99, all algorithm participants are executed within threads in current process)
  -s|--size <size_in_bytes>                  Size of messages sent by a client (must be in interval [22,65515])
  -S|--site <siteFile_name>                  Name (including path) of the sites file to be used
  -w|--warmupCooldown <number>               [optional] Number in [0,99] representing percentage of PerfMessage sessionLayer messages which will be considered as part of warmup phase or cool down phase and thus will not be measured for ping (By default, percentage is 0%)
```

For instance, you can open 3 terminals and run:

- `./fbae -a S -c t -n 3 -r 0 -s 32 -S ../../../res/sites_3_local.json` on terminal 0 (In this example, we first launch `fbae` executable with rank 0, because we want to invoke Sequencer total-order broadcast algorithm. And the role of the sequencer process is given to the first site specified in json file).
- `./fbae -a S -c t -n 20 -r 1 -s 32 -S ../../../res/sites_3_local.json` on terminal 1.
- `./fbae -a S -c t -n 20 -r 2 -s 32 -S ../../../res/sites_3_local.json` on terminal 2.

After a while (depending on the number of messages to be sent you specified), `fbae` displays the statistics (structured in CSV format) observed for this process, e.g.:

```txt
algoLayer,commLayer,frequency,maxBatchSize,nbMsg,warmupCooldown,rank,sizeMsg,siteFile,nbPing,Average (in ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max,Elapsed time (in sec),CPU time (in sec),Throughput (in Mbps)
Sequencer,TCP,0,2147483647,3,0%,2,1024,../../res/sites_3_local.json,3,0.410093,0.307098,0.307098,0.361872,0.561308,0.561308,0.561308,0.561308,0.561308,0.001000,0.001937,98.304000
```

Note that, for testing purpose, it is possible to launch a single instance of `fbae` which will execute all activities in different threads. To do so, give value `99` to the rank, e.g. `./fbae -a S -c t -n 20 -r 99 -s 32 -S ../../../res/sites_3_local.json`

### Launch *fbae* executable thanks to a script

`utils` directory contains `launch_fbae.py` Python script. Its usage is:

``` shell
python3 launch_fbae.py path_to_fbae_executable path_to_result_directory all_fbae_arguments_except_-r_or_--rank
```

For instance, if file `sites_8_machines.json` contains the specification of 8 hosts/ports, command:

``` shell
python3 launchFBAE.py /absolute_path/FBAE /absolute_path/FBAE/results/ -a B -c t -n 5 -s 32 -S /absolute_path/FBAE/sites_8_machines.json
```

launches instances of fbae on each of these 8 hosts/ports. When these instances are done, each generates a result file in `/aboslute_path/FBAE/results`, the name of the file containing the rank of the instance, in its last character. For instance, file `/absolute_path/FBAE/results/result_-a_B_-c_t_-n_5_-s_32_-S__netfs_inf_simatic_FBAE_sites_8_b313.json_--rank_0`
contains the results produced by instance ranked 0 (see `--rank_0` at the end of the name of the file.

### Configure level of traces to be displayed

Level of traces to be displayed is configured in `res/fbae_logger.properties` file. By default, all traces of level `WARN`, `ERROR` and `FATAL` are displayed in all packages.

If you want to change the default level of traces displayed, specify `INFO` (to activate all traces), `ERROR`, `FATAL` or even `OFF` (for no traces at all) instead of `WARN` in the line of `res/fbae_logger.properties`:

```properties
log4j.rootLogger = WARN, stderr
```

If you want to generate a file in addition to displaying the traces on `stderr`, replace the line:

```properties
log4j.rootLogger = WARN, stderr
```

by the line:

```properties
log4j.rootLogger = WARN, stderr, file
```

Note: The name and directory of the written file is specified by the line (`/tmp/traces.log` in our example):

```properties
log4j.appender.file.File = /tmp/traces.log
```

If you want to change the level of traces in one or several *FBAE* packages, specify `INFO` (to activate all traces), `ERROR`, `FATAL` or even `OFF` (for no traces at all) after the `=` sign corresponding to this package in `res/fbae_logger.properties` file. For instance, if you want `INFO` traces concerning FBAE's `AlgoLayer` package, replace the line:

```properties
log4j.logger.fbae.core.AlgoLayer =
```

by the line:

```properties
log4j.logger.fbae.core.AlgoLayer = INFO
```

## Current status of *FBAE*

### Total-Order broadcast algorithms

This section lists the total-order broadcast algorithms which can be currently evaluated with *FBAE*.

#### *BBOBB* Algorithm

This algorithm is described in paper [BBOBB: A total order broadcast algorithm achieving low latency and high throughput](https://hal.science/hal-01316509v1/file/BBOBB_c_%20A%20total%20order%20broadcast%20algorithm%20achieving%20low%20latency%20and%20high%20throughput.pdf) by Michel Simatic and Benoit Tellier, 46th Annual IEEE/IFIP International Conference on Dependable Systems and Networks (DSN 2016), Toulouse, France, June 2016.

In *BBOBB* algorithm implementation, all sites broadcast messages.

#### *LCR* Algorithm

This algorithm is described in paper "Throughput optimal total order broadcast for cluster environments" by Guerraoui, R., Levy, R. R., Pochon, B., and Quéma, V. in ACM Trans. Comput. Syst., 28 :5 :1–5 :32 (2010).

In *LCR* algorithm implementation, all sites broadcast messages.

#### *Sequencer*-based algorithm

In *Sequencer*-based algorithm, one site (the first one specified in the sites file) is given the role of Sequencer. The other sites specified in the site file are given the role of Broadcasters.

When a Broadcaster wants to broadcast a message, it sends its message to the Sequencer which sends it back to all Broadcasters, so that they can deliver it.

### Communication protocols

This section lists the communication protocols which can be currently invoked for algorithm evaluation with *FBAE*.

#### *TCP*

*TCP* communication protocols is implemented thanks to [Boost](http://www.boost.org) library.

#### (deprecated) *ENet*

[ENet](http://enet.bespin.org) is a reliable UDP networking library. It is also able to guarantee FIFO on communication channels. We used all of these properties in *FBAE*. Unfortunately, *Enet* experiences reentrancy problems with *BBOBB* algorithm*. Thus, it is not anymore offered in *FBAE*.

## Extending *FBAE*

To extend *FBAE*, take a look at **doc/designDocument.md** and read the following subsections.

### Global architecture of *FBAE*

*FBAE* is structured into 3 layers :

- *Session layer* (made of `SessionLayer` class) is the layer responsible for performance evaluation. It requests Total-order broadcasts from the *Algorithm layer*.
- *Algorithm layer* (made of `AlgoLayer` factory class and its subclasses) is the layer responsible for putting in place Total-order guarantees, once messages have been exchanged between processes by the *Communication layer*.
- *Communication layer* (made of `CommLayer` factory class and its subclasses) is the layer responsible for exchanging messages between processes.

The interfaces between these layers are the following:

- *Session layer* / *Algorithm layer*
  - *Session layer* ==> *Algorithm layer*
    - *Session layer* calls `AlgoLayer::execute()` method to launch *Algorithm layer*. Note that we stay in `execute()` method until *Algorithm layer* is done executing.
    - Then it calls `AlgoLayer::totalOrderBroadcast()` method for each total-order broadcast it has to make.
    - Finally it calls `AlgoLayer::terminate()` method to tell the *Algorithm layer* than it can shutdwon.
  - *Algorithm layer* ==> *Session layer*
    - Once *Algorithm layer* considers it is fully initialized locally, it calls `SessionLayer::callbackInitDone()` method in *Session layer*.
    - Each time a message can be delivered, it calls `SessionLayer::callbackDeliver()` method.
- *Algorithm layer* / *Communication layer*
  - *Algorithm layer* ==> *Communication layer*
    - *Algorithm layer* calls `CommLayer::openDestAndWaitIncomingMsg()` method when it wants to:
      1. accept commpunication links (e.g. connections in the case of TCP) from other processes,
      2. establish communication links with other processes,
      3. wait for incoming messages until `CommLayer::terminate()` is called.
    - When *Algorithm layer* needs to send a message to another process, it can use:
      - `CommLayer::send()` method to send a message to a single process.
      - `CommLayer::multicastMsg()` method to multicast a message to all processes with which the current process has established communication links.
    - *Algorithm layer* calls `CommLayer::terminate()` method when it wants to stop all of the communications with remote processes.
  - *Communication layer* ==> *Algorithm layer*
    - When *CommunicationLayer* is fully initailized, it calls `AlgoLayer::callbackInitDone()`.
    - When *Communication layer* receives a message, it calls `AlgoLayer::callbackReceive()` method.
    - Note:
      1. *CommunicationLayer* guarantees that there is no call to `AlgoLayer::callbackReceive()` before `AlgoLayer::callbackInitDone()` is done executing.
      2. Even if it may be multithreaded, *CommunicationLayer* guarantees that there will never be simultaneous calls to `AlgoLayer::callbackReceive()`.

### Logging traces to your implementation

*FBAE* relies on [log4cxx](https://logging.apache.org/log4cxx/latest_stable/index.html) logging library.

The following trace levels are used:

- `FATAL` when *FBAE* cannot do any more useful work.
- `ERROR` when *FBAE* experienced an error, but can proceed (probably at a reduced level of functionality or performance).
- `WARN` when *FBAE* experienced an unexpected event, but can proceed.
- `INFO` when you want to trace an event.

Note:

- `DEBUG` and `TRACE` levels are not used.
- When you write a trace which contains a data of type `uint8_t` (e.g. `rank_t` or `MsgId_t`), specify the format `{:d}` so that this data is displayed as a number, not a character.

### Adding another Total-Order broadcast algorithm

This section describes how to add another Total-Order broadcast algorithm to *FBAE*. To illustrate the procedure, it gives examples assuming that your algorithm is named `Foo`.

Before presenting the procedure, you need to understand the difference between:

- The rank of a process participating, i.e. the rank which has be given to this process when running `fbae` executable.
- The position of process participating as a broadcaster.

For instance:

- In *Sequencer* algorithm
  - Sequencer process always has *rank* 0. Sequencer process has no position, as it does not participate as a broadcaster.
  - The other processes (ranked 1, 2, 3, etc.) participate as broadcastersGroup. So, they have a position (as broadcaster) which is respectively 0, 1, 2, etc.
- In *BBOBB* algorithm, all processes participate as braodcasters. Each of them has a position (as broadcaster) which corresponds to its rank.

Now we can present the procedure for adding another Total-Order broadcast algorithm:

1. Draw several Message Sequence Charts (MSC) to illustrate the algorithm behavior. To do so, you can use a tool like [plantuml](https://plantuml.com/fr/sequence-diagram) (see examples in `doc` directory).
2. Create a subdirectory of `src/lib/AlgoLayer` directory, named after the name of your algorithm. For instance, `src/lib/AlgoLayer/Foo` directory.
3. Create a file for defining messages exchanged between the prcesses executing your algorithm, e.g. `src/lib/AlgoLayer/Foo/FooMsg.h`, containing:
   1. A first include which **must** be `#include "../adaptCereal.h"` followed by `#include "cereal/archives/binary.hpp"`.
   2. Definition of a namespace dedicated to your algorithm.
   3. Definition of a `enum class MsgId : MsgId_t` containing the message identifiers used by your algorithm.
   4. Definition of the structure of the different messages. Note: This definition must include [Cereal](http://uscilab.github.io/cereal) serialization method (for examples, see implementation of *Sequencer* or *BBOBB* algorithms and [Cereal quick start](http://uscilab.github.io/cereal/quickstart.html)).
4. Create the class which will implement your algorithm, e.g. `src/lib/AlgoLayer/Foo/Foo.h` (to define your class) and `src/lib/AlgoLayer/Foo/Foo.cpp` (to implement your class).
   - Note: `src/lib/CMakeLists.txt` must be modified to contain `AlgoLayer/Foo/Foo.cpp` and `AlgoLayer/Foo/Foo.h` lines.
5. Implement `Foo::toString()` method.
6. Implement `Foo::execute()` method to handle the different messages your algorithm can receive.
   - Build the vector containing rank of participants which will behave as braodcasters during execution. Call `setBroadcastersGroup(std::move(thisVzector))`.
   - Build the vector containing rank of participants your process needs to establish a communication link with. Call `getSessionLayer()->getCommLayer()->openDestAndWaitIncomingMsg()` with this vector.
7. Implement `Foo::totalOrderBroadcast()` method. To do so, you must decide if:
   - Your algorithm sends immediately a message to one or several processes, like *Sequencer* algorithm. In that case, you must override `AlgoLayer::totalOrderBroadcast()` method.
   - Your algorithm stores the message to broadcast in a "batch" of messages waiting to be broadcast by the algorithm, like *BBOBB* algorithm. In that case, you do not need to implement anything as your class will use `AlgoLayer::totalOrderBroadcast()` method.
8. If your algorithm works with "batch" of messages, it is very likely that you need to implement `Foo::callbackInitDone()` method to override `AlgoLayer::callbackInitDone()` in order to send one or several messages to initiate execution of your algorithm (e.g. `BBOBB::callbackInitDone()` sends an initial `Step` message).
   - Note: *FBAE* guarantees that you will not receive any message from other processes before the call to `Foo::callbackInitDone()` is done.
9. Implement `Foo::callbackReceive()` method to handle all of the messages your algorithm is manipulating. In particular, if a sessionLayer message can be delivered, call `getAlgoLayer()->batchNoDeadlockCallbackDeliver()` if your algorithm is using "batch" of messages and `getSessionLayer()->callbackDeliver()` otherwise.
   - Note: Even if it may be multithreaded, *Communication Layer* guarantees that there will never be simultaneous calls to `Foo::callbackReceive()`.
10. Implement `Foo::terminate()` method.
11. Implement [GoogleTest](https://google.github.io/googletest/) tests in a dedicated `.cpp` file in `tests/AlgoLayer` directory (e.g. `tests/AlgoLayer/testFoo.cpp`). Run your tests.
    - Note: To compile properly, the required `.h` and `.cpp` files must be mentioned in `tests/CMakeLists.txt`.
12. Modify `main.cpp` to integrate your new class:
    - In `main` function, modify `parser` variable intialization in order to specify (in the same way as letter `'S'` in string `"\n\t\t\t\t\t\tS = Sequencer based"` specifies `'S'` as the letter for *Sequencer algorithm*) the letter which will specify your added algorithm.
    - In `concreteAlgoLayer()` function, add a `case` with this new letter to create an instance of your class.
13. Run integration tests.

### Adding another communication protocol

If you want to add another communication protocol:

1. Make a new subclass of `CommLayer` by inspiring yourself from `Tcp.h` and `Tcp.cpp`. Note:
   - *FBAE* must guarantee that, when `AlgoLayer::callbackInitDone()` method is called, `AlgoLayer::callbackReceive()` was not previously called. This is the role of:
     - `getInitDoneCalled().wait();` instruction in `Tcp::handleIncomingConn()`
     - `getInitDoneCalled().count_down();` instruction in `Tcp::openDestAndWaitIncomingMsg()`
   - *FBAE* must guarantee that, when `AlgoLayer::callbackReceive()` method is called, it is not called concurrently by another thread of `CommLayer` on the same instance of `AlgoLayer` subclass. Thus, you must pay attention to protect the call to `getAlgoLayer()->callbackReceive()` with a mutual exclusion (see an example with line `std::scoped_lock lock(mtxCallbackHandleMessage);` in `Tcp::handleIncomingConn()`).
2. Modify `main.cpp` to integrate your new class:
   - In `main` function, modify `parser` variable intialization in order to specify (in the same way as letter `'e'` in string `"\n\t\t\t\t\ŧ\t\te = Enet (reliable)"` specifies `'e'` as the letter for *Enet (reliable)*) the letter which will specify your added communication protocol.
   - In `concreteCommLayer()` function, add a `case` with this new letter to create an instance of your `CommLayer` subclass.
