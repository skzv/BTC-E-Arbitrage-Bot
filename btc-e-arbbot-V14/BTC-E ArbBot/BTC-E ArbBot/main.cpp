#include "btc_api.hpp"

int main(int argc, char **argv) {
    
    btc_api api("your_key", "your_secret");
    
    // getInfo method
    json_data response = api.get_info();
    std::clog << ">> getInfo: " <<response<< "\n\n";
    
    // Trade method, sell ltc_rur
    response = api.trade(btc_e::ltc_rur(), btc_e::type::sell, 224, 0.1);
    std::clog << "\n>> Trade: " << response << "\n\n";
    
    // Trade method, buy ltc_rur
    response = api.trade(btc_e::ltc_rur(), btc_e::type::buy, 220, 0.1);
    
    response = api.active_orders();
    std::clog << "\n>> Active orders: " << response << "\n";
    
    // public api: fee, trades, ticker, depth
    std::clog << "fee: "    << api.fee(btc_e::btc_usd()) << "\n";
    std::clog << "trades: " << api.trades(btc_e::btc_usd()) << "\n";
    std::clog << "ticker: " << api.ticker(btc_e::btc_usd()) << "\n";
    std::clog << "depth: "  << api.depth(btc_e::btc_usd()) << "\n";

    return 0;
}