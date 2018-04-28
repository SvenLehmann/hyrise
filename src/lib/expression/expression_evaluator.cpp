#include "expression_evaluator.hpp"

#include <type_traits>

#include "abstract_expression.hpp"
#include "array_expression.hpp"
#include "abstract_predicate_expression.hpp"
#include "binary_predicate_expression.hpp"
#include "all_parameter_variant.hpp"
#include "arithmetic_expression.hpp"
#include "logical_expression.hpp"
#include "in_expression.hpp"
#include "pqp_column_expression.hpp"
#include "pqp_select_expression.hpp"
#include "value_expression.hpp"
#include "operators/abstract_operator.hpp"
#include "storage/materialize.hpp"
#include "scheduler/current_scheduler.hpp"
#include "sql/sql_query_plan.hpp"
#include "resolve_type.hpp"
#include "utils/lambda_visitor.hpp"
#include "storage/value_column.hpp"
#include "utils/assert.hpp"

namespace opossum {

ExpressionEvaluator::ExpressionEvaluator(const std::shared_ptr<const Chunk>& chunk):
_chunk(chunk)
{
  _column_materializations.resize(_chunk->column_count());
}


template<bool null_from_values, bool value_from_null>
struct OperatorTraits {
  static constexpr auto may_produce_null_from_values = null_from_values;
  static constexpr auto may_produce_value_from_null = value_from_null;
  static constexpr auto supported = false;
};

template<typename Functor, typename O, typename L, typename R, bool null_from_values, bool value_from_null>
struct BinaryFunctorWrapper : public OperatorTraits<null_from_values, value_from_null> {
  void operator()(O &result_value,
                  bool& result_null,
                  const L left_value,
                  const bool left_null,
                  const R right_value,
                  const bool right_null) const {
    result_value = Functor{}(left_value, right_value);
    result_null = left_null || right_null;
  }
};

template<template<typename> typename Fn, typename O, typename L, typename R, typename Enable = void>
struct Comparison {
  static constexpr auto supported = false;
};
template<template<typename> typename Fn, typename O, typename L, typename R>
struct Comparison<Fn, O, L, R, std::enable_if_t<std::is_same_v<int32_t, O> && std::is_same_v<std::string, L> == std::is_same_v<std::string, R>>> {
  static constexpr auto supported = true;
  using impl = BinaryFunctorWrapper<Fn<std::common_type_t<L, R>>, O, L, R, false, false>;
};

template<template<typename> typename Fn, typename O, typename L, typename R, typename Enable = void>
struct Logical {
  static constexpr auto supported = false;
};
template<template<typename> typename Fn, typename O, typename L, typename R>
struct Logical<Fn, O, L, R, std::enable_if_t<std::is_same_v<int32_t, O> && std::is_same_v<int32_t, L> && std::is_same_v<int32_t, R>>> {
  static constexpr auto supported = true;
  using impl = BinaryFunctorWrapper<Fn<int32_t>, O, L, R, false, false>;
};

template<template<typename> typename Fn, typename O, typename L, typename R, bool null_from_values, bool value_from_null, typename Enable = void>
struct ArithmeticFunctor {
  static constexpr auto supported = false;
};
template<template<typename> typename Fn, typename O, typename L, typename R, bool null_from_values, bool value_from_null>
struct ArithmeticFunctor<Fn, O, L, R, null_from_values, value_from_null, std::enable_if_t<!std::is_same_v<std::string, O> && !std::is_same_v<std::string, L> && !std::is_same_v<std::string, R>>> {
  static constexpr auto supported = true;
  using impl = BinaryFunctorWrapper<Fn<O>, O, L, R, false, false>;
};

// clang-format off
template<typename O, typename L, typename R> using Equals = Comparison<std::equal_to, O, L, R>;
template<typename O, typename L, typename R> using NotEquals = Comparison<std::not_equal_to, O, L, R>;
template<typename O, typename L, typename R> using GreaterThan = Comparison<std::greater, O, L, R>;
template<typename O, typename L, typename R> using GreaterThanEquals = Comparison<std::greater_equal, O, L, R>;
template<typename O, typename L, typename R> using LessThan = Comparison<std::less, O, L, R>;
template<typename O, typename L, typename R> using LessThanEquals = Comparison<std::less_equal, O, L, R>;
template<typename O, typename L, typename R> using And = Logical<std::logical_and, O, L, R>;
template<typename O, typename L, typename R> using Or = Logical<std::logical_or, O, L, R>;

template<typename O, typename L, typename R> using Addition = ArithmeticFunctor<std::plus, O, L, R, true, true>;
template<typename O, typename L, typename R> using Subtraction = ArithmeticFunctor<std::minus, O, L, R, true, true>;
template<typename O, typename L, typename R> using Multiplication = ArithmeticFunctor<std::multiplies, O, L, R, true, true>;
// clang-format on

// clang-format off
template<typename T> bool ExpressionEvaluator::is_nullable_values(const ExpressionResult<T>& result)      { return result.which() == 0; }
template<typename T> bool ExpressionEvaluator::is_non_nullable_values(const ExpressionResult<T>& result)  { return result.which() == 1; }
template<typename T> bool ExpressionEvaluator::is_value(const ExpressionResult<T>& result)                { return result.which() == 2; }
template<typename T> bool ExpressionEvaluator::is_null(const ExpressionResult<T>& result)                 { return result.which() == 3; }
template<typename T> bool ExpressionEvaluator::is_nullable_array(const ExpressionResult<T>& result)       { return result.which() == 4; }
template<typename T> bool ExpressionEvaluator::is_non_nullable_array(const ExpressionResult<T>& result)   { return result.which() == 5; }
// clang-format on

template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_expression(const AbstractExpression& expression) {
  switch (expression.type) {
    case ExpressionType::Arithmetic:
      return evaluate_arithmetic_expression<T>(static_cast<const ArithmeticExpression&>(expression));

    case ExpressionType::Logical:
      return evaluate_logical_expression<T>(static_cast<const LogicalExpression&>(expression));

    case ExpressionType::Predicate: {
      const auto& predicate_expression = static_cast<const AbstractPredicateExpression&>(expression);

      if (is_ordering_predicate_condition(predicate_expression.predicate_condition)) {
        return evaluate_binary_predicate_expression<T>(static_cast<const BinaryPredicateExpression&>(expression));
      } else if (predicate_expression.predicate_condition == PredicateCondition::In) {
        return evaluate_in_expression<T>(static_cast<const InExpression&>(expression));
      } else {
        Fail("Unsupported Predicate Expression");
      }
    }

    case ExpressionType::Select: {
      const auto* pqp_select_expression = dynamic_cast<const PQPSelectExpression*>(&expression);
      Assert(pqp_select_expression, "Can only evaluate PQPSelectExpression");

      return evaluate_select_expression<T>(*pqp_select_expression);
    }

    case ExpressionType::Column: {
      const auto* pqp_column_expression = dynamic_cast<const PQPColumnExpression*>(&expression);
      Assert(pqp_column_expression, "Can only evaluate PQPColumnExpressions");

      const auto& column = *_chunk->get_column(pqp_column_expression->column_id);

      std::vector<T> values;
      materialize_values(column, values);

      if (pqp_column_expression->is_nullable()) {
        std::vector<bool> nulls;
        materialize_nulls<T>(column, nulls);

        return std::make_pair(values, nulls);
      }

      return values;
    }

    case ExpressionType::Value: {
      const auto& value_expression = static_cast<const ValueExpression &>(expression);
      const auto& value = value_expression.value;

      Assert(value.type() == typeid(T), "Can't evaluate ValueExpression to requested type T");

      return boost::get<T>(value);
    }

    default:
      Fail("ExpressionType evaluation not yet implemented");
  }
}

template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_arithmetic_expression(const ArithmeticExpression& expression) {
  const auto& left = *expression.left_operand();
  const auto& right = *expression.right_operand();

  // clang-format off
  switch (expression.arithmetic_operator) {
    case ArithmeticOperator::Addition:       return evaluate_binary_expression<T, Addition>(left, right);
    case ArithmeticOperator::Subtraction:    return evaluate_binary_expression<T, Subtraction>(left, right);
    case ArithmeticOperator::Multiplication: return evaluate_binary_expression<T, Multiplication>(left, right);

    default:
      Fail("ArithmeticOperator evaluation not yet implemented");
  }
  // clang-format on
}

template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_binary_predicate_expression(const BinaryPredicateExpression& expression) {
  const auto& left = *expression.left_operand();
  const auto& right = *expression.right_operand();

  // clang-format off
  switch (expression.predicate_condition) {
    case PredicateCondition::Equals:            return evaluate_binary_expression<T, Equals>(left, right);
    case PredicateCondition::NotEquals:         return evaluate_binary_expression<T, NotEquals>(left, right);
    case PredicateCondition::LessThan:          return evaluate_binary_expression<T, LessThan>(left, right);
    case PredicateCondition::LessThanEquals:    return evaluate_binary_expression<T, LessThanEquals>(left, right);
    case PredicateCondition::GreaterThan:       return evaluate_binary_expression<T, GreaterThan>(left, right);
    case PredicateCondition::GreaterThanEquals: return evaluate_binary_expression<T, GreaterThanEquals>(left, right);

    default:
      Fail("PredicateCondition evaluation not yet implemented");
  }
  // clang-format on
}


template<typename T, template<typename, typename, typename> typename Functor>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_binary_expression(
const AbstractExpression& left_operand,
const AbstractExpression& right_operand) {
  const auto left_data_type = left_operand.data_type();
  const auto right_data_type = right_operand.data_type();

  ExpressionResult<T> result;

  resolve_data_type(left_data_type, [&](const auto left_data_type_t) {
    using LeftDataType = typename decltype(left_data_type_t)::type;

    const auto left_operands = evaluate_expression<LeftDataType>(left_operand);

    resolve_data_type(right_data_type, [&](const auto right_data_type_t) {
      using RightDataType = typename decltype(right_data_type_t)::type;

      const auto right_operands = evaluate_expression<RightDataType>(right_operand);

      using ConcreteFunctor = Functor<T, LeftDataType, RightDataType>;

      if constexpr (ConcreteFunctor::supported) {
        result = evaluate_binary_operator<T, LeftDataType, RightDataType>(left_operands, right_operands, typename ConcreteFunctor::impl{});
      } else {
        Fail("Operation not supported on the given types");
      }
    });
  });

  return result;
}

template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_logical_expression(const LogicalExpression& expression) {
  Fail("LogicalExpression can only output int32_t");
}

template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_in_expression(const InExpression& expression) {
  Fail("InExpression supports only int32_t as result");
}

template<typename ResultDataType,
typename LeftOperandDataType,
typename RightOperandDataType,
typename Functor>
ExpressionEvaluator::ExpressionResult<ResultDataType> ExpressionEvaluator::evaluate_binary_operator(const ExpressionResult<LeftOperandDataType>& left_operands,
                                                                                                    const ExpressionResult<RightOperandDataType>& right_operands,
                                                                                                    const Functor &functor) {
  const auto left_is_nullable = left_operands.type() == typeid(NullableValues<LeftOperandDataType>);
  const auto left_is_values = left_operands.type() == typeid(NonNullableValues<LeftOperandDataType>);
  const auto left_is_value = left_operands.type() == typeid(LeftOperandDataType);
  const auto left_is_null = left_operands.type() == typeid(NullValue);
  const auto right_is_nullable = right_operands.type() == typeid(NullableValues<RightOperandDataType>);
  const auto right_is_values = right_operands.type() == typeid(NonNullableValues<RightOperandDataType>);
  const auto right_is_value = right_operands.type() == typeid(RightOperandDataType);
  const auto right_is_null = right_operands.type() == typeid(NullValue);

  const auto result_size = _chunk->size();
  std::vector<ResultDataType> result_values(result_size);
  std::vector<bool> result_nulls;

  // clang-format off
  if (left_is_null && right_is_null) return NullValue{};

  auto result_value = ResultDataType{};
  auto result_null = false;
  auto left_value = LeftOperandDataType{};
  auto right_value = RightOperandDataType{};

  /**
   * Compute single value/null cases
   */
  if (left_is_value) left_value = boost::get<LeftOperandDataType>(left_operands);
  else if (right_is_value) right_value = boost::get<RightOperandDataType>(right_operands);
  else if (left_is_value && right_is_null) functor(result_value, result_null, left_value, false, right_value, true);
  else if (left_is_null && right_is_value) functor(result_value, result_null, left_value, true, right_value, false);
  else if (left_is_value && right_is_value) functor(result_value, result_null, left_value, false, right_value, false);

  if ((left_is_value || left_is_null) && (right_is_value || right_is_null)) {
    if (result_null) {
      return NullValue{};
    } else {
      return result_value;
    }
  }

  /**
   * Per-row cases
   */
  const std::vector<LeftOperandDataType>* left_values = nullptr;
  const std::vector<bool>* left_nulls = nullptr;
  const std::vector<RightOperandDataType>* right_values = nullptr;
  const std::vector<bool>* right_nulls = nullptr;

  if (left_is_nullable) {
    const auto& values_and_nulls = boost::get<NullableValues<LeftOperandDataType>>(left_operands);
    left_values = &values_and_nulls.first;
    left_nulls = &values_and_nulls.second;
  }
  if (left_is_values) left_values = &boost::get<NonNullableValues<LeftOperandDataType>>(left_operands);

  if (right_is_nullable) {
    const auto& values_and_nulls = boost::get<NullableValues<RightOperandDataType>>(right_operands);
    right_values = &values_and_nulls.first;
    right_nulls = &values_and_nulls.second;
  }
  if (right_is_values) right_values = &boost::get<NonNullableValues<RightOperandDataType>>(right_operands);

  const auto result_is_nullable = left_is_nullable || left_is_null || right_is_nullable || right_is_null || Functor::may_produce_null_from_values;

  if (result_is_nullable) result_nulls.resize(result_size);

  /**
   *
   */

  const auto evaluate_per_row = [&](const auto& fn) {
    for (auto chunk_offset = ChunkOffset{0}; chunk_offset < result_size; ++chunk_offset) {
      fn(chunk_offset);
    }
  };

  if (left_is_nullable && right_is_nullable) {
    evaluate_per_row([&](const auto chunk_offset) {
      functor(result_values[chunk_offset], result_null, (*left_values)[chunk_offset],
              (*left_nulls)[chunk_offset], (*right_values)[chunk_offset], (*right_nulls)[chunk_offset]);
      result_nulls[chunk_offset] = result_null;
    });
  }
  else if (left_is_nullable && right_is_values) {
    evaluate_per_row([&](const auto chunk_offset) {
      functor(result_values[chunk_offset], result_null,
              (*left_values)[chunk_offset], (*left_nulls)[chunk_offset],
              (*right_values)[chunk_offset], false);
      result_nulls[chunk_offset] = result_null;
    });
  }
  else if (left_is_nullable && right_is_value) {
    evaluate_per_row([&](const auto chunk_offset) {
      functor(result_values[chunk_offset], result_null,
              (*left_values)[chunk_offset], (*left_nulls)[chunk_offset],
              right_value, false);
      result_nulls[chunk_offset] = result_null;
    });
  }
  else if (left_is_values && right_is_nullable) {
    evaluate_per_row([&](const auto chunk_offset) {
      functor(result_values[chunk_offset], result_null,
              (*left_values)[chunk_offset], false,
              (*right_values)[chunk_offset], (*right_nulls)[chunk_offset]);
      result_nulls[chunk_offset] = result_null;
    });
  }
  else if (left_is_values && right_is_values) {
    if (result_is_nullable) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                (*left_values)[chunk_offset], false,
                (*right_values)[chunk_offset], false);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null /* dummy */,
                (*left_values)[chunk_offset], false,
                (*right_values)[chunk_offset], false);
      });
    }
  }
  else if (left_is_values && right_is_value) {
    if (result_is_nullable) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                (*left_values)[chunk_offset], false,
                right_value, false);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null /* dummy */,
                (*left_values)[chunk_offset], false,
                right_value, false);
      });
    }
  }
  else if (left_is_values && right_is_null) {
    if constexpr (Functor::may_produce_value_from_null) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                left_values[chunk_offset], false,
                right_value, true);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      std::fill(result_nulls.begin(), result_nulls.end(), true);
    }
  }
  else if (left_is_value && right_is_nullable) {
    evaluate_per_row([&](const auto chunk_offset) {
      functor(result_values[chunk_offset], result_null,
              left_value, false,
              (*right_values)[chunk_offset], (*right_nulls)[chunk_offset]);
      result_nulls[chunk_offset] = result_null;
    });
  }
  else if (left_is_value && right_is_values) {
    if (result_is_nullable) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                left_value, false,
                (*right_values)[chunk_offset], false);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null /* dummy */,
                left_value, false,
                (*right_values)[chunk_offset], false);
      });
    }
  }
  else if (left_is_null && right_is_nullable) {
    if constexpr (Functor::may_produce_value_from_null) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                left_value, true,
                (*right_values)[chunk_offset], (*right_nulls)[chunk_offset]);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      std::fill(result_nulls.begin(), result_nulls.end(), true);
    }
  }
  else if (left_is_null && right_is_values) {
    if constexpr (Functor::may_produce_value_from_null) {
      evaluate_per_row([&](const auto chunk_offset) {
        functor(result_values[chunk_offset], result_null,
                left_value, true,
                (*right_values)[chunk_offset], false);
        result_nulls[chunk_offset] = result_null;
      });
    } else {
      std::fill(result_nulls.begin(), result_nulls.end(), true);
    }
  }
  else {
    Fail("Operand types not implemented");
  }

  if (result_is_nullable) {
    return std::make_pair(result_values, result_nulls);
  } else {
    return result_values;
  }
  // clang-format on

  return result_values;
}


