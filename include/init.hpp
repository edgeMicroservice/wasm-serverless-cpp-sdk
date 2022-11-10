#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <functional>

#include "api.hpp"

#define WASM_EXPORT __attribute__((used)) __attribute__((visibility("default")))
#define WASM_EXPORT_AS(NAME) WASM_EXPORT __attribute__((export_name(NAME)))
#define WASM_IMPORT(MODULE, NAME) \
  __attribute__((import_module(MODULE))) __attribute__((import_name(NAME)))

WASM_IMPORT("env", "mk_module_api_request")
void mk_module_api_request(int32_t sessionId, uint32_t memAddr, uint32_t len);

WASM_IMPORT("env", "mk_module_get_buffer")
void mk_module_get_buffer(int32_t memAddr, int64_t bufRef,
                        int32_t strLen);

WASM_IMPORT("env", "mk_module_set_buffer")
void mk_module_set_buffer(int64_t bufRef, int32_t memAddr,
                              int32_t strlen);

extern "C" {
  void WASM_EXPORT_AS("mk_module_on_event") mk_module_on_event(int64_t buf, uint32_t len);
  int32_t WASM_EXPORT_AS("mk_module_on_api_response") mk_module_on_api_response(int64_t sid, int64_t buf,
                                              uint32_t len);
  void WASM_EXPORT_AS("mk_module_abi_version_2_0_0") mk_module_abi_version_2_0_0();                         
}

namespace mimik {
namespace wasm {
using rpc_callback = std::function<int(const std::vector<uint8_t> &)>;

class rpc {
 public:
  static int32_t register_callback(rpc_callback callback);
  static void call(int32_t id, const std::vector<uint8_t>& json);
};

using mk_module_http_event_callback = std::function<void(const mimik::wasm::model::http_event&)>;

class mk_module_event_handler {
  public:
    mk_module_http_event_callback http_event_callback;
};

void init_mk_module_event_handler(const mk_module_event_handler&);

}  // namespace wasm
}  // namespace mimik

