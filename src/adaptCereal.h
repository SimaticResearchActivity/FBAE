/**

    @file      adaptCereal.h
    @brief     Include file to adapt default behaviour of Cereal to FBAE requirements
    @details   This file is to be included before any file related to Cereal, so that it will change Cereal default
               behaviour.
    @author    Michel Simatic
    @date      2/21/24
    @copyright GNU Affero General Public License

**/
#ifndef FBAE_ADAPT_CEREAL_H
#define FBAE_ADAPT_CEREAL_H

/**
 * @brief By default, Cereal serializes size_t as uint64_t (in include/cereal/macros.hpp) ==> We define it as uint32_t
 * in order to prevent bytes waste in serialization.
 */
#define CEREAL_SIZE_TYPE uint32_t
#endif //FBAE_ADAPT_CEREAL_H
