#include <boost/asio.hpp>
#include <iostream>
#include "server.hpp"
#include "client.hpp"

int main(int argc, char* argv[]) {
    boost::asio::io_context io;
    std::string filepath = "";

     ssl::context ssl_ctx(ssl::context::tlsv12_client);
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_verify_mode(boost::asio::ssl::verify_none);

    if (argc < 2) {
        std::cout << "Usage: FTP.exe server|client\n";
        return 1;
    }

    std::string mode = argv[1];
    filepath = argv[2];

    if (mode == "server") {
        Server server(io, 9000);
        io.run();
    } 
    else if (mode == "client") {
        Client client(io, ssl_ctx,"127.0.0.1", "9000",filepath);
        io.run();
    }

    return 0;
}
