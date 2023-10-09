#include <iostream>
#include <thread>
#include "EnetCommLayer.h"
#include "OptParserExtended.h"
#include "SequencerAlgoLayer.h"
#include "SessionLayer.h"
#include "BBOBBAlgoLayer.h"

using namespace std;
using namespace mlib;

unique_ptr<AlgoLayer> concreteAlgoLayer(OptParserExtended const &parser)
{
    char algoId = parser.getoptStringRequired('a')[0];
    switch(algoId)
    {
        case 'S': return make_unique<SequencerAlgoLayer>();
        case 'B' : return make_unique<BBOBBAlgoLayer>();
        default:
            std::cerr << "ERROR: Argument for Broadcast Algorithm is " << algoId
                      << " which is not the identifier of a defined algorithm"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
    }
}

unique_ptr<CommLayer> concreteCommLayer(OptParserExtended const &parser)
{
    char commId = parser.getoptStringRequired('c')[0];
    switch(commId)
    {
        case 'e': return make_unique<EnetCommLayer>();
        default:
            std::cerr << "ERROR: Argument for Broadcast Algorithm is \"" << commId << "\""
                      << " which is not the identifier of a defined communication layer"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    //
    // Take care of program arguments
    //
    OptParserExtended parser{
            "a:algo algo_identifier \t Broadcast Algorithm\n\t\t\t\t\t\tS = Sequencer based",
            "c:comm communicationLayer_identifier \t Communication layer to be used\n\t\t\t\t\t\te = Enet (reliable)",
            "h|help \t Show help message",
            "n:nbMsg number \t Number of messages to be sent",
            "r:rank rank_number \t Rank of process in site file (if 99, all algorithm participants are executed within threads in current process)",
            "s:size size_in_bytes \t Size of messages sent by a client (must be in interval [22,65515])",
            "S:site siteFile_name \t Name (including path) of the sites file to be used",
            "v|verbose \t [optional] Verbose display required"
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

    Param param{parser};

    //
    // Launch the application
    //
    if (param.getRank() != specialRankToRequestExecutionInThreads)
    {
        SessionLayer session{param, param.getRank(), concreteAlgoLayer(parser), concreteCommLayer(parser)};
        session.execute();
    }
    else
    {
        size_t nbSites{param.getSites().size()};
        vector<unique_ptr<SessionLayer>> sessions;
        vector<jthread> sessionThreads;
        for (int rank = 0 ; rank < nbSites ; ++rank)
        {
            sessions.emplace_back(make_unique<SessionLayer>(param, rank, concreteAlgoLayer(parser), concreteCommLayer(parser)));
            sessionThreads.emplace_back(&SessionLayer::execute, sessions.back().get());
        }
        for (auto& t: sessionThreads)
            t.join();
    }
    return EXIT_SUCCESS;
}
