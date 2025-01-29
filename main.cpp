#include <iostream>
#include <map>
#include <nghttp2/asio_http2_server.h>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

std::map<int, std::string> userData = {
    {1, "Lyn"},
    {2, "Gojo"}
};


int main() {
    boost::system::error_code ec;
    http2 server;

    server.handle("/", [](const request &req, const response &res) {
    std::cout << "" << req.method() << std::endl;

    res.write_head(200);
    res.end("This is Lyn's Example! Hello, World :>\n");
    });

    if (server.listen_and_serve(ec, "localhost", "3000")) {
    std::cerr << "error: " << ec.message() << std::endl;
    }
}
