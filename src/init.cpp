
#include <stdio.h>

#include <string>
#include <map>
#include <vector>

#include "init.hpp"

static int64_t rpc_count = 0;
static std::map<int64_t, mimik::wasm::rpc_callback> rpc_callbacks;

static std::string extract_data(int64_t strAddr, uint32_t strLen);
static mimik::wasm::mk_module_event_handler global_module_event_handler;

////////////////////////////////////////////
static void mk_module_event_handler(const std::string &jsonrpc) {
  auto evt = mimik::wasm::api::request::parse(jsonrpc);
  global_module_event_handler.http_event_callback(evt);
}

/////////////////// WASM EXPORT BEGIN //////////////
void mk_module_on_event(int64_t buf, uint32_t len) {
  auto json = extract_data(buf, len);
  mk_module_event_handler(json);
}

void mk_module_abi_version_1_0_0() {}

int32_t WASM_EXPORT mk_module_on_api_response(int64_t sid, int64_t buf,
                                              uint32_t len) {
  auto json = extract_data(buf, len);

  if (!sid) {
    // exception
    return 0;
  }

  auto iter = rpc_callbacks.find(sid);

  if (iter == rpc_callbacks.end()) {
    // exception
    return 0;
  }

  auto callback = iter->second;
  int ret = callback(json);
  //
  // rpc_callbacks.erase(iter);
  //
  return ret;
}
/////////////////// WASM EXPORT END //////////////

static std::string extract_data(int64_t strAddr, uint32_t strLen) {
  std::vector<char> buf(strLen);

  mk_module_get_buffer((uint32_t)&buf[0], strAddr, strLen);

  return std::string(&buf[0], strLen);
}

int32_t mimik::wasm::rpc::register_callback(rpc_callback callback) {
  rpc_count++;
  rpc_callbacks[rpc_count] = callback;

  return rpc_count;
}

void mimik::wasm::rpc::call(const std::string &json) {
  uint8_t *buf = (uint8_t *)json.c_str();
  int32_t len = json.size();
  mk_module_api_request((uint32_t)buf, len);
}

#if __clang_major__ > 12
extern "C" {
extern void __wasm_call_ctors();
}
#endif

void mimik::wasm::init_mk_module_event_handler(
    const mk_module_event_handler &handler) {
#if __clang_major__ > 12
  __wasm_call_ctors();
#endif
  global_module_event_handler = handler;
}
