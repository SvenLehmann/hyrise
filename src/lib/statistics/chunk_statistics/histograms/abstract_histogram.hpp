#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "statistics/chunk_statistics/abstract_filter.hpp"
#include "storage/value_column.hpp"
#include "types.hpp"

namespace opossum {

class Table;

template <typename T>
class AbstractHistogram : public AbstractFilter {
  friend class HistogramPrivateTest;

 public:
  explicit AbstractHistogram(const std::shared_ptr<const Table>& table);
  AbstractHistogram(const std::shared_ptr<const Table>& table, const std::string& supported_characters);
  AbstractHistogram(const std::shared_ptr<const Table>& table, const std::string& supported_characters,
                    const uint64_t string_prefix_length);
  virtual ~AbstractHistogram() = default;

  virtual std::shared_ptr<AbstractHistogram<T>> clone() const = 0;

  virtual HistogramType histogram_type() const = 0;
  std::string description() const;
  const std::string& supported_characters() const;

  void generate(const ColumnID column_id, const size_t max_num_buckets);

  float estimate_selectivity(const PredicateCondition predicate_type, const T value,
                             const std::optional<T>& value2 = std::nullopt) const;
  float estimate_cardinality(const PredicateCondition predicate_type, const T value,
                             const std::optional<T>& value2 = std::nullopt) const;
  float estimate_distinct_count(const PredicateCondition predicate_type, const T value,
                                const std::optional<T>& value2 = std::nullopt) const;
  bool can_prune(const PredicateCondition predicate_type, const AllTypeVariant& variant_value,
                 const std::optional<AllTypeVariant>& variant_value2 = std::nullopt) const override;

  T min() const;
  T max() const;

  T get_previous_value(const T value, const bool pad_and_trim = true) const;
  T get_next_value(const T value, const bool pad_and_trim = true) const;

  virtual size_t num_buckets() const = 0;
  virtual uint64_t total_count() const = 0;
  virtual uint64_t total_count_distinct() const = 0;

 protected:
  const std::shared_ptr<const Table> _get_value_counts(const ColumnID column_id) const;

  static
      // typename std::enable_if_t<std::is_arithmetic_v<T>, std::vector<std::pair<T, uint64_t>>>
      std::vector<std::pair<T, uint64_t>>
      _calculate_value_counts(const std::shared_ptr<const BaseColumn>& column);

  static std::vector<std::pair<std::string, uint64_t>> _calculate_value_counts(
      const std::shared_ptr<const BaseColumn>& column, const std::string& supported_characters,
      const uint64_t string_prefix_length);
  static std::vector<std::pair<T, uint64_t>> _sort_value_counts(const std::unordered_map<T, uint64_t>& value_counts);

  virtual void _generate(const std::shared_ptr<const ValueColumn<T>> distinct_column,
                         const std::shared_ptr<const ValueColumn<int64_t>> count_column,
                         const size_t max_num_buckets) = 0;

  int64_t _convert_string_to_number_representation(const std::string& value) const;
  std::string _convert_number_representation_to_string(const int64_t value) const;
  float _bucket_share(const BucketID bucket_id, const T value) const;

  virtual T _bucket_width(const BucketID index) const;

  virtual BucketID _bucket_for_value(const T value) const = 0;
  virtual BucketID _lower_bound_for_value(const T value) const = 0;
  virtual BucketID _upper_bound_for_value(const T value) const = 0;

  virtual T _bucket_min(const BucketID index) const = 0;
  virtual T _bucket_max(const BucketID index) const = 0;
  virtual uint64_t _bucket_count(const BucketID index) const = 0;
  virtual uint64_t _bucket_count_distinct(const BucketID index) const = 0;

  const std::weak_ptr<const Table> _table;
  std::string _supported_characters;
  uint64_t _string_prefix_length;
};

}  // namespace opossum
