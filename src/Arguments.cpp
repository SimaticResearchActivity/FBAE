#include "Arguments.h"
#include <fstream>
#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>

Arguments::Arguments(std::vector<HostTuple> const& sites)
    : sites{sites}
{
}

Arguments::Arguments(mlib::OptParserExtended const& parser)
: nbMsg{parser.getoptIntRequired('n')}
, rank{static_cast<rank_t>(parser.getoptIntRequired('r'))}
, sizeMsg{parser.getoptIntRequired('s')}
, siteFile{parser.getoptStringRequired('S')}
, verbose{parser.hasopt ('v')}
{
    if (parser.hasopt('f')) {
        frequency = parser.getoptIntRequired('f');
        if (frequency == 0) {
            std::cerr << "ERROR: Argument for frequency must be greater than 0 (zero)"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('m')) {
        maxBatchSize = parser.getoptIntRequired('m');
        if (maxBatchSize < sizeMsg) {
            std::cerr << "ERROR: Argument for maxBatchSize must be greater than argument for sizeMsg"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('w')) {
        warmupCooldown = parser.getoptIntRequired('w');
        if (warmupCooldown > 99) {
            std::cerr << "ERROR: Argument for warmupCooldown must be in [0,99]"
                      << std::endl
                      << parser.synopsis () << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (sizeMsg < minSizeClientMessageToBroadcast || sizeMsg > maxLength)
    {
        std::cerr << "ERROR: Argument for size of messages is " << sizeMsg
             << " which is not in interval [ " << minSizeClientMessageToBroadcast << " , " << maxLength << " ]"
             << std::endl
             << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }

    // Initialize sites with contents of siteFile
    std::ifstream ifs(siteFile);
    if(ifs.fail()){
        std::cerr << "ERROR: JSON file \"" << siteFile << "\" does not exist\n"
                << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }
    cereal::JSONInputArchive iarchive(ifs); // Create an input archive
    iarchive(sites);

    if (verbose)
    {
        std::cout << "Contents of " << siteFile << "\n";
        for (auto const& [host, port]: sites) {
            std::cout << "\tSite " << host << ":" << port << "\n";
        }
    }

    // Check that rank value is consistent with contents of site file
    if ((rank != specialRankToRequestExecutionInTasks) && (rank > sites.size() - 1))
    {
        std::cerr << "ERROR: You specifed a rank of " << rank << ", but there are only " << sites.size() << " sites specified in JSON file \"" << siteFile << "\"\n"
                << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }
}

[[nodiscard]] std::string
Arguments::asCsv(std::string const &algoStr, std::string const &commLayerStr, std::string const &rankStr) const
{
    return std::string {
        algoStr + ","
        + commLayerStr + ","
        + std::to_string(frequency) + ","
        + std::to_string(maxBatchSize) + ","
        + std::to_string(nbMsg) + ","
        + std::to_string(warmupCooldown) + "%,"
        + rankStr  + ","
        + std::to_string(sizeMsg) + ","
        + siteFile};
}

std::string Arguments::csvHeadline()
{
    return std::string { "algoLayer,commLayer,frequency,maxBatchSize,nbMsg,warmupCooldown,rank,sizeMsg,siteFile"};
}

int Arguments::getFrequency() const {
    return frequency;
}

int Arguments::getMaxBatchSize() const {
    return maxBatchSize;
}

int64_t Arguments::getNbMsg() const {
    return nbMsg;
}

rank_t Arguments::getRank() const
{
    return rank;
}

std::vector<std::tuple<std::string, int>> Arguments::getSites() const
{
    return sites;
}

int Arguments::getSizeMsg() const {
    return sizeMsg;
}
bool Arguments::getVerbose() const {
    return verbose;
}

int Arguments::getWarmupCooldown() const {
    return warmupCooldown;
}
