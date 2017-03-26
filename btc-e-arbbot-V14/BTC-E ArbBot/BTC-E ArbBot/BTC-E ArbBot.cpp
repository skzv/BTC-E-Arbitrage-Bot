// BTC-E ArbBot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "btc_api.hpp"
#include "BellmanFord.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include "getinfo.h"
#include <unordered_set>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include "split.h"


using namespace rapidjson;

int main()
{
	btc_api api("your_key", "your_secret");
	
	// Initialize currencies
	std::map<std::string, int> currency_map;
	// Initialize info
	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };
	json_data response = api.info();
	Document info;
	info.Parse(response.c_str());
	const Value& pairs = info["pairs"]; // Object
	std::string currency_pair;
	std::vector<std::string> cur_tokens;
	const char delim ='_';
	int count = 0;
	int num_pairs = 0;
	for (Value::ConstMemberIterator itr = pairs.MemberBegin(); itr != pairs.MemberEnd(); ++itr)
	{
		num_pairs++;
		currency_pair = itr->name.GetString();
		// add currencies
		cur_tokens = split(currency_pair, delim);
		for (auto i = cur_tokens.begin(); i != cur_tokens.end(); ++i) {
			if (!currency_map.count((*i))) {
				count++;
				currency_map.insert(std::pair<std::string, int>((*i), count));
			}
		}
	}

	std::cout << "Currency Map" << std::endl;
	for (auto i = currency_map.begin(); i != currency_map.end(); ++i) {
		std::cout << (*i).first << "\t" << (*i).second << std::endl;
	}

	///* Let us create the graph */
	int V = currency_map.size();  // Number of vertices in graph
	int E = num_pairs*2;  // Number of edges in graph, 2 because bi-directional
	struct Graph* graph = createGraph(V, E);
	int edge = 0;

	for (Value::ConstMemberIterator itr = pairs.MemberBegin(); itr != pairs.MemberEnd(); ++itr)
	{
		num_pairs++;
		bool first = true;
		currency_pair = itr->name.GetString();
		const Value& pair_value = pairs[currency_pair.c_str()];
		//get ticker
		json_data ticker = api.ticker(btc_e::pair(currency_pair));
		Document t;
		t.Parse(ticker.c_str());
		Value& ticker_v = t["ticker"];
		float sell = ticker_v["sell"].GetFloat();
		float buy = ticker_v["buy"].GetFloat();
		float fee = pair_value["fee"].GetFloat();

		// add currencies
		cur_tokens = split(currency_pair, delim);
		for (auto i = cur_tokens.begin(); i != cur_tokens.end(); ++i) {
			int cur_num = currency_map[(*i)];
			if (first) {
				// is src
				graph->edge[edge].src = cur_num;
				// set the buy price
				graph->edge[edge].weight = -log(sell);
				// set the sell price
				graph->edge[edge+1].dest = cur_num;
				graph->edge[edge+1].weight = -log(buy);
			}
			else {
				// is dest
				graph->edge[edge].dest = cur_num;
				graph->edge[edge+1].src = cur_num;
				edge=edge+2;
			}
			first = false;
		}
	}

	BellmanFord(graph, 0);
	getchar();
	return 0;
}

