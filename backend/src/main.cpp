#include <express/legacy/in/WebApp.h>
#include <express/middleware/StaticMiddleware.h>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <deque>
#include <mutex>
#include <sstream>
#include <string>

namespace {

using json = nlohmann::json;
using LegacyWebApp = express::legacy::in::WebApp;

std::mutex eventMutex;
std::deque<json> recentEvents;

void pushEvent(const std::string& type, const json& payload) {
    std::scoped_lock lock(eventMutex);
    recentEvents.push_front({{"event", type}, {"payload", payload}});
    if (recentEvents.size() > 100) {
        recentEvents.pop_back();
    }
}

std::string makeSseFrame(const std::string& eventName, const json& data) {
    return "event: " + eventName + "\n" +
           "data: " + data.dump() + "\n\n";
}

json okJson(const json& payload = json::object()) {
    json result = payload;
    result["ok"] = true;
    return result;
}

json failJson(const std::string& error) {
    return {{"ok", false}, {"error", error}};
}

} // namespace

int main(int argc, char* argv[]) {
    express::WebApp::init(argc, argv);

    LegacyWebApp app;
    app.use(express::middleware::StaticMiddleware("./public"));

    app.get("/api/health", [] APPLICATION(req, res) {
        res->type("application/json");
        res->send(okJson({{"service", "inja-templatebuilder"}, {"backend", "SNode.C express"}, {"transport", "REST+SSE"}}).dump());
    });

    app.get("/api/examples", [] APPLICATION(req, res) {
        json payload = json::array({
            {
                {"name", "Greeting"},
                {"template", "Hello {{ user.name }}! Today is {{ day }}."},
                {"data", {{"user", {{"name", "Ada"}}}, {"day", "Tuesday"}}}
            },
            {
                {"name", "Loop"},
                {"template", "Shopping list:\n{% for item in items %}- {{ item }}\n{% endfor %}"},
                {"data", {{"items", {"Coffee", "Milk", "Chocolate"}}}}
            }
        });

        res->type("application/json");
        res->send(payload.dump());
    });

    app.post("/api/validate", [] APPLICATION(req, res) {
        try {
            auto body = json::parse(std::string(req->body.begin(), req->body.end()));
            const auto templateText = body.value("template", std::string{});
            const json dataValue = body.contains("data") ? body["data"] : json::parse("{}");
            const auto dataJson = dataValue.is_string() ? json::parse(dataValue.get<std::string>()) : dataValue;

            inja::Environment env;
            [[maybe_unused]] auto rendered = env.render(templateText, dataJson);

            res->type("application/json");
            res->send(okJson({{"valid", true}}).dump());
        } catch (const std::exception& ex) {
            res->status(400);
            res->type("application/json");
            res->send(failJson(ex.what()).dump());
        }
    });

    app.post("/api/render", [] APPLICATION(req, res) {
        try {
            auto body = json::parse(std::string(req->body.begin(), req->body.end()));
            const auto templateText = body.value("template", std::string{});
            const json dataValue = body.contains("data") ? body["data"] : json::parse("{}");
            const auto dataJson = dataValue.is_string() ? json::parse(dataValue.get<std::string>()) : dataValue;

            inja::Environment env;
            const std::string output = env.render(templateText, dataJson);

            const json telemetry = {
                {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
                {"templateSize", templateText.size()},
                {"outputSize", output.size()}
            };

            pushEvent("render-success", telemetry);

            res->type("application/json");
            res->send(okJson({{"output", output}, {"meta", telemetry}}).dump());
        } catch (const std::exception& ex) {
            const json telemetry = {
                {"ts", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
                {"error", ex.what()}
            };
            pushEvent("render-failure", telemetry);

            res->status(400);
            res->type("application/json");
            res->send(failJson(ex.what()).dump());
        }
    });

    // SSE endpoint: emits a connected frame plus buffered render events.
    // Clients reconnect automatically, so this endpoint can stay lightweight.
    app.get("/api/events", [] APPLICATION(req, res) {
        std::ostringstream stream;
        stream << makeSseFrame("connected", json{{"ts", std::time(nullptr)}});

        {
            std::scoped_lock lock(eventMutex);
            for (const auto& event : recentEvents) {
                stream << makeSseFrame(event.at("event"), event.at("payload"));
            }
        }

        res->type("text/event-stream");
        res->set("Cache-Control", "no-cache");
        res->set("Connection", "keep-alive");
        res->send(stream.str());
    });

    app.listen(3000, [](const LegacyWebApp::SocketAddress& socketAddress, const core::socket::State& state) {
        if (state == core::socket::State::OK) {
            std::cout << "SNode.C INJA Templatebuilder listening on " << socketAddress.toString() << std::endl;
        }
    });

    return express::WebApp::start();
}
