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
 * UriScheme.h
 *
 *
 */

#ifndef UriScheme_H_
#define UriScheme_H_

#include "UriScheme_anyOf.h"
#include <nlohmann/json.hpp>

namespace oai::model::common {

/// <summary>
///
/// </summary>
class UriScheme {
 public:
  UriScheme();
  virtual ~UriScheme() = default;

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

  bool operator==(const UriScheme& rhs) const;
  bool operator!=(const UriScheme& rhs) const;

  /////////////////////////////////////////////
  /// UriScheme members

  UriScheme_anyOf getValue() const;
  void setValue(UriScheme_anyOf value);
  UriScheme_anyOf::eUriScheme_anyOf getEnumValue() const;
  std::string getEnumString() const;
  void setEnumValue(UriScheme_anyOf::eUriScheme_anyOf value);
  friend void to_json(nlohmann::json& j, const UriScheme& o);
  friend void from_json(const nlohmann::json& j, UriScheme& o);
  friend void to_json(nlohmann::json& j, const UriScheme_anyOf& o);
  friend void from_json(const nlohmann::json& j, UriScheme_anyOf& o);

 protected:
  UriScheme_anyOf m_value;
};

}  // namespace oai::model::common

#endif /* UriScheme_H_ */
