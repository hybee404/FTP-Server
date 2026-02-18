#include "client.hpp"
#include <iostream>
#include <fstream>
#include <openssl/sha.h>
#include "checksum.hpp"

constexpr std::size_t CHUNK_SIZE = 65536; // for 64 KB

Client::Client(boost::asio::io_context &io_context, ssl::context &ssl_ctx, const std::string &host, const std::string &port, const std::string &filepath)
    : resolver_(io_context), socket_(io_context, ssl_ctx)
{
    connect(host, port, filepath);
}

// Server response to a successful connection
void Client::connect(const std::string &host, const std::string &port, const std::string &filepath)
{
    resolver_.async_resolve(host, port,
                            [this, filepath](const boost::system::error_code &ec, tcp::resolver::results_type endpoints)
                            {
                                if (!ec)
                                {
                                    boost::asio::async_connect(socket_.lowest_layer(), endpoints,
                                                               [this, filepath](const boost::system::error_code &ec, const tcp::endpoint &endpoint)
                                                               {
                                                                   if (!ec)
                                                                   {
                                                                       // Start SSL handshake
                                                                       socket_.async_handshake(boost::asio::ssl::stream_base::client,
                                                                                               [this, filepath](const boost::system::error_code &ec)
                                                                                               {
                                                                                                   if (!ec)
                                                                                                   {
                                                                                                       std::cout << "Securely connected!\n";
                                                                                                       std::string path_to_send = filepath;
                                                                                                       this->send_file(path_to_send);
                                                                                                   }
                                                                                                   else
                                                                                                   {
                                                                                                       std::cerr << "[ERROR] SSL Handshake failed: " << ec.message() << "\n";
                                                                                                       handle_error(ec);
                                                                                                   }
                                                                                               });
                                                                   }
                                                                   else
                                                                   {
                                                                       std::cerr << "[ERROR] Connection failed: " << ec.message() << "\n";
                                                                       handle_error(ec);
                                                                   }
                                                               });
                                }
                                else
                                {
                                    std::cerr << "[ERROR] Resolve failed: " << ec.message() << "\n";
                                }
                            });
}

void Client::handle_error(const boost::system::error_code &ec)
{
    std::cerr << "Closing connection due to error: " << ec.message() << std::endl;

    // Shutdown the socket so the io_context.run() can eventually finish
    boost::system::error_code ignored_ec;
    socket_.lowest_layer().close(ignored_ec);
}

void Client::send_file(std::string &path)
{   
    std::ifstream file(path, std::ios::binary | std::ios::ate); // ios binary helps read whatever format
    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return;
    }

    std::string file_hash = Checksum::calculate_sha256(path, true);
    std::cout << "[CLIENT] File Hash: " << file_hash << "\n";
   
    size_t pos = path.find_last_of("/\\");
    std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    uint32_t name_len = filename.size();
    // writing into socket
    boost::asio::write(socket_, boost::asio::buffer(&name_len, sizeof(name_len)));
    boost::asio::write(socket_, boost::asio::buffer(filename));
     boost::asio::write(socket_, boost::asio::buffer(file_hash));;
    uint64_t offset;
    boost::asio::read(socket_, boost::asio::buffer(&offset, sizeof(offset))); // receive offset from server(i.e last reading point)
    std::cout << "Resuming from offset: " << offset << "\n";

    file.seekg(0, std::ios::end);
    uint64_t file_size = file.tellg();

    uint64_t resume_offset = offset;          // check where connection breaks
    file.seekg(resume_offset, std::ios::beg); // for resumption

    boost::asio::write(socket_, boost::asio::buffer(&file_size, sizeof(file_size)));

    std::vector<char> buffer(CHUNK_SIZE);
    auto start_time = std::chrono::steady_clock::now();
uint64_t sent = resume_offset;
uint64_t last_sent = sent;
auto last_time = start_time;
 uint64_t received = offset;

    while (true)
    {
        file.read(buffer.data(), buffer.size());
        std::streamsize bytes_read = file.gcount();

        
        if (bytes_read <= 0)
            break;

           
        
        boost::asio::write(socket_, boost::asio::buffer(buffer.data(), bytes_read));
            received += bytes_read;
            sent = received;
               int progress = static_cast<int>(
        (static_cast<double>(received) / file_size) * 100
    );

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
        
    if (elapsed.count() >= 10) {
        double speed_MBps =
            ((sent - last_sent) / 1024.0 / 1024.0) /
            (elapsed.count() / 1000.0);

        std::cout << "\r[SENDING] "
                  << progress << "% | "
                  << speed_MBps << " MB/s"
                  << std::flush;
      

        last_sent = sent;
        last_time = now;
    }
    }
     auto end_time = std::chrono::steady_clock::now();
double total_seconds =
    std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

double avg_speed =
    (file_size / 1024.0 / 1024.0) / total_seconds;

    if(total_seconds != 0){

std::cout << "\n[SUCCESS] Transfer complete in "
          << total_seconds << " s (avg "
          << avg_speed << " MB/s)\n";

    std::cout << "File sent in 64KB chunks\n";
    }else{
        std::cout <<"\n File exist";
    }
}
