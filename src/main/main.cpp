#include <future>
#include <iostream>
#include "OptParserExtended.h"
#include "AlgoLayer/Sequencer/Sequencer.h"
#include "AlgoLayer/LCR/LCR.h"
#include "SessionLayer/PerfMeasures/PerfMeasures.h"
#include "CommLayer/Tcp/Tcp.h"
#include "AlgoLayer/BBOBB/BBOBB.h"
#include "Logger/Logger.h"

using namespace std;
using namespace mlib;

unique_ptr<CommLayer> concreteCommLayer(OptParserExtended const &parser)
{
    char commId = parser.getoptStringRequired('c')[0];
    switch(commId)
    {
        case 't': return make_unique<Tcp>();
        default:
            std::cerr << "ERROR: Argument for Broadcast Algorithm is \"" << commId << "\""
                      << " which is not the identifier of a defined communication layer"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
    }
}

unique_ptr<AlgoLayer> concreteAlgoLayer(OptParserExtended const &parser)
{
    char algoId = parser.getoptStringRequired('a')[0];
    switch(algoId)
    {
        case 'S': return make_unique<Sequencer>(concreteCommLayer(parser));
        case 'B' : return make_unique<BBOBB>(concreteCommLayer(parser));
        case 'L' : return make_unique<LCR>(concreteCommLayer(parser));
        default:
            std::cerr << "ERROR: Argument for Broadcast Algorithm is " << algoId
                      << " which is not the identifier of a defined algorithm"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
//    initializeLogger();
    //
    // Take care of program arguments
    //
    OptParserExtended parser{
            "a:algo algo_identifier \t Broadcast Algorithm\n\t\t\t\t\t\tB = BBOBB\n\t\t\t\t\t\tS = Sequencer base\n\t\t\t\t\t\tL = LCR",
            "c:comm communicationLayer_identifier \t Communication layer to be used\n\t\t\t\t\t\tt = TCP",
            "f:frequency number \t [optional] Number of PerfMessage sessionLayer messages which must be sent each second (By default, a PerfMessage is sent when receiving a PerfResponse)",
            "h|help \t Show help message",
            "m:maxBatchSize number_of_messages \t [optional] Maximum size of batch of messages (if specified algorithm allows batch of messages; By default, maxBatchSize is unlimited)",
            "n:nbMsg number \t Number of messages to be sent",
            "r:rank rank_number \t Rank of process in site file (if 99, all algorithm participants are executed within threads in current process)",
            "s:size size_in_bytes \t Size of messages sent by a client (must be in interval [31,65515])",
            "S:site siteFile_name \t Name (including path) of the sites file to be used",
            "v|verbose \t [optional] Verbose display required",
            "w:warmupCooldown number \t [optional] Number in [0,99] representing percentage of PerfMessage sessionLayer messages which will be considered as part of warmup phase or cool down phase and thus will not be measured for ping (By default, percentage is 0%)"
    };

    int nonopt;
    if (int ret ; (ret = parser.parse (argc, argv, &nonopt)) != 0)
    {
        if (ret == 1)
            cerr << "Unknown option: " << argv[nonopt] << " Valid options are : " << endl
                 << parser.synopsis () << endl;
        else if (ret == 2)
            cerr << "Option " << argv[nonopt] << " requires an argument." << endl;
        else if (ret == 3)
            cerr << "Invalid options combination: " << argv[nonopt] << endl;
        exit (1);
    }
    if (nonopt < argc)
    {
        cerr << "ERROR: There is a non-option argument '" << argv[nonopt]
             << "' which cannot be understood. Please run again program but without this argument" << endl
             << parser.synopsis () << endl;
        exit(1);
    }

    if ((argc == 1) || parser.hasopt ('h'))
    {
        //No arguments on command line or help required. Show help and exit.
        cerr << "Usage:" << endl;
        cerr << parser.synopsis () << endl;
        cerr << "Where:" << endl
             << parser.description () << endl;
        exit (0);
    }

    Arguments arguments{parser};

    //
    // Launch the application
    //
    if (arguments.getRank() != specialRankToRequestExecutionInTasks)
    {
        PerfMeasures session{arguments, arguments.getRank(), concreteAlgoLayer(parser)};
        session.execute();
    }
    else
    {
        size_t nbSites{arguments.getSites().size()};
        vector<unique_ptr<PerfMeasures>> sessions;
        vector<future<void>> sessionTasks;
        for (uint8_t rank = 0 ; rank < static_cast<uint8_t>(nbSites) ; ++rank)
        {
            sessions.emplace_back(make_unique<PerfMeasures>(arguments, rank, concreteAlgoLayer(parser)));
            sessionTasks.emplace_back(std::async(std::launch::async, &PerfMeasures::execute, sessions.back().get()));
        }
        for (auto& t: sessionTasks)
            t.get();
    }
    return EXIT_SUCCESS;
}
