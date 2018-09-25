#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abstract_histogram.hpp"
#include "types.hpp"

namespace opossum {

template <typename T>
struct EqualElementCountBinStats {
  std::vector<T> mins;
  std::vector<T> maxs;
  std::vector<size_t> heights;
  size_t distinct_count_per_bin;
  size_t bin_count_with_extra_value;
};

/**
 * Distinct-balanced histogram.
 * Bins contain roughly the same number of distinct values actually occurring in the data.
 * There might be gaps between bins.
 */
template <typename T>
class EqualElementCountHistogram : public AbstractHistogram<T> {
 public:
  using AbstractHistogram<T>::AbstractHistogram;
  EqualElementCountHistogram(const std::vector<T>& mins, const std::vector<T>& maxs, const std::vector<size_t>& heights,
                             const size_t distinct_count_per_bin, const size_t bin_count_with_extra_value);
  EqualElementCountHistogram(const std::vector<std::string>& mins, const std::vector<std::string>& maxs,
                             const std::vector<size_t>& heights, const size_t distinct_count_per_bin,
                             const size_t bin_count_with_extra_value, const std::string& supported_characters,
                             const size_t string_prefix_length);

  /**
   * Create a histogram based on the data in a given segment.
   * @param segment The segment containing the data.
   * @param max_bin_count The number of bins to create. The histogram might create fewer, but never more.
   * @param supported_characters A sorted, consecutive string of characters supported in case of string histograms.
   * Can be omitted and will be filled with default value.
   * @param string_prefix_length The prefix length used to calculate string ranges.
   * * Can be omitted and will be filled with default value.
   */
  static std::shared_ptr<EqualElementCountHistogram<T>> from_segment(
      const std::shared_ptr<const BaseSegment>& segment, const size_t max_bin_count,
      const std::optional<std::string>& supported_characters = std::nullopt,
      const std::optional<size_t>& string_prefix_length = std::nullopt);

  HistogramType histogram_type() const override;
  size_t total_distinct_count() const override;
  size_t total_count() const override;
  size_t bin_count() const override;

 protected:
  /**
   * Creates bins and their statistics.
   */
  static EqualElementCountBinStats<T> _get_bin_stats(const std::vector<std::pair<T, size_t>>& value_counts,
                                                     const size_t max_bin_count);

  BinID _bin_for_value(const T value) const override;
  BinID _upper_bound_for_value(const T value) const override;

  T _bin_min(const BinID index) const override;
  T _bin_max(const BinID index) const override;
  size_t _bin_height(const BinID index) const override;
  size_t _bin_distinct_count(const BinID index) const override;

 private:
  // Min values on a per-bin basis.
  std::vector<T> _mins;

  // Max values on a per-bin basis.
  std::vector<T> _maxs;

  // Number of values on a per-bin basis.
  std::vector<size_t> _heights;

  // Number of distinct values per bin.
  size_t _distinct_count_per_bin;

  // Number of bins which have an additional distinct value.
  BinID _bin_count_with_extra_value;
};

}  // namespace opossum
