#include <boost/container/flat_map.hpp>
#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include "BinanceSession.hpp"
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <atomic>
#include "Metrics.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

struct OrderBookEntry 
    {
    double price;
    double quantity;
    };

boost::container::flat_map<double, double, std::greater<double>> bids; // Key: Price, Value: Quantity
boost::container::flat_map<double, double, std::less<double>> asks; // Key: Price, Value: Quantity

uint64_t currentUpdateId = 0;
std::atomic<uint64_t> total_latency_ns{0};
std::atomic<uint64_t> total_messages_processed{0};
std::atomic<uint64_t> total_orders_processed{0};

enum class OrderSide 
{
        BUY,
        SELL
};


template <typename T>
void ShowOrderBook(const T& book, const std::string& title) {
    std::cout << "=== " << title << " ===" << std::endl;
    for (const auto& entry : book) {
        std::cout << "Price: " << entry.first << ", Quantity: " << entry.second << std::endl;
    }
    std::cout << std::endl;
}

void UILoop() {
#ifdef __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0);
#endif

    while (true) {
        // Sleep for exactly 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Atomically grab the current values and reset them to 0 instantly
        //uint64_t msgs = total_messages_processed.exchange(0, std::memory_order_relaxed);
        uint64_t total_ns = total_latency_ns.exchange(0, std::memory_order_relaxed);
        uint64_t orders = total_orders_processed.exchange(0, std::memory_order_relaxed);

        //uint64_t avg_latency = total_ns / msgs;
        uint64_t avg_order_latency = total_ns / orders;

        std::cout << "--1s Window-- \n Order Processed: " << orders << " Average Order Latency: " << avg_order_latency << " ns\n";
    }
}

void UpdateBook(OrderSide side, double price, double qty)
{
    switch (side) {
        case OrderSide::BUY:
            bids[price] = qty;
            break;
        case OrderSide::SELL:
            asks[price] = qty;
            break;
        default:
            std::cerr << "Invalid side: " << (side == OrderSide::BUY ? "BUY" : "SELL") << std::endl;
    }
}

void FetchBinanceSnapshot(net::io_context& ioc, ssl::context& ctx) {
    try {
        net::ip::tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        // SNI is strictly required by Binance to establish a secure SSL connection
        if(!SSL_set_tlsext_host_name(stream.native_handle(), "api.binance.com")) {
            std::cerr << "Failed to set SNI Hostname\n";
            return;
        }

        // 1. Resolve DNS and Connect
        auto const results = resolver.resolve("api.binance.com", "443");
        beast::get_lowest_layer(stream).connect(results);
        stream.handshake(ssl::stream_base::client);

        // 2. Build the HTTP GET Request (limit=1000 gives us the deepest book possible)
        http::request<http::string_body> req{http::verb::get, "/api/v3/depth?symbol=BTCUSDT&limit=1000", 11};
        req.set(http::field::host, "api.binance.com");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // 3. Send Request and Read Response
        http::write(stream, req);
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        // 4. Pass the JSON body to a temporary Parser
        Parser snapshotParser;
        snapshotParser.ParseSnapshotJson(res.body());

        // 5. Gracefully close the HTTPS connection
        beast::error_code ec;
        stream.shutdown(ec);
        
    } catch(std::exception const& e) {
        std::cerr << "Error fetching snapshot: " << e.what() << std::endl;
    }
}


int main()
{
    // 1. Create the I/O context (The Event Loop)
    boost::asio::io_context ioc;

    // 2. Create the SSL context for WSS (Secure WebSockets)
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    
    FetchBinanceSnapshot(ioc, ctx); // Fetch the initial order book snapshot before starting the WebSocket session

    std::thread ui_thread(UILoop);
    ui_thread.detach();
    
    // 3. Connection Details for Binance L2 Stream
    std::string host = "stream.binance.com";
    std::string port = "9443";
    std::string path = "/ws/btcusdt@depth";


    // 4. Launch the connection!
    std::make_shared<BinanceSession>(ioc, ctx)->run(host, port, path);

    // 5. Run the I/O service. This blocks forever, listening for network events!
    ioc.run();

    return 0;
}