#include "Param.h"
#include <fstream>
#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>

Param::Param(mlib::OptParserExtended const& parser)
: nbMsg{parser.getoptIntRequired('n')}
, rank{static_cast<uint8_t>(parser.getoptIntRequired('r'))}
, sizeMsg{parser.getoptIntRequired('s')}
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


    // Check that rank value is consistent with contents of site file
    if ((rank != specialRankToRequestExecutionInTasks) && (rank > sites.size() - 1))
    {
        std::cerr << "ERROR: You specifed a rank of " << rank << ", but there are only " << sites.size() << " sites specified in JSON file \"" << siteFile << "\"\n"
                << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }
}

[[nodiscard]] std::string
Param::asCsv(std::string const &algoStr, std::string const &rankStr) const
{
    return std::string {
        algoStr + ","
        + std::to_string(frequency) + ","
        + std::to_string(maxBatchSize) + ","
        + std::to_string(nbMsg) + ","
        + std::to_string(warmupCooldown) + "%,"
        + rankStr  + ","
        + std::to_string(sizeMsg) + ","
        + siteFile};
}

std::string Param::csvHeadline()
{
    return std::string { "algoLayer,frequency,maxBatchSize,nbMsg,warmupCooldown,rank,sizeMsg,siteFile"};
}

int Param::getFrequency() const {
    return frequency;
}

int Param::getMaxBatchSize() const {
    return maxBatchSize;
}

int64_t Param::getNbMsg() const {
    return nbMsg;
}

uint8_t Param::getRank() const
{
    return rank;
}

std::vector<std::tuple<std::string, int>> Param::getSites() const
{
    return sites;
}

int Param::getSizeMsg() const {
    return sizeMsg;
}
bool Param::getVerbose() const {
    return verbose;
}

int Param::getWarmupCooldown() const {
    return warmupCooldown;
}

