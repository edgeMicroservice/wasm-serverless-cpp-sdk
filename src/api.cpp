#include "init.hpp"
#include "api.hpp"

using cppjson = nlohmann::json;

namespace mimik {
namespace wasm {
namespace model {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(http_request, url, method, headers,
                                   jsonBody);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(http_client_option, url, method, headers,
                                   data, mode);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(callback_result, status, headers, data);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(edge_decrypt_option, type, data, token);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(edge_request_bep_option, code);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(http_event_context, info, env);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(http_event, context, request);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jsonrpc_function, jsonrpc, id, method,
                                   params);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(storage_item_option, key, value, tag);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(storage_item_each_option, tag);

void to_json(cppjson& j, const http_send_option& p) {
  j = cppjson{{"statusCode", p.statusCode}};

  if (!p.fileMimeType.empty()) {
    j["fileMimeType"] = p.fileMimeType;
  }

  if (!p.filePath.empty()) {
    j["filePath"] = p.filePath;
  }
}
}  // namespace model
}  // namespace wasm
}  // namespace mimik

using namespace mimik::wasm::api;
using namespace mimik::wasm::model;
using namespace mimik::wasm;

using rpc_callback_v2 = std::function<void(
    const std::shared_ptr<callback_error>&, const callback_result&)>;

using rpc_callback_v3 = std::function<int(
    const std::shared_ptr<callback_error>&, const callback_result&)>;

static int64_t register_result_callback(rpc_callback_v2 callback) {
  auto cb = rpc::register_callback([callback](const std::vector<uint8_t>& jsonrpc) {
    auto doc = cppjson::from_msgpack(jsonrpc);
    auto rpc = doc.get<jsonrpc_function>();

    if (rpc.method == "oncallback.success") {
      callback_result res = rpc.params.at("data");
      callback({}, res);
    } else if (rpc.method == "oncallback.error_with_data") {
      std::string message = rpc.params.at("error").at("message");
      callback_result res = rpc.params.at("data");
      callback(std::make_shared<callback_error>(message), res);
    } else {
      std::string message = rpc.params.at("error").at("message");
      callback(std::make_shared<callback_error>(message), {});
    }

    return 0;
  });

  return cb;
}

static int64_t register_result_ret_callback(rpc_callback_v3 callback) {
  auto cb = rpc::register_callback([callback](const std::vector<uint8_t>& jsonrpc) {
    auto doc = cppjson::from_msgpack(jsonrpc);
    auto rpc = doc.get<jsonrpc_function>();

    if (rpc.method == "oncallback.success") {
      callback_result res = rpc.params.at("data");
      return callback({}, res);
    } else if (rpc.method == "oncallback.error_with_data") {
      std::string message = rpc.params.at("error").at("message");
      callback_result res = rpc.params.at("data");
      return callback(std::make_shared<callback_error>(message), res);
    } else {
      std::string message = rpc.params.at("error").at("message");
      return callback(std::make_shared<callback_error>(message), {});
    }
  });

  return cb;
}

static std::vector<uint8_t> jsonrpcStringify(int32_t id, const std::string& method,
                                    const nlohmann::json& params) {
  jsonrpc_function rpc;
  rpc.id = id;
  rpc.method = method;
  rpc.params = params;

  cppjson jsonrpc = rpc;

  return cppjson::to_msgpack(jsonrpc);
}

callback_error::callback_error(const std::string& message) : message(message) {}

http_event request::parse(const std::vector<uint8_t>& jsonrpc) {
  auto doc = cppjson::from_msgpack(jsonrpc);
  auto rpc = doc.get<jsonrpc_function>();

  auto request = rpc.params.get<http_event>();

  return request;
}

void request::handle_form_upload(form_request_found_handler found,
                                 form_request_get_handler get,
                                 form_request_store_handler store) {
  auto id = register_result_ret_callback(
      [found, get, store](const std::shared_ptr<callback_error>& error,
                          const callback_result& result) {
        auto formAction = result.status;
        if (formAction == 101) {
          std::string key = result.headers.at("key");
          std::string filename = result.headers.at("filename");

          int64_t pathref = std::stoll(result.headers.at("path_ref"));
          size_t pathlen = std::stoll(result.headers.at("path_len"));
          std::string path;
          auto action = found(key, filename, path);

          if (!path.empty()) {
            mk_module_set_buffer(pathref, (uint32_t)path.c_str(),
                                 path.length());
          }

          return (int)action;
        } else if (formAction == 102) {
          std::string key = result.headers.at("key");
          std::string value = result.headers.at("value");

          get(key, value);
        } else if (formAction == 103) {
          std::string path = result.headers.at("path");
          size_t file_size = std::stoll(result.headers.at("file_size"));

          store(path, file_size);
        }

        return (int)0;
      });
  //////////////
  nlohmann::json dummy;
  auto jsonrpc = jsonrpcStringify(id, "request.handle_form_upload", dummy);

  rpc::call(id, jsonrpc);
}

std::vector<uint8_t> response::_send(const std::string& data) {
  auto params = cppjson::array();
  params.push_back(data);

  // [TODO]
  return jsonrpcStringify(0, "response.send", params);
}

std::vector<uint8_t> response::_send(const std::string& data,
                            const model::http_send_option& option) {
  auto params = cppjson::array();
  params.push_back(data);
  params.push_back(option);

  // [TODO]
  return jsonrpcStringify(0, "response.send", params);
}

void response::send(const std::string& data) {
  auto jsonrpc = response::_send(data);

  rpc::call(0, jsonrpc);
}

