#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
using namespace std;

bool helper_sort(pair<string, double> p1, pair<string, double> p2)
{
	return (p1.second > p2.second);
}

bool helper_unique(pair<string, double> p1, pair<string, double> p2)
{
	return (p1.first == p2.first);
}

struct hyperlink
{
	string src, des;
};

class network
{
private:
	vector<hyperlink> hyperlinks;
	map<string, vector<string>> pages_keywords;
	map<string, int> pages_impressions;
	map<string, double> pages_rank;
	map<string, double> pages_CTR;
	map<string, double> pages_score;
	map<string, list<string>> adj_list;
	map<string, list<string>> adj_list_rev;
	map<int, string> results_dis;
	set<string> pages;
	int user_choice;
public:
	network();
	void initiallize_data();
	void page_score_calc();
	void search(char[]);
	void launch();
	void new_search();
	void open_webpage();
	void export_data();
};

network::network()
{
	initiallize_data();
	page_score_calc();
	launch();
}

void network::initiallize_data()
{
	ifstream links("Network Edges.csv");
	string temp_str, part_str;
	vector<string> temp_str_vec;
	hyperlink temp_hyp;
	while (links >> temp_str)
	{
		stringstream temp_stream(temp_str);
		while (getline(temp_stream, part_str, ','))
			temp_str_vec.push_back(part_str);
		temp_hyp.src = temp_str_vec[0];
		temp_hyp.des = temp_str_vec[1];
		hyperlinks.push_back(temp_hyp);
		temp_str_vec.clear();
	}
	for (const auto i : hyperlinks)
	{
		pages.insert(i.src);
		pages.insert(i.des);
	}
	for (const auto i : hyperlinks)
	{
		adj_list[i.src].push_back(i.des);
		adj_list_rev[i.des].push_back(i.src);
	}
	ifstream keywords("Keywords.csv");
	string temp_str_keywords, part_str_keywords;
	vector<string> temp_str_vec_keywords, temp_insert_vec_keywords;
	char* pt_check;
	while (getline(keywords, temp_str_keywords))
	{
		stringstream temp_stream_keywords(temp_str_keywords);
		while (getline(temp_stream_keywords, part_str_keywords, ','))
		{
			temp_str_vec_keywords.push_back(part_str_keywords);
			if (part_str_keywords.find(" ") != string::npos)
			{
				char* check_chr = const_cast<char*>(part_str_keywords.c_str());
				char* tokens = strtok_s(check_chr, " ", &pt_check);
				while (tokens != NULL)
				{
					temp_str_vec_keywords.push_back(tokens);
					tokens = strtok_s(NULL, " ", &pt_check);
				}
			}
		}
		for (int i = 1; i < temp_str_vec_keywords.size(); i++)
		{
			
			temp_insert_vec_keywords.push_back(temp_str_vec_keywords[i]);
		}
		pages_keywords.insert(make_pair(temp_str_vec_keywords[0], temp_insert_vec_keywords));
		temp_str_vec_keywords.clear();
		temp_insert_vec_keywords.clear();
	}
	ifstream impressions("Impressions.csv");
	string temp_str_imps, part_str_imps;
	vector<string> temp_str_vec_imps;
	while (impressions >> temp_str_imps)
	{
		stringstream temp_stream_imps(temp_str_imps);
		while (getline(temp_stream_imps, part_str_imps, ','))
			temp_str_vec_imps.push_back(part_str_imps);
		pages_impressions.insert(make_pair(temp_str_vec_imps[0], stoi(temp_str_vec_imps[1])));
		temp_str_vec_imps.clear();
	}
	ifstream CTR("CTR.csv");
	string temp_str_ctr, part_str_ctr;
	vector<string> temp_str_vec_ctr;
	while (CTR >> temp_str_ctr)
	{
		stringstream temp_stream_ctr(temp_str_ctr);
		while (getline(temp_stream_ctr, part_str_ctr, ','))
			temp_str_vec_ctr.push_back(part_str_ctr);
		pages_CTR.insert(make_pair(temp_str_vec_ctr[0], stoi(temp_str_vec_ctr[1])));
		temp_str_vec_ctr.clear();
	}
}

