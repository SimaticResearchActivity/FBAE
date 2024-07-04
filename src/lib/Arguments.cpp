#include "Arguments.h"
#include <fstream>
#include <format>
#include "cereal/archives/json.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

const std::string_view networkLevelMulticastAddressForTests{"239.255.0.1"};

Arguments::Arguments(std::vector<HostTuple> const& sites, bool isUsingNetworkLevelMulticast)
    : sites{sites}
    , usingNetworkLevelMulticast{isUsingNetworkLevelMulticast}
{
    if (isUsingNetworkLevelMulticast) {
        networkLevelMulticastAddress = networkLevelMulticastAddressForTests;
    }
}

Arguments::Arguments(mlib::OptParserExtended const& parser)
: nbMsg{parser.getoptIntRequired('n', logger)}
, rank{static_cast<rank_t>(parser.getoptIntRequired('r', logger))}
, sizeMsg{parser.getoptIntRequired('s', logger)}
, siteFile{parser.getoptStringRequired('S', logger)}
{
    if (parser.hasopt('f')) {
        frequency = parser.getoptIntRequired('f', logger);
        if (frequency == 0) {
            LOG4CXX_FATAL_FMT(logger, "Argument for frequency must be greater than 0 (zero) \n {}", parser.synopsis());
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('m')) {
        maxBatchSize = parser.getoptIntRequired('m', logger);
        if (maxBatchSize < sizeMsg) {
            LOG4CXX_FATAL_FMT(logger, "Argument for maxBatchSize must be greater than argument for sizeMsg \n {}", parser.synopsis());
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('M')) {
        if (!parser.getopt('M', networkLevelMulticastAddress)) {
            LOG4CXX_FATAL_FMT(logger, "Option -M is missing\n\nUsage:\n{}\nWhere:\n{}\n", parser.synopsis(), parser.description());
            exit(1);
        }
        usingNetworkLevelMulticast = true;
    }

    if (parser.hasopt('P')) {
        networkLevelMulticastPort = static_cast<uint16_t>(parser.getoptIntRequired('P', logger));
    }

    if (parser.hasopt('w')) {
        warmupCooldown = parser.getoptIntRequired('w', logger);
        if (warmupCooldown > 99) {
            LOG4CXX_FATAL_FMT(logger, "Argument for warmupCooldown must be in [0,99] \n {}", parser.synopsis());
            exit(EXIT_FAILURE);
        }
    }

    if (sizeMsg < minSizeClientMessageToBroadcast || sizeMsg > maxLength)
    {
        LOG4CXX_FATAL_FMT(logger, "Argument for size of messages is {} which is not in interval [ {} , {} ] \n {}", sizeMsg, minSizeClientMessageToBroadcast, maxLength, parser.synopsis());
        exit(EXIT_FAILURE);
    }

    // Initialize sites with contents of siteFile
    std::ifstream ifs(siteFile);
    if(ifs.fail()){
        LOG4CXX_FATAL_FMT(logger, "JSON file \"{}\" does not exist\n {}", siteFile, parser.synopsis());
        exit(EXIT_FAILURE);
    }
    cereal::JSONInputArchive iarchive(ifs); // Create an input archive
    iarchive(sites);

    std::string dump;
    for (auto const& [host, port] : sites) {
        dump += std::format("Site {}:{}\n", host, port);
    }
    LOG4CXX_INFO_FMT(logger, "Contents of {}\n{}", siteFile, dump);

    // Check that rank value is consistent with contents of site file
    if ((rank != specialRankToRequestExecutionInTasks) && (rank > sites.size() - 1))
    {
        LOG4CXX_FATAL_FMT(logger, "You specified a rank of {}, but there are only {} sites specified in JSON file \"{}\"\n{}",
                          rank,
                          sites.size(),
                          siteFile,
                          parser.synopsis());
        exit(EXIT_FAILURE);
    }
}

[[nodiscard]] std::string
Arguments::asCsv(std::string const &algoStr, std::string const &commLayerStr, std::string const &rankStr) const
{
    return std::format("{},{},{},{},{},{:d},{}%,{},{},{}",
        algoStr,
        commLayerStr,
        (usingNetworkLevelMulticast ? "true" : "false"),
        frequency,
        maxBatchSize,
        nbMsg,
        warmupCooldown,
        rankStr,
        sizeMsg,
        siteFile
        );
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

int Arguments::getWarmupCooldown() const {
    return warmupCooldown;
}

bool Arguments::isUsingNetworkLevelMulticast() const {
    return usingNetworkLevelMulticast;
}