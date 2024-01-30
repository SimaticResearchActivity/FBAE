# Python script to launch fbae on all of the machines mentionned in
# a site_file
#
# Usage
#   python3 launch_fbae.py path_to_fbae_executable path_to_result_directory all_fbae_arguments_except_-r_or_--rank
#
# Example
#   If sites_8_machines.json contains the specification of 8 hosts/ports,
#   command
#      python3 launchFBAE.py /absolute_path/FBAE /aboslute_path/FBAE/results/ -a B -c t -n 5 -s 32 -S /absolute_path/FBAE/sites_8_machines.json
#   launches instances of fbae on each of these 8 hosts/ports. When these
#   instances are done, each generates a result file in
#   /aboslute_path/FBAE/results, the name of the file containing the
#   rank of the instancen, in its last character.
#   For instance, file /aboslute_path/FBAE/results/result_-a_B_-c_t_-n_5_-s_32_-S__netfs_inf_simatic_FBAE_sites_8_b313.json_--rank_0
#   contains the results produced by instance ranked 0 (see "--rank_0"
#   at the end of the name of the file.
#
import sys
import json
import subprocess

# Check there are enough arguments
if len(sys.argv) < 13:
    print(f"USAGE: {sys.argv[0]} path_to_fbae_executable path_to_result_directory all_fbae_arguments_except_-r_or_--rank")
    exit(1)

# Check that rank of process is not defined in argument list (with -r or --rank)
try:
    sys.argv.index("-r")
    print(f"-r found in argument list! You must remove it as {sys.argv[0]} script generates it automatically.")
    exit(1)
except ValueError:
    try:
        sys.argv.index("--rank")
        print(f"--rank found in argument list! You must remove it as {sys.argv[0]} script generates it automatically.")
        exit(1)
    except ValueError:
        # Neither -r, nor --rank were found in argument list
        pass

# Find the name of the sites file to be used (It relative path to fbae executable
# is defined thanks to -S or --site argument)
try:
    idx = sys.argv.index("-S")
except ValueError:
    try:
        idx = sys.argv.index("--site")
    except ValueError:
        print("Missing -S or --site argument in argument list")
        exit(1)
siteFile = sys.argv[idx+1]

# Load json contents of siteFile
with open(siteFile) as f:
    s = ""
    for line in f.readlines():
        s += line.strip()
contentsSiteFile = json.loads(s)

# Launch fbae on the different hosts mentionned in contentsSiteFile
commonArgsWithSpace = " ".join(sys.argv[3:])
commonArgsWithUnderscore = ("_".join(sys.argv[3:])).replace('/','_')
rank = 0
for site in contentsSiteFile['sites']:
    commandArgs = commonArgsWithSpace + " --rank " + str(rank)
    commandArgsWithUnderscore = commonArgsWithUnderscore + "_--rank_" + str(rank)
    cmd = "ssh " + site['tuple_element0'] + " 'nohup " + sys.argv[1] +"/fbae " + commandArgs + " &> " + sys.argv[2] + "/result_" + commandArgsWithUnderscore + "' &"
    print(cmd)
    subprocess.run(cmd, shell=True)
    rank += 1


