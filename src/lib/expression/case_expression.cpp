#include "case_expression.hpp"

namespace opossum {

CaseExpression::CaseExpression(const std::shared_ptr<AbstractExpression>& when,
               const std::shared_ptr<AbstractExpression>& then,
               const std::shared_ptr<AbstractExpression>& else_):
  AbstractExpression(AbstractExpression::Case, {when, then , else_})  {}

const std::shared_ptr<AbstractExpression>& CaseExpression::when() const {
  return arguments[0];
}

const std::shared_ptr<AbstractExpression>& CaseExpression::then() const {
  return arguments[1];
}

const std::shared_ptr<AbstractExpression>& CaseExpression::else_() const {
  return arguments[2];
}

std::shared_ptr<AbstractExpression> CaseExpression::deep_copy() const {
  return std::make_shared<CaseExpression>(when(), then(), else_());
}

}  // namespace opossum
