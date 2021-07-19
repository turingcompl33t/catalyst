/**
 * @file optimizer.hpp
 * @brief Simple, tree-style expression optimizer.
 */

#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <string>
#include <memory>
#include <vector>

#include "ast.hpp"

/**
 * Represents a single optimization transformation.
 */
class Transform {
public:
  /**
   * Factory function.
   * @param name The name of the transform
   * @param input_pattern The input pattern
   * @param output_pattern The output pattern
   * @return The Transform instance
   */
  static auto Make(std::string name, std::unique_ptr<Expr>&& input_pattern, std::unique_ptr<Expr>&& output_pattern)
      -> std::unique_ptr<Transform> {
    return std::make_unique<Transform>(std::move(name), std::move(input_pattern), std::move(output_pattern));
  }

  /**
   * Construct a new Transform instance.
   * @param input_pattern The input subtree
   * @param output_pattern The output subtree
   */
  Transform(std::string name, std::unique_ptr<Expr>&& input_pattern, std::unique_ptr<Expr>&& output_pattern)
    : name_{name}, input_pattern_{std::move(input_pattern)}, output_pattern_{std::move(output_pattern)} {}

  /** @return The name of the transform */
  auto Name() const -> std::string const& {
    return name_;
  }

  /** @return The input pattern */
  auto InputPattern() const -> Expr const* {
    return input_pattern_.get();
  }

  /** @return The output pattern */
  auto OutputPattern() const -> Expr const* {
    return output_pattern_.get();
  }

private:
  /** Human-readable identifier for the transform */
  const std::string name_;
  /** The input subtree */
  std::unique_ptr<Expr> input_pattern_;
  /** The output subtree */
  std::unique_ptr<Expr> output_pattern_;
};

/** Static class */
class Optimizer {
public:
  /**
   * Optimize an expression AST.
   * @param root The root of the expression AST
   * @return The optimized AST
   */
  static auto Optimize(Expr* root) -> std::unique_ptr<Expr>;

  /**
   * Match a pattern AST against a query AST.
   * @param pattern The template AST
   * @param query The query AST
   * @return `true` if `pattern` and `query` 
   */
  static auto Match(Expr const* pattern, Expr const* query) -> bool;

private:
  /**
   * Create the transformation for leftwise binary addition with zero.
   *  Ex: 0 + 1 -> 1
   * 
   * @return The transform 
   */
  static auto TransformBinaryAdditionWithZeroOnLeft() -> std::unique_ptr<Transform>;

  /**
   * Create the transformation for rightwise binary addition with zero.
   *  Ex: 1 + 0 -> 1
   * 
   * @return The transform
   */
  static auto TransformBinaryAdditionWithZeroOnRight() -> std::unique_ptr<Transform>;

  /**
   * Apply a transformation to an AST.
   * @param transform The transform to apply
   * @param root The root of the AST to which the transformation is applied
   * @return The result of applying the transform to the AST
   */
  static auto ApplyTransform(Transform const& transform, Expr const* root) -> std::unique_ptr<Expr>;

  /**
   * Apply a transformation by replacing the matched subtree with
   * with the output subtree specified in the transform rules.
   * @param transform The transformation to apply
   * @param position The position at which the transformation is applied
   * @return The replacement subtree 
   */
  static auto ApplyTransformAt(Transform const& transform, Expr const* position) -> std::unique_ptr<Expr>;

  /**
   * Recursively apply a transformation and generate the output subtree.
   * @param pattern The input pattern
   * @param expressions The flattened collection of expressions for the transform
   * @param identifiers The flattened collection of identifiers for the transform
   * @return The result of the transformation
   */
  static auto ApplyTransformAt(Expr const* pattern,
    std::vector<Expr const*> const& expressions, std::vector<Identifier> const& identifiers) -> std::unique_ptr<Expr>;
};

#endif // OPTIMIZER_HPP
