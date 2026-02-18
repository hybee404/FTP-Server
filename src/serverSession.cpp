#include <iostream>
#include <string>
#include <fstream>
#include <cstdio> // Required for remove()
#include "serverSession.hpp"
#include "checksum.hpp"

constexpr std::size_t CHUNK_SIZE = 65536; //for 64 KB


Session::Session(tcp::socket socket, ssl::context& ctx)
    : socket_(std::move(socket), ctx),
     ctx(boost::asio::ssl::context::tls_server)
{
    std::cout << "SESSION[" << ++session_id << "] created" << std::endl;
}

void Session::start_session() {
// Start reading user input

auto self(shared_from_this()); // Ensure session stays alive during async ops
    
    // Use the async version of handshake for a non-blocking server
    socket_.async_handshake(boost::asio::ssl::stream_base::server,
        [this, self](const boost::system::error_code& error) {
            if (!error) {
                std::cout << "[SESSION] SSL handshake successful\n";
                receive_file(); 
            } else {
                std::cerr << "[SESSION] Handshake failed: " << error.message() << "\n";
            }
        });
}


void Session::receive_file() {

    try{
    uint32_t name_len;
    boost::asio::read(socket_, boost::asio::buffer(&name_len, sizeof(name_len)));

    std::string filename(name_len, '\0');
    boost::asio::read(socket_, boost::asio::buffer(filename));

    std::string file_hash(64, '\0'); 
    boost::asio::read(socket_, boost::asio::buffer(&file_hash[0], file_hash.size()));

    saved_file = "received_" + filename;
     uint64_t offset = 0;

{
    std::ifstream in(saved_file, std::ios::binary | std::ios::ate); //read size of received
    if (in) {
        offset = in.tellg();
    }
}

boost::asio::write(socket_, boost::asio::buffer(&offset, sizeof(offset)));

    std::ofstream out(saved_file, std::ios::binary | std::ios::app);

    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t received = offset;
    uint64_t file_size;
    boost::asio::read(socket_, boost::asio::buffer(&file_size, sizeof(file_size)));

if (offset > file_size) {
    throw std::runtime_error("Invalid resume offset");
}
     int last_progress = -1; 
     auto start_time = std::chrono::steady_clock::now();
uint64_t last_bytes = received;
auto last_time = start_time;

    while (received < file_size) {
        std::size_t to_read =
            std::min<uint64_t>(CHUNK_SIZE, file_size - received);

        std::size_t bytes_read =
            boost::asio::read(socket_, boost::asio::buffer(buffer.data(), to_read));

        out.write(buffer.data(), bytes_read);
        received += bytes_read;

          int progress = static_cast<int>(
        (static_cast<double>(received) / file_size) * 100
    );

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);

    if (elapsed.count() >= 500) {
        double seconds = elapsed.count() / 1000.0;
        double speed_MBps = ((received - last_bytes) / 1024.0 / 1024.0) / seconds;

        std::cout << "\r[RECEIVING] "
                  << progress << "% | "
                  << speed_MBps << " MB/s | "
                  << received / 1024 / 1024 << " / "
                  << file_size / 1024 / 1024 << " MB"
                  << std::flush;

        last_bytes = received;
        last_time = now;
    }
          
    }
   on_transfer_complete(saved_file,file_hash,received);
   auto end_time = std::chrono::steady_clock::now();
double total_seconds =
    std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

double avg_speed =
    (file_size / 1024.0 / 1024.0) / total_seconds;

std::cout << "\n[SUCCESS] Transfer complete in "
          << total_seconds << " s (avg "
          << avg_speed << " MB/s)\n";
    std::cout << "File received in 64KB chunks\n";

}catch (const std::exception& e) {
        std::cout << "Connection dropped. Resume possible.\n";
    }
}


// After the file is fully written. Check hash and if it does not match, file is deleted
void Session::on_transfer_complete(const std::string& temp_filename, const std::string& expected_hash, uint64_t received) {
      std::cout << (received)/1000000<<"MB received\n";
    std::cout << "[SERVER] Transfer finished. Verifying integrity...\n";

    //Calculate and verify hash the hash of what we actually received
    std::string actual_hash = Checksum::calculate_sha256(temp_filename, true);

    if (actual_hash == expected_hash) {
        std::cout << "[SUCCESS] Hashes match! Finalizing file...\n";
    } else {
        if (std::remove(temp_filename.c_str()) == 0) {
    std::cout << "[SERVER] Corrupt file deleted successfully.\n";
} else {
    std::perror("[SERVER] Error deleting corrupt file");
}
    }
}
