#include "../../../base_test.hpp"
#include "gtest/gtest.h"

#include "statistics/chunk_statistics/histograms/equal_width_histogram.hpp"
#include "statistics/chunk_statistics/histograms/histogram_utils.hpp"
#include "utils/load_table.hpp"

namespace opossum {

class EqualWidthHistogramTest : public BaseTest {
  void SetUp() override {
    _int_float4 = load_table("src/test/tables/int_float4.tbl");
    _float2 = load_table("src/test/tables/float2.tbl");
    _int_int4 = load_table("src/test/tables/int_int4.tbl");
    _string3 = load_table("src/test/tables/string3.tbl");
  }

 protected:
  std::shared_ptr<Table> _int_float4;
  std::shared_ptr<Table> _float2;
  std::shared_ptr<Table> _int_int4;
  std::shared_ptr<Table> _string3;
};

TEST_F(EqualWidthHistogramTest, Basic) {
  auto hist = EqualWidthHistogram<int32_t>::from_column(_int_int4->get_chunk(ChunkID{0})->get_column(ColumnID{1}), 6u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, -1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, -1), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 0));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0), 5 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1), 5 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 4));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 4), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 5));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 5), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 6));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 7));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 7), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 10));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 10), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 11));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 11), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 13));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 13), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 14));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 14), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 15));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 15), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 17));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 17), 1 / 1.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 18));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 18), 0.f);
}

TEST_F(EqualWidthHistogramTest, UnevenBins) {
  auto hist = EqualWidthHistogram<int32_t>::from_column(_int_int4->get_chunk(ChunkID{0})->get_column(ColumnID{1}), 4u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, -1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, -1), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 0));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0), 6 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1), 6 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2), 6 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3), 6 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 4));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 4), 6 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 5));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 5), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 6));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 7));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 7), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 9));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 9), 1 / 1.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 10));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 10), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 11));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 11), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 13));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 13), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 14));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 14), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 15));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 15), 2 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 17));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 17), 2 / 2.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 18));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 18), 0.f);
}

TEST_F(EqualWidthHistogramTest, MoreBinsThanDistinctValuesIntEquals) {
  auto hist =
      EqualWidthHistogram<int32_t>::from_column(_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 10u);
  EXPECT_EQ(hist->num_bins(), 10u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 11));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 11), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 100));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 100), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 123));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 123), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1'000), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 10'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 10'000), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 12'345));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12'345), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 12'356));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12'356), 4 / 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 12'357));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12'357), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 20'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 20'000), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 50'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 50'000), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 100'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 100'000), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 123'456));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 123'456), 3 / 1.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 123'457));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 123'457), 0.f);
}

TEST_F(EqualWidthHistogramTest, MoreBinsThanDistinctValuesIntLessThan) {
  auto hist =
      EqualWidthHistogram<int32_t>::from_column(_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 10u);
  EXPECT_EQ(hist->num_bins(), 10u);

  constexpr auto hist_min = 12;
  constexpr auto hist_max = 123'456;

  // First five bins are one element "wider", because the range of the column is not evenly divisible by 10.
  constexpr auto bin_width = (hist_max - hist_min + 1) / 10;
  constexpr auto bin_0_min = hist_min;
  constexpr auto bin_9_min = hist_min + 9 * bin_width + 5;

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 100));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 100),
                  4.f * (100 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 123));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123),
                  4.f * (123 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 1'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1'000),
                  4.f * (1'000 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 10'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 10'000),
                  4.f * (10'000 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 12'345));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12'345),
                  4.f * (12'345 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 12'356));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12'356),
                  4.f * (12'356 - bin_0_min) / (bin_width + 1));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 12'357));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12'357), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 20'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 20'000), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 50'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 50'000), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 100'000));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 100'000), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, bin_9_min));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, bin_9_min), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, bin_9_min + 1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, bin_9_min + 1), 4.f + 3 * (1.f / bin_width));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 123'456));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'456),
                  4.f + 3.f * (123'456 - bin_9_min) / bin_width);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 123'457));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'457), 7.f);
}

