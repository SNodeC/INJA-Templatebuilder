#include "inja.hpp"
#include <express/legacy/in/WebApp.h>
#include <express/middleware/StaticMiddleware.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>
#include <string>

namespace {

using json = nlohmann::json;
using LegacyWebApp = express::legacy::in::WebApp;

json okJson(const json &payload = json::object()) {
  json result = payload;
  result["ok"] = true;
  return result;
}

json failJson(const std::string &error, const json &meta = json::object()) {
  json result = {{"ok", false}, {"error", error}};
  if (!meta.empty()) {
    result["meta"] = meta;
  }
  return result;
}

} // namespace

int main(int argc, char *argv[]) {
  express::WebApp::init(argc, argv);

  LegacyWebApp app;

  app.use(express::middleware::StaticMiddleware("./public"));

  app.get("/api/health", [] APPLICATION(req, res) {
    res->type("application/json");
    res->send(okJson({{"service", "inja-templatebuilder"},
                      {"backend", "SNode.C express"},
                      {"transport", "REST"}})
                  .dump());
  });

  app.get("/api/examples", [] APPLICATION(req, res) {
    json payload = json::array(
        {{{"name", "Greeting"},
          {"template", "Hello {{ user.name }}! Today is {{ day }}."},
          {"data", {{"user", {{"name", "Ada"}}}, {"day", "Tuesday"}}}},
         {{"name", "Loop"},
          {"template",
           "Shopping list:\n{% for item in items %}- {{ item }}\n{% endfor %}"},
          {"data", {{"items", {"Coffee", "Milk", "Chocolate"}}}}}});

    res->type("application/json");
    res->send(payload.dump());
  });

  app.post("/api/validate", [] APPLICATION(req, res) {
    try {
      auto body = json::parse(std::string(req->body.begin(), req->body.end()));
      const auto templateText = body.value("template", std::string{});
      const json dataValue =
          body.contains("data") ? body["data"] : json::parse("{}");
      const auto dataJson = dataValue.is_string()
                                ? json::parse(dataValue.get<std::string>())
                                : dataValue;

      inja::Environment env;
      [[maybe_unused]] auto rendered = env.render(templateText, dataJson);

      const json telemetry = {
          {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count()},
          {"templateSize", templateText.size()},
          {"dataType", dataJson.type_name()}};

      res->type("application/json");
      res->send(okJson({{"valid", true}, {"meta", telemetry}}).dump());
    } catch (const std::exception &ex) {
      const json telemetry = {
          {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count()},
          {"error", ex.what()}};
      res->status(400);
      res->type("application/json");
      res->send(failJson(ex.what(), telemetry).dump());
    }
  });

  app.post("/api/render", [] APPLICATION(req, res) {
    try {
      auto body = json::parse(std::string(req->body.begin(), req->body.end()));
      const auto templateText = body.value("template", std::string{});
      const json dataValue =
          body.contains("data") ? body["data"] : json::parse("{}");
      const auto dataJson = dataValue.is_string()
                                ? json::parse(dataValue.get<std::string>())
                                : dataValue;

      inja::Environment env;
      const std::string output = env.render(templateText, dataJson);

      const json telemetry = {
          {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count()},
          {"templateSize", templateText.size()},
          {"outputSize", output.size()}};

      res->type("application/json");
      res->send(okJson({{"output", output}, {"meta", telemetry}}).dump());
    } catch (const std::exception &ex) {
      const json telemetry = {
          {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count()},
          {"error", ex.what()}};

      res->status(400);
      res->type("application/json");
      res->send(failJson(ex.what(), telemetry).dump());
    }
  });

  app.getConfig()->setReuseAddress();

  app.listen(3000, [](const LegacyWebApp::SocketAddress &socketAddress,
                      const core::socket::State &state) {
    if (state == core::socket::State::OK) {
      std::cout << "SNode.C INJA Templatebuilder listening on "
                << socketAddress.toString() << std::endl;
    }
  });

  return express::WebApp::start();
}
