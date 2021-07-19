/**
 * @file eval.cpp
 * @brief Simple expression evaluator.
 */

#include "eval.hpp"

#include "ast.hpp"

auto eval(const Expr& root) -> std::size_t {
  return root.Evaluate();
}
