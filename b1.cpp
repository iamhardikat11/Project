#include <bits/stdc++.h>
#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

const long double INF = 1e18;
class node {
    public:
    double longitude;
    double latitude;
    string id;
    // long long uid;
    string name;
    node() {
        name = "NA";
        longitude = 0.0;
        latitude = 0.0;
        id = "NA";
    }
};

class way {
    public:
    string id;
    vector<string> IncludedNode;
};

int nodes_count;
int ways_count;
// vector<node> node_list;
map<string, node> node_list;
vector<way> way_list;
map<string, vector<pair<string, long double>>> graph;

long double toRadians(const long double degree)
{
    long double one_deg = (M_PI) / 180;
    return (one_deg * degree);
}

bool comp(pair<long double, node> p1, pair<long double, node> p2) {
    return (p1.first < p2.first);
}

long double distance_calculate(long double lat1, long double long1, long double lat2, long double long2)
{
    // Convert the latitudes and longitudes from degree to radians.
    lat1 = toRadians(lat1);
    long1 = toRadians(long1);
    lat2 = toRadians(lat2);
    long2 = toRadians(long2);
     
    // Haversine Formula
    long double dlong = long2 - long1;
    long double dlat = lat2 - lat1;
 
    long double ans = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlong / 2), 2);
 
    ans = 2 * asin(sqrt(ans));
 
    // Radius of Earth in
    // Kilometers, R = 6371
    // Use R = 3956 for miles
    long double R = 6371;
     
    // Calculate the result
    ans = ans * R;
 
    return ans;
}

void kthSmallestDistance(string uid, int k) 
{
    node centre;
    for(auto n : node_list) {
        if(n.first == uid) {
            centre.id = uid;
            centre.latitude = n.second.latitude;
            centre.longitude = n.second.longitude;
            centre.name = n.second.name;
        }
    }

    if(centre.id == "NA") {
        cout<<"ID not found in the list!\n";
        return;
    }

    vector<pair<long double, node>> dist;
    for(auto n : node_list) {
        dist.push_back({distance_calculate(centre.latitude, centre.longitude, n.second.latitude, n.second.longitude), n.second});
    }
    sort(dist.begin(), dist.end(), comp);
    if(k > node_list.size()-1) {
        cout<<"k is entered greater then the number of nodes present in the database!\n";
        cout<<"Printing all the data points from the database...\n";
        k = node_list.size()-1;
    }

    cout<<"Central node data: \n";
    cout<<"Name: "<<centre.name<<"\n";
    cout<<"ID: "<<centre.id<<"\n";
    cout<<"Longitude: "<<centre.longitude<<"\n";
    cout<<"Latitude: "<<centre.latitude<<"\n\n";
    for(int i=1; i<=k; i++) {
        cout<<i<<". Distance from the Node(in km): "<<dist[i].first<<", Place ID: "<<dist[i].second.id<<", Name: "<<dist[i].second.name<<"\n";
    }
}

void make_graph()
{
    node temp1, temp2;
    for(way w : way_list) {
        for(int i=0; i<w.IncludedNode.size()-1; i++) {
            temp1 = node_list[w.IncludedNode[i]];
            temp2 = node_list[w.IncludedNode[i+1]];
            long double dist = distance_calculate(temp1.latitude, temp1.longitude, temp2.latitude, temp2.longitude);
            graph[w.IncludedNode[i]].push_back({w.IncludedNode[i+1], dist});
            graph[w.IncludedNode[i+1]].push_back({w.IncludedNode[i], dist});
        }
    }
}

void Dijkstra(string m, string n)
{
	string source = m, destination = n;

    map<string, long double> dist;
    map<string, string> parent;
	set<pair<long double, string>> S; // (dist[u], u);
	for(auto n : node_list) dist[n.first] = INF;
	
	dist[source] = 0.0;
	S.insert(make_pair(0.0, source));
 
	while(!S.empty()) {
		string a = S.begin() -> second;
		S.erase(S.begin());
		for(pair<string, long double> pp : graph[a]){
			if(dist[a] + pp.second < dist[pp.first]){
				S.erase(make_pair(dist[pp.first], pp.first));
				dist[pp.first] = dist[a] + pp.second;
				parent[pp.first] = a;
				S.insert(make_pair(dist[pp.first], pp.first));
			}
		}
 
	}
	if(dist[destination] >= INF) {
		cout << "Path not found!";
		return;
	}

    cout<<dist[destination]<<"\n";
    cout<<"Do you also want to print the paths taken to go from source to destination? (Y/N): ";
    char c; 
    cin >> c;
    if(c == 'Y' || c == 'y') {
        vector<string> path;
        string x = destination;
        path.push_back(destination);
        while(x != source){
            path.push_back(x = parent[x]);
        }
        reverse(path.begin(), path.end());
        for(int i=0; i<path.size(); i++) {
            cout<<i+1<<". "<<path[i]<<"\n";
        }
    }
}

