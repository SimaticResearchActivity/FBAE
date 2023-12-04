#include "Param.h"
#include <fstream>
#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>

Param::Param(mlib::OptParserExtended const& parser)
: nbMsg{parser.getoptIntRequired('n')}
, rank{parser.getoptIntRequired('r')}
, sizeMsg{parser.getoptIntRequired('s')}
, siteFile{parser.getoptStringRequired('S')}
, verbose{parser.hasopt ('v')}
{
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
    return std::string { algoStr + "," + commLayerStr + "," + std::to_string(nbMsg) + "," + std::to_string(rank)  + "," + std::to_string(sizeMsg) + "," + siteFile};
}

std::string Param::csvHeadline()
{
    return std::string { "algo,commLayer,nbMsg,rank,sizeMsg,siteFile"};
}

int Param::getNbMsg() const {
    return nbMsg;
}

int Param::getRank() const
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