TEST_F(EqualWidthHistogramTest, MoreBinsThanRepresentableValues) {
  auto hist = EqualWidthHistogram<int32_t>::from_column(_int_int4->get_chunk(ChunkID{0})->get_column(ColumnID{1}), 19u);
  // There must not be more bins than representable values in the column domain.
  EXPECT_EQ(hist->num_bins(), 17 - 0 + 1);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, -1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, -1), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 0));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0), 1.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1), 3.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 3));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 4));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 4), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 5));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 5), 0.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 6));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 7));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 7), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 8));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 8), 0.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 9));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 9), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 10));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 10), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 11));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 11), 0.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 13));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 13), 1.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 14));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 14), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 15));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 15), 0.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 16));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 16), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 17));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 17), 1.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 18));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 18), 0.f);
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 19));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 19), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, 0));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 0), 0.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 1));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1), 1.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 2));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 2), 4.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 3));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3), 5.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 4));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 4), 5.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 5));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 5), 6.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 6));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 6), 6.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 7));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 7), 6.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 8));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 8), 7.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 9));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 9), 7.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 10));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 10), 7.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 11));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 11), 8.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 12));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12), 8.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 13));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 13), 8.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 14));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 14), 9.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 15));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 15), 10.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 16));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 16), 10.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 17));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 17), 10.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 18));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 18), 11.f);
  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, 19));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 19), 11.f);
}

TEST_F(EqualWidthHistogramTest, Float) {
  auto hist = EqualWidthHistogram<float>::from_column(_float2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 4u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 0.4f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0.4f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 0.5f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0.5f), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1.1f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1.1f), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1.3f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1.3f), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 1.9f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1.9f), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2.0f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.0f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2.2f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.2f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2.3f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.3f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2.5f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.5f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 2.9f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.9f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.1f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.1f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.2f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.2f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.3f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.3f), 7 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.4f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.4f), 3 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.6f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.6f), 3 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 3.9f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.9f), 3 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 4.4f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 4.4f), 3 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 4.5f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 4.5f), 3 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, 6.1f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6.1f), 1 / 1.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, 6.2f));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6.2f), 0.f);
}

TEST_F(EqualWidthHistogramTest, LessThan) {
  auto hist =
      EqualWidthHistogram<int32_t>::from_column(_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  // The first bin's range is one value wider (because (123'456 - 12 + 1) % 3 = 1).
  const auto bin_width = (123'456 - 12 + 1) / 3;

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{12}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{70}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 70), (70.f - 12) / (bin_width + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1'234}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1'234),
                  (1'234.f - 12) / (bin_width + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{12'346}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12'346),
                  (12'346.f - 12) / (bin_width + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{80'000}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 80'000), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{123'456}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'456),
                  4.f + (123'456.f - (12 + 2 * bin_width + 1)) / bin_width * 3);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{123'457}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'457), 4.f + 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1'000'000}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1'000'000), 4.f + 3.f);
}

TEST_F(EqualWidthHistogramTest, FloatLessThan) {
  auto hist = EqualWidthHistogram<float>::from_column(_float2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  const auto bin_width = std::nextafter(6.1f - 0.5f, 6.1f - 0.5f + 1) / 3;

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{0.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 0.5f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan,
                               AllTypeVariant{std::nextafter(0.5f + bin_width, 0.5f + bin_width + 1)}));
  EXPECT_FLOAT_EQ(
      hist->estimate_cardinality(PredicateCondition::LessThan, std::nextafter(0.5f + bin_width, 0.5f + bin_width + 1)),
      4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1.0f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1.0f), (1.0f - 0.5f) / bin_width * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1.7f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1.7f), (1.7f - 0.5f) / bin_width * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{2.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 2.5f),
                  4.f + (2.5f - (0.5f + bin_width)) / bin_width * 7);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.0f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.0f),
                  4.f + (3.0f - (0.5f + bin_width)) / bin_width * 7);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.3f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.3f),
                  4.f + (3.3f - (0.5f + bin_width)) / bin_width * 7);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.6f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.6f),
                  4.f + (3.6f - (0.5f + bin_width)) / bin_width * 7);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.9f),
                  4.f + (3.9f - (0.5f + bin_width)) / bin_width * 7);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan,
                               AllTypeVariant{std::nextafter(0.5f + 2 * bin_width, 0.5f + 2 * bin_width + 1)}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan,
                                             std::nextafter(0.5f + 2 * bin_width, 0.5f + 2 * bin_width + 1)),
                  4.f + 7.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{4.4f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 4.4f),
                  4.f + 7.f + (4.4f - (0.5f + 2 * bin_width)) / bin_width * 3);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{5.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 5.9f),
                  4.f + 7.f + (5.9f - (0.5f + 2 * bin_width)) / bin_width * 3);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{std::nextafter(6.1f, 6.1f + 1)}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, std::nextafter(6.1f, 6.1f + 1)),
                  4.f + 7.f + 3.f);
}

