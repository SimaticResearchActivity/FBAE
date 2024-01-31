#include <algorithm>
#include <cassert>
#include <numeric>
#include "Measures.h"

Measures::Measures(size_t nb_rtts_max)
: rtts(nb_rtts_max+100)
{
}

void Measures::add(std::chrono::duration<double, std::milli> const& elapsed)
{
    size_t index{nbRtts++};
    assert( index < rtts.size());
    rtts[index] = elapsed;
}

std::string Measures::csvHeadline()
{
    return std::string { "nbReceivedMsg,nbSentMsg,ratio nbRcv/nbSent,Average (in ms),Min,Q(0.25),Q(0.5),Q(0.75),Q(0.99),Q(0.999),Q(0.9999),Max"};
}

std::string Measures::asCsv(int argNbMsg, size_t argNbClients)
{
    rtts.resize(nbRtts);
    std::ranges::sort(rtts);

    return std::string {
            std::to_string(rtts.size()) + ","
            + std::to_string(argNbMsg * argNbClients) + ","
            + std::to_string(static_cast<double>(nbRtts) / argNbMsg / static_cast<double>(argNbClients)) + ","
            + std::to_string((std::reduce(rtts.begin(), rtts.end()) / rtts.size()).count()) + ","
            + std::to_string(rtts[0].count()) + ","
            + std::to_string(rtts[rtts.size()/4].count()) + ","
            + std::to_string(rtts[rtts.size()/2].count()) + ","
            + std::to_string(rtts[rtts.size()*3/4].count()) + ","
            + std::to_string(rtts[rtts.size()*99/100].count()) + ","
            + std::to_string(rtts[rtts.size()*999/1000].count()) + ","
            + std::to_string(rtts[rtts.size()*9999/10000].count()) + ","
            + std::to_string(rtts[rtts.size()-1].count())
    };
}

