/**

    @file      SessionLayerMsg.cpp
    @brief     Implementation of methods declared in SessionLayerMsg.h
    @author    Michel Simatic
    @date      3/9/24
    @copyright GNU Affero General Public License

**/

#include "SessionLayerMsg.h"

#include <cassert>

std::string fbae_SessionLayer::SessionBaseClass::getPayload() {
  assert(false);
  return "SessionBaseClass::getPayload() should never be called";
}

std::string fbae_SessionLayer::SessionTest::getPayload() { return payload; }
