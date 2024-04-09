#include "Arguments.h"
#include <fstream>
#include <iostream>
#include "cereal/archives/json.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

auto networkLevelMulticastAddressForTests{"239.255.0.1"};

Arguments::Arguments(std::vector<HostTuple> const& sites, bool isUsingNetworkLevelMulticast)
    : sites{sites}
    , usingNetworkLevelMulticast{isUsingNetworkLevelMulticast}
{
    if (isUsingNetworkLevelMulticast) {
        networkLevelMulticastAddress = networkLevelMulticastAddressForTests;
    }
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

    if (parser.hasopt('M')) {
        if (!parser.getopt('M', networkLevelMulticastAddress)) {
            std::cout << "Option -M is missing\n\n";
            std::cout << "Usage:" << std::endl;
            std::cout << parser.synopsis() << std::endl;
            std::cout << "Where:" << std::endl
                      << parser.description() << std::endl;
            exit(1);
        }
        usingNetworkLevelMulticast = true;
    }

    if (parser.hasopt('P')) {
        networkLevelMulticastPort = static_cast<uint16_t>(parser.getoptIntRequired('P'));
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
            + (usingNetworkLevelMulticast ? "true" : "false") + ","
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
    return std::string { "algoLayer,commLayer,isUsingNetworkLevelMulticast,frequency,maxBatchSize,nbMsg,warmupCooldown,rank,sizeMsg,siteFile"};
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

std::string_view Arguments::getNetworkLevelMulticastAddress() const {
    return networkLevelMulticastAddress;
}

uint16_t Arguments::getNetworkLevelMulticastPort() const {
    return networkLevelMulticastPort;
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

bool Arguments::isUsingNetworkLevelMulticast() const {
    return usingNetworkLevelMulticast;
}