void network::page_score_calc()
{
	for (const auto i : pages)
		pages_rank.insert(make_pair(i, 1 / double(pages.size())));
	map<string, double> temp_rank_map(pages_rank);
	double temp_rank = 0;
	for (const auto i : pages)
	{
		for (const auto j : adj_list_rev[i])
			temp_rank += temp_rank_map[j] / double(adj_list[j].size());
		pages_rank[i] = temp_rank;
		temp_rank = 0;
	}
	for (const auto i : pages)
		pages_score[i] = (0.4 * pages_rank[i]) + ((((1 - (0.1 * pages_impressions[i] / (1 + (0.1 * pages_impressions[i])))) * pages_rank[i]) + ((0.1 * pages_impressions[i] / (1 + (0.1 * pages_impressions[i]))) * pages_CTR[i])) * 0.6);
}

void network::search(char words [])
{
	vector<string> keywords_vec;
	list<pair<string, double>> results;
	string check(words);
	char *pt1, *pt2, *pt3, *pt4;
	int order = 1;
	bool found = false;
	if (check.find("OR") != string::npos)
	{
		char* tokens = strtok_s(words, "OR", &pt1);
		while (tokens != NULL)
		{
			keywords_vec.push_back(tokens);
			tokens = strtok_s(NULL, "OR", &pt1);
		}
		for (const auto i : keywords_vec)
			for (const auto j : pages_keywords)
				if (find(j.second.begin(), j.second.end(), i) != j.second.end() && find(results.begin(), results.end(), make_pair(j.first, pages_score[j.first])) == results.end())
				{
					results.push_back(make_pair(j.first, pages_score[j.first]));
					pages_impressions[j.first]++;
				}
	}
	else if (check.find("AND") != string::npos)
	{
		char* tokens = strtok_s(words, "AND", &pt2);
		while (tokens != NULL)
		{
			keywords_vec.push_back(tokens);
			tokens = strtok_s(NULL, "AND", &pt2);
		}
		for (const auto i : pages_keywords)
		{
			int matchings = 0;
			for (const auto j : keywords_vec)
				if (find(i.second.begin(), i.second.end(), j) != i.second.end())
					matchings++;
			if (matchings == keywords_vec.size() && find(results.begin(), results.end(), make_pair(i.first, pages_score[i.first])) == results.end())
			{
				results.push_back(make_pair(i.first, pages_score[i.first]));
				pages_impressions[i.first]++;
			}
		}
	}
	else if (check.find(" ") != string::npos && check.find('/"') == string::npos)
	{
		char* tokens = strtok_s(words, " ", &pt3);
		while (tokens != NULL)
		{
			keywords_vec.push_back(tokens);
			tokens = strtok_s(NULL, " ", &pt3);
		}
		for (const auto i : keywords_vec)
			for (const auto j : pages_keywords)
				if (find(j.second.begin(), j.second.end(), i) != j.second.end() && find(results.begin(), results.end(), make_pair(j.first, pages_score[j.first])) == results.end())
				{
					results.push_back(make_pair(j.first, pages_score[j.first]));
					pages_impressions[j.first]++;
				}
	}
	else if (check.find("\"") != string::npos)
	{
		char* tokens = strtok_s(words, "\"", &pt4);
		while (tokens != NULL)
		{
			keywords_vec.push_back(tokens);
			tokens = strtok_s(NULL, "\"", &pt4);
		}
		for (const auto i : keywords_vec)
			for (const auto j : pages_keywords)
				if (find(j.second.begin(), j.second.end(), i) != j.second.end() && find(results.begin(), results.end(), make_pair(j.first, pages_score[j.first])) == results.end())
				{
					results.push_back(make_pair(j.first, pages_score[j.first]));
					pages_impressions[j.first]++;
				}
	}
	else
	{
		keywords_vec.push_back(check);
		for (const auto i : keywords_vec)
			for (const auto j : pages_keywords)
				if (find(j.second.begin(), j.second.end(), i) != j.second.end() && find(results.begin(), results.end(), make_pair(j.first, pages_score[j.first])) == results.end())
				{
					results.push_back(make_pair(j.first, pages_score[j.first]));
					pages_impressions[j.first]++;
				}
	}
	results.sort(helper_sort);
	results.erase(unique(results.begin(), results.end(), helper_unique), results.end());
	for (auto i : results)
	{
		results_dis.insert(make_pair(order, i.first));
		order++;
	}
	cout << "Search results: " << endl << endl;
	for (const auto i : results_dis)
		cout << i.first << ". " << i.second << endl;
	cout << endl;
}

