# Energy Consumption Measures

## Introduction

This document is a synthesis of the work made on the issue 78 of the FBAE project : [Add energy consumption measures by exploiting RAPL registers #78](https://github.com/SimaticResearchActivity/FBAE/issues/78).

The solutions are sorted in the same order as they were tested.  

## Summary

In order to made energy consumption measures, 4 solutions have been proposed, that I will present in this order :

- I - [Ecofloc](#i---ecofloc)
  - I.1 - [Benefits](#i1---benefits)
  - I.2 - [Installation](#i2---installation)
  - I.3 - [Limitations](#i3---limitations)

- II - [Likwid](#ii---likwid)
  - II.1 - [Benefits](#ii1---benefits)
  - II.2 - [Installation](#ii2---installation)
  - II.3 - [Utilisation](#ii3---utilisation)
  - II.4 - [Limitations](#ii4---limitations)

- III - [Scaphandre](#iii---scaphandre)
  - III.1 - [Benefits](#iii1---benefits)
  - III.2 - [Installation](#iii2---installation)
  - III.3 - [Utilisation](#iii3---utilisation)
  - III.4 - [Limitations](#iii4---limitations)

- VI - [PowerAPI](#iv---powerapi)
  - IV.1 - [Benefits](#iv1---benefits)
  - IV.2 - [Installation](#iv2---installation)
  - IV.3 - [Utilisation](#iv3---utilisation)
  - IV.4 - [Limitations](#iv4---limitations)

## I - [Ecofloc](https://github.com/labDomolandes/ecofloc)

### I.1 - Benefits

- Open-Source under licence Apache 2.0;
- Allow to do **CPU**, **GPU**, **RAM**, **storage devices** (SD), and **network interface controllers** (NIC) mesures. Here we will mostly discuss the CPU measures.

### I.2 - Installation

Clone the repo :

```bash
git clone https://github.com/labDomolandes/ecofloc
```

And then follow the build instructions written in the README: <https://github.com/labDomolandes/ecofloc?tab=readme-ov-file#automated-installation>

In order to make cpu measures, you need to have `rdmsr` installed on your system. You can install it with the command :

```bash
apt-get install msr-tools
```

### I.3 - Limitations

In order to make cpu measures, `rdmsr` needs processor that support the `Intel x64` architecture. You can see the documentation [here](https://www.felixcloutier.com/x86/rdmsr).

The library maybe is a little to recent, (first commit on Jul 14, 2024, 3 weeks ago).

## II - [Likwid](https://github.com/RRZE-HPC/likwid/)

### II.1 - Benefits

- LIKWID is usable on a large range of architecture, making it one of the most likely to be use in the project.
- It permits to do CPU and GPU (not tested here) measures.
- The C++ library makes it easier to implement in the FBAE project.

### II.2 - Installation

A simple command is needed :

``` bash
sudo apt-get install likwid
```

If you want to have a full documentation of the C/C++ library, you can clone the repo and build the HTML documentation :

```bash
git clone https://github.com/RRZE-HPC/likwid
cd likwid
make docs
```

### II.3 - Utilisation

You can find an example of the use of the C++ library of LIKWID in the [Power_measures_test/Test_LIKWID](./Power_mesures_test/Test_LIKWID/) folder with the [main.cpp](./Power_mesures_test/Test_LIKWID/main.cpp), and the [CMakeList](./Power_mesures_test/Test_LIKWID/CMakeLists.txt) instructions in order to correctly build the project.

### II.4 - Limitations

For an unknown reason, the test program do not work on a computer with an `AMD Ryzen 7 5700U processor`, which means a Zen 2 architecture ([Source](https://www.techpowerup.com/cpu-specs/ryzen-7-5700u.c2744)), normally supported by LIKWID.

Also, we did test on other computers with an  `Intel(R) Core(TM) i5-9500 processor`. These computers are the one on which most of our tests will be hold, so the compatibility is required, but it did not work.

## III - [Scaphandre](https://github.com/hubblo-org/scaphandre)

### III.1 - Benefits

- Available on Gnu/Linux and Windows 10 & 11
- Export the results in a Json format
- Can monitor CPU, RAM and Disk usage per process

### III.2 - Installation

Scaphandre is made in Rust, so we need to install a Rust compiler to build it. To get the cargo compiler :

``` bash
sudo apt install cargo
```

In the directory of your choice, clone the Scaphandre repo :

```bash
git clone https://github.com/hubblo-org/scaphandre.git
cd scaphandre
cargo build
```

Then add `$(DIRECTORY_OF_YOUR_CHOICE)/scaphandre/target/release` to the environment variable PATH.

/!\ Do not use `~/` but `$HOME/` /!\

To read the Json result file, install `nlohmann-json` :

```bash
sudo apt-get install nlohmann-json3-dev
```

### III.3 - Utilisation

Before using the `scaphandre` command, you need to use the [init.sh](./Power_mesures_test/Test_SCAPHANDRE/init.sh) script, like advice in the [Troubleshooting](https://hubblo-org.github.io/scaphandre-documentation/troubleshooting.html) section.

You can use the

```bash
scaphandre json -s 2 --step-nano 1000000 --max-top-consumers 10 --resources -f results.json
```

You can see an example of a Json result file here : [save_results.json](./Power_mesures_test/Test_SCAPHANDRE/save_results.json).

You can find an example of how the `scaphandre` can be used in the project in the [Power_measures_test/Test_SCAPHANDRE](./Power_mesures_test/Test_SCAPHANDRE/) folder with the [main.cpp](./Power_mesures_test/Test_SCAPHANDRE/main.cpp), and the [CMakeList](./Power_mesures_test/Test_LIKWID/CMakeLists.txt) instructions to include `nlohmann-json` to the project.

There is also an option `--process-regex` not used here that allow to filter the processes.

### III.4 - Limitations

When the results of the measures are exported in Json, there is a problem that make the data hard to analyse :

```json
{
    "": ... // First Measure
}
{
    "": ... // Second Measure
}
```

The measures are all put in the same file, and each of them is a "Json object", so at the end the file `result.json` is not a valid Json file.

In order to conter it, it is possible to set correctly the timeout and frequency of the measures, which means you should know the execution time **before** its execution.

## IV - [PowerAPI](https://powerapi.org)

### IV.1 - Benefits

- Can release measures through time and does not require toi set a time before the execution.

### IV.2 - Installation

In order to install the SmartWatts Formula (the HardWare Performance Counter (HWPC) Sensor and Smartwatt), you can find a script [here](https://powerapi.org/script/smartwatts_install.sh) and then run it :

```bash
bash ./smartwatts_install
```

If you want to use the MongoDB output, you also need to install the MongoDB library for C++ :

```bash
curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.10.1/mongo-cxx-driver-r3.10.1.tar.gz
tar -xzf mongo-cxx-driver-r3.10.1.tar.gz
cd mongo-cxx-driver-r3.10.1/build
```

```bash
cmake ..                                \
    -DCMAKE_BUILD_TYPE=Release          \
    -DMONGOCXX_OVERRIDE_DEFAULT_INSTALL_PREFIX=OFF
```

```bash
cmake --build .
sudo cmake --build . --target install
```

Be sure to check the [CMakeList.txt](./Power_mesures_test/Test_POWERAPI/CMakeLists.txt) file because MongoDb is not installed in the directory you may think.

### IV.3 - Utilisation

Here there is 2 solutions that have been tested : the use of MongoDB or CSV file. You can find the configuration files of the commands [here](./Power_mesures_test/Test_POWERAPI/res/).

The test program [Test_POWERAPI/main.cpp](./Power_mesures_test/Test_POWERAPI/main.cpp) is working with the MongoDB version.

Before using it, you need to do 2 previous steps :

- Create a local MongoDB server :

```bash
sudo docker run --name mongo_destination -p 27017:27017 mongo
```

- Create the sensor :

```bash
sudo docker run --rm  \
--net=host \
--privileged \
--pid=host \
-v /sys:/sys \
-v /var/lib/docker/containers:/var/lib/docker/containers:ro \
-v /tmp/powerapi-sensor-reporting:/reporting \
-v $(pwd):/srv \
-v $(pwd)/sensor_config_file.json:/sensor_config_file.json \
powerapi/hwpc-sensor --config-file /sensor_config_file.json
```

If you want to use the csv version, you need to add the line :

```bash
-v $(pwd)/sensor_output:/sensor_output \
```

### IV.4 - Limitations

- For the CSV solution, a csv file should be created manually for all processes running in order to write the results in them.

- For the MongoDB solution, it seems complicated to ask the user to do so many steps before executing the program, when in general we tend to make it self-sufficient.

Overall, PowerAPI requires the user to be super user in order to create the MongoDB server and the sensor.
