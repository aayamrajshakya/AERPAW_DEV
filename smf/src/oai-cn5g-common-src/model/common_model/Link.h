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
 * Link.h
 *
 *
 */

#ifndef Link_H_
#define Link_H_

#include <string>
#include <nlohmann/json.hpp>

namespace oai::model::common {

/// <summary>
///
/// </summary>
class Link {
 public:
  Link();
  virtual ~Link() = default;

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

  bool operator==(const Link& rhs) const;
  bool operator!=(const Link& rhs) const;

  /////////////////////////////////////////////
  /// Link members

  /// <summary>
  ///
  /// </summary>
  std::string getHref() const;
  void setHref(std::string const& value);
  bool hrefIsSet() const;
  void unsetHref();

  friend void to_json(nlohmann::json& j, const Link& o);
  friend void from_json(const nlohmann::json& j, Link& o);

 protected:
  std::string m_Href;
  bool m_HrefIsSet;
};

}  // namespace oai::model::common

#endif /* Link_H_ */
