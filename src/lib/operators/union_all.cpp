#include "union_all.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "utils/assert.hpp"

namespace opossum {
UnionAll::UnionAll(const std::shared_ptr<const AbstractOperator> left_in,
                   const std::shared_ptr<const AbstractOperator> right_in)
    : AbstractReadOnlyOperator(left_in, right_in) {
  // nothing to do here
}

const std::string UnionAll::name() const { return "UnionAll"; }

std::shared_ptr<const Table> UnionAll::_on_execute() {
  DebugAssert(input_table_left()->column_definitions() == input_table_right()->column_definitions(),
              "Input tables must have same number of columns");
  DebugAssert(input_table_left()->type() == input_table_left()->type(), "Input tables must have the same type");

  auto output = std::make_shared<Table>(input_table_left()->column_definitions(), input_table_left()->type());

  // add positions to output by iterating over both input tables
  for (const auto& input : {input_table_left(), input_table_right()}) {
    // iterating over all chunks of table input
    for (ChunkID in_chunk_id{0}; in_chunk_id < input->chunk_count(); in_chunk_id++) {
      // creating empty chunk to add columns with positions
      ChunkColumnList output_columns;

      // iterating over all columns of the current chunk
      for (ColumnID column_id{0}; column_id < input->column_count(); ++column_id) {
        // While we don't modify the column, we need to get a non-const pointer so that we can put it into the chunk
        output_columns.emplace_back(input->get_chunk(in_chunk_id)->get_mutable_column(column_id));
      }

      // adding newly filled chunk to the output table
      output->add_chunk_new(output_columns);
    }
  }

  return output;
}
}  // namespace opossum
