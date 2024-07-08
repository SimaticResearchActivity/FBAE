#include <future>
#include <iostream>
#include "OptParserExtended.h"
#include "AlgoLayer/Sequencer/Sequencer.h"
#include "AlgoLayer/LCR/LCR.h"
#include "SessionLayer/PerfMeasures/PerfMeasures.h"
#include "CommLayer/Tcp/Tcp.h"
#include "AlgoLayer/BBOBB/BBOBB.h"

using namespace std;
using namespace mlib;

unique_ptr<CommLayer> concreteCommLayer(OptParserExtended const &parser, fbae::LoggerPtr logger)
{
    char commId = parser.getoptStringRequired('c', logger)[0];
    switch(commId)
    {
        case 't': return make_unique<Tcp>();
        default:
            LOG4CXX_FATAL_FMT(logger, "Argument for Broadcast Algorithm is \"{}\" which is not the identifier of a defined communication layer. \n{}", commId, parser.synopsis());
            exit(EXIT_FAILURE);
    }
}

unique_ptr<AlgoLayer> concreteAlgoLayer(OptParserExtended const &parser, fbae::LoggerPtr logger)
{
    char algoId = parser.getoptStringRequired('a', logger)[0];
    switch(algoId)
    {
        case 'S': return make_unique<Sequencer>(concreteCommLayer(parser, logger));
        case 'B' : return make_unique<BBOBB>(concreteCommLayer(parser, logger));
        case 'L' : return make_unique<LCR>(concreteCommLayer(parser, logger));
        default:
            LOG4CXX_FATAL_FMT(logger, "Argument for Broadcast Algorithm is \"{}\" which is not the identifier of a defined algorithm. \n{}", algoId, parser.synopsis());
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    auto logger = fbae::getLogger("fbae");

    //
    // Take care of program arguments
    //
    OptParserExtended parser{
            "a:algo algo_identifier \t Broadcast Algorithm\n\t\t\t\t\t\tB = BBOBB\n\t\t\t\t\t\tS = Sequencer base\n\t\t\t\t\t\tL = LCR",
            "A:algoArgument string \t [optional] String to specify an argument to be used by a specific broadcast algorithm (e.g. trainsNb=2 to specify that Trains algorithm must use 2 trains in parallel)",
            "c:comm communicationLayer_identifier \t Communication layer to be used\n\t\t\t\t\t\tt = TCP",
            "C:commArgument string \t [optional] String to specify an argument to be used by a specific communication layer (e.g. tcpMaxSizeForOneWrite=32768 to specify that Tcp communication layer will send a message and its length inside a single message as long as message length is below 32768 bytes)",
            "f:frequency number \t [optional] Number of PerfMessage sessionLayer messages which must be sent each second (By default, a PerfMessage is sent when receiving a PerfResponse)",
            "h|help \t Show help message",
            "m:maxBatchSize number_of_messages \t [optional] Maximum size of batch of messages (if specified algorithm allows batch of messages; By default, maxBatchSize is unlimited)",
            "M:multicastAddress IP_address \t [optional] If specified, indicates that participant must use network-level multicast and the IP address to be used (IP address can be either IPv4 e.g. 239.255.0.1 or IPv6 e.g. ff31::8000:1234)",
            "n:nbMsg number \t Number of messages to be sent",
            "P:multicastPort number \t [optional] When -M|--multicastAddress is specified, indicates port number to be used for network-level multicast (By default, port 30001 is used)",
            "r:rank rank_number \t Rank of process in site file (if 99, all algorithm participants are executed within threads in current process)",
            "s:size size_in_bytes \t Size of messages sent by a client (must be in interval [31,65515])",
            "S:site siteFile_name \t Name (including path) of the sites file to be used",
            "w:warmupCooldown number \t [optional] Number in [0,99] representing percentage of PerfMessage sessionLayer messages which will be considered as part of warmup phase or cool down phase and thus will not be measured for ping (By default, percentage is 0%)"
    };

    int nonopt;
    if (int ret ; (ret = parser.parse (argc, argv, &nonopt)) != 0)
    {
        if (ret == 1)
            LOG4CXX_FATAL_FMT(logger, "Unknown option: {} Valid options are : \n{}", argv[nonopt], parser.synopsis());
        else if (ret == 2)
            LOG4CXX_FATAL_FMT(logger, "Option {} requires an argument.", argv[nonopt]);
        else if (ret == 3)
            LOG4CXX_FATAL_FMT(logger, "Invalid options combination: {}", argv[nonopt]);
        exit (1);
    }
    if (nonopt < argc)
    {
        LOG4CXX_FATAL_FMT(logger, "'There is a non-option argument {}' which cannot be understood. Please run again program but without this argument. \n{}", argv[nonopt], parser.synopsis());
        exit(1);
    }

    if ((argc == 1) || parser.hasopt ('h'))
    {
        //No arguments on command line or help required. Show help and exit.
        LOG4CXX_FATAL_FMT(logger, "Usage: \n{}\n Where: \n{}", parser.synopsis(), parser.description());
        exit (0);
    }

    Arguments arguments{parser};

    //
    // Launch the application
    //
    if (rank_t argRank = arguments.getRank(); argRank != specialRankToRequestExecutionInTasks)
    {
        PerfMeasures session{arguments, argRank, concreteAlgoLayer(parser, logger)};
        session.execute();
    }
    else
    {
        size_t nbSites{arguments.getSites().size()};
        vector<unique_ptr<PerfMeasures>> sessions;
        vector<future<void>> sessionTasks;
        for (uint8_t rank = 0 ; rank < static_cast<uint8_t>(nbSites) ; ++rank)
        {
            sessions.emplace_back(make_unique<PerfMeasures>(arguments, rank, concreteAlgoLayer(parser, logger)));
            sessionTasks.emplace_back(std::async(std::launch::async, &PerfMeasures::execute, sessions.back().get()));
        }
        for (auto& t: sessionTasks)
            t.get();
    }
    return EXIT_SUCCESS;
}
