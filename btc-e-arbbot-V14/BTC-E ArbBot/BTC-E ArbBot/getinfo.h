#pragma once
#ifndef GETINFO_HPP
#define GETINFO_HPP

#include <string>
#include "http/connection.hpp"

using url = std::string;
using json_data = std::string;

namespace btc_e {

	class info {
	public:
		info() {
			http::connection c(geturl());
			c.request(http::post());
			_data = c.get_response();
		}
		json_data getdata() { return _data; }
		url geturl() { return _url; }
	private:
		json_data _data;
		const url _url = "https://btc-e.com/api/3/info";
	};

} // ns btc_e

#endif