TEST_F(EqualWidthHistogramTest, StringLessThan) {
  auto hist = EqualWidthHistogram<std::string>::from_column(_string3->get_chunk(ChunkID{0})->get_column(ColumnID{0}),
                                                            4u, "abcdefghijklmnopqrstuvwxyz", 4u);

  // "abcd"
  const auto hist_lower = 0 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                          1 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 2 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                          3 * (ipow(26, 0)) + 1;

  // "yyzz"
  const auto hist_upper = 24 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                          24 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 25 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                          25 * (ipow(26, 0)) + 1;

  const auto hist_width = hist_upper - hist_lower + 1;
  // Convert to float so that calculations further down are floating point divisions.
  // The division here, however, must be integral.
  const auto bin_width = static_cast<float>(hist_width / 4u);

  // hist_width % bin_width == 1, so there is one bin storing one additional value.
  const auto bin_1_width = bin_width + 1;
  const auto bin_2_width = bin_width;
  const auto bin_3_width = bin_width;
  const auto bin_4_width = bin_width;

  const auto bin_1_lower = hist_lower;
  const auto bin_2_lower = bin_1_lower + bin_1_width;
  const auto bin_3_lower = bin_2_lower + bin_2_width;
  const auto bin_4_lower = bin_3_lower + bin_3_width;

  constexpr auto bin_1_count = 4.f;
  constexpr auto bin_2_count = 5.f;
  constexpr auto bin_3_count = 4.f;
  constexpr auto bin_4_count = 3.f;
  constexpr auto total_count = bin_1_count + bin_2_count + bin_3_count + bin_4_count;

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, "aaaa"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "aaaa"), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, "abcd"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "abcd"), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "abce"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "abce"), 1 / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "abcf"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "abcf"), 2 / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "cccc"));
  EXPECT_FLOAT_EQ(
      hist->estimate_cardinality(PredicateCondition::LessThan, "cccc"),
      (2 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 2 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) +
       1 + 2 * (ipow(26, 1) + ipow(26, 0)) + 1 + 2 * (ipow(26, 0)) + 1 - bin_1_lower) /
          bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "dddd"));
  EXPECT_FLOAT_EQ(
      hist->estimate_cardinality(PredicateCondition::LessThan, "dddd"),
      (3 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 3 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) +
       1 + 3 * (ipow(26, 1) + ipow(26, 0)) + 1 + 3 * (ipow(26, 0)) + 1 - bin_1_lower) /
          bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ghbo"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ghbo"),
                  (bin_1_width - 2) / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ghbp"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ghbp"),
                  (bin_1_width - 1) / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ghbq"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ghbq"), bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ghbr"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ghbr"),
                  1 / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ghbs"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ghbs"),
                  2 / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "jjjj"));
  EXPECT_FLOAT_EQ(
      hist->estimate_cardinality(PredicateCondition::LessThan, "jjjj"),
      (9 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 9 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) +
       1 + 9 * (ipow(26, 1) + ipow(26, 0)) + 1 + 9 * (ipow(26, 0)) + 1 - bin_2_lower) /
              bin_2_width * bin_2_count +
          bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "kkkk"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "kkkk"),
                  (10 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   10 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 10 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   10 * (ipow(26, 0)) + 1 - bin_2_lower) /
                          bin_2_width * bin_2_count +
                      bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "lzzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "lzzz"),
                  (11 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   25 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 25 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   25 * (ipow(26, 0)) + 1 - bin_2_lower) /
                          bin_2_width * bin_2_count +
                      bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnaz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnaz"),
                  (bin_2_width - 3) / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnb"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnb"),
                  (bin_2_width - 2) / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnba"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnba"),
                  (bin_2_width - 1) / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnbb"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnbb"), bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnbc"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnbc"),
                  1 / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnbd"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnbd"),
                  2 / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "pppp"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "pppp"),
                  (15 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   15 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 15 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   15 * (ipow(26, 0)) + 1 - bin_3_lower) /
                          bin_3_width * bin_3_count +
                      bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "qqqq"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "qqqq"),
                  (16 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   16 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 16 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   16 * (ipow(26, 0)) + 1 - bin_3_lower) /
                          bin_3_width * bin_3_count +
                      bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "qllo"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "qllo"),
                  (16 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   11 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 11 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   14 * (ipow(26, 0)) + 1 - bin_3_lower) /
                          bin_3_width * bin_3_count +
                      bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "stal"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "stal"),
                  (bin_3_width - 2) / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "stam"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "stam"),
                  (bin_3_width - 1) / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "stan"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "stan"),
                  bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "stao"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "stao"),
                  1 / bin_4_width * bin_4_count + bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "stap"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "stap"),
                  2 / bin_4_width * bin_4_count + bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "vvvv"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "vvvv"),
                  (21 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   21 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 21 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   21 * (ipow(26, 0)) + 1 - bin_4_lower) /
                          bin_4_width * bin_4_count +
                      bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "xxxx"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "xxxx"),
                  (23 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   23 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 23 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   23 * (ipow(26, 0)) + 1 - bin_4_lower) /
                          bin_4_width * bin_4_count +
                      bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ycip"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ycip"),
                  (24 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                   2 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 8 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                   15 * (ipow(26, 0)) + 1 - bin_4_lower) /
                          bin_4_width * bin_4_count +
                      bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "yyzy"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "yyzy"),
                  (bin_4_width - 2) / bin_4_width * bin_4_count + bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "yyzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "yyzz"),
                  (bin_4_width - 1) / bin_4_width * bin_4_count + bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "yz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "yz"), total_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "zzzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "zzzz"), total_count);
}