template<typename T>
ExpressionEvaluator::ExpressionResult<T> ExpressionEvaluator::evaluate_select_expression(const PQPSelectExpression& expression) {
  for (const auto& parameter : expression.parameters) {
    _ensure_column_materialization(parameter);
  }

  NonNullableValues<T> result;
  result.reserve(_chunk->size());

  std::vector<AllParameterVariant> parameter_values(expression.parameters.size());

  for (auto chunk_offset = ChunkOffset{0}; chunk_offset < _chunk->size(); ++chunk_offset) {
    for (auto parameter_idx = size_t{0}; parameter_idx < expression.parameters.size(); ++parameter_idx) {
      const auto parameter_column_id = expression.parameters[parameter_idx];
      const auto& column = *_chunk->get_column(parameter_column_id);

      resolve_data_type(column.data_type(), [&](const auto data_type_t) {
        using ColumnDataType = typename decltype(data_type_t)::type;
        parameter_values[parameter_idx] = AllTypeVariant{static_cast<ColumnMaterialization<ColumnDataType>&>(*_column_materializations[parameter_column_id]).values[chunk_offset]};
      });
    }

    auto row_pqp = expression.pqp->recreate(parameter_values);

    SQLQueryPlan query_plan;
    query_plan.add_tree_by_root(row_pqp);
    const auto tasks = query_plan.create_tasks();
    CurrentScheduler::schedule_and_wait_for_tasks(tasks);

    const auto result_table = row_pqp->get_output();

    Assert(result_table->column_count() == 1, "Expected precisely one column");
    Assert(result_table->row_count() == 1, "Expected precisely one row");
    Assert(result_table->column_data_type(ColumnID{0}) == data_type_from_type<T>(), "Expected different DataType");

    const auto& result_column = *result_table->get_chunk(ChunkID{0})->get_column(ColumnID{0});

    std::vector<T> result_value;
    materialize_values(result_column, result_value);

    result.emplace_back(result_value[0]);
  }

  return result;
}

