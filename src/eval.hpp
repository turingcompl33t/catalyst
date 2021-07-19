/**
 * @file eval.hpp
 * @brief Simple expression evaluator.
 */

#ifndef EVAL_HPP
#define EVAL_HPP

#include <cstdlib>

class Expr;

/**
 * Evaluate an expression.
 * @param root The root of the expression.
 * @return The result of expression evaluation
 */
auto eval(const Expr& root) -> std::size_t;

#endif // EVAL_HPP