void network::new_search()
{
	results_dis.clear();
	cout << "Type the keywords for search: ";
	string inpt;
	cin.ignore();
	getline(cin, inpt);
	char* inpt_chr = const_cast<char*>(inpt.c_str());
	cout << endl << endl;
	search(inpt_chr);
	cout << endl << "Would you like to: " << endl << endl << "1. Choose a webpage to open" << endl << "2. New search" << endl << "3. Exit" << endl << endl << "Type in your choice: ";
	cin >> user_choice;
	cout << endl;
	if (user_choice == 1)
		open_webpage();
	else if (user_choice == 2)
		new_search();
	else if (user_choice == 3)
	{
		export_data();
		return;
	}
	else do
	{
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		cout << "Invalid input ... restarting the process" << endl << endl;
		cout << "Would you like to: " << endl << endl << "1. Choose a webpage to open" << endl << "2. New search" << endl << "3. Exit" << endl << endl << "Type in your choice: ";
		cin >> user_choice;
		if (user_choice == 1)
			open_webpage();
		else if (user_choice == 2)
			new_search();
		else if (user_choice == 3)
		{
			export_data();
			return;
		}
	} while (user_choice == 1 || user_choice == 2 || user_choice == 3);
}

void network::open_webpage()
{
	int web_choice;
	cout << "Choose a website by number: ";
	cin >> web_choice;
	if (cin)
	{
		cout << endl;
		cout << "You are now viewing " << results_dis[web_choice] << endl << endl << "Would you like to: " << endl << endl << "1. Back to search results" << endl << "2. New Search" << endl << "3. Exit" << endl << endl << "Type in your choice: ";
		pages_CTR[results_dis[web_choice]] = ((pages_CTR[results_dis[web_choice]] * pages_impressions[results_dis[web_choice]]) + 1) / pages_impressions[results_dis[web_choice]];
		cin >> user_choice;
		cout << endl;
		if (user_choice == 1)
		{
			for (const auto i : results_dis)
				cout << i.first << ". " << i.second << endl << endl;
			open_webpage();
		}
		else if (user_choice == 2)
			new_search();
		else if (user_choice == 3)
		{
			export_data();
			return;
		}
		else
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "Invalid input ... restarting the process" << endl << endl << "Search results: " << endl;
			for (const auto i : results_dis)
				cout << i.first << ". " << i.second << endl;
			cout << endl;
			open_webpage();
		}
	}
	else 
	{
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		cout << endl << "Invalid input ... restarting the process" << endl << endl << "Search results: " << endl;
		for (const auto i : results_dis)
			cout << i.first << ". " << i.second << endl;
		cout << endl;
		open_webpage();
	}
}

void network::export_data()
{
	ofstream CTR_export("CTR_new.csv");
	for (const auto i : pages_CTR)
		CTR_export << i.first << "," << i.second << endl;
	CTR_export.close();
	remove("CTR.csv");
	rename("CTR_new.csv", "CTR.csv");
	ofstream Impressions_export("Impressions_new.csv");
	for (const auto i : pages_impressions)
		Impressions_export << i.first << "," << i.second << endl;
	Impressions_export.close();
	remove("Impressions.csv");
	rename("Impressions_new.csv", "Impressions.csv");
}

void network::launch()
{
	cout << "Welcome!" << endl << endl << "What would you like to do?" << endl << endl << "1. New Search" << endl << "2. Exit" << endl << endl << "Type your choice: ";
	cin >> user_choice;
	cout << endl;
	if (user_choice == 1)
		new_search();
	else if (user_choice == 2)
	{
		export_data();
		return;
	}
	else
	{
		cin.clear();
		cin.ignore(INT_MAX, '\n');
		cout << "Invalid input ... relaunching" << endl << endl;
		launch();
	}
}

int main()
{
	network n;
	return 0;
}