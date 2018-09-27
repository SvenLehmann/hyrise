#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abstract_histogram.hpp"
#include "types.hpp"

namespace opossum {

template <typename T>
struct EqualElementCountBinData {
  std::vector<T> bin_minimums;
  std::vector<T> bin_maximums;
  std::vector<HistogramCountType> bin_heights;
  HistogramCountType distinct_count_per_bin;
  BinID bin_count_with_extra_value;
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
  EqualElementCountHistogram(const std::vector<T>& bin_minimums, const std::vector<T>& bin_maximums,
                             const std::vector<HistogramCountType>& bin_heights,
                             const HistogramCountType distinct_count_per_bin, const BinID bin_count_with_extra_value);
  EqualElementCountHistogram(const std::vector<std::string>& bin_minimums, const std::vector<std::string>& bin_maximums,
                             const std::vector<HistogramCountType>& bin_heights,
                             const HistogramCountType distinct_count_per_bin, const BinID bin_count_with_extra_value,
                             const std::string& supported_characters, const uint32_t string_prefix_length);

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
      const std::shared_ptr<const BaseSegment>& segment, const BinID max_bin_count,
      const std::optional<std::string>& supported_characters = std::nullopt,
      const std::optional<uint32_t>& string_prefix_length = std::nullopt);

  HistogramType histogram_type() const override;
  HistogramCountType total_distinct_count() const override;
  HistogramCountType total_count() const override;
  BinID bin_count() const override;

 protected:
  /**
   * Creates bins and their statistics.
   */
  static EqualElementCountBinData<T> _build_bins(const std::vector<std::pair<T, HistogramCountType>>& value_counts,
                                                 const BinID max_bin_count);

  BinID _bin_for_value(const T value) const override;
  BinID _upper_bound_for_value(const T value) const override;

  T _bin_minimum(const BinID index) const override;
  T _bin_maximum(const BinID index) const override;
  HistogramCountType _bin_height(const BinID index) const override;
  HistogramCountType _bin_distinct_count(const BinID index) const override;

 private:
  /**
   * We use multiple vectors rather than a vector of structs for ease-of-use with STL library functions.
   */

  // Min values on a per-bin basis.
  std::vector<T> _bin_minimums;

  // Max values on a per-bin basis.
  std::vector<T> _bin_maximums;

  // Number of values on a per-bin basis.
  std::vector<HistogramCountType> _bin_heights;

  // Number of distinct values per bin.
  HistogramCountType _distinct_count_per_bin;

  // Number of bins which have an additional distinct value.
  BinID _bin_count_with_extra_value;
};

}  // namespace opossum
