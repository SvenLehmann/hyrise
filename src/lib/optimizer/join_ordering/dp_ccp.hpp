#pragma once

#include "abstract_dp_algorithm.hpp"
#include "dp_subplan_cache_best.hpp"

namespace opossum {

class DpCcp : public AbstractDpAlgorithm {
 public:
  explicit DpCcp(const std::shared_ptr<const AbstractCostModel>& cost_model);

 protected:
  void _on_execute() override;
};

}  // namespace opossum
