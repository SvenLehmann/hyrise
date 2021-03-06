#pragma once

#include <unordered_set>

#include "base_column_statistics.hpp"
#include "column_statistics.hpp"
#include "resolve_type.hpp"
#include "storage/create_iterable_from_segment.hpp"
#include "storage/table.hpp"

namespace opossum {

/**
 * Generate the statistics of a single column. Used by generate_table_statistics()
 */
template <typename ColumnDataType>
std::shared_ptr<BaseColumnStatistics> generate_column_statistics(const Table& table, const ColumnID column_id) {
  std::unordered_set<ColumnDataType> distinct_set;

  auto null_value_count = size_t{0};

  auto min = std::numeric_limits<ColumnDataType>::max();
  auto max = std::numeric_limits<ColumnDataType>::lowest();

  for (ChunkID chunk_id{0}; chunk_id < table.chunk_count(); ++chunk_id) {
    const auto base_segment = table.get_chunk(chunk_id)->get_segment(column_id);

    resolve_segment_type<ColumnDataType>(*base_segment, [&](auto& segment) {
      auto iterable = create_iterable_from_segment<ColumnDataType>(segment);
      iterable.for_each([&](const auto& segment_value) {
        if (segment_value.is_null()) {
          ++null_value_count;
        } else {
          distinct_set.insert(segment_value.value());
          min = std::min(min, segment_value.value());
          max = std::max(max, segment_value.value());
        }
      });
    });
  }

  const auto null_value_ratio =
      table.row_count() > 0 ? static_cast<float>(null_value_count) / static_cast<float>(table.row_count()) : 0.0f;
  const auto distinct_count = static_cast<float>(distinct_set.size());

  if (distinct_count == 0.0f) {
    min = std::numeric_limits<ColumnDataType>::min();
    max = std::numeric_limits<ColumnDataType>::max();
  }

  return std::make_shared<ColumnStatistics<ColumnDataType>>(null_value_ratio, distinct_count, min, max);
}

template <>
std::shared_ptr<BaseColumnStatistics> generate_column_statistics<std::string>(const Table& table,
                                                                              const ColumnID column_id);

}  // namespace opossum
