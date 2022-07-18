#pragma once

#include <nlohmann/json.hpp>

namespace mimik {
namespace wasm {
namespace model {
class http_request_extra {
 public:
  std::string replayNonce;
};

class http_request {
 public:
  std::string url;
  std::string method;
  std::map<std::string, std::string> headers;
  std::string jsonBody;
  http_request_extra extra;
};

class callback_result {
 public:
  int status;
  std::map<std::string, std::string> headers;
  std::string data;
};

class http_send_option {
 public:
  int statusCode = 200;
  std::string filePath;
  std::string fileMimeType;
};

class http_client_option {
 public:
  std::string url;
  std::string method;
  std::map<std::string, std::string> headers;
  std::string data;
  std::string mode;
  // std::string serviceMeshSecret;
  // std::string replayNonceUri;
};

class edge_decrypt_option {
 public:
  std::string type;
  std::string data;
  std::string token;
};

class edge_request_bep_option {
 public:
  std::string code;
};

class jsonrpc_function {
 public:
  std::string jsonrpc = "2.0";
  int id;
  std::string method;
  nlohmann::json params;
};

class http_event_context {
 public:
  std::map<std::string, std::string> info;
  std::map<std::string, std::string> env;
};

class http_event {
 public:
  http_event_context context;
  http_request request;
};

class callback_error {
 public:
  callback_error(const std::string&);
  std::string message;
};

class storage_item_option {
 public:
  std::string key;
  std::string value;
  std::string tag;
};

class storage_item_each_option {
 public:
  std::string tag;
};

using callback_error_ptr = std::shared_ptr<callback_error>;

}  // namespace model
}  // namespace wasm
}  // namespace mimik
