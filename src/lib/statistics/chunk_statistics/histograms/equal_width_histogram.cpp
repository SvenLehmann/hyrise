#include "equal_width_histogram.hpp"

#include <memory>
#include <numeric>

#include "histogram_utils.hpp"

namespace opossum {

template <typename T>
EqualWidthHistogram<T>::EqualWidthHistogram(const T min, const T max, const std::vector<uint64_t>& counts,
                                            const std::vector<uint64_t>& distinct_counts,
                                            const uint64_t num_bins_with_larger_range)
    : AbstractHistogram<T>(),
      _min(min),
      _max(max),
      _counts(counts),
      _distinct_counts(distinct_counts),
      _num_bins_with_larger_range(num_bins_with_larger_range) {}

template <>
EqualWidthHistogram<std::string>::EqualWidthHistogram(const std::string& min, const std::string& max,
                                                      const std::vector<uint64_t>& counts,
                                                      const std::vector<uint64_t>& distinct_counts,
                                                      const uint64_t num_bins_with_larger_range,
                                                      const std::string& supported_characters,
                                                      const uint64_t string_prefix_length)
    : AbstractHistogram<std::string>(supported_characters, string_prefix_length),
      _min(min),
      _max(max),
      _counts(counts),
      _distinct_counts(distinct_counts),
      _num_bins_with_larger_range(num_bins_with_larger_range) {
  Assert(min.find_first_not_of(supported_characters) == std::string::npos, "Unsupported characters.");
  Assert(max.find_first_not_of(supported_characters) == std::string::npos, "Unsupported characters.");
}

template <typename T>
EqualWidthBinStats<T> EqualWidthHistogram<T>::_get_bin_stats(const std::vector<std::pair<T, uint64_t>>& value_counts,
                                                             const size_t max_num_bins) {
  // Bins shall have the same range.
  const auto min = value_counts.front().first;
  const auto max = value_counts.back().first;
  const T base_width = next_value(max - min);

  // Never have more bins than representable values.
  // TODO(anyone): fix for floats. This is more of a theoretical issue, however.
  auto num_bins = max_num_bins;
  if constexpr (std::is_integral_v<T>) {
    num_bins = max_num_bins <= static_cast<uint64_t>(base_width) ? max_num_bins : base_width;
  }

  const T bin_width = base_width / num_bins;
  uint64_t num_bins_with_larger_range;
  if constexpr (std::is_integral_v<T>) {
    num_bins_with_larger_range = base_width % num_bins;
  } else {
    num_bins_with_larger_range = 0u;
  }

  std::vector<uint64_t> counts;
  std::vector<uint64_t> distinct_counts;
  counts.reserve(num_bins);
  distinct_counts.reserve(num_bins);

  T current_begin_value = min;
  auto current_begin_it = value_counts.cbegin();
  auto current_begin_index = 0l;
  for (auto current_bin_id = 0u; current_bin_id < num_bins; current_bin_id++) {
    T next_begin_value = current_begin_value + bin_width;
    T current_end_value = previous_value(next_begin_value);

    if constexpr (std::is_integral_v<T>) {
      if (current_bin_id < num_bins_with_larger_range) {
        current_end_value++;
        next_begin_value++;
      }
    }

    if constexpr (std::is_floating_point_v<T>) {
      // This is intended to compensate for the fact that floating point arithmetic is not exact.
      // Adding up floating point numbers adds an error over time, and the more bins there are, the larger it gets.
      // So this is how we make sure that the last bin contains the rest of the values.
      if (current_bin_id == num_bins - 1) {
        current_end_value = max;
      }
    }

    auto next_begin_it = current_begin_it;
    while (next_begin_it != value_counts.cend() && (*next_begin_it).first <= current_end_value) {
      next_begin_it++;
    }

    const auto next_begin_index = std::distance(value_counts.cbegin(), next_begin_it);
    counts.emplace_back(std::accumulate(value_counts.cbegin() + current_begin_index,
                                        value_counts.cbegin() + next_begin_index, uint64_t{0},
                                        [](uint64_t a, const std::pair<T, uint64_t>& b) { return a + b.second; }));
    distinct_counts.emplace_back(next_begin_index - current_begin_index);

    current_begin_value = next_begin_value;
    current_begin_index = next_begin_index;
  }

  return {min, max, counts, distinct_counts, num_bins_with_larger_range};
}

template <>
EqualWidthBinStats<std::string> EqualWidthHistogram<std::string>::_get_bin_stats(
    const std::vector<std::pair<std::string, uint64_t>>& value_counts, const size_t max_num_bins) {
  // TODO(tim): disable
  Fail("Not supported.");
}

template <>
EqualWidthBinStats<std::string> EqualWidthHistogram<std::string>::_get_bin_stats(
    const std::vector<std::pair<std::string, uint64_t>>& value_counts, const size_t max_num_bins,
    const std::string& supported_characters, const uint64_t string_prefix_length) {
  // Bins shall have the same range.
  const auto min = value_counts.front().first;
  const auto max = value_counts.back().first;

  const auto num_min = convert_string_to_number_representation(min, supported_characters, string_prefix_length);
  const auto num_max = convert_string_to_number_representation(max, supported_characters, string_prefix_length);

  // Never have more bins than representable values.
  const auto num_bins =
      max_num_bins <= static_cast<uint64_t>(num_max - num_min + 1) ? max_num_bins : num_max - num_min + 1;

  std::vector<uint64_t> counts;
  std::vector<uint64_t> distinct_counts;
  counts.reserve(num_bins);
  distinct_counts.reserve(num_bins);

  const auto base_width = num_max - num_min + 1;
  const auto bin_width = base_width / num_bins;
  const uint64_t num_bins_with_larger_range = base_width % num_bins;

  auto current_begin_value = min;
  auto current_begin_it = value_counts.cbegin();
  auto current_begin_index = 0l;

  // TODO(tim): look into refactoring of begin/end values, feels like there could be less strings

  for (auto current_bin_id = 0u; current_bin_id < num_bins; current_bin_id++) {
    auto num_current_begin_value =
        convert_string_to_number_representation(current_begin_value, supported_characters, string_prefix_length);
    auto current_end_value = convert_number_representation_to_string(num_current_begin_value + bin_width - 1u,
                                                                     supported_characters, string_prefix_length);
    auto next_begin_value = convert_number_representation_to_string(num_current_begin_value + bin_width,
                                                                    supported_characters, string_prefix_length);

    if (current_bin_id < num_bins_with_larger_range) {
      current_end_value = next_begin_value;
      next_begin_value = convert_number_representation_to_string(num_current_begin_value + bin_width + 1u,
                                                                 supported_characters, string_prefix_length);
    }

    auto next_begin_it = current_begin_it;
    while (next_begin_it != value_counts.cend() && (*next_begin_it).first <= current_end_value) {
      next_begin_it++;
    }

    const auto next_begin_index = std::distance(value_counts.cbegin(), next_begin_it);
    counts.emplace_back(std::accumulate(value_counts.begin() + current_begin_index,
                                        value_counts.begin() + next_begin_index, uint64_t{0},
                                        [](uint64_t a, std::pair<std::string, uint64_t> b) { return a + b.second; }));
    distinct_counts.emplace_back(next_begin_index - current_begin_index);

    current_begin_value = next_begin_value;
    current_begin_index = next_begin_index;
  }

  return {min, max, counts, distinct_counts, num_bins_with_larger_range};
}

template <typename T>
std::shared_ptr<EqualWidthHistogram<T>> EqualWidthHistogram<T>::from_segment(
    const std::shared_ptr<const BaseSegment>& segment, const size_t max_num_bins,
    const std::optional<std::string>& supported_characters, const std::optional<uint64_t>& string_prefix_length) {
  std::string characters;
  uint64_t prefix_length;
  if constexpr (std::is_same_v<T, std::string>) {
    const auto pair = AbstractHistogram<T>::_get_or_check_prefix_settings(supported_characters, string_prefix_length);
    characters = pair.first;
    prefix_length = pair.second;
  }

  const auto value_counts = AbstractHistogram<T>::_calculate_value_counts(segment);

  if (value_counts.empty()) {
    return nullptr;
  }

  if constexpr (std::is_same_v<T, std::string>) {
    const auto bin_stats =
        EqualWidthHistogram<T>::_get_bin_stats(value_counts, max_num_bins, characters, prefix_length);
    return std::make_shared<EqualWidthHistogram<T>>(bin_stats.min, bin_stats.max, bin_stats.counts,
                                                    bin_stats.distinct_counts, bin_stats.num_bins_with_larger_range,
                                                    characters, prefix_length);
  } else {
    const auto bin_stats = EqualWidthHistogram<T>::_get_bin_stats(value_counts, max_num_bins);
    return std::make_shared<EqualWidthHistogram<T>>(bin_stats.min, bin_stats.max, bin_stats.counts,
                                                    bin_stats.distinct_counts, bin_stats.num_bins_with_larger_range);
  }
}

template <typename T>
std::shared_ptr<AbstractHistogram<T>> EqualWidthHistogram<T>::clone() const {
  return std::make_shared<EqualWidthHistogram<T>>(_min, _max, _counts, _distinct_counts, _num_bins_with_larger_range);
}

template <>
std::shared_ptr<AbstractHistogram<std::string>> EqualWidthHistogram<std::string>::clone() const {
  return std::make_shared<EqualWidthHistogram<std::string>>(
      _min, _max, _counts, _distinct_counts, _num_bins_with_larger_range, _supported_characters, _string_prefix_length);
}

template <typename T>
HistogramType EqualWidthHistogram<T>::histogram_type() const {
  return HistogramType::EqualWidth;
}

template <typename T>
size_t EqualWidthHistogram<T>::num_bins() const {
  return _counts.size();
}

template <typename T>
uint64_t EqualWidthHistogram<T>::_bin_count(const BinID index) const {
  DebugAssert(index < _counts.size(), "Index is not a valid bin.");
  return _counts[index];
}

template <typename T>
uint64_t EqualWidthHistogram<T>::total_count() const {
  return std::accumulate(_counts.begin(), _counts.end(), 0ul);
}

template <typename T>
uint64_t EqualWidthHistogram<T>::total_count_distinct() const {
  return std::accumulate(_distinct_counts.begin(), _distinct_counts.end(), 0ul);
}

template <typename T>
uint64_t EqualWidthHistogram<T>::_bin_count_distinct(const BinID index) const {
  DebugAssert(index < _distinct_counts.size(), "Index is not a valid bin.");
  return _distinct_counts[index];
}

template <>
std::string EqualWidthHistogram<std::string>::_bin_width(const BinID /*index*/) const {
  Fail("Not supported for string histograms. Use _string_bin_width instead.");
}

template <typename T>
T EqualWidthHistogram<T>::_bin_width(const BinID index) const {
  DebugAssert(index < num_bins(), "Index is not a valid bin.");

  const auto base_width = this->get_next_value(_max - _min) / this->num_bins();

  if constexpr (std::is_integral_v<T>) {
    return base_width + (index < _num_bins_with_larger_range ? 1 : 0);
  }

  return base_width;
}

template <typename T>
T EqualWidthHistogram<T>::_bin_min(const BinID index) const {
  DebugAssert(index < num_bins(), "Index is not a valid bin.");

  // If it's the first bin, return _min.
  if (index == 0u) {
    return _min;
  }

  return this->get_next_value(this->_bin_max(index - 1));
}

template <typename T>
T EqualWidthHistogram<T>::_bin_max(const BinID index) const {
  DebugAssert(index < num_bins(), "Index is not a valid bin.");

  // If it's the last bin, return max.
  if (index == num_bins() - 1) {
    return _max;
  }

  // Calculate the lower edge of the bin right after index, assuming every bin has the same width.
  // If there are no wider bins, take the previous value from the lower edge of the following bin to get the
  // upper edge of this one.
  // Otherwise, add the index to compensate one element for every bin preceding this bin.
  // Add at most _num_bins_with_larger_range - 1 because we already start adding from the next bin's lower edge.
  if constexpr (std::is_same_v<T, std::string>) {
    const auto num_min = this->_convert_string_to_number_representation(_min);
    const auto num_max = this->_convert_string_to_number_representation(_max);
    const auto base = num_min + (index + 1u) * ((num_max - num_min + 1) / num_bins());
    const auto bin_max = _num_bins_with_larger_range == 0u ? previous_value(base)
                                                           : base + std::min(index, _num_bins_with_larger_range - 1u);
    return this->_convert_number_representation_to_string(bin_max);
  } else {
    const auto base = _min + (index + 1u) * _bin_width(num_bins() - 1u);
    return _num_bins_with_larger_range == 0u ? previous_value(base)
                                             : base + std::min(index, _num_bins_with_larger_range - 1u);
  }
}

template <typename T>
BinID EqualWidthHistogram<T>::_bin_for_value(const T value) const {
  if (value < _min || value > _max) {
    return INVALID_BIN_ID;
  }

  if (_num_bins_with_larger_range == 0u || value <= _bin_max(_num_bins_with_larger_range - 1u)) {
    // All bins up to that point have the exact same width, so we can use index 0.
    return (value - _min) / _bin_width(0u);
  }

  // All bins after that point have the exact same width as well, so we use that as the new base and add it up.
  return _num_bins_with_larger_range +
         (value - _bin_min(_num_bins_with_larger_range)) / _bin_width(_num_bins_with_larger_range);
}

template <>
BinID EqualWidthHistogram<std::string>::_bin_for_value(const std::string value) const {
  if (value < _min || value > _max) {
    return INVALID_BIN_ID;
  }

  const auto num_value = this->_convert_string_to_number_representation(value);

  BinID bin_id;
  if (_num_bins_with_larger_range == 0u || value <= _bin_max(_num_bins_with_larger_range - 1u)) {
    const auto num_min = this->_convert_string_to_number_representation(_min);
    bin_id = (num_value - num_min) / this->_string_bin_width(0u);
  } else {
    const auto num_base_min = this->_convert_string_to_number_representation(_bin_min(_num_bins_with_larger_range));
    bin_id =
        _num_bins_with_larger_range + (num_value - num_base_min) / this->_string_bin_width(_num_bins_with_larger_range);
  }

  // We calculate numerical values for strings with substrings, and the bin edge calculation works with that.
  // Therefore, if the search string is longer than the supported prefix length and starts with the upper bin edge,
  // we have to return the next bin.
  return bin_id + (value.length() > _string_prefix_length && value.find(_bin_max(bin_id)) == 0 ? 1 : 0);
}

template <typename T>
BinID EqualWidthHistogram<T>::_upper_bound_for_value(const T value) const {
  if (value < _min) {
    return 0u;
  }

  const auto index = _bin_for_value(value);
  return index < num_bins() - 2 ? index + 1 : INVALID_BIN_ID;
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(EqualWidthHistogram);

}  // namespace opossum
