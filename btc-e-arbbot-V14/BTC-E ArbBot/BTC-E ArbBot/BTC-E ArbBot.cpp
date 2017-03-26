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
	for (Value::ConstMemberIterator itr = pairs.MemberBegin(); itr != pairs.MemberEnd(); ++itr)
	{
		num_pairs++;
		currency_pair = itr->name.GetString();
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
	
	while (true) {
		std::cout << "Polling" << std::endl;
		e.clear();
		for (Value::ConstMemberIterator itr = pairs.MemberBegin(); itr != pairs.MemberEnd(); ++itr)
		{
			currency_pair = itr->name.GetString();
			const Value& pair_value = pairs[currency_pair.c_str()];
			//get ticker
			json_data ticker = api.ticker(btc_e::pair(currency_pair));
			Document t;
			t.Parse(ticker.c_str());
			Value& ticker_v = t["ticker"];
			double sell = ticker_v["sell"].GetDouble();
			double buy = ticker_v["buy"].GetDouble();
			double fee = pair_value["fee"].GetDouble();

			// add currencies
			cur_tokens = split(currency_pair, delim);
			std::string cur1 = cur_tokens[0];
			std::string cur2 = cur_tokens[1];
			int cur1_num = currency_map[cur1];
			int cur2_num = currency_map[cur2];

			w[cur1_num][cur2_num] = (sell*(1 - fee / 100));
			w[cur2_num][cur1_num] = 1 / (buy*(1 - fee / 100));
			// transform
			l[cur1_num][cur2_num] = -log(w[cur1_num][cur2_num]);
			l[cur2_num][cur1_num] = -log(w[cur2_num][cur1_num]);
			//printf("cur1: %d cur2: %d sell:%f buy:%f lns:%f lnb:%f\n", cur1_num, cur2_num, sell, buy, -log(sell*(1 - fee / 100)), -log(buy*(1 - fee / 100)));
		}
		e = createGraph(l, V);
		//perform bf
		int src = 0;
		double profit = 1;
		std::vector<int> path = solve(e, V, E, src);
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
		//// Calculate USD->LTC-BTC-USD
		//float ulbu = w[1][4] * w[4][0] * w[0][1];
		////if (ulbu > 1) {
		//	printf("\nUSD->LTC->BTC->USD\n");
		//	printf("%f * %f * %f = %f\n", w[1][4], w[4][0], w[0][1], ulbu);
		//	float lulbu = l[1][4] + l[4][0] + l[0][1];
		//	float llulbu = -log(ulbu);
		//	printf("ulbu: %f lulbu: %f lllbu: %f\n", ulbu, lulbu, llulbu);
		////}
		//int src = 0;
		//printf("starting at %d\n", src);

		////perform bellman ford
		//double* dis = new double[V];
		//int* pre = new int[V];

		//for (int i = 0; i < V; ++i)
		//	dis[i] = DBL_MAX, pre[i] = -1;

		//dis[src] = 0;

		//for (int k = 0; k < V; ++k)
		//	for (int i = 0; i < V; ++i)
		//		for (int j = 0; j < V; ++j)
		//			if (dis[i] + l[i][j] < dis[j])
		//				dis[j] = dis[i] + l[i][j], pre[j] = i;

		////for (int k = 0; k < V; ++k)
		////	for (int i = 0; i < V; ++i)
		////		if(w[k][i]!=DBL_MAX)
		////			printf("w[%d][%d]=>%f\n",k,i,w[k][i]);

		//for (int k = 0; k < V; ++k)
		//	printf("j=> %d\t dist=> %f\n", k, dis[k]);

		//// check for negative cycles
		//for (int i = 0; i < V; ++i)
		//	for (int j = 0; j < V; ++j)
		//		if (dis[i] + l[i][j] < dis[j]) {
		//			//printf("please dear jesus \n");
		//			// Node j is part of a negative cycle
		//			printf("Node %d is part of a negative cycle\n", j);
		//		}

		////You can then use the pre array to find the negative cycles.
		//// Start with pre[source] and work your way back.

		//for (int k = 0; k < V; ++k)
		//	printf("pre[%d]=>%d\n", k, pre[k]);

		// start at src
		Sleep(1 * 1000);
	}

	getchar();
	return 0;
}

