// BTC-E ArbBot.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "btc_api.hpp"
#include "BellmanFord.h"
#include "bellmanford2.h"
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
#include <limits>


using namespace rapidjson;

int main()
{
	btc_api api("your_key", "your_secret");
	
	// Initialize currencies
	std::map<std::string, int> currency_map;
	std::map<int, std::string> reverse_currency_map;
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
	int src = 0;
	double profit = 1;
	double fee = 0.2; // get this from json later
	std::vector<int> path;
	std::vector<btc_e::pair> all_currency_pairs;
	for (Value::ConstMemberIterator itr = pairs.MemberBegin(); itr != pairs.MemberEnd(); ++itr)
	{
		num_pairs++;
		currency_pair = itr->name.GetString();
		all_currency_pairs.push_back(btc_e::pair(currency_pair));
		// add currencies
		cur_tokens = split(currency_pair, delim);
		for (auto i = cur_tokens.begin(); i != cur_tokens.end(); ++i) {
			if (!currency_map.count((*i))) {
				currency_map.insert(std::pair<std::string, int>((*i), count));
				reverse_currency_map.insert(std::pair<int, std::string>(count,(*i)));
				count++;
			}
		}
	}

	std::cout << "Currency Map" << std::endl;
	for (auto i = currency_map.begin(); i != currency_map.end(); ++i) {
		std::cout << (*i).first << "\t" << (*i).second << std::endl;
	}

	///* Let us create the graph */
	int V = currency_map.size();  // Number of vertices in graph
	int E = num_pairs;
	double** w = new double*[V];
	for (int i = 0; i < V; ++i)
		w[i] = new double[V];

	//initialize to inf
	for (int k = 0; k < V; ++k)
		for (int i = 0; i < V; ++i)
			w[k][i] = DBL_MAX;

	double** l = new double*[V];
	for (int i = 0; i < V; ++i)
		l[i] = new double[V];

	//initialize to inf
	for (int k = 0; k < V; ++k)
		for (int i = 0; i < V; ++i)
			l[k][i] = DBL_MAX;

	std::vector<edge> e;
	//get ticker
	json_data ticker;
	Document ticker_d;
	
	while (true) {
		std::cout << "Polling" << std::endl;
		// TODO: FIX memory leaks
		ticker = api.all_ticker(all_currency_pairs);
		ticker_d.Parse(ticker.c_str());
		for (Value::ConstMemberIterator itr = ticker_d.MemberBegin(); itr != ticker_d.MemberEnd(); ++itr)
		{
			currency_pair = itr->name.GetString();
			Value& ticker_v = ticker_d[currency_pair.c_str()];
			double sell = ticker_v["sell"].GetDouble();
			double buy = ticker_v["buy"].GetDouble();

			// add currencies
			cur_tokens = split(currency_pair, delim);
			std::string cur1 = cur_tokens[0];
			std::string cur2 = cur_tokens[1];
			int cur1_num = currency_map[cur1];
			int cur2_num = currency_map[cur2];

			w[cur1_num][cur2_num] = (sell*(1 - fee / 100));
			w[cur2_num][cur1_num] = 1 / (buy)*(1 - fee / 100);
			// transform
			l[cur1_num][cur2_num] = -log(w[cur1_num][cur2_num]);
			l[cur2_num][cur1_num] = -log(w[cur2_num][cur1_num]);
			//printf("cur1: %d cur2: %d sell:%f buy:%f lns:%f lnb:%f\n", cur1_num, cur2_num, sell, buy, -log(sell*(1 - fee / 100)), -log(buy*(1 - fee / 100)));
		}
		destroyeGraph(e);
		createGraph(l, V, e);
		profit = 1;
		//perform bf
		solve(e, V, E, src, path);
		if (path.size() > 0) {
			std::cout << "Opportunity!" << std::endl;
			for (size_t i = 0; i < path.size(); ++i) {
				std::cout << reverse_currency_map[path[i]];
				if (i < path.size() - 1)
					std::cout << " -> ";
			}
			std::cout << std::endl;
			for (size_t i = 1; i < path.size(); ++i) {
				profit *= w[path[i - 1]][path[i]];
				std::cout << w[path[i-1]][path[i]];
				if (i < path.size() - 1)
					std::cout << " * ";
			}
			std::cout << " = " << profit;
			std::cout << std::endl << std::endl;
		}
		Sleep(1000);
	}

	getchar();
	return 0;
}

