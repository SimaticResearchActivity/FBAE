# FBAE
Framework for Broadcast Algorithms Evaluation

## Introduction

*FBAE* is a software framework, developped in C++, to evaluate performances (latency and throughput) of Total-Order Broadcast Algorithms when there are no failures. It has been tested on Linux and Windows. It should also compile and execute on MacOS.

Thanks to *FBAE*, it is possible to evaluate (and thus compare) the performances of different Total-Order broadcast algorithms using different communication protocols ([ENet](http://enet.bespin.org) and TCP are available, but we recommend that you use systematically TCP protocol as ENet experiences reentrancy problems).

## Compilation of *FBAE*

Clone *FBAE* repository and apply *cmake* procedure of [this document](http://www-inf.telecom-sudparis.eu/COURS/JIN/sir/new_site/Supports/Documents/ToolsTeachingUnit/toolsTeachingUnit.html).

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

Note `Resources` directory of *FBAE* repository contains samples of site files.

### Launch *fbae* executable
Once your site file is ready, you can run `fbae` executable according to the following usage:

```txt
fbae -a|--algo <algo_identifier> -c|--comm <communicationLayer_identifier> -h|--help -n|--nbMsg <number> -r|--rank <rank_number> -s|--size <size_in_bytes> -S|--site <siteFile_name> -v|--verbose
Where:
  -a|--algo <algo_identifier>                Broadcast Algorithm
                                                S = Sequencer based
  -c|--comm <communicationLayer_identifier>  Communication layer to be used
                                                e = Enet (reliable)
                                                t = TCP
  -h|--help                                  Show help message
  -n|--nbMsg <number>                        Number of messages to be sent
  -r|--rank <rank_number>                    Rank of process in site file (if 99, all algorithm participants are executed within threads in current process)
  -s|--size <size_in_bytes>                  Size of messages sent by a client (must be in interval [22,65515])
  -S|--site <siteFile_name>                  Name (including path) of the sites file to be used
  -v|--verbose                               [optional] Verbose display required
```

For instance, you can open 3 terminals and run:

- `./fbae -a S -c t -n 20 -r 2 -s 32 -S ../../Resources/sites_3_local.json` on terminal 2 (In this example, we first launch `fbae` executable with rank 2, because we want to invoke Sequencer total-order broadcast algorithm. And the role of the sequencer process is given to the last site specified in json file).
- `./fbae -a S -c t -n 20 -r 1 -s 32 -S ../../Resources/sites_3_local.json` on terminal 1
- `./fbae -a S -c t -n 20 -r 0 -s 32 -S ../../Resources/sites_3_local.json` on terminal 0

After a while (depending on the number of messages to be sent you specified), `fbae` displays the statistics (structured in CSV format) observed for this process, e.g.:

```txt
algo,commLayer,nbMsg,rank,sizeMsg,siteFile,nbReceivedMsg,nbSentMsg,ratio nbRcv/nbSent,Average (in ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max
Sequencer,TCP,100,99,32,../../Resources/sites_3_local.json,100,200,0.500000,0.241571,0.130782,0.239602,0.244068,0.248755,0.417237,0.417237,0.417237,0.417237
```

Note that, for testing purpose, it is possible to launch a single instance of `fbae` which will execute all activities in different threads. To do so, give value `99` to the rank, e.g. `./fbae -a S -c t -n 20 -r 99 -s 32 -S ../../Resources/sites_3_local.json`

## Current status of *FBAE*

### Total-Order broadcast algorithms

#### Sequencer-Based algorithm

In this algorithm, one site (the last one specified in the sites file) is given the role of Sequencer. The other sites specified in the site file are given the role of Broadcasters.

When a Broadcaster wants to broadcast a message, it sends its message to the Sequencer which sends it back to all Broadcasters, so that they can deliver it.

### Communication protocols

#### ENet

[ENet](http://enet.bespin.org) is a reliable UDP networking library. It is also able to guarantee FIFO on communication channels. We use all of these properties in *FBAE*. Unfortunately, Enet experiences reentrancy problems with some Total-Order broadcast algorithms. Thus, we recommend not to use this communication protocol in FBAE.

#### TCP

TCP communication protocols is implemented thanks to [Boost](http://www.boost.org) library.

## Extending *FABE*

### Global architecture of *FBAE*

*FBAE* is structured into 3 layers :

- *Session layer* (made of `SessionLayer` class) is the layer responsible for performance evaluation. It requests Total-order broadcasts from the *Algorithm layer*.
- *Algorithm layer* (made of `AlgoLayer` factory class and its subclasses) is the layer responsible for putting in place Total-order guarantees, once messages have been exchanged between processes by the *Communication layer*.
- *Communication layer* (made of `CommLayer` factory class and its subclasses, and also of `CommPeer` factory class and its subclasses) is the layer responsible for exchanging messages between processes.

The interfaces between these layers are the following:

- *Session layer* / *Algorithm layer*

    - *Session layer* ==> *Algorithm layer*

         - *Session layer* calls `AlgoLayer`'s `executeAndProducedStatistics()` method to launch *Algorithm layer*. Note that we stay in `executeAndProducedStatistics()` method until *Algorithm layer* is done executing.
         - Then it calls `totalOrderBroadcast()` method for each total-order broadcast it has to make. 
         - Finally it calls `terminate()` method to tell the *Algorithm layer* than it can shutdwon.

     - *Algorithm layer* ==> *Session layer*
         - Once *Algorithm layer* considers it is fully initialized locally, it calls `callbackInitDone()` method in *Session layer*.
         - Each time a message can be delivered, it calls `callbackDeliver()` method.

- *Algorithm layer* / *Communication layer*

    - *Algorithm layer* ==> *Communication layer*

         - *Algorithm layer* calls `CommLayer`'s `initHost()` method when it wants to be able to accept connections from other processes.
         - And/or it calls `CommLayer`'s `connectToHost()` method when it wants to establish a connection with another process. Note: If *Algorithm layer* need to call `initHost()` and also one or several time `connectToHost()`, it must make the calls to `connectToHost()` in a dedicated thread in order to call `waitForMsg()` as soon as possible (see example in *BBOBB* algorithm layer implementation).
         - Then it calls `CommLayer`'s `waitForMsg()` method. Note that we stay in `waitForMsg()` method until *Communication layer* detect a pre-specified number of disconnections (see parameter `maxDisconnections` of `waitForMsg()` method).
         - When *Algorithm layer* needs to send a message to another process, it can use:

             - `CommPeer`'s `send()` lethod to send a message to a single process.
             - `CommLayer`'s `broadcastMsg()` method to broadcast a message to all processes connected to this process.

         - *Algorithm layer* calls `CommPeer`'s `disconnect()` method when it wants to disconnect from a peer process.

    - *Communication layer* ==> *Algorithm layer*

         - When *Communication layer* receives a message, it calls `callbackHandleMessage()` method in *Algorithm layer*.

### Adding another Total-Order broadcast algorithm

If you want to add another Total-Order broadcast algorithm:

1. Make a new subclass of `AlgoLayer` by inspiring yourself from:

    - `SequencerAlgoLayer.h` to declare the class implementing your algorithm,
    - `SequencerAlgoLayer.cpp` to define this class,
    - `SequencerAlgoLayerMsg.h` to define the identifier and the structure of each message used by your algorithm.

2. Note concerning your *Algorithm layer*:

     - When `callbackHandleMessage()` method is called in the context of an instance of your `AlgoLayer` subclass, *FBAE* guarantees that `callbackHandleMessage()` will not be called concurrently n the context of the same instance of `AlgoLayer` subclass.
    - Each call to `CommLayer`'s `connectToHost()` method must be followed by the sending of a `MsgId::RankInfo` message to the host you connected to.
    - Your `terminate` method must always send  `MsgId::DisconnectIntent` message to the host you are connected to.
    - Moreover, upon receiving this `MsgId::DisconnectIntent` message (see `callbackHandleMessage()` method), your code must answer a `MsgId::AckDisconnectIntent` message to the sender.
    - Finally, upon receiving this `MsgId::AckDisconnectIntent` message (see `callbackHandleMessage()` method), your code must call `disconnect()` method of the communication peer representing the host you are connected to.

3. Modify `main.cpp` to integrate your new class:

    - In `main` function, modify `parser` variable intialization in order to specify (in the same way as letter `'S'` in string `"\n\t\t\t\t\t\tS = Sequencer based"` specifies `'S'` as the letter for *Sequencer algorithm*) the letter which will specify your added algorithm.
    - In `concreteAlgoLayer()` function, add a `case` with this new letter to create an instance of your class.

### Adding another communication protocol

If you want to add another communication protocol:

1. Make a new subclass of `CommLayer` by inspiring yourself from:

    - `EnetCommLayer.h` to declare the class implementing your usage of the protocol,
    - `EnetCommLayer.cpp` to define this class,
    - Note: *FBAE* must guarantee that, when `callbackHandleMessage()` method is called in the context of an instance of an `AlgoLayer` subclass, `callbackHandleMessage()` will not be called concurrently n the context of the same instance of `AlgoLayer` subclass. Thus, you must pay attention to protect the call to `getAlgoLayer()->callbackHandleMessage()` with a mutual exclusion (see an example in `TcpCommLayer::handleIncomingConn()`).

2. Make a new subclass of `CommPeer` by inspiring yourself from:
    - `EnetCommPeer.h` to declare the classe representing a peer in your usage of the protocol,
    - `EnetCommPeer.cpp` to define this class.

3. Modify `main.cpp` to integrate your new class:

    - In `main` function, modify `parser` variable intialization in order to specify (in the same way as letter `'e'` in string `"\n\t\t\t\t\ŧ\t\te = Enet (reliable)"` specifies `'e'` as the letter for *Enet (reliable)*) the letter which will specify your added communication protocol.
    - In `concreteCommLayer()` function, add a `case` with this new letter to create an instance of your `CommLayer` subclass.
    - Note: There is nothing to do in `main.cpp` concerning new `CommPeer` subclass, as creation of instances of this subclass will be done inside your `CommLayer` subclass.