void ExpressionEvaluator::_ensure_column_materialization(const ColumnID column_id) {
  Assert(column_id < _chunk->column_count(), "Column out of range");

  if (_column_materializations[column_id]) return;

  const auto& column = *_chunk->get_column(column_id);

  resolve_data_type(column.data_type(), [&](const auto data_type_t) {
    using ColumnDataType = typename decltype(data_type_t)::type;

    auto column_materialization = std::make_unique<ColumnMaterialization<ColumnDataType>>();
    materialize_values(column, column_materialization->values);
    _column_materializations[column_id] = std::move(column_materialization);
  });
}

std::shared_ptr<BaseColumn> ExpressionEvaluator::evaluate_expression_to_column(const AbstractExpression& expression) {
  const auto data_type = expression.data_type();

  std::shared_ptr<BaseColumn> column;

  resolve_data_type(data_type, [&](const auto data_type_t) {
    using ColumnDataType = typename decltype(data_type_t)::type;

    const auto result = evaluate_expression<ColumnDataType>(expression);

    pmr_concurrent_vector<ColumnDataType> values;
    pmr_concurrent_vector<bool> nulls;

    auto has_nulls = false;

    if (result.type() == typeid(ExpressionEvaluator::NonNullableValues<ColumnDataType>)) {
      const auto& result_values = boost::get<ExpressionEvaluator::NonNullableValues<ColumnDataType>>(result);
      values = pmr_concurrent_vector<ColumnDataType>(result_values.begin(), result_values.end());

    } else if (result.type() == typeid(ExpressionEvaluator::NullableValues<ColumnDataType>)) {
      const auto& result_values_and_nulls = boost::get<ExpressionEvaluator::NullableValues<ColumnDataType>>(result);
      const auto& result_values = result_values_and_nulls.first;
      const auto& result_nulls = result_values_and_nulls.second;
      has_nulls = true;

      values = pmr_concurrent_vector<ColumnDataType>(result_values.begin(), result_values.end());
      nulls = pmr_concurrent_vector<bool>(result_nulls.begin(), result_nulls.end());

    } else if (result.type() == typeid(NullValue)) {
      values.resize(_chunk->size(), ColumnDataType{});
      nulls.resize(_chunk->size(), true);
      has_nulls = true;

    } else if (result.type() == typeid(ColumnDataType)) {
      values.resize(_chunk->size(), boost::get<ColumnDataType>(result));

    }

    if (has_nulls) {
      column = std::make_shared<ValueColumn<ColumnDataType>>(std::move(values), std::move(nulls));
    } else {
      column = std::make_shared<ValueColumn<ColumnDataType>>(std::move(values));
    }
  });

  return column;
}

