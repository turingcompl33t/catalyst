## Catalyst

This repository conatins a toy implementation of a Catalyst-style optimizer as described in [this paper](https://people.csail.mit.edu/matei/papers/2015/sigmod_spark_sql.pdf). Instead of optimizing Spark query plans (perhaps somewhat involved for a toy example), we optimize simple algebraic expressions. My hope is that this conveys the elegance of this approach to optimization without getting bogged-down in the complexity of the input AST.
