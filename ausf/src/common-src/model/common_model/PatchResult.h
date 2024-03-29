/**
 * Common Data Types
 * Common Data Types for Service Based Interfaces. © 2022, 3GPP Organizational
 * Partners (ARIB, ATIS, CCSA, ETSI, TSDSI, TTA, TTC). All rights reserved.
 *
 * The version of the OpenAPI document: 1.2.7
 *
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * (https://openapi-generator.tech). https://openapi-generator.tech Do not edit
 * the class manually.
 */
/*
 * PatchResult.h
 *
 *
 */

#ifndef PatchResult_H_
#define PatchResult_H_

#include "ReportItem.h"
#include <vector>
#include <nlohmann/json.hpp>

namespace oai::model::common {

/// <summary>
///
/// </summary>
class PatchResult {
 public:
  PatchResult();
  virtual ~PatchResult() = default;

  /// <summary>
  /// Validate the current data in the model. Throws a ValidationException on
  /// failure.
  /// </summary>
  void validate() const;

  /// <summary>
  /// Validate the current data in the model. Returns false on error and writes
  /// an error message into the given stringstream.
  /// </summary>
  bool validate(std::stringstream& msg) const;

  /// <summary>
  /// Helper overload for validate. Used when one model stores another model and
  /// calls it's validate. Not meant to be called outside that case.
  /// </summary>
  bool validate(std::stringstream& msg, const std::string& pathPrefix) const;

  bool operator==(const PatchResult& rhs) const;
  bool operator!=(const PatchResult& rhs) const;

  /////////////////////////////////////////////
  /// PatchResult members

  /// <summary>
  ///
  /// </summary>
  std::vector<oai::model::common::ReportItem> getReport() const;
  void setReport(std::vector<oai::model::common::ReportItem> const& value);

  friend void to_json(nlohmann::json& j, const PatchResult& o);
  friend void from_json(const nlohmann::json& j, PatchResult& o);

 protected:
  std::vector<oai::model::common::ReportItem> m_Report;
};

}  // namespace oai::model::common

#endif /* PatchResult_H_ */
