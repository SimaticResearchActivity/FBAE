# Measures

## Tools

### Csv

To convert csv in a format with `;` instead of `,`, use :
```bash
sed -i -e 's/,/;/g' -e 's/\./,/g' result_*
```

### Check process

To check and end all running fbae process if some are still running :

```bash
for i in 01 02 03 04 05 06 07 08 09 10 11 12 13; do ssh b313-$i ps aux | grep fbae | wc -l; done
```

```bash
for i in 01 02 03 04 05 06 07 08 09 10 11 12 13; do ssh b313-$i killall -9 fbae; done
```

## Launch command

You can find the commands done to do the measures of the Train algotithm and TCP [here](./Trains_TCP_Commands.md).

You can use the [launch_fbae](../../utils/launch_fbae.py) python script to launch fbae on multiple computer at the same time.

Here is ho you use it :

```bash
python launch_fbae.py [PATH_TO_PRIVATE_KEY_FOR_SSH] [FBAE_EXECUTABLE_DIRECTORY] [PATH_TO_OUTPUT_DIRECTOTY] [FBAE_ARGUMENTS]
```

`FBAE_ARGUMENTS` must not have an argument for the rank.

For example, a command using 12 sites for 1 minute :

```bash
python launch_fbae.py /netfs/tsp/student/2022/tschneider/.ssh/B313_Key /netfs/tsp/student/2022/tschneider/Documents/FBAE/build/src/main/ /netfs/tsp/student/2022/tschneider/Documents/FBAE/results -a T -c t -f 1000 -n 60000 -e 60 -E ./res/externalMeasure.json -s 1024 -S ./res/sites_b313/sites_12_b313.json -w 10
```


## MPI Launch command

You can find the commands done to do the measures using MPI [here](./MPI_Commands.md).

The command to do measures using MPI is :

```bash
mpiexec -np [SITES_NUMBER] --output-filename [PATH_TO_RESULT_DIRECTORY] -hostfile [PATH_TO_HOSTFILE] ./fbae [FBAE_ARGUMENTS]
```

For an example of `hostfile`, there is examples in tne resource folder [here](../../res/sites_b313/)

`FBAE_ARGUMENTS` needs to contain an argument for the rank which will be ignored, and cannot be `99`.

For example, a command using 2 sites for 1 minute :

```bash
mpiexec -np 2 --output-filename ../../../results/29075 -hostfile ./res/sites_b313/hosts_2_b313.txt ./fbae -a M -r 0 -f 1000 -e 60 -n 60000 -E ./res/externalMeasure.json -s 1024 -S ./res/sites_b313/sites_2_b313.json -w 10
```