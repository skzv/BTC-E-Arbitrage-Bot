#pragma once
#ifndef GETINFO_HPP
#define GETINFO_HPP

#include <string>
#include "http/connection.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using url = std::string;
using json_data = std::string;
using namespace rapidjson;

namespace btc_e {

	class info {
	public:
		info() {
			_parsed_data.Parse(_json_data.c_str());
		}
		json_data& get_json_data() { return _json_data; }
		Document& get_parsed_data() { return _parsed_data; }
	private:
		json_data _json_data;
		Document _parsed_data;
	};

} // ns btc_e

#endif