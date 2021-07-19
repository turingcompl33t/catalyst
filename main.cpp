/**
 * @file main.cpp
 * @brief Optimizer-driver.
 */

#include <cstdlib>
#include <iostream>

#include "ast.hpp"
#include "eval.hpp"
#include "optimizer.hpp"

auto test_leftwise_optimization() -> void {
  // 0 + 1
  auto input = BinaryAdditionExpr::Make(
    NumericConstantExpr::Make(0UL),
    NumericConstantExpr::Make(1UL));
  // 0 + 1 -> 1
  auto output = Optimizer::Optimize(input.get());
  assert(eval(*input) == eval(*output));
}

auto test_rightwise_optimization() -> void {
  // 1 + 0
  auto input = BinaryAdditionExpr::Make(
    NumericConstantExpr::Make(1UL),
    NumericConstantExpr::Make(0UL));
  // 1 + 0 -> 1
  auto output = Optimizer::Optimize(input.get());
  assert(eval(*input) == eval(*output));
}

auto main() -> int {
  test_leftwise_optimization();
  test_rightwise_optimization();
  
  std::cout << "All tests passed!\n";
  return EXIT_SUCCESS;
}
