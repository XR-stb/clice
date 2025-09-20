#pragma once

#include "../Basic.h"

namespace clice::proto {

struct DefinitionClientCapabilities {};

using DefinitionOptions = WorkDoneProgressOptions;

struct PartialResultParams {};

using DefinitionResult = std::vector<Location>;

struct DefinitionParams : TextDocumentPositionParams, PartialResultParams {};

}  // namespace clice::proto
