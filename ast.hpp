/**
 * @file ast.hpp
 * @brief AST definition for simple expression evaluator.
 */

#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <cstdint>
#include <cassert>
#include <variant>

/** Identifier for nodes in ASTs (for optimization use) */
using Identifier = std::string;

/**
 * Enumerates the supported expression types. 
 */
enum class ExprType {
  NUMERIC_CONSTANT,
  BINARY_ADDITION,
};

/**
 * The base class for all expressions types.
 */
class Expr {
public:
  /** Pure virtual destructor forces override */
  virtual ~Expr() = 0;

  /** Evaluate the expression */
  virtual auto Evaluate() const -> std::size_t = 0;

  /** Clone the expression */
  virtual auto Clone() const -> std::unique_ptr<Expr> = 0;

  /** @return The type of the expression */
  ExprType Type() const {
    return type_;
  }

protected:
    /**
     * Construct a new Expr instance.
     * @param type The expression type
     */
  explicit Expr(ExprType type) : type_{type} {}

private:
  /** The type of the expression */
  ExprType type_;
};

/**
 * Represents a numeric
 */
class NumericConstantExpr : public Expr {
public:
  /** A dummy type to represent "any number" */
  struct AnyNumber {
    /** Only a single instance of type AnyNumber */
    auto operator==(AnyNumber const&) const -> bool {
        return true;
    }
  };

  /** The numeric sum-type for use in the optimizer */
  using Numeric = std::variant<std::size_t, AnyNumber>;

  /**
   * Factory function.
   * @param value The underlying value
   * @return The NumericConstantExpr instance
   */
  static auto Make(Numeric value) -> std::unique_ptr<NumericConstantExpr> {
    return std::unique_ptr<NumericConstantExpr>{new NumericConstantExpr{value, ""}};
  }

  /**
   * Factory function.
   * @param value The underlying value
   * @param id The identifier
   * @return The NumericConstantExpr instance
   */
  static auto Make(Numeric value, Identifier id) -> std::unique_ptr<NumericConstantExpr> {
    return std::unique_ptr<NumericConstantExpr>{new NumericConstantExpr{value, std::move(id)}};
  }

  /** Pure virtual destructor forces override */
  ~NumericConstantExpr() override;

  /** Evaluate the NumericConstantExpr */
  auto Evaluate() const -> std::size_t override;

  /** Clone the NumericConstantExpr */
  auto Clone() const -> std::unique_ptr<Expr> override;

  /** @return The underlying value */
  auto Value() const -> Numeric {
    return value_;
  }

  /** @return The identifier */
  auto Id() const -> Identifier const& {
    return id_;
  } 

private:
  /**
   * Construct a new NumericConstantExpr instance.
   * @param value The underlying value
   * @return The NumericConstantExpr instance
   */
  explicit NumericConstantExpr(Numeric value, Identifier id) 
    : Expr{ExprType::NUMERIC_CONSTANT}, value_{value}, id_{std::move(id)} {}

  /** The underlying value */
  const Numeric value_;
  /** The node identifier */
  const Identifier id_;
};

/**
 * Represents a binary addition operation (e.g. 1 + 2).
 */
class BinaryAdditionExpr : public Expr {
public:
  /**
   * Factory function.
   * @param left The left subexpression
   * @param right The right subexpression
   * @return The BinaryAdditionExpr instance
   */
  static auto Make(std::unique_ptr<Expr>&& left, std::unique_ptr<Expr>&& right) -> std::unique_ptr<BinaryAdditionExpr> {
    return std::unique_ptr<BinaryAdditionExpr>{
      new BinaryAdditionExpr{std::move(left), std::move(right), ""}};
  }

  /**
   * Factory function.
   * @param left The left subexpression
   * @param right The right subexpression
   * @param id The identifier
   * @return The BinaryAdditionExpr instance
   */
  static auto Make(std::unique_ptr<Expr>&& left, std::unique_ptr<Expr>&& right, Identifier id) -> std::unique_ptr<BinaryAdditionExpr> {
    return std::unique_ptr<BinaryAdditionExpr>{
      new BinaryAdditionExpr{std::move(left), std::move(right), std::move(id)}};
  }

  /** Pure virtual destructor forces override */
  ~BinaryAdditionExpr() override;

  /** Evaluate the BinaryAdditionExpr */
  auto Evaluate() const -> std::size_t override;

  /** Clone the BinaryAdditionExpr */
  auto Clone() const -> std::unique_ptr<Expr> override ;

  /** @return A non-owning pointer to the left subexpression */
  auto Left() const -> Expr const* {
    return left_.get();
  }

  /** @return A non-owning pointer to the right subexpression */
  auto Right() const -> Expr const* {
    return right_.get();
  }

  /**
   * Replace the left subtree with `left`.
   * @param left The new left subtree
   */
  auto ReplaceLeft(std::unique_ptr<Expr>&& left) -> void {
    left_ = std::move(left);
  }

  /**
   * Replace the right subtree with `right`.
   * @param right The new right subtree
   */
  auto ReplaceRight(std::unique_ptr<Expr>&& right) -> void {
    right_ = std::move(right);
  }

  /** @return The identifier */
  auto Id() const -> Identifier const& {
    return id_;
  }

private:
  /**
   * Construct a new BinaryAdditionExpr instance.
   * @param left The left subexpresion
   * @param right The right subexpression
   * @param id The identifier
   */
  BinaryAdditionExpr(std::unique_ptr<Expr>&& left, std::unique_ptr<Expr>&& right, Identifier id)
    : Expr{ExprType::BINARY_ADDITION}, left_{std::move(left)}, right_{std::move(right)}, id_{std::move(id)} {}

  /** The left subexpression */
  std::unique_ptr<Expr> left_;
  /** The right subexpression */
  std::unique_ptr<Expr> right_;
  /** The identifier */
  const Identifier id_;
};

#endif // AST_HPP
