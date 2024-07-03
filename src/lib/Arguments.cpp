#include "Arguments.h"
#include <fstream>
#include <iostream>
#include <format>
#include "cereal/archives/json.hpp"
#include "cereal/types/tuple.hpp"
#include "cereal/types/vector.hpp"

Arguments::Arguments(std::vector<HostTuple> const& sites)
    : sites{sites}
{
}

Arguments::Arguments(mlib::OptParserExtended const& parser)
: nbMsg{parser.getoptIntRequired('n')}
, rank{static_cast<rank_t>(parser.getoptIntRequired('r'))}
, sizeMsg{parser.getoptIntRequired('s')}
, siteFile{parser.getoptStringRequired('S')}
{
    if (parser.hasopt('f')) {
        frequency = parser.getoptIntRequired('f');
        if (frequency == 0) {
            LOG4CXX_FATAL_FMT(logger, "Argument for frequency must be greater than 0 (zero) \n {}", parser.synopsis());
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('m')) {
        maxBatchSize = parser.getoptIntRequired('m');
        if (maxBatchSize < sizeMsg) {
            LOG4CXX_FATAL_FMT(logger, "Argument for maxBatchSize must be greater than argument for sizeMsg \n {}", parser.synopsis());
            exit(EXIT_FAILURE);
        }
    }

    if (parser.hasopt('w')) {
        warmupCooldown = parser.getoptIntRequired('w');
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
        std::cerr << "ERROR: You specifed a rank of " << rank << ", but there are only " << sites.size() << " sites specified in JSON file \"" << siteFile << "\"\n"
                << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }
}

[[nodiscard]] std::string
Arguments::asCsv(std::string const &algoStr, std::string const &commLayerStr, std::string const &rankStr) const
{
    return std::format("{},{},{},{},{:d},{}%,{},{},{}",
        algoStr,
        commLayerStr,
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

int Arguments::getWarmupCooldown() const {
    return warmupCooldown;
}
