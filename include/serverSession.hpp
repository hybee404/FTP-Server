#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <boost/asio/ssl.hpp>


using boost::asio::ip::tcp;

namespace ssl = boost::asio::ssl; //for wrapping socket with tls


class Session : public std::enable_shared_from_this<Session> {
public:
    unsigned int session_count;
    explicit Session(tcp::socket socket, ssl::context& ctx);
    void start_session();

private:
    void receive_file();
    void on_transfer_complete(const std::string& temp_filename, const std::string& expected_hash, uint64_t received);
    ssl::stream<tcp::socket> socket_;
    std::array<char, 1024> data_;
    bool logged_in_ = false;
    std::string current_account_;
    std::string expected_hash_;
    std::string saved_file;
    bool account_verified_ = false;
    int session_id = 0;
     boost::asio::ssl::context ctx;
};