void SearchValues() {
    string inp;
    cout<<"Enter name of place you want to search: ";
    cin>>inp;
    int ctr = 0;
    for(auto n: node_list) {
        string s = n.second.name;
        transform(s.begin(), s.end(), s.begin(), ::tolower);
        transform(inp.begin(), inp.end(), inp.begin(), ::tolower);
        if(s != "na" && s.find(inp) != string::npos) {
            ctr++;
            cout<<n.second.id<<" "<<n.second.name<<"\n";
        }
    }
    cout<<"Total "<<ctr<<" match found!\n";
    return;
}

void PrintKNodes() {
    string uid;
    int k;
    cout<<"Enter the ID of the place: ";
    cin >> uid;
    cout<<"Enter the value of k: ";
    cin >> k;

    kthSmallestDistance(uid, k);
    return;
}

void PrintShortestNodes() {
    string uid1, uid2;
    cout<<"Enter Source ID: ";
    cin >> uid1;
    cout<<"Enter Destination ID: ";
    cin >> uid2;
    make_graph();
    Dijkstra(uid1, uid2);
    return;
}

xml_document<> doc;
xml_node<> * root_node = NULL;
   
int main()
{
    cout << "\nParsing the given open source map file data (map.osm)....." << endl;
   
    // Read the sample.xml file
    ifstream theFile ("map.osm");
    vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
    buffer.push_back('\0');
   
    // Parse the buffer
    doc.parse<0>(&buffer[0]);
   
    // Find out the root node
    root_node = doc.first_node("osm");

    
    // Iterate over the student nodes
    // xml_node <> *end_node = root_node->last_node("node");
    xml_node<> * _node;
    node place;
    for (_node = root_node->first_node("node"); _node->first_attribute("id")->value() != root_node->last_node("node")->first_attribute("id")->value(); _node = _node->next_sibling())
    {
        nodes_count++;
        place.longitude = stod(_node->first_attribute("lon")->value());
        place.latitude = stod(_node->first_attribute("lat")->value());
        place.id = _node->first_attribute("id")->value();
        // place.uid = _node->first_attribute("uid")->value();

        for(xml_node <> * name_node = _node->first_node("tag"); name_node; name_node = name_node->next_sibling()) {
            string cmp = "name";
            if(name_node->first_attribute("k")->value() == cmp) {
                place.name = name_node->first_attribute("v")->value();
                break;
            }
        }
        node_list[place.id] = place;
        place = node();
    }
    nodes_count++;
    place.longitude = stod(_node->first_attribute("lon")->value());
    place.latitude = stod(_node->first_attribute("lat")->value());
    place.id = _node->first_attribute("id")->value();
    
    for(xml_node <> * name_node = _node->first_node("tag"); name_node; name_node = name_node->next_sibling()) 
    {
        if(name_node->first_attribute("k")->value() == "name") {
            place.name = name_node->first_attribute("v")->value();
            break;
        }
    }
    node_list[place.id] = place;

    xml_node <> * _way;
    way path;
    for(_way = root_node->first_node("way"); _way->first_attribute("id")->value() != root_node->last_node("way")->first_attribute("id")->value(); _way = _way->next_sibling())
    {
        ways_count++;
        path.id = _way->first_attribute("id")->value();
        
        for(xml_node <> * connecting_node = _way->first_node("nd"); connecting_node->first_attribute("ref")->value() != _way->last_node("nd")->first_attribute("ref")->value(); connecting_node = connecting_node->next_sibling())
        {
            path.IncludedNode.push_back(connecting_node->first_attribute("ref")->value());
        }
        path.IncludedNode.push_back(_way->last_node("nd")->first_attribute("ref")->value());
        way_list.push_back(path);
        path = way();
    }

    ways_count++;
    path.id = _way->first_attribute("id")->value();
        
    for(xml_node <> * connecting_node = _way->first_node("nd"); connecting_node->first_attribute("ref")->value() != _way->last_node("nd")->first_attribute("ref")->value(); connecting_node = connecting_node->next_sibling())
    {
        path.IncludedNode.push_back(connecting_node->first_attribute("ref")->value());
    }
    path.IncludedNode.push_back(_way->last_node("nd")->first_attribute("ref")->value());
    way_list.push_back(path);

    cout<<"Number of nodes = "<<nodes_count<<"\n";
    cout<<"Number of ways = "<<ways_count<<"\n";

    string s;
    while(true) {
        cout<<endl;
        cout<<"1. Search Nodes\n";
        cout<<"2. Print k smallest\n";
        cout<<"3. Shortest distance between two nodes\n";   
        cout<<"$. Exit\n";
        cout<<"Enter Your Choice: ";
        cin >> s;
        if(s == "1") SearchValues();
        else if(s == "2") PrintKNodes();
        else if(s == "3") PrintShortestNodes();
        else break;
    }


    return 0;
}