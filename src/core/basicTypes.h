//
// Created by simatic on 1/31/24.
//

#ifndef FBAE_BASIC_TYPES_H
#define FBAE_BASIC_TYPES_H

#include <cstdint>

namespace fbae::core {

/**
 * @brief Type for storing ranks of participants to protocol
 */
using rank_t = uint8_t;

/**
 * @brief Type for storing message identifiers
 */
using MsgId_t = uint8_t;

}  // namespace fbae::core

#endif  // FBAE_BASIC_TYPES_H
