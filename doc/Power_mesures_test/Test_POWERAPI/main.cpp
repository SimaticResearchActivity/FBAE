#include <iostream>
#include <csignal>
#include <sys/wait.h>

#include <bsoncxx/builder/basic/document.hpp>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

using namespace std;

pid_t startMeasures() {
    // Fork a child process for smartwatts
    pid_t const smartwatts_pid = fork();
    if (smartwatts_pid == -1) {
        cerr << "Smartwatts Fork failed." << endl;
    }
    else if (smartwatts_pid == 0) {
        string const smartwatts_command = "python3 -m smartwatts --config-file ../res/smartwatts_MongoDB_config_file.json";

        setpgid(0, 0);
        execl("/bin/sh", "sh", "-c", smartwatts_command.c_str(), nullptr);

        cerr << "Execution failed" << endl;
        exit(EXIT_FAILURE);
    }
    return smartwatts_pid;
}

void stopMeasures(pid_t const smartwatts_pid) {
    // Kill child processes
    if (waitpid(smartwatts_pid, nullptr, WNOHANG) == 0) {
        std::cout << "Killing first child process...\n";

        killpg(smartwatts_pid, SIGTERM);

        if (killpg(smartwatts_pid, SIGKILL) == -1) {
            cerr << "Failed to kill smartwatts_pid" << endl;
        }
    }
    else {
        std::cout << "First child process has already terminated.\n";
    }

    // Wait for the child processes to ensure they are terminated
    waitpid(smartwatts_pid, nullptr, 0);
}

void analyseMesures(mongocxx::collection collection) {
    auto cursor_all = collection.find({});

    //auto cursor_filtered = collection.find(make_document(kvp("power", make_document(kvp("$gt", 0)))));

    std::cout << "collection " << collection.name()
              << " contains these documents:" << std::endl;

    for (auto doc : cursor_all) {
        std::cout << bsoncxx::to_json(doc, bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
    }
    std::cout << std::endl;
}

int main()
{
    string const DB_name = "PowerAPI_db";
    string const DB_uri = "mongodb://localhost:27017";
    string const sensor_collection_name = "sensor_values";
    string const power_collection_name = "power_values";

    mongocxx::instance instance{}; // This should be done only once.

    // Create a connection to the MongoDB server
    mongocxx::uri const uri(DB_uri);
    mongocxx::client const client(uri);

    auto const db = client[DB_name];

    // Drop collections
    db[sensor_collection_name].drop();
    db[power_collection_name].drop();

    cout << "Start Measures" << endl;
    //startMeasures(DB_name, sensor_collection_name, &sensor_pid, power_collection_name, &smartwatts_pid);
    pid_t const smartwatts_pid = startMeasures();

    cout << "pid created: power: " + to_string(smartwatts_pid) << endl;
    cout << "Wait..." << endl;
    sleep(10);

    std::cout << "Stop Measures" << std::endl;
    //stopMeasures(sensor_pid, smartwatts_pid);
    stopMeasures(smartwatts_pid);

    analyseMesures(db[power_collection_name]);
    return 0;
}
