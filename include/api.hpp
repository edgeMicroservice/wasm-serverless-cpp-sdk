#pragma once

#include "model.hpp"

namespace mimik {
namespace wasm {
namespace api {
using httpclient_callback =
    std::function<void(const model::callback_error_ptr& error,
                       const model::callback_result& result)>;

using edge_function_callback =
    std::function<void(const mimik::wasm::model::callback_error_ptr& error,
                       const std::string& data)>;

using storage_callback =
    std::function<void(const mimik::wasm::model::callback_error_ptr& error,
                       const std::string& data)>;

using storage_each_item_callback =
    std::function<int(const std::string& key, const std::string& value)>;

enum form_request_found_action {
  skip = 100,
  get = 101,
  store = 102,
  abort = 200
};
using form_request_found_handler = std::function<form_request_found_action(
    const std::string& key, const std::string& filename, std::string& path)>;

using form_request_get_handler =
    std::function<void(const std::string& key, const std::string& value)>;

using form_request_store_handler =
    std::function<void(const std::string& path, const size_t file_size)>;

class request {
 public:
  static mimik::wasm::model::http_event parse(const std::vector<uint8_t>& jsonrpc);

  static void handle_form_upload(form_request_found_handler found,
                                 form_request_get_handler get,
                                 form_request_store_handler store);
};

class response {
 public:
  static std::vector<uint8_t> _send(const std::string& data);
  static std::vector<uint8_t> _send(const std::string& data,
                           const model::http_send_option& option);

 public:
  void send(const std::string& data);
  void send(const std::string& data, const model::http_send_option& option);
};

class httpclient {
 public:
  static std::vector<uint8_t> _fetch(int32_t id, const mimik::wasm::model::http_client_option& opt);

 public:
  void fetch(const mimik::wasm::model::http_client_option& opt,
             httpclient_callback callback);
};

enum CLUSTER_TYPE { LOCAL = 100, ACCOUNT = 101, PROXIMITY = 102 };

class edge {
 public:
  static std::vector<uint8_t>  _decrypt(
      int32_t id,
      const mimik::wasm::model::edge_decrypt_option& opt);

  static std::vector<uint8_t> _bep(
      int32_t id,
      const mimik::wasm::model::edge_request_bep_option& opt);

 public:
  void decrypt(const CLUSTER_TYPE& type, const std::string& data,
               const std::string& token, edge_function_callback callback);

  void request_bep(const std::string& accessToken, const std::string& nodeId,
                   int httpPort, edge_function_callback callback);
};

class storage {
 public:
  void get_item(const std::string& key, storage_callback callback);
  void set_item(const std::string& key, const std::string& value,
                storage_callback callback);
  void set_item_with_tag(const std::string& key, const std::string& value,
                         const std::string& tag, storage_callback callback);
  void remove_item(const std::string& key, storage_callback callback);

  void each_item(storage_each_item_callback callback);

  void each_item(const std::string& tag, storage_each_item_callback callback);
};

class WebSocketBroker{
  public:
    void publish(const std::string& type, const std::string& message);
};

}  // namespace api
}  // namespace wasm
}  // namespace mimik