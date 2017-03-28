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
#include <ctime>

using namespace rapidjson;

int main()
{
	btc_api api("your_key", "your_secret");
	// TODO: free unused memory
	// TODO: create bellmanford class
	// TODO: organize everything 
	// TODO: instantiate BTC-E stuff in own class
	// TODO: add trading for BTC-E
	// TODO: add more exchanges
	
	// TESTING VARIABLES
	// <time, <path, prices, profit>>
	std::vector<struct tm> times;
	std::vector<std::vector<int>> paths;
	std::vector<std::vector<double>> prices;
	std::vector<double> profits;

	int delay = 20;
	int refresh = 0;

	// Initialize currencies
	std::map<std::string, int> currency_map;
	std::map<int, std::string> reverse_currency_map;
	// Initialize info
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
	bool first = true;
	int opps = 0;
	int pricechanges = 0;
	int polls = 0;

	//get ticker	
	while (true) {
		polls++;
		//std::cout << "Polling" << std::endl;
		// TODO: FIX memory leaks
		json_data ticker;
		ticker = api.all_ticker(all_currency_pairs);
		Document ticker_d;
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
		if (first) {
			first = false;
			createGraph(l, V, e);
		}
		else
			updateGraph(l, V, e, pricechanges);
		profit = 1;
		//perform bf
		solve(e, V, E, src, path);
		if (path.size() > 0) {
			//std::cout << "Opportunity!" << std::endl;
			// get current time
			time_t t = time(0);   // get time now
			struct tm now;
			localtime_s(&now, &t);
			times.push_back(now);
			// push paths
			paths.push_back(path);
			opps++;
			for (size_t i = 0; i < path.size(); ++i) {
				//std::cout << reverse_currency_map[path[i]];
				//if (i < path.size() - 1)
				//	std::cout << " -> ";
			}
			// get prices
			std::vector<double> price;
			//std::cout << std::endl;
			for (size_t i = 1; i < path.size(); ++i) {
				price.push_back(w[path[i - 1]][path[i]]);
				profit *= w[path[i - 1]][path[i]];
				//std::cout << w[path[i-1]][path[i]];
				//if (i < path.size() - 1)
				//	std::cout << " * ";
			}
			prices.push_back(price);
			//std::cout << " = " << profit;
			//std::cout << std::endl << std::endl;
			path.clear();
		}
		profits.push_back(profit);
		//Sleep(1000);
		refresh++;

		if (refresh > delay) {
			refresh = 0;
			system("CLS");
			std::cout << "opportunities: " << opps << std::endl;
			// print all opportunities
			for (int j = 0; j < opps; j++) {
				// time
				std::cout << (times[j].tm_year + 1900) << '-'
					<< (times[j].tm_mon + 1) << '-'
					<< times[j].tm_mday << '-'
					<< times[j].tm_hour << '-'
					<< times[j].tm_min << '-'
					<< times[j].tm_sec;
				std::cout << "\t";
				// path
				for (size_t i = 0; i < paths[j].size(); ++i) {
					std::cout << reverse_currency_map[paths[j][i]];
					if (i < paths[j].size() - 1)
						std::cout << " -> ";
				}
				std::cout << "\t";
				// prices
				for (size_t i = 0; i < prices[j].size(); ++i) {
					std::cout << prices[j][i];
					if (i < prices[j].size() - 1)
						std::cout << " -> ";
				}
				std::cout << " = " << profits[j] << std::endl;
			}
			std::cout << "price changes: " << pricechanges << std::endl;
			std::cout << "polls: " << polls << std::endl;
		}
	}
	getchar();
	return 0;
}