void response::send(const std::string& data,
                    const model::http_send_option& option) {
  auto jsonrpc = response::_send(data, option);

  rpc::call(0, jsonrpc);
}

std::vector<uint8_t> httpclient::_fetch(int32_t id, const http_client_option& opt) {
  return jsonrpcStringify(id, "httpclient.fetch", opt);
}

void httpclient::fetch(const http_client_option& opt,
                       httpclient_callback callback) {
  auto id = register_result_callback(callback);

  http_client_option opt1 = opt;

  auto jsonrpc = httpclient::_fetch(id, opt1);

  rpc::call(id, jsonrpc);
}

std::vector<uint8_t> edge::_decrypt(int32_t id, const edge_decrypt_option& opt) {
  return jsonrpcStringify(id, "edge.decrypt", opt);
}

std::vector<uint8_t> edge::_bep(int32_t id,
                       const mimik::wasm::model::edge_request_bep_option& opt) {
  return jsonrpcStringify(id, "edge.request_bep", opt);
}

void edge::request_bep(const std::string& token, const std::string& nodeId,
                       int httpPort, edge_function_callback callback) {
  long long expireAt = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count() +
                       10 * 60;  // 10 min from now;

  std::string expiry = std::to_string(expireAt);

  auto getRequestBepHmacCode =
      [](const std::string& token, const std::string& nodeId, int httpPort,
         const std::string& expiry, edge_function_callback cb) {
        http_client_option opt;
        opt.url = std::string("http://localhost:") + std::to_string(httpPort) +
                  std::string("/jsonrpc/v1");
        opt.method = "POST";

        cppjson d;
        auto params = cppjson::array();
        params.push_back(token);
        params.push_back(expiry);
        params.push_back(nodeId);
        params.push_back("jsapi://requestBep");

        d["id"] = 1;
        d["jsonrpc"] = "2.0";
        d["method"] = "getEdgeHmacCode";
        d["params"] = params;

        opt.data = d.dump();

        httpclient().fetch(
            opt, [cb](const std::shared_ptr<callback_error>& errorPtr,
                      const callback_result& res) {
              if (errorPtr) {
                cb(errorPtr, "");
                return;
              }

              auto r = cppjson::parse(res.data);
              if (!r.count("result")) {
                cb(std::make_shared<callback_error>(
                       "invalid getEdgeHmacCode jsonrpc response: " + res.data),
                   "");
                return;
              }

              std::string edgeHmacCode;
              r.at("result").at("edgeHmacCode").get_to(edgeHmacCode);

              cb(std::shared_ptr<callback_error>(), edgeHmacCode);
            });
      };

  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  getRequestBepHmacCode(
      token, nodeId, httpPort, expiry,
      [callback, id](const std::shared_ptr<callback_error>& error,
                     const std::string token) {
        if (error) {
          callback(error, "");
          return;
        }

        auto opt = edge_request_bep_option{token};

        auto jsonrpc = edge::_bep(id, opt);

        rpc::call(id,jsonrpc);
      });
}

void edge::decrypt(const CLUSTER_TYPE& aType, const std::string& data,
                   const std::string& token, edge_function_callback callback) {
  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  std::string type;
  switch (aType) {
    case CLUSTER_TYPE::LOCAL:
      type = "local";
      break;
    case CLUSTER_TYPE::ACCOUNT:
      type = "account";
      break;
    case CLUSTER_TYPE::PROXIMITY:
      type = "proximity";
      break;
  }

  auto opt = edge_decrypt_option{type, data, token};

  auto jsonrpc = edge::_decrypt(id, opt);

  rpc::call(id,jsonrpc);
}

void storage::get_item(const std::string& key, storage_callback callback) {
  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  auto opt = storage_item_option({key});

  auto jsonrpc = jsonrpcStringify(id, "storage.get_item", opt);

  rpc::call(id, jsonrpc);
}

void storage::set_item(const std::string& key, const std::string& value,
                       storage_callback callback) {
  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  auto opt = storage_item_option({key, value});

  auto jsonrpc = jsonrpcStringify(id, "storage.set_item", opt);

  rpc::call(id, jsonrpc);
}

void storage::set_item_with_tag(const std::string& key,
                                const std::string& value,
                                const std::string& tag,
                                storage_callback callback) {
  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  auto opt = storage_item_option({key, value, tag});

  auto jsonrpc = jsonrpcStringify(id, "storage.set_item_with_tag", opt);

  rpc::call(id, jsonrpc);
}

void storage::remove_item(const std::string& key, storage_callback callback) {
  auto id = register_result_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        callback(error, result.data);
      });

  auto opt = storage_item_option({key});

  auto jsonrpc = jsonrpcStringify(id, "storage.remove_item", opt);

  rpc::call(id,jsonrpc);
}

void storage::each_item(storage_each_item_callback callback) {
  auto id = register_result_ret_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        if (result.headers.empty()) {
          return 0;
        }

        auto iter = result.headers.begin();
        return callback(iter->first, iter->second);
      });

  auto opt = storage_item_each_option();

  auto jsonrpc = jsonrpcStringify(id, "storage.each_item", opt);

  rpc::call(id, jsonrpc);
}

void storage::each_item(const std::string& tag,
                        storage_each_item_callback callback) {
  auto id = register_result_ret_callback(
      [callback](const std::shared_ptr<callback_error>& error,
                 const callback_result& result) {
        if (result.headers.empty()) {
          return 0;
        }

        auto iter = result.headers.begin();
        return callback(iter->first, iter->second);
      });

  auto opt = storage_item_each_option({tag});

  auto jsonrpc = jsonrpcStringify(id, "storage.each_item", opt);

  rpc::call(id, jsonrpc);
}