template<>
ExpressionEvaluator::ExpressionResult<int32_t> ExpressionEvaluator::evaluate_logical_expression<int32_t>(const LogicalExpression& expression) {
  const auto& left = *expression.left_operand();
  const auto& right = *expression.right_operand();

  // clang-format off
  switch (expression.logical_operator) {
    case LogicalOperator::Or:  return evaluate_binary_expression<int32_t, Or>(left, right);
    case LogicalOperator::And: return evaluate_binary_expression<int32_t, And>(left, right);
  }
  // clang-format on
}

template<>
ExpressionEvaluator::ExpressionResult<int32_t> ExpressionEvaluator::evaluate_in_expression<int32_t>(const InExpression& in_expression) {
  const auto& left_expression = *in_expression.value();
  const auto& right_expression = *in_expression.set();

  std::vector<int32_t> result_values;
  std::vector<bool> result_nulls;

  if (right_expression.type == ExpressionType::Array) {
    const auto& array_expression = static_cast<const ArrayExpression&>(right_expression);

    /**
     * To keep the code simply for now, transform the InExpression like this:
     * "a IN (x, y, z)"   ---->   "a = x OR a = y OR a = z"
     *
     * But first, out of array_expression.elements(), pick those expressions whose type can be compared with
     * in_expression.value() so we're not getting "Can't compare Int and String" when doing something crazy like
     * "5 IN (6, 5, "Hello")
     */
    const auto left_is_string = left_expression.data_type() == DataType::String;
    std::vector<std::shared_ptr<AbstractExpression>> type_compatible_elements;
    for (const auto& element : array_expression.elements()) {
      if ((element->data_type() == DataType::String) == left_is_string) {
        type_compatible_elements.emplace_back(element);
      }
    }

    if (type_compatible_elements.empty()) {
      // NULL IN () is NULL, <not_null> IN () is FALSE
      Fail("Not supported yet");
    }

    std::shared_ptr<AbstractExpression> predicate_disjunction = std::make_shared<BinaryPredicateExpression>(PredicateCondition::Equals, in_expression.value(), type_compatible_elements.front());
    for (auto element_idx = size_t{1}; element_idx < type_compatible_elements.size(); ++element_idx) {
      const auto equals_element = std::make_shared<BinaryPredicateExpression>(PredicateCondition::Equals, in_expression.value(), type_compatible_elements[element_idx]);
      predicate_disjunction = std::make_shared<LogicalExpression>(LogicalOperator::Or, predicate_disjunction, equals_element);
    }

    return evaluate_expression<int32_t>(*predicate_disjunction);
  } else if (right_expression.type == ExpressionType::Select) {
    Fail("Unsupported ExpressionType used in InExpression");
  } else {
    Fail("Unsupported ExpressionType used in InExpression");
  }

  return {};
}

}  // namespace opossum