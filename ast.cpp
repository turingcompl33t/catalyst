/**
 * @file ast.cpp
 * @brief AST definition for simple expression evaluator.
 */

#include "ast.hpp"

Expr::~Expr() {};

NumericConstantExpr::~NumericConstantExpr() {};

auto NumericConstantExpr::Evaluate() const -> std::size_t {
  assert(std::holds_alternative<std::size_t>(value_)
      && "Logic error to evaluate numeric expression with AnyNumber");
  return std::get<std::size_t>(value_);
}

auto NumericConstantExpr::Clone() const -> std::unique_ptr<Expr> {
  return NumericConstantExpr::Make(value_, id_);
}

BinaryAdditionExpr::~BinaryAdditionExpr() {};

auto BinaryAdditionExpr::Evaluate() const -> std::size_t {
  return left_->Evaluate() + right_->Evaluate();
}

auto BinaryAdditionExpr::Clone() const -> std::unique_ptr<Expr> {
  return BinaryAdditionExpr::Make(left_->Clone(), right_->Clone(), id_);
}
