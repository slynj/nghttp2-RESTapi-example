#include <iostream>
#include <map>
#include <nghttp2/asio_http2_server.h>
#include <json/json.h>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

std::map<int, std::string> userData = {
    {753951, "Lyn"},
    {574269, "Gojo"}
};


Json::Value getUsersJson() {
    Json::Value users(Json::arrayValue);

    for (const auto &user : userData) {
        Json::Value userJson;
        userJson["id"] = user.first;
        userJson["name"] = user.second;
        users.append(userJson);
    }

    // std::cout << users << std::endl;

    return users;
}


int main() {
    boost::system::error_code ec;
    http2 server;

    server.handle("/", [](const request &req, const response &res) {
        std::cout << req.method() << std::endl;

        res.write_head(200);
        res.end("This is Lyn's Example! Hello, World :>\n");
    });

    server.handle("/users", [](const request &req, const response &res) {
        if (req.method() == "GET") {
            Json::StreamWriterBuilder writer;
            std::string jsonString = Json::writeString(writer, getUsersJson()) + "\n";

            res.write_head(200);
            res.end(jsonString);
            




        } else if (req.method() == "POST") {

        } else {

        }
    });

    if (server.listen_and_serve(ec, "localhost", "3000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}
