#include<bits/stdc++.h>
#include "rapidxml.hpp"
using namespace std;
using namespace rapidxml;

class Node {
    public:
    string id;
    string name;
    long double latitude;
    long double longitude;
    Node()
    {
        id = "";
        name = "NULL";
        longitude=0.0;
        latitude=0.0;
    }
};

class Way {
    public:
    string id;
    vector<string> nodes_in_way;
    Way()
    {
        id = "";
    }
};

map<string,Node> node_map;
vector<Way> way_list;
map<string,vector<string>> AdjList; //Mapping Node id to an array of Node ids to which it is connected

long double toRadians(long double degree)
{
    long double one_degree = (M_PI)/180;
    return (one_degree * degree);
}

long double calcDistance(string id1,string id2)
{
// Convert the latitudes
    // and longitudes
    // from degree to radians.
    if(node_map.find(id1) == node_map.end() || node_map.find(id2) ==node_map.end())
    {
        return 10000000;
    }
    Node n1 = node_map[id1];
    Node n2 = node_map[id2];
    long double lat1 = toRadians(n1.latitude);
    long double long1 = toRadians(n1.longitude);
    long double lat2 = toRadians(n2.latitude);
    long double long2 = toRadians(n2.longitude);
     
    // Haversine Formula
    long double dlong = long2 - long1;
    long double dlat = lat2 - lat1;
 
    long double ans = pow(sin(dlat / 2), 2) +
                          cos(lat1) * cos(lat2) *
                          pow(sin(dlong / 2), 2);
 
    ans = 2 * asin(sqrt(ans));
 
    // Radius of Earth in
    // Kilometers, R = 6371
    // Use R = 3956 for miles
    long double R = 6371.0;
     
    // Calculate the result
    ans = ans * R;
 
    return ans;
}

string to_lower(string p)
{
    int size = p.length();
    string s = p;
    for(int i=0;i<size;i++)
    {
        if(s[i]<='Z' && s[i]>='A') s[i]=s[i]-'A'+'a';
    }
    return s;
}

void kNeartestNodes(string id,int k)
{
    if(node_map.find(id) == node_map.end())
    {
        cout<<"Node ID Not Found!!"<<'\n';
        return;
    }
    if(k==0)return;

    map<long double,Node> m;
    for(auto p : node_map)
    {  
        if(p.first != id)
        {
            long double dist = calcDistance(p.first,id);
            m.insert({dist,p.second});
        }
    }
    int count = 0;
    for(auto p : m)
    {
        count++;
        cout<<count<<". Node ID : "<<p.second.id<<" Name : "<<p.second.name<<" Distance : "<<p.first<<'\n';
        if(count == k)break;
    }
}

xml_document<> doc;  
xml_node<> * root_node = NULL;

void Parse()
{
    // Read map.osm
    ifstream osm ("map.osm");
    vector<char> buffer((istreambuf_iterator<char>(osm)), istreambuf_iterator<char>());
    buffer.push_back('\0');

    // Parse the buffer
    doc.parse<0>(&buffer[0]);

    // Find out the root node
    root_node = doc.first_node("osm");

    int node_count = 0;
    // Iterate over the Nodes 
    xml_node<> * end_node = root_node->last_node("node");
    for (xml_node<> * node = root_node->first_node("node"); node; node = node->next_sibling())
    {
        node_count++;
        Node n;
        n.id = node->first_attribute("id")->value();
        n.latitude = stod(node->first_attribute("lat")->value());
        n.longitude = stod(node->first_attribute("lon")->value());
        string check="name";
        for(xml_node<> * tag = node->first_node("tag"); tag ; tag = tag->next_sibling())
        {
            if(tag->first_attribute("k")->value() == check)
            {
                n.name = tag->first_attribute("v")->value();
                break;
            }
        }
        node_map[n.id] = n;
        if(node->first_attribute("id")->value() == end_node->first_attribute("id")->value())
            break;    
    }
    int way_count = 0;
    //Iterate over the Ways
    xml_node<> * end_way = root_node->last_node("way");
    for(xml_node<> * way = root_node->first_node("way");way; way = way->next_sibling())
    {
        way_count++;
        Way w;
        w.id = way->first_attribute("id")->value();
        xml_node<> * end_nd = way->last_node("nd");
        for(xml_node<> * nd = way->first_node("nd");nd->first_attribute("ref")->value() != end_nd->first_attribute("ref")->value();nd = nd->next_sibling())
        {
            w.nodes_in_way.push_back(nd->first_attribute("ref")->value());
        }
        w.nodes_in_way.push_back(end_nd->first_attribute("ref")->value());
        way_list.push_back(w);
        if(way->first_attribute("id")->value() == end_way->first_attribute("id")->value())
            break;
    }
   
    cout<<"Number of Nodes : "<<node_count<<'\n';
    cout<<"Number of Ways : "<<way_count<<'\n';
}

