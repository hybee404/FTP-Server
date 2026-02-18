#include <iostream>
#include "server.hpp"
#include "serverSession.hpp"

Server::Server(boost::asio::io_context &io, short port)
    : acceptConnect(io, tcp::endpoint(tcp::v4(), port)), 
      ctx(boost::asio::ssl::context::tlsv12_server) 
{
   ctx.set_options(boost::asio::ssl::context::default_workarounds |
                    boost::asio::ssl::context::no_sslv2 |
                    boost::asio::ssl::context::single_dh_use);
    
    ctx.use_certificate_chain_file("server.crt");
    ctx.use_private_key_file("server.key", boost::asio::ssl::context::pem);
    std::cout << "[SERVER] Listening on port " << port << "...\n";
    accept_connection();
}


void Server::accept_connection(){
    acceptConnect.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket){
                if (!ec) {
                    std::cout << "New client: " << socket.remote_endpoint().address().to_string()
                     << ":" << socket.remote_endpoint().port() << "\n";
                    std::make_shared<Session>(std::move(socket), ctx)->start_session();
                } else {
                    std::cout << "Accept error: " << ec.message() << std::endl;
                }
         accept_connection();
        }
    );
}
