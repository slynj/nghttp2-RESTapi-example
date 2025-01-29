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
            // curl --http2-prior-knowledge -X GET http://localhost:3000/users

            Json::StreamWriterBuilder writer;
            std::string jsonString = Json::writeString(writer, getUsersJson()) + "\n";

            res.write_head(200);
            res.end(jsonString);
        } else if (req.method() == "POST") {
            // curl --http2-prior-knowledge -X POST http://localhost:3000/users -d '{"id":123456, "name":"Satoru"}'
            // curl --http2-prior-knowledge -X POST http://localhost:3000/users -d '{"ids":123456, "names":"Satoru"}'

            req.on_data([&res](const uint8_t *data, std::size_t len) {
                std::string body(reinterpret_cast<const char *>(data), len);
                Json::CharReaderBuilder reader;
                Json::Value requestBody;
                std::string errors;
                std::istringstream iss(body);

                if (!requestBody.isMember("name") || !requestBody.isMember("id")) {
                    res.write_head(404);
                    res.end("{\"error\": \"Key not found\"}\n");
                }

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
            // curl --http2-prior-knowledge -X PUT http://localhost:3000/users

            res.write_head(405);
            res.end("{\"error\": \"Method not allowed\"}\n");
        }
    });

    server.handle("/users/", [](const request &req, const response &res) {
        std::string path = req.uri().path;
        std::string userIDStr = path.substr(path.find_last_of('/') + 1);
        int userID = std::stoi(userIDStr);

        if (req.method() == "GET") {
            // curl --http2-prior-knowledge -X GET http://localhost:3000/users/753951
            // curl --http2-prior-knowledge -X GET http://localhost:3000/users/000000

            if (userData.find(userID) != userData.end()) {
                Json::Value userJson;
                userJson["id"] = userID;
                userJson["name"] = userData[userID];

                Json::StreamWriterBuilder writer;
                std::string jsonString = Json::writeString(writer, userJson) + "\n";

                res.write_head(200);
                res.end(jsonString);
            } else {
                res.write_head(404);
                res.end("{\"error\": \"User not found\"}\n");
            }
            
        } else if (req.method() == "PUT") {
            // curl --http2-prior-knowledge -X PUT http://localhost:3000/users/753951 -d '{"name":"Lyn Jeong"}'
            // curl --http2-prior-knowledge -X PUT http://localhost:3000/users/000000 -d '{"name":"Geto"}'
            // curl --http2-prior-knowledge -X PUT http://localhost:3000/users/753951 -d '{"names":"Lyn Jeong"}'

            req.on_data([&res, userID](const uint8_t *data, std::size_t len) {
                std::string body(reinterpret_cast<const char *>(data), len);
                Json::CharReaderBuilder reader;
                Json::Value requestBody;
                std::string errors;
                std::istringstream iss(body);

                if (Json::parseFromStream(reader, iss, &requestBody, &errors)) {
                    if (!requestBody.isMember("name")) {
                        res.write_head(404);
                        res.end("{\"error\": \"Key not found\"}\n");
                    }

                    if (userData.find(userID) != userData.end()) {
                        userData[userID] = requestBody["name"].asString();

                        Json::Value responseJson;
                        responseJson["message"] = "User updated successfully";
                        responseJson["id"] = userID;
                        responseJson["name"] = userData[userID];

                        Json::StreamWriterBuilder writer;
                        std::string jsonResponse = Json::writeString(writer, responseJson) + "\n";

                        res.write_head(200);
                        res.end(jsonResponse);
                    } else {
                        res.write_head(404);
                        res.end("{\"error\": \"User not found\"}\n");
                    }
                } else {
                    res.write_head(400);
                    res.end(R"({"error": "Invalid JSON"})");
                }
            });

        } else if (req.method() == "DELETE") {
            // curl --http2-prior-knowledge -X DELETE http://localhost:3000/users/753951
            // curl --http2-prior-knowledge -X DELETE http://localhost:3000/users/000000

            if (userData.find(userID) != userData.end()) {
                std::string userName = userData[userID];

                if (userData.erase(userID) > 0) {
                    res.write_head(200);
                    res.end("{\"message\": \"User " + userName + " deleted successfully\"}\n");
                } else {
                    res.write_head(404);
                    res.end("{\"error\": \"Failed to delete " + userName + "\"}\n");
                }

            } else {
                res.write_head(404);
                res.end("{\"error\": \"User not found\"}\n");
            }
        } else {
            // curl --http2-prior-knowledge -X POST http://localhost:3000/users/753951 -d '{"id":789012, "name":"Geto"}'

            res.write_head(405);
            res.end("{\"error\": \"Method not allowed\"}\n");
        }
    });

    if (server.listen_and_serve(ec, "localhost", "3000")) {
        std::cerr << "error: " << ec.message() << std::endl;
    }
}