void MatchName(string subs)
{
    int count_matches=0;
    for(auto p : node_map)
    {
        Node n = p.second;
        if(n.name != "NULL" && to_lower(n.name).find(to_lower(subs)) != string::npos)
        {
            count_matches++;
            cout<<"ID : "<<n.id<<" Name : "<<n.name<<'\n';    
        }
    }
    cout<<count_matches<<" Matches Found !!"<<'\n';
}

void CreateAdjacencyList()
{
    for(Way w : way_list)
    {
        int size = w.nodes_in_way.size();
        for(int i=0;i<size;i++)
        {
            if(i-1>=0)
            {
                AdjList[w.nodes_in_way[i]].push_back(w.nodes_in_way[i-1]);
            }
            if(i+1<size)
            {
                AdjList[w.nodes_in_way[i]].push_back(w.nodes_in_way[i+1]);                   
            }
        }
    }
}

void Dijkstra(string start,string end)
{
    if(node_map.find(start) == node_map.end() || node_map.find(end) == node_map.end())
    {
        cout<<"Nodes Not Found!!\n";
        return;
    }
    priority_queue<pair<long double,string>,vector<pair<long double, string>>, greater <pair<long double, string>>> pq;
    pq.push({0.0,start});
    map<string,long double> dist_map;
    map<string,string> prev_node;
    
    dist_map[start]=0.0;

    
    while(!pq.empty() &&  !(pq.top().second == end))
    {      
        auto p = pq.top();
        pq.pop();
        if(p.first > dist_map[p.second])
        {
            cout<<'a';
            continue;
        }
        
        for(string s : AdjList[p.second])
        {
            
            if(dist_map.find(s) == dist_map.end())
            {
                dist_map[s] = p.first + calcDistance(s,p.second);
                prev_node[s] = p.second;
                pq.push({dist_map[s],s});
            }
            else if(p.first + calcDistance(s,p.second) < dist_map[s])
            {
                dist_map[s] = p.first + calcDistance(s,p.second);
                prev_node[s] = p.second;
                pq.push({dist_map[s],s});
            }
        }
    }
    if(pq.empty())cout<<"Way Does Not Exist!!\n";
    else 
    cout<<"Distance of Shortest Path between Nodes "<<start<<" "<<end<<" is : "<<dist_map[end]<<" km"<<'\n';
}

void UserCase1()
{
    cout<<"Enter Name of Node You Wish to Find : ";
    string subs;
    cin>>subs;
    MatchName(subs);   
}

void UserCase2()
{
    //--------------PART 2------------------//
    cout<<'\n'<<"Enter Node you wish to Search(by ID) : ";
    string n_id;
    cin>>n_id;
    cout<<"Enter Number of Nodes you wish to find : ";
    int k;
    cin>>k;
    kNeartestNodes(n_id,k);
}

void UserCase3()
{
    CreateAdjacencyList();
    cout<<"Enter the IDs of Nodes between which you wish to find Shortest Path\n";
    string n_id1,n_id2;
    cout<<"Node ID1 : ";cin>>n_id1;
    cout<<"Node ID2 : ";cin>>n_id2;
    Dijkstra(n_id1,n_id2);
}

int main(void)
{
    cout << "\nParsing Map Data (map.osm)....." << endl;
    Parse();
    UserCase1();
    UserCase2();
    UserCase3();

    return 0;
}