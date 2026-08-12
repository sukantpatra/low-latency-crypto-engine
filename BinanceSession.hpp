#pragma once // Prevents the compiler from including this file twice

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <memory>
#include "Parser.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;


class BinanceSession : public std::enable_shared_from_this<BinanceSession> {
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string path_;

public:
    explicit BinanceSession(net::io_context& ioc, ssl::context& ctx);
    void run(const std::string& host, const std::string& port, const std::string& path);

private:
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_ws_handshake(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    Parser parser;

};