#pragma once

#include "abstract_table_scan_impl.hpp"
#include "expression/abstract_expression.hpp"

namespace opossum {

class Table;

/**
 * Uses the ExpressionEvaluator::evaluate_expression_to_pos_list() for a fallback implementation of the
 * AbstractTableScanImpl. This is likely slower than any specialized `AbstractTableScanImpl` and should thus only be
 * used if a particular expression type doesn't have a specialized `AbstractTableScanImpl`.
 */
class ExpressionEvaluatorTableScanImpl : public AbstractTableScanImpl {
 public:
  ExpressionEvaluatorTableScanImpl(const std::shared_ptr<const Table>& in_table, const std::shared_ptr<AbstractExpression>& expression);
  std::shared_ptr<PosList> scan_chunk(ChunkID chunk_id) override;

 private:
  std::shared_ptr<const Table> _in_table;
   std::shared_ptr<AbstractExpression> _expression;
};

}  // namespace opossum
