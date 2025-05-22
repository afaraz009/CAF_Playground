// caf_http_client_openai.cpp
//
// Simple CAF HTTP client that sends a POST to OpenAI's Chat Completions API
// and prints out the response.

#include "caf/net/http/client.hpp"
#include "caf/net/http/with.hpp"
#include "caf/net/ip.hpp"
#include "caf/net/middleman.hpp"
#include "caf/net/octet_stream/transport.hpp"
#include "caf/net/tcp_stream_socket.hpp"
#include "caf/net/ssl/context.hpp"
//#include "caf/net/ssl/verify.hpp"
#include "caf/actor_system.hpp"
#include "caf/actor_system_config.hpp"
#include "caf/caf_main.hpp"
#include "caf/event_based_actor.hpp"
#include "caf/span.hpp"
#include "openssl/ssl.h"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <string>
#include <string_view>

using namespace std::literals;
namespace http = caf::net::http;
namespace ssl = caf::net::ssl;
using namespace caf;


// we always do a POST
constexpr auto default_method = http::method::post;

struct config : actor_system_config {
    config() {
        // no load<…>() calls here
        opt_group{ custom_options_, "global" }
            .add<caf::net::http::method>("method,m",
                "HTTP method to use (ignored, always POST)")
            .add<std::string>("payload,p", "Payload (ignored)");
        /*opt_group{ custom_options_, "tls" }
            .add<std::string>("ca-file",
                "Path to CA bundle (e.g. cacert.pem)");*/
    }

    caf::settings dump_content() const override {
        auto result = actor_system_config::dump_content();
        caf::put_missing(result, "method", default_method);
        return result;
    }
};

namespace {
    std::atomic<bool> shutdown_flag{ false };
}

int caf_main(actor_system& sys, const config& cfg) {
    // allow SIGTERM to break out
    signal(SIGTERM, [](int) { shutdown_flag = true; });

    // require exactly one positional: the URL
    auto args = cfg.remainder();
    if (args.size() != 1) {
        sys.println("*** expected positional argument: URL");
        return EXIT_FAILURE;
    }

    // parse URL
    uri resource;
    if (auto err = parse(args[0], resource)) {
        sys.println("*** bad URI: {}", err);
        return EXIT_FAILURE;
    }

    // fetch --tls.ca-file (key is "tls.ca-file")
    // auto ca_file = caf::get_or(cfg, "tls.ca-file", std::string{});

    // read API key
    auto* key = std::getenv("OPENAI_API_KEY");
    if (!key) {
        sys.println("*** please set OPENAI_API_KEY in the environment");
        return EXIT_FAILURE;
    }
    std::string api_key = key;

    // our JSON body
    auto payload = R"({
    "model":"gpt-3.5-turbo",
    "messages":[
      {"role":"system","content":"You are a helpful assistant."},
      {"role":"user","content":"Hello from CAF!"}
    ]
  })"sv;

    // build the request
    auto result = http::with(sys)
        .context_factory([]() -> caf::expected<ssl::context> {


        return ssl::emplace_client(ssl::tls::v1_2)();
        })
        .connect(resource)
        .retry_delay(1s)
        .max_retry_count(5)
        .connection_timeout(250ms)
        .add_header_field("User-Agent", "CAF-OpenAI-Client")
        .add_header_field("Authorization", "Bearer " + api_key)
        .add_header_field("Content-Type", "application/json")
        .request(default_method, payload);

    if (!result) {
        sys.println("*** Failed to initiate connection: {}", result.error());
        return EXIT_FAILURE;
    }

    // handle the response
    sys.spawn([res = result->first](event_based_actor* self) {
        res.bind_to(self).then(
            [self](const http::response& r) {
                self->println("HTTP {} {}", uint16_t(r.code()), phrase(r.code()));
                for (auto& [k, v] : r.header_fields())
                    self->println("{}: {}", k, v);
                if (r.body().empty()) return;
                if (is_valid_utf8(r.body()))
                    self->println("Body: {}", to_string_view(r.body()));
                else {
                    self->println("Body (hex):");
                    auto bytes = r.body();
                    while (!bytes.empty()) {
                        auto row = bytes.subspan(0, std::min<size_t>(8, bytes.size()));
                        bytes = bytes.subspan(row.size());
                        self->println("  {}", to_hex_str(row));
                    }
                }
            },
            [self](const error& err) {
                self->println("*** HTTP error: {}", err);
            }
        );
        });

    return EXIT_SUCCESS;
}

CAF_MAIN(caf::net::middleman)
