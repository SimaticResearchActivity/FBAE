//
// Created by simatic on 11/25/23.
//

#include "TcpCommPeer.h"
#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"

TcpCommPeer::TcpCommPeer(boost::asio::ip::tcp::socket *psock)
        : psock{psock}
{
}

void TcpCommPeer::disconnect() {
    psock->close();
}

struct ForLength
{
    size_t size{};

    // This method lets cereal know which data members to serialize
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(size); // serialize things by passing them to the archive
    }
};

void TcpCommPeer::sendMsg(std::string_view msg) {
    ForLength forLength{msg.length()};
    std::stringstream oStream;
    {
        cereal::BinaryOutputArchive oarchive(oStream); // Create an output archive
        oarchive(forLength); // Write the data to the archive
    } // archive goes out of scope, ensuring all contents are flushed

    auto sWithLength = oStream.str();
    sWithLength.append(msg);

    boost::asio::write(*psock, boost::asio::buffer(sWithLength.data(), sWithLength.length()));}
