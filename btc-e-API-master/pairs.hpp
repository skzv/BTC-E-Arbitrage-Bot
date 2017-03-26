#ifndef PAIRS_HPP
#define PAIRS_HPP

#include <string>

using url = std::string;

namespace btc_e {

class pair {
public:
    pair() {}
    pair(const url& u): _url(_base_url + u), name_(u) {}
    virtual ~pair() {}
    
    virtual url fee() const {
        return _url + "/fee";
    }
    virtual url ticker() const {
        return _url + "/ticker";
    }
    
    virtual url trades() const {
        return _url + "/trades";
    }
    
    virtual url depth() const {
        return _url + "/depth";
    }
    
    virtual std::string name() const {
        return name_;
    }
    
private:
    static url _base_url;
    url _url;
    std::string name_;
};
url pair::_base_url = "https://btc-e.com/api/2/";

class btc_usd: public pair {
public:
    btc_usd(): pair("btc_usd") {}
};

class btc_eur: public pair {
public:
    btc_eur(): pair("btc_eur") {}
};

class btc_rur: public pair {
public:
    btc_rur(): pair("btc_rur") {}
};

class ltc_btc: public pair {
public:
    ltc_btc(): pair("ltc_btc") {}
};

class ltc_usd: public pair {
public:
    ltc_usd(): pair("ltc_usd") {}
};

class ltc_rur: public pair {
public:
    ltc_rur(): pair("ltc_rur") {}
};

class nmc_btc: public pair {
public:
    nmc_btc(): pair("nmc_btc") {}
};

class usd_rur: public pair {
public:
    usd_rur(): pair("usd_rur") {}
};

class eur_usd: public pair {
public:
    eur_usd(): pair("eur_usd") {}
};

enum class type {
    sell,
    buy
};

} // ns btc_e

#endif