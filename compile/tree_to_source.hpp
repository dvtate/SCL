//
// Created by tate on 20/04/2021.
//

#ifndef SCL_TREE_TO_SOURCE_HPP
#define SCL_TREE_TO_SOURCE_HPP

#include <string>

#include "../parse/parse.hpp"

/**
 * Converts and AST into representative source code
 * @param tree
 * @return
 */
std::string tree_to_source(AST& tree);

#endif //SCL_TREE_TO_SOURCE_HPP
