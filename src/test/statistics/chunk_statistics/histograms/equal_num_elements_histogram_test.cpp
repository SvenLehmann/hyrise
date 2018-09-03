#include "../../../base_test.hpp"
#include "gtest/gtest.h"

#include "statistics/chunk_statistics/histograms/equal_num_elements_histogram.hpp"
#include "statistics/chunk_statistics/histograms/histogram_utils.hpp"
#include "utils/load_table.hpp"

namespace opossum {

class EqualNumElementsHistogramTest : public BaseTest {
  void SetUp() override {
    _int_float4 = load_table("src/test/tables/int_float4.tbl");
    _float2 = load_table("src/test/tables/float2.tbl");
    _string2 = load_table("src/test/tables/string2.tbl");
    _string3 = load_table("src/test/tables/string3.tbl");
  }

 protected:
  std::shared_ptr<Table> _int_float4;
  std::shared_ptr<Table> _float2;
  std::shared_ptr<Table> _string2;
  std::shared_ptr<Table> _string3;
};

TEST_F(EqualNumElementsHistogramTest, Basic) {
  const auto hist = EqualNumElementsHistogram<int32_t>::from_column(
      this->_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 2u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{0}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{12}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 1.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1'234}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1'234), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{123'456}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 123'456), 2.5f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1'000'000}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1'000'000), 0.f);
}

TEST_F(EqualNumElementsHistogramTest, UnevenBins) {
  auto hist =
      EqualNumElementsHistogram<int32_t>::from_column(_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{0}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{12}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 12), 1.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1'234}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1'234), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{123'456}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 123'456), 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1'000'000}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1'000'000), 0.f);
}

TEST_F(EqualNumElementsHistogramTest, Float) {
  auto hist =
      EqualNumElementsHistogram<float>::from_column(_float2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{0.4f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0.4f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{0.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 0.5f), 4 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1.1f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1.1f), 4 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{1.3f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 1.3f), 4 / 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{2.2f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.2f), 4 / 4.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{2.3f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.3f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{2.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.5f), 6 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{2.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 2.9f), 6 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{3.3f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.3f), 6 / 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{3.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.5f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{3.6f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.6f), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{3.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 3.9f), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{6.1f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6.1f), 4 / 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, AllTypeVariant{6.2f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, 6.2f), 0.f);
}

TEST_F(EqualNumElementsHistogramTest, String) {
  auto hist =
      EqualNumElementsHistogram<std::string>::from_column(_string2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 4u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "a"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "a"), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "aa"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "aa"), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "ab"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "ab"), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "b"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "b"), 3 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "birne"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "birne"), 3 / 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "biscuit"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "biscuit"), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "bla"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "bla"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "blubb"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "blubb"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "bums"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "bums"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "ttt"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "ttt"), 4 / 3.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "turkey"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "turkey"), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "uuu"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "uuu"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "vvv"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "vvv"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "www"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "www"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "xxx"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "xxx"), 4 / 3.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "yyy"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "yyy"), 4 / 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "zzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "zzz"), 4 / 2.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "zzzzzz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Equals, "zzzzzz"), 0.f);
}

