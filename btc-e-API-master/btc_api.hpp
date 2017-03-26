/**
 * 
 * C++11 library for btc-e.com API
 * https://btc-e.com/tapi/docs
 * 
 * Version: 1.0
 * Compiler: g++
 * Dependencies: CURL, OpenSSL
 * Author: hlc
 * 
 **/

#ifndef BTC_API_HPP
#define BTC_API_HPP

#include "http/connection.hpp"
#include "hmac_sha512.hpp"
#include "pairs.hpp"
#include <ctime>
#include <cassert>
#include <unistd.h>

template <typename... Args>
std::string params(const std::string& str, Args... args) {
    std::string res = str;
    res += "&";
    res += params(args...);
    return res;
}

template <>
std::string params(const std::string& str) {
    return str;
}

class btc_api: public uncopyable {
public:
    btc_api(const std::string& key, const std::string& secret)
        : nonce_(0), key_(key), secret_(secret), connection_("https://btc-e.com/tapi")
    {}
    
    json_data call(const std::string& method, const std::string& p) {
        std::string params = "nonce=";
        nonce_ = ::time(nullptr);
        params.append(std::to_string(nonce_));
        params.append("&method=" + method);
        if (p.size() != 0) {
            params.append("&");
        }
        params.append(p);
        
        std::map<std::string, std::string> headers;
        headers["Content-type"] = "application/x-www-form-urlencoded";
        headers["Key"] = key_;
        headers["Sign"] = this->signature(params);
        
        connection_.request(http::post(), params, headers);
        return connection_.get_response();
    }
    
    /**
     * @brief returns current balance information
     * @example : std::string response = btc_api_object.get_info();
     **/
    json_data get_info() {
        return this->call("getInfo", "");
    }
    
    /**
     * @brief returns trade trade_history
     * Information about parameters: https://btc-e.com/tapi/docs#TradeHistory
     * @param   : specify params via params() function, @e params("param0=value0", "param1=value1", ...);
     * @example : std::string response = btc_api_object.trade_history(params("from=0", "count=10"));
     **/
    json_data trade_history(const std::string& params = "") {
        return this->call("TradeHistory", params);
    }
    
    /**
     * @brief returns transaction trade_history
     * Information about parameters: https://btc-e.com/tapi/docs#TransHistory
     * @param   : specify params via params() function, @e params("param0=value0", "param1=value1", ...);
     * @example : std::string response = btc_api_object.trans_history(params("from=0", "count=10"));
     **/
    json_data trans_history(const std::string& params = "") {
        return this->call("TransHistory", params);
    }
    
    json_data trade(const btc_e::pair& pair, btc_e::type type, unsigned int rate, double amount) {
        assert(rate >= 0);
        assert(amount >= 0);
        
        std::string params = "pair=";
        params += pair.name();
        params += "&type=";
        params += (type == btc_e::type::buy ? "buy" : "sell");
        params += "&rate=";
        params += std::to_string(rate);
        params += "&amount=";
        params += std::to_string(amount);
        
        return this->call("Trade", params);
    }
    
    /**
     * @param pair - [btc_e::pair] @optional
     * @returns list your active orders
     * @example std::string response = btc_api_object.active_orders()
     * @example std::string response = btc_api_object.active_orders(btc_e::pair::btc_usd())
     **/
    json_data active_orders() {
        return this->call("ActiveOrders", "");
    }
    
    json_data active_orders(const btc_e::pair& pair) {
        return this->call("ActiveOrders", "pair=" + pair.name());
    }
    
    /**
     * @param order - [numeric] order's identifier
     * @example : std::string response = btc_api_object.order_info("782734")
     **/
    json_data order_info(const std::string& order) {
        return this->call("OrderInfo", "order_id=" + order);
    }
    
    /**
     * @param order_id - [numeric]
     * @example : std::string response = btc_api_object.cancel_order("782734")
     **/
    json_data cancel_order(const std::string& order_id) {
        return this->call("CancelOrder", "order_id=" + order_id);
    }
    
    // public api
    /**
     * @example std::string response = btc_api_object.fee(btc_e::btc_usd());
     **/
    json_data fee(const btc_e::pair& p) {
        http::connection c(p.fee());
        c.request(http::post());
        return c.get_response();
    }
    
    /**
     * @example : std::string response = btc_api_object.ticker(btc_e::btc_usd());
     **/
    json_data ticker(const btc_e::pair& p) {
        http::connection c(p.ticker());
        c.request(http::post());
        return c.get_response();
    }
    
    /**
     * @example : std::string response = btc_api_object.trades(btc_e::btc_usd());
     **/
    json_data trades(const btc_e::pair& p) {
        http::connection c(p.trades());
        c.request(http::post());
        return c.get_response();
    }
    
    /**
     * @example : std::string response = btc_api_object.depth(btc_e::btc_usd());
     **/
    json_data depth(const btc_e::pair& p) {
        http::connection c(p.depth());
        c.request(http::post());
        return c.get_response();
    }
    
private:
    std::string signature(const std::string& params) {
        HMAC_SHA512 hmac_sha512(secret_, params);
        return hmac_sha512.hex_digest();
    }
    
private:
    unsigned long nonce_;
    std::string key_;
    std::string secret_;
    
    http::connection connection_;
};

#endif