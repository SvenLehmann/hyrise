#include "base_column_statistics.hpp"

namespace opossum {

BaseColumnStatistics::BaseColumnStatistics(const DataType data_type, const float null_value_ratio)
    : _data_type(data_type), _null_value_ratio(null_value_ratio) {}

DataType BaseColumnStatistics::data_type() const { return _data_type; }

float BaseColumnStatistics::null_value_ratio() const { return _null_value_ratio; }

float BaseColumnStatistics::non_null_value_ratio() const { return 1.0f - _null_value_ratio; }

void BaseColumnStatistics::set_null_value_ratio(const float null_value_ratio) { _null_value_ratio = null_value_ratio; }

std::shared_ptr<BaseColumnStatistics> BaseColumnStatistics::without_null_values() const {
  auto clone = this->clone();
  clone->_null_value_ratio = 0.0f;
  return clone;
}

}  // namespace opossum
