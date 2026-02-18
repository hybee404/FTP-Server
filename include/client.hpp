#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

class Client {
public:
    Client(boost::asio::io_context& io_context,ssl::context &ssl_ctx, const std::string& host, const std::string& port, const std::string &filepath);
    void connect(const std::string& host, const std::string& port, const std::string &filepath);

    //function to send file from client to server;
    void send_file(std::string& path);

    
   void handle_error(const boost::system::error_code& ec);

    void close();
    
private:
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> socket_;
    // tcp::socket socket_;
};