TEST_F(EqualNumElementsHistogramTest, StringPruning) {
  /**
   * 4 bins
   *  [aa, b, birne]    -> [aa, bir]
   *  [bla, bums, ttt]  -> [bla, ttt]
   *  [uuu, www, xxx]   -> [uuu, xxx]
   *  [yyy, zzz]        -> [yyy, zzz]
   */
  auto hist = EqualNumElementsHistogram<std::string>::from_column(
      _string2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 4u, "abcdefghijklmnopqrstuvwxyz", 3u);

  // These values are smaller than values in bin 0.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, ""));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "a"));

  // These values fall within bin 0.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "aa"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "aaa"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "b"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "bir"));

  // Even though these values are greater than the stored bin edge (birne truncated to three characters),
  // these values are not prunable because their truncated substring is the same as the bin edge.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "bira"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "birne"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "birz"));

  // These values are between bin 0 and 1.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "bis"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "biscuit"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "bja"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "bk"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "bkz"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "bl"));

  // These values fall within bin 1.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "bla"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "c"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "mmopasdasdasd"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "s"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "t"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "tt"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "ttt"));

  // Even though these values are greater than the stored bin edge (ttt),
  // these values are not prunable because their truncated substring is the same as the bin edge.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "tttu"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "tttzzzzz"));

  // These values are between bin 1 and 2.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "turkey"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "uut"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "uutzzzzz"));

  // These values fall within bin 2.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "uuu"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "uuuzzz"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "uv"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "uvz"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "v"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "w"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "wzzzzzzzzz"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "x"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "xxw"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "xxx"));

  // Even though these values are greater than the stored bin edge (xxx),
  // these values are not prunable because their truncated substring is the same as the bin edge.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "xxxa"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "xxxzzzzzz"));

  // These values are between bin 2 and 3.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "xy"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "xyzz"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "y"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "yyx"));
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Equals, "yyxzzzzz"));

  // These values fall within bin 3.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "yyy"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "yyyzzzzz"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "yz"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "z"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "zzz"));

  // Even though these values are greater than the stored bin edge (zzz),
  // these values are not prunable because their truncated substring is the same as the bin edge.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "zzza"));
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Equals, "zzzzzzzzz"));
}

TEST_F(EqualNumElementsHistogramTest, LessThan) {
  auto hist =
      EqualNumElementsHistogram<int32_t>::from_column(_int_float4->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{12}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{70}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 70), (70.f - 12) / (123 - 12 + 1) * 2);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1'234}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1'234), 2.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{12'346}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 12'346), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{123'456}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'456), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{123'457}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 123'457), 7.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1'000'000}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1'000'000), 7.f);
}

TEST_F(EqualNumElementsHistogramTest, FloatLessThan) {
  auto hist =
      EqualNumElementsHistogram<float>::from_column(_float2->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 3u);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{0.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 0.5f), 0.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1.0f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1.0f),
                  (1.0f - 0.5f) / std::nextafter(2.2f - 0.5f, 2.2f - 0.5f + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{1.7f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 1.7f),
                  (1.7f - 0.5f) / std::nextafter(2.2f - 0.5f, 2.2f - 0.5f + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{std::nextafter(2.2f, 2.2f + 1)}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, std::nextafter(2.2f, 2.2f + 1)), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{2.5f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 2.5f), 4.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.0f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.0f),
                  4.f + (3.0f - 2.5f) / std::nextafter(3.3f - 2.5f, 3.3f - 2.5f + 1) * 6);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.3f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.3f),
                  4.f + (3.3f - 2.5f) / std::nextafter(3.3f - 2.5f, 3.3f - 2.5f + 1) * 6);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{std::nextafter(3.3f, 3.3f + 1)}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, std::nextafter(3.3f, 3.3f + 1)), 4.f + 6.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.6f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.6f), 4.f + 6.f);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{3.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 3.9f),
                  4.f + 6.f + (3.9f - 3.6f) / std::nextafter(6.1f - 3.6f, 6.1f - 3.6f + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{5.9f}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, 5.9f),
                  4.f + 6.f + (5.9f - 3.6f) / std::nextafter(6.1f - 3.6f, 6.1f - 3.6f + 1) * 4);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, AllTypeVariant{std::nextafter(6.1f, 6.1f + 1)}));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, std::nextafter(6.1f, 6.1f + 1)),
                  4.f + 6.f + 4.f);
}

