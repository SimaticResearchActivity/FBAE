#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>
#include <thread>

using json = nlohmann::json;
using namespace std;

void extract_json_data(const char* resultFilePath) {
    ifstream file(resultFilePath);
    json data = json::parse(file);

    cout << "Host consumption :" << data["host"]["consumption"] << endl;

    file.close();
}

void call_scaphandre() {
    string const resultFilePath = "results.json";

    // Fork a child process
    if (pid_t const pid = fork(); pid == -1) {
        cerr << "Fork failed." << endl;
    }
    else if (pid == 0) {
        string const command = "scaphandre json -s 0 --step-nano 1000000 --max-top-consumers 10 --resources -f " + resultFilePath;

        // Child process: execute the command
        setpgid(0, 0);
        //system(command.c_str());
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        cout << "Command execution finished" << endl;
    }
    else {
        // Parent process

        this_thread::sleep_for(10ms);
        cout << "Wait over" << endl;

        // Check if the child process is still running
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == 0) {
            // Child process is still running; terminate it
            cout << "Terminating child process." << endl;
            kill(pid, SIGTERM);

            // Optionally, wait for the child process to terminate gracefully
            //sleep(1);

            // Ensure the process is terminated
            kill(pid, SIGKILL);

            // Wait for the child process to clean up
            waitpid(pid, &status, 0);

            cout << "Child process terminated." << endl;
        } else {
            // Child process terminated on its own
            cout << "Command executed and finished within 10 seconds." << endl;
        }

        //extract_json_data(resultFilePath.c_str());
    }
}

/*
void call_scaphandre() {
    string const resultFilePath = "results.json";

    //extract_data(resultFilePath.c_str());

    //return 0;

    // Fork a child process
    if (pid_t const pid = fork(); pid == -1) {
        cerr << "Fork failed." << endl;
    }
    else if (pid == 0) {
        string const command = "scaphandre json -t 1 -s 0 --step-nano 1000000 --max-top-consumers 10 --resources -f " + resultFilePath;

        // Child process: execute the command
        setpgid(0, 0);
        //system(command.c_str());
        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        cout << "Command execution finished" << endl;
    }
    else {
        // Parent process

        this_thread::sleep_for(500ms);
        cout << "Wait over" << endl;

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            cout << "Waitpid failed" << endl;
        }
        else {
            if (!(WIFEXITED(status))) {
                cout << "Scaphandre command did not exit normally";
                return;
            }

            if (int const exitStatus = WEXITSTATUS(status); exitStatus == 0) {
                cout << "Scaphandre command executed successfully" << endl;
            }
            else {
                cout << "Scaphandre command exited with status " << std::to_string(exitStatus) << endl;
            }
        }

        extract_json_data(resultFilePath.c_str());
    }
}*/


int main() {
    call_scaphandre();

    cout << "Scaphandre called" << endl;

    return 0;
}
