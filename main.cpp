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
            // curl --http2-prior-knowledge GET http://localhost:3000/users

            Json::StreamWriterBuilder writer;
            std::string jsonString = Json::writeString(writer, getUsersJson()) + "\n";

            res.write_head(200);
            res.end(jsonString);
        } else if (req.method() == "POST") {
            // curl --http2-prior-knowledge POST http://localhost:3000/users -d '{"id":123456, "name":"Satoru"}'

            req.on_data([&res](const uint8_t *data, std::size_t len) {
                std::string body(reinterpret_cast<const char *>(data), len);
                Json::CharReaderBuilder reader;
                Json::Value requestBody;
                std::string errors;
                std::istringstream iss(body);

                if (Json::parseFromStream(reader, iss, &requestBody, &errors)) {
                    int userID = requestBody["id"].asInt();
                    std::string userName = requestBody["name"].asString();
                    userData[userID] = userName;

                    Json::Value responseJson;
                    responseJson["message"] = "User added successfully";
                    responseJson["id"] = userID;
                    responseJson["name"] = userName;

                    Json::StreamWriterBuilder writer;
                    std::string jsonResponse = Json::writeString(writer, responseJson) + "\n";

                    res.write_head(201);
                    res.end(jsonResponse);
                } else {
                    res.write_head(400);
                    res.end(R"({"error": "Invalid JSON"})");
                }
            });

        } else {

        }
    });

    if (server.listen_and_serve(ec, "localhost", "3000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}