TEST_F(EqualNumElementsHistogramTest, StringLessThan) {
  auto hist = EqualNumElementsHistogram<std::string>::from_column(
      _string3->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 4u, "abcdefghijklmnopqrstuvwxyz", 4u);

  // "abcd"
  const auto bin_1_lower = 0 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           1 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 2 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           3 * (ipow(26, 0)) + 1;
  // "efgh"
  const auto bin_1_upper = 4 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           5 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 6 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           7 * (ipow(26, 0)) + 1;
  // "ijkl"
  const auto bin_2_lower = 8 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           9 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 10 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           11 * (ipow(26, 0)) + 1;
  // "mnop"
  const auto bin_2_upper = 12 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           13 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 14 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           15 * (ipow(26, 0)) + 1;
  // "oopp"
  const auto bin_3_lower = 14 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           14 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 15 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           15 * (ipow(26, 0)) + 1;
  // "qrst"
  const auto bin_3_upper = 16 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           17 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 18 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           19 * (ipow(26, 0)) + 1;
  // "uvwx"
  const auto bin_4_lower = 20 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           21 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 22 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           23 * (ipow(26, 0)) + 1;
  // "yyzz"
  const auto bin_4_upper = 24 * (ipow(26, 3) + ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 +
                           24 * (ipow(26, 2) + ipow(26, 1) + ipow(26, 0)) + 1 + 25 * (ipow(26, 1) + ipow(26, 0)) + 1 +
                           25 * (ipow(26, 0)) + 1;

  const auto bin_1_width = (bin_1_upper - bin_1_lower + 1.f);
  const auto bin_2_width = (bin_2_upper - bin_2_lower + 1.f);
  const auto bin_3_width = (bin_3_upper - bin_3_lower + 1.f);
  const auto bin_4_width = (bin_4_upper - bin_4_lower + 1.f);

  constexpr auto bin_1_count = 4.f;
  constexpr auto bin_2_count = 6.f;
  constexpr auto bin_3_count = 3.f;
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

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "abcf"));
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

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "efgg"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "efgg"),
                  (bin_1_width - 2) / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "efgh"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "efgh"),
                  (bin_1_width - 1) / bin_1_width * bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "efgi"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "efgi"), bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ijkl"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ijkl"), bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ijkm"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ijkm"),
                  1 / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "ijkn"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "ijkn"),
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

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnoo"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnoo"),
                  (bin_2_width - 2) / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnop"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnop"),
                  (bin_2_width - 1) / bin_2_width * bin_2_count + bin_1_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "mnoq"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "mnoq"), bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "oopp"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "oopp"), bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "oopq"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "oopq"),
                  1 / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "oopr"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "oopr"),
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

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "qrss"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "qrss"),
                  (bin_3_width - 2) / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "qrst"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "qrst"),
                  (bin_3_width - 1) / bin_3_width * bin_3_count + bin_1_count + bin_2_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "qrsu"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "qrsu"),
                  bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "uvwx"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "uvwx"),
                  bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "uvwy"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "uvwy"),
                  1 / bin_4_width * bin_4_count + bin_1_count + bin_2_count + bin_3_count);

  EXPECT_FALSE(hist->can_prune(PredicateCondition::LessThan, "uvwz"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::LessThan, "uvwz"),
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

TEST_F(EqualNumElementsHistogramTest, StringLikePrefix) {
  auto hist = EqualNumElementsHistogram<std::string>::from_column(
      _string3->get_chunk(ChunkID{0})->get_column(ColumnID{0}), 4u, "abcdefghijklmnopqrstuvwxyz", 4u);

  // First bin: [abcd, efgh], so everything before is prunable.
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

  // Use upper bin boundary as range limit, since there are no other values starting with e in other bins.
  EXPECT_FALSE(hist->can_prune(PredicateCondition::Like, "e%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "e%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "e", "efgh"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "e%"),
                  hist->estimate_cardinality(PredicateCondition::Between, "e", "ezzz"));

  // Second bin starts at ijkl, so there is a gap between efgh and ijkl.
  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "f%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "f%"), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "ii%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "ii%"), 0.f);

  EXPECT_TRUE(hist->can_prune(PredicateCondition::Like, "iizzzzzzzz%"));
  EXPECT_FLOAT_EQ(hist->estimate_cardinality(PredicateCondition::Like, "iizzzzzzzz%"), 0.f);
}

}  // namespace opossum