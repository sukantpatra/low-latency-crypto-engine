#include "BinanceSession.hpp"
#include <iostream>
#include <boost/asio/connect.hpp>
#include "simdjson.h"
#include <chrono>
#include "Metrics.hpp"

// Constructor
BinanceSession::BinanceSession(net::io_context& ioc, ssl::context& ctx)
    : resolver_(boost::asio::make_strand(ioc)), ws_(boost::asio::make_strand(ioc), ctx) {}

void BinanceSession::run(const std::string& host, const std::string& port, const std::string& path) {
    host_ = host;
    path_ = path;
    resolver_.async_resolve(host, port,
        beast::bind_front_handler(&BinanceSession::on_resolve, shared_from_this()));
}

void BinanceSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        std::cerr << "Resolve Error: " << ec.message() << "\n";
        return;
    }
    net::async_connect(beast::get_lowest_layer(ws_), results,
        beast::bind_front_handler(&BinanceSession::on_connect, shared_from_this()));
}

void BinanceSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
    if (ec) {
        std::cerr << "Connect Error: " << ec.message() << "\n";
        return;
    }
    
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        std::cerr << "SNI Error: " << ec.message() << "\n";
        return;
    }

    ws_.next_layer().async_handshake(ssl::stream_base::client,
        beast::bind_front_handler(&BinanceSession::on_ssl_handshake, shared_from_this()));
}

void BinanceSession::on_ssl_handshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "SSL Handshake Error: " << ec.message() << "\n";
        return;
    }
    ws_.async_handshake(host_, path_,
        beast::bind_front_handler(&BinanceSession::on_ws_handshake, shared_from_this()));
}

void BinanceSession::on_ws_handshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "WS Handshake Error: " << ec.message() << "\n";
        return;
    }
    do_read();
}

void BinanceSession::do_read() {
    ws_.async_read(buffer_,
        beast::bind_front_handler(&BinanceSession::on_read, shared_from_this()));
}

void BinanceSession::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        std::cerr << "Read Error: " << ec.message() << "\n";
        return;
    }

    buffer_.prepare(simdjson::SIMDJSON_PADDING);

    const char* rawData = static_cast<const char *>(buffer_.data().data());
    size_t length = buffer_.size();
    size_t capacity = length + simdjson::SIMDJSON_PADDING;
    // --- HOT PATH MEASUREMENT START ---
    auto start_time = std::chrono::high_resolution_clock::now();

    uint64_t orderCount = parser.ParseJson(rawData, length, capacity); 
    
    auto end_time = std::chrono::high_resolution_clock::now();
    // --- HOT PATH MEASUREMENT END ---
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();

    // LOCK-FREE METRICS UPDATE
    total_latency_ns.fetch_add(latency, std::memory_order_relaxed);
    //total_messages_processed.fetch_add(1, std::memory_order_relaxed);
    total_orders_processed.fetch_add(orderCount, std::memory_order_relaxed);

    buffer_.consume(length); // Clear the buffer after processing
    do_read(); 
}