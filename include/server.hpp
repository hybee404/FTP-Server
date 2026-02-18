#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, short port); 
    
private:
    void accept_connection(); 
    boost::asio::ssl::context ctx;
    tcp::acceptor acceptConnect;
};


