/**
 * @file optimizer.cpp
 * @brief Simple, tree-style expression optimizer.
 */

#include <vector>
#include <stdexcept>

#include "optimizer.hpp"

#define UNREACHABLE() (throw std::runtime_error{"unreachable"})

/** The null identifier */
static const Identifier NULL_ID{""};

/* ----------------------------------------------------------------------------
  Utility Functions
---------------------------------------------------------------------------- */

/**
 * Flatten expressions to the specified vector via post-order traversal.
 * @param root The root of the AST
 * @param expressions The collection of expressions to populate
 */
static auto FlattenExpressionsTo(Expr const* root, std::vector<Expr const*>* expressions) -> void {
  switch (root->Type()) {
    case ExprType::BINARY_ADDITION: {
      FlattenExpressionsTo(static_cast<BinaryAdditionExpr const*>(root)->Left(), expressions);
      FlattenExpressionsTo(static_cast<BinaryAdditionExpr const*>(root)->Right(), expressions);
      expressions->push_back(root);
      break;
    }
    case ExprType::NUMERIC_CONSTANT: {
      expressions->push_back(root);
      break;
    }
    default:
      UNREACHABLE();
  }
}

/**
 * Flatten identifiers to the specified vector via post-order traversal.
 * @param root The root of the AST
 * @param expressions The collection of identifiers to populate
 */
static auto FlattenIdentifiersTo(Expr const* root, std::vector<Identifier>* identifiers) -> void {
  switch(root->Type()) {
    case ExprType::BINARY_ADDITION: {
      FlattenIdentifiersTo(static_cast<BinaryAdditionExpr const*>(root)->Left(), identifiers);
      FlattenIdentifiersTo(static_cast<BinaryAdditionExpr const*>(root)->Right(), identifiers);
      identifiers->push_back(static_cast<BinaryAdditionExpr const*>(root)->Id());
      break;
    }
    case ExprType::NUMERIC_CONSTANT: {
      identifiers->push_back(static_cast<NumericConstantExpr const*>(root)->Id());
      break;
    }
    default:
      UNREACHABLE();
  }
}

/* ----------------------------------------------------------------------------
  Transform Definitions
---------------------------------------------------------------------------- */

auto Optimizer::TransformBinaryAdditionWithZeroOnLeft() -> std::unique_ptr<Transform> {
  auto input_pattern = BinaryAdditionExpr::Make(
    NumericConstantExpr::Make(0UL),
    NumericConstantExpr::Make(NumericConstantExpr::AnyNumber{}, "right"));
  auto output_pattern = NumericConstantExpr::Make(NumericConstantExpr::AnyNumber{}, "right");
  return Transform::Make("Left-wise Binary Addition with Zero",
    std::move(input_pattern), std::move(output_pattern));
}

/**
 * Create the transformation for rightwise binary addition with zero.
 *  Ex: 1 + 0 -> 1
 * 
 * @return The transform
 */
auto Optimizer::TransformBinaryAdditionWithZeroOnRight() -> std::unique_ptr<Transform> {
  auto input_pattern = BinaryAdditionExpr::Make(
    NumericConstantExpr::Make(NumericConstantExpr::AnyNumber{}, "left"),
    NumericConstantExpr::Make(0UL));
  auto output_pattern = NumericConstantExpr::Make(NumericConstantExpr::AnyNumber{}, "left");
  return Transform::Make("Right-wise Binary Addition with Zero",
    std::move(input_pattern), std::move(output_pattern));
}

/* ----------------------------------------------------------------------------
  Internal Members
---------------------------------------------------------------------------- */

auto Optimizer::ApplyTransform(Transform const& transform, Expr const* root) -> std::unique_ptr<Expr> {
  switch (root->Type()) {
    case ExprType::BINARY_ADDITION: {
      if (Optimizer::Match(transform.InputPattern(), root)) {
        return ApplyTransformAt(transform, root);
      }
      // No match on current root, recursively consider left and right subtrees
      return BinaryAdditionExpr::Make(
        ApplyTransform(transform, static_cast<BinaryAdditionExpr const*>(root)->Left()),
        ApplyTransform(transform, static_cast<BinaryAdditionExpr const*>(root)->Right()));
    }
    case ExprType::NUMERIC_CONSTANT: {
      // Nothing to do
      return root->Clone();
    }
    default:
      UNREACHABLE();
  }
}

/**
 * Apply a transformation by replacing the matched subtree with
 * with the output subtree specified in the transform rules.
 * @param transform The transformation to apply
 * @param position The position at which the transformation is applied
 * @return The replacement subtree 
 */
