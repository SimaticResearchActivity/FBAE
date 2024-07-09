#pragma once

#include "adaptCereal.h"
#include "cereal/archives/binary.hpp"

/**
 * @brief Returns structure @S deserialized from @msgString
 * @details
 * Usage: Use <tt>auto data = deserializeStruct<StructName>(msgString)</tt>
 * @tparam S Name of structure to be read
 * @param msgString String containing structure
 * @return Structure deserialized from @msgString
 */
template <typename S>
S deserializeStruct(std::string&& msgString) {
  std::istringstream msgStream{std::move(msgString)};
  cereal::BinaryInputArchive archive(msgStream);  // Create an input archive
  S structure{};
  archive(structure);  // Read the structure from the archive
  return structure;
}

/**
 * @brief Returns a string containing serialized structure @S
 * Usage: Use <tt>auto sv = prepareMsg<EnumName,DataStructName>(msgId,
 * structure)</tt>
 * @tparam S Name of (Data) structure to be written in message
 * @param msgId to be stored in message
 * @param structure to be stored in message
 * @return String containing serialized structure @S
 */
template <typename S>
std::string serializeStruct(S structure) {
  std::stringstream o_stream;
  {
    cereal::BinaryOutputArchive archive(o_stream);  // Create an output archive
    archive(structure);  // Write the structure to the archive
  }  // archive goes out of scope, ensuring all contents are flushed
  return o_stream.str();
}