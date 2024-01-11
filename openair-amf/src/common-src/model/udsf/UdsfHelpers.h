#ifndef Udsf_Helpers_H_
#define Udsf_Helpers_H_

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <ctime>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ClientId.h"
#include "Helpers.h"
#include "NFType.h"
#include "RetrieveRecords.h"
#include "SearchExpression.h"
using namespace oai::model::udsf;

namespace oai::model::common::helpers {
bool fromStringValue(
    const std::string& inStr, oai::model::udsf::ClientId& value);
bool fromStringValue(
    const std::string& inStr, oai::model::udsf::SearchExpression& value);
bool fromStringValue(
    const std::string& inStr, oai::model::udsf::ClientId& value);
bool fromStringValue(
    const std::string& inStr, oai::model::udsf::SearchExpression& value);
bool fromStringValue(
    const std::string& inStr, oai::model::udsf::RetrieveRecords& value);

void fromPistacheToStdStringOptional(
    const Pistache::Optional<Pistache::Http::Header::Raw>& inOpt,
    std::optional<std::string>& outOpt);

template<typename T>
void fromPistacheStdOptional(
    const Pistache::Optional<T>& inOpt, std::optional<T>& outOpt) {
  if (!inOpt.isEmpty()) {
    outOpt = std::optional<T>(inOpt.get());
  }
}

}  // namespace oai::model::common::helpers

#endif  // Helpers_H_
