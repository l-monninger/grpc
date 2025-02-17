//
// Copyright 2016 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef GRPC_SRC_CORE_LIB_SERVICE_CONFIG_SERVICE_CONFIG_CALL_DATA_H
#define GRPC_SRC_CORE_LIB_SERVICE_CONFIG_SERVICE_CONFIG_CALL_DATA_H

#include <grpc/support/port_platform.h>

#include <stddef.h>

#include <map>
#include <memory>
#include <utility>

#include "src/core/lib/gprpp/ref_counted_ptr.h"
#include "src/core/lib/gprpp/unique_type_name.h"
#include "src/core/lib/service_config/service_config.h"
#include "src/core/lib/service_config/service_config_parser.h"

namespace grpc_core {

/// Stores the service config data associated with an individual call.
/// A pointer to this object is stored in the call_context
/// GRPC_CONTEXT_SERVICE_CONFIG_CALL_DATA element, so that filters can
/// easily access method and global parameters for the call.
class ServiceConfigCallData {
 public:
  class CallAttributeInterface {
   public:
    virtual ~CallAttributeInterface() = default;
    virtual UniqueTypeName type() const = 0;
  };

  using CallAttributes = std::map<UniqueTypeName, CallAttributeInterface*>;

  ServiceConfigCallData() : method_configs_(nullptr) {}

  ServiceConfigCallData(
      RefCountedPtr<ServiceConfig> service_config,
      const ServiceConfigParser::ParsedConfigVector* method_configs,
      CallAttributes call_attributes)
      : service_config_(std::move(service_config)),
        method_configs_(method_configs),
        call_attributes_(std::move(call_attributes)) {}

  ServiceConfig* service_config() { return service_config_.get(); }

  ServiceConfigParser::ParsedConfig* GetMethodParsedConfig(size_t index) const {
    return method_configs_ != nullptr ? (*method_configs_)[index].get()
                                      : nullptr;
  }

  ServiceConfigParser::ParsedConfig* GetGlobalParsedConfig(size_t index) const {
    return service_config_->GetGlobalParsedConfig(index);
  }

  // Must be called when holding the call combiner (legacy filter) or from
  // inside the activity (promise-based filter).
  void SetCallAttribute(CallAttributeInterface* value) {
    call_attributes_.emplace(value->type(), value);
  }

  CallAttributeInterface* GetCallAttribute(UniqueTypeName name) const {
    auto it = call_attributes_.find(name);
    if (it == call_attributes_.end()) return nullptr;
    return it->second;
  }

 private:
  RefCountedPtr<ServiceConfig> service_config_;
  const ServiceConfigParser::ParsedConfigVector* method_configs_;
  CallAttributes call_attributes_;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_LIB_SERVICE_CONFIG_SERVICE_CONFIG_CALL_DATA_H