auto Optimizer::ApplyTransformAt(Transform const& transform, Expr const* position) -> std::unique_ptr<Expr> {
  // Flatten the expressions in the input AST
  std::vector<Expr const*> expressions{};
  FlattenExpressionsTo(position, &expressions);

  // Flatten the identifiers in the input pattern
  std::vector<Identifier> identifiers{};
  FlattenIdentifiersTo(transform.InputPattern(), &identifiers);

  // We now have an implicit mapping between identifiers in the
  // transform pattern and the expressions on which substitutions
  // need to be applied; now, for each identifier in the resulting 
  // subtree, locate its corresponding expression in the input AST
  // and perform the appropriate value substitution

  return ApplyTransformAt(transform.OutputPattern(), expressions, identifiers);
}


auto Optimizer::ApplyTransformAt(Expr const* pattern, std::vector<Expr const*> const& expressions, std::vector<Identifier> const& identifiers) -> std::unique_ptr<Expr> {
  switch (pattern->Type()) {
    case ExprType::BINARY_ADDITION: {
      // Nothing to do at this level, just recursive into subtrees
      return BinaryAdditionExpr::Make(
        ApplyTransformAt(static_cast<BinaryAdditionExpr const*>(pattern)->Left(), expressions, identifiers),
        ApplyTransformAt(static_cast<BinaryAdditionExpr const*>(pattern)->Right(), expressions, identifiers));
    }
    case ExprType::NUMERIC_CONSTANT: {
      auto* numeric = static_cast<NumericConstantExpr const*>(pattern);
      if (std::holds_alternative<NumericConstantExpr::AnyNumber>(numeric->Value())) {
        // The pattern contains a placeholder and an identifier for the node
        // in the input pattern from which we must copy the new value
        auto it = std::find(identifiers.cbegin(), identifiers.cend(), numeric->Id());
        assert(it != identifiers.cend());

        // Locate the corresponding expression in the input AST
        auto const index = std::distance(identifiers.cbegin(), it);
        // Grab the value from this expression to complete the transform
        return NumericConstantExpr::Make(
          static_cast<NumericConstantExpr const*>(expressions.at(index))->Value());

      } else {
        // Otherwise the pattern contains a literal value
        return NumericConstantExpr::Make(numeric->Value());
      }
    }
    default:
      UNREACHABLE();
  }
}

/* ----------------------------------------------------------------------------
  Exported Members
---------------------------------------------------------------------------- */

auto Optimizer::Optimize(Expr* root) -> std::unique_ptr<Expr> {
  // Setup the transformations we want to apply
  std::vector<std::unique_ptr<Transform>> transforms{};
  transforms.push_back(TransformBinaryAdditionWithZeroOnLeft());
  transforms.push_back(TransformBinaryAdditionWithZeroOnRight());

  // Apply each transformation iteratively
  Expr* current = root;
  for (auto const& transform : transforms) {
    auto transformed = ApplyTransform(*transform, current);
    current = transformed.release();
  }

  // Return the final result of optimization
  return std::unique_ptr<Expr>(current);
}  

auto Optimizer::Match(Expr const* pattern, Expr const* query) -> bool {
  if (pattern->Type() != query->Type()) {
    return false;
  }

  switch (pattern->Type()) {
    case ExprType::BINARY_ADDITION: {
      auto binary_pattern = static_cast<BinaryAdditionExpr const*>(pattern);
      auto binary_query = static_cast<BinaryAdditionExpr const*>(query);
      // Recursively match subtrees
      return Match(binary_pattern->Left(), binary_query->Left()) 
        && Match(binary_pattern->Right(), binary_query->Right());
    }
    case ExprType::NUMERIC_CONSTANT: {
      auto* numeric_pattern = static_cast<NumericConstantExpr const*>(pattern);
      auto* numeric_query = static_cast<NumericConstantExpr const*>(query);
      const auto v0 = numeric_pattern->Value();
      const auto v1 = numeric_query->Value();
      if (std::holds_alternative<std::size_t>(v0) && std::holds_alternative<std::size_t>(v1)) {
        // If both hold an actual value, compare them for equality
        return std::get<std::size_t>(v0) == std::get<std::size_t>(v1);
      }
      // Otherwise, one or both of the numerics must be AnyNumber,
      // which always matches a literal value or another AnyNumber
      return true;
    }
    default:
      UNREACHABLE();
  }
}
