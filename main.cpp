#include <nghttp2/asio_http2_server.h>
#include <json/json.h>
#include <iostream>
#include <map>

using namespace nghttp2::asio_http2::server;

std::map<int, std::string> userData = {
    {1, "Lyn"},
    {2, "Gojo"}
};

int main() {
    boost::system::error_code ec;
    http2 server;

    // GET, POST /api/users
    server.handle("/api/users", [](const request &req, const response &res) {
        if (req.method() == "GET") {
            Json::Value jsonResponse;
            for (const auto &[id, name] : userData) {
                jsonResponse[std::to_string(id)] = name;
            }

            Json::StreamWriterBuilder writer;
            std::string jsonResponseStr = Json::writeString(writer, jsonResponse);

            res.write_head(200);
            res.end(jsonResponseStr);
        } else if (req.method() == "POST") {
            std::string body = req.body();
            Json::CharReaderBuilder reader;
            Json::Value jsonRequest;
            std::string errs;

            if (Json::parseFromStream(reader, body, &jsonRequest, &errs)) {
                int newId = userData.size() + 1;
                std::string newName = jsonRequest["name"].asString();
                userData[newId] = newName;

                res.write_head(201);
                res.end(R"({"message": "User created", "id": )" + std::to_string(newId) + "}");
            } else {
                std::cerr << "JSON parsing error: " << errs << std::endl;
                res.write_head(400);
                res.end(R"({"error": "Invalid JSON format"})");
            }
        } else {
            res.write_head(405);
            res.end(R"({"error": "Method Not Allowed"})");
        }
    });

    // GET, PUT, DELETE /api/users/{id}
    server.handle("/api/users/{id}", [](const request &req, const response &res) {
        std::string idStr = req.uri().path.substr(std::string("/api/users/").length());
        int userId = std::stoi(idStr);

        if (req.method() == "GET") {
            if (userData.find(userId) != userData.end()) {
                Json::Value jsonResponse;
                jsonResponse["id"] = userId;
                jsonResponse["name"] = userData[userId];

                Json::StreamWriterBuilder writer;
                std::string jsonResponseStr = Json::writeString(writer, jsonResponse);

                res.write_head(200);
                res.end(jsonResponseStr);
            } else {
                res.write_head(404);
                res.end(R"({"error": "User not found"})");
            }
        } else if (req.method() == "PUT") {
            if (userData.find(userId) != userData.end()) {
                std::string body = req.body();
                Json::CharReaderBuilder reader;
                Json::Value jsonRequest;
                std::string errs;

                if (Json::parseFromStream(reader, body, &jsonRequest, &errs)) {
                    userData[userId] = jsonRequest["name"].asString();
                    res.write_head(200);
                    res.end(R"({"message": "User updated"})");
                } else {
                    std::cerr << "JSON parsing error: " << errs << std::endl;
                    res.write_head(400);
                    res.end(R"({"error": "Invalid JSON format"})");
                }
            } else {
                res.write_head(404);
                res.end(R"({"error": "User not found"})");
            }
        } else if (req.method() == "DELETE") {
            if (userData.find(userId) != userData.end()) {
                userData.erase(userId);
                res.write_head(200);
                res.end(R"({"message": "User deleted"})");
            } else {
                res.write_head(404);
                res.end(R"({"error": "User not found"})");
            }
        } else {
            res.write_head(405);
            res.end(R"({"error": "Method Not Allowed"})");
        }
    });

    if (server.listen_and_serve(ec, "0.0.0.0", "8080")) {
        std::cerr << "Error: " << ec.message() << std::endl;
    }

    return 0;
}

/*
GET
curl --http2 -v https://localhost:8080/api/users --insecure
curl --http2 -v https://localhost:8080/api/users/1 --insecure

POST
curl --http2 -v -X POST -H "Content-Type: application/json" -d '{"name": "Charlie"}' https://localhost:8080/api/users --insecure

PUT
curl --http2 -v -X PUT -H "Content-Type: application/json" -d '{"name": "Updated Name"}' https://localhost:8080/api/users/1 --insecure

DELETE
curl --http2 -v -X DELETE https://localhost:8080/api/users/1 --insecure
*/