TEST_F(EqualWidthHistogramTest, StringLikePrefix) {
  auto hist = EqualWidthHistogram<std::string>::from_column(_string3->get_chunk(ChunkID{0})->get_column(ColumnID{0}),
                                                            4u, "abcdefghijklmnopqrstuvwxyz", 4u);
  // First bin: [abcd, ghbp], so everything before is prunable.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "a"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "a"), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "aa%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "aa%"), 0.f);

  // Complexity of prefix pattern does not matter for pruning decision.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "aa%zz%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "aa%zz%"), 0.f);

  // Even though "aa%" is prunable, "a%" is not!
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "a%"));
  // Since there are no values smaller than "abcd", [abcd, azzz] is the range that "a%" covers.
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "a%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "abcd", "azzz"));

  // No wildcard, no party.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "abcd"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "abcd"),
                  hist->estimate_cardinality(PredicateCondition::Equals, "abcd"));

  // Classic cases for prefix search.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "ab%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "ab%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "ab", "abzz"));

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "c%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "c%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "c", "czzz"));

  // There are values matching "g%" in two bins, make sure both are included.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "g%"));
  EXPECT_GT(hist->estimate_cardinality(PredicateCondition::Like, "g%"),
            hist->estimate_cardinality(PredicateCondition::Between, "g", "ghbp"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "g%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "g", "gzzz"));

  // Use upper bin boundary as range limit, since there are no other values starting with y in other bins.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "y%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "y%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "y", "yyzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "y%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "y", "yzzz"));

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "z%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "z%"), 0.f);
}

}  // namespace opossum