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
    if ((rank != specialRankToRequestExecutionInThreads) && (rank > sites.size() - 1))
    {
        std::cerr << "ERROR: You specifed a rank of " << rank << ", but there are only " << sites.size() << " sites specified in JSON file \"" << siteFile << "\"\n"
                << parser.synopsis () << std::endl;
        exit(EXIT_FAILURE);
    }
}

[[nodiscard]] std::string Param::asCsv(std::string const& algoStr, std::string const& commLayerStr) const
{
    return std::string { algoStr + "," + commLayerStr + "," + std::to_string(frequency) + "," + std::to_string(maxBatchSize) + "," + std::to_string(nbMsg) + "," + std::to_string(rank)  + "," + std::to_string(sizeMsg) + "," + siteFile};
}

std::string Param::csvHeadline()
{
    return std::string { "algo,commLayer,frequency,maxBatchSize,nbMsg,rank,sizeMsg,siteFile"};
}

int Param::getFrequency() const {
    return frequency;
}

int Param::getMaxBatchSize() const {
    return maxBatchSize;
}

int Param::getNbMsg() const {
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

