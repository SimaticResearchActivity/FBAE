/**

    @file      testSerializationOverhead.cpp
    @brief     Unitary tests related to serialization overhead
    @details
    @author    Michel Simatic
    @date      2/5/24
    @copyright GNU Affero General Public License

**/
#include <gtest/gtest.h>

#include <vector>

#include "Arguments.h"

namespace fbae_test_arguments {

using namespace std;

const auto expectedTrainsNb{4};
const auto expectedTcpMaxSizeForOneWrite{32768};

class ArgumentsTest : public testing::Test {
 protected:
  const vector<HostTuple> sites;
};

TEST_F(ArgumentsTest, algoArgumentFound) {
  Arguments arguments{
      sites, "anotherArgument=42,trainsNb=4,yetAnotherArgument=43", ""};
  EXPECT_EQ(expectedTrainsNb, arguments.getIntInAlgoArgument("trainsNb", 0));
}

TEST_F(ArgumentsTest, algoArgumentNotFound) {
  Arguments arguments{sites, "nonsense=4", ""};
  EXPECT_EQ(expectedTrainsNb,
            arguments.getIntInAlgoArgument("trainsNb", expectedTrainsNb));
}

TEST_F(ArgumentsTest, algoArgumentNonsense) {
  Arguments arguments{sites, "trainsNb=notAnInteger", ""};
  EXPECT_EQ(expectedTrainsNb,
            arguments.getIntInAlgoArgument("trainsNb", expectedTrainsNb));
}

TEST_F(ArgumentsTest, commArgumentFound) {
  Arguments arguments{
      sites, "",
      "anotherArgument=42,tcpMaxSizeForOneWrite=32768,yetAnotherArgument=43"};
  EXPECT_EQ(expectedTcpMaxSizeForOneWrite,
            arguments.getIntInCommArgument("tcpMaxSizeForOneWrite", 0));
}

TEST_F(ArgumentsTest, commArgumentNotFound) {
  Arguments arguments{sites, "", "nonsense=32768"};
  EXPECT_EQ(expectedTcpMaxSizeForOneWrite,
            arguments.getIntInCommArgument("tcpMaxSizeForOneWrite",
                                           expectedTcpMaxSizeForOneWrite));
}

TEST_F(ArgumentsTest, commArgumentNonsense) {
  Arguments arguments{sites, "", "tcpMaxSizeForOneWrite=notAnInteger"};
  EXPECT_EQ(expectedTcpMaxSizeForOneWrite,
            arguments.getIntInCommArgument("tcpMaxSizeForOneWrite",
                                           expectedTcpMaxSizeForOneWrite));
}
}  // namespace fbae_test_arguments
