#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <algorithm> //max...
#include <utility> //pair
#include <complex>
#include <climits> //int, ll...
#include <limits> //double...
#include <cmath> //abs, atan...

#include <cstring> //memset
#include <string>


using namespace std;

typedef long long ll;


typedef pair<int,int> ii;

typedef pair<ll, ll> ll_ll;
typedef vector<int> vi;
typedef map<int, int> mii;
typedef vector<ii> vii;
typedef vector<ll> vll;
typedef vector<vi> vvi;




struct Server{
    int row, n_gap, cap, size, pool, index;
};

vector<Server> server_list;
vvi gaps;
int R, S, U, P, M;

struct Node;
int lower_heuristic(Node & n);
int compute_cost(vector<Server> my_servers);

struct Node{
    Node (int last_processed, vector<Server> & my_servers, vvi & used_gaps){
        _my_servers = my_servers;
        _last_processed = last_processed;
        _used_gaps = used_gaps;
        cost = compute_cost(_my_servers);
        heuristic_cost = lower_heuristic(*this);
        if (!_my_servers.empty()){
            Server s = my_servers.back();
            if(server_list[last_processed].index == s.index){
                _used_gaps[s.row][s.n_gap] -= s.cap;
            }
        }
    }
    int cost, heuristic_cost;
    vector<Server> _my_servers;
    int _last_processed;
    vvi _used_gaps; // the remaining size of each gap
};

class ord{
public:
  ord() {}
  bool operator() (const Server & a, const Server &b) const{
      return a.cap/(a.size+0.0) > b.cap/(b.size+0.0);
}};

class ord_heur{
public:
  ord_heur() {}
  bool operator() (const Node & a, const Node & b) const{
      return a.heuristic_cost < b.heuristic_cost;
}};

int compute_cost(vector<Server> my_servers){
    vvi cap_rp = vvi(P, vi(R, 0)); //capacity per row and per pool
    //vector<int> sum(P, 0);
    for(auto & server: my_servers){
        cap_rp[server.pool][server.row] += server.cap;
    }
    int sum, max, min = 1 << 30 ;
    for(auto & p_rows: cap_rp){
        sum = 0;
        max = -1;
        for(auto & c: p_rows){
            sum += c;
            if(max < c) max = c;
        }
        sum -= max;
        if (min > sum) min = sum;
    }
    return min;
}

int lower_heuristic(Node & n) {
    vector<Server> my_servers = n._my_servers;
    vvi used_gaps = n._used_gaps ; // the remaining size of each gap
    set<Server, ord> unused;
    for (unsigned int i = n._last_processed + 1; i < server_list.size(); i++){
        unused.insert(server_list[i]);
    }
    while(true){
        vvi cap_rp = vvi(P, vi(R, 0)); //capacity per row and per pool
        //vector<int> sum(P, 0);
        for(auto & server: n._my_servers){
            cap_rp[server.pool][server.row] += server.cap;
        }
        int sum, max, min = 1 << 30, idx_max, aux, pool;
        for(unsigned int i = 0; i < cap_rp.size(); i++){
            sum = 0;
            max = -1;
            for(unsigned int j = 0; j < cap_rp[i].size(); j++){
                sum += cap_rp[i][j];
                if(max < cap_rp[i][j]){
                    max = cap_rp[i][j];
                    aux = j;
                }
            }
            sum -= max;
            if (min > sum){
                pool = i;
                min = sum;
                idx_max = aux;
            }
        }
        bool done = false;
        for(auto server: unused){
            for(int i = 0; i < R && !done; i++){
                if (i == idx_max)
                    continue;
                for(unsigned int j = 0 ; j < used_gaps[i].size() && !done; j++){
                    if (used_gaps[i][j] > server.size){
                        done = true;
                        Server s = server;
                        s.row = i;
                        s.n_gap = j;
                        s.pool = pool;
                        used_gaps[i][j] -= s.cap;
                        unused.erase(server);
                        my_servers.push_back(s);
                    }
                }
            }
            if(done)
                break;
        }
        if(!done)
            break;
    }
    return compute_cost(my_servers);
}



int steps = 0;

void a_star(){
    vector<Server> ini;
    Node n(-1, ini, gaps);
    priority_queue<Node, vector<Node>, ord_heur> pq;
    pq.push(n);
    while(true){
        n = pq.top();
        pq.pop();
        steps++;
        if (steps % 10 == 0){
            cout << n.heuristic_cost << endl;
        }
        Node m = Node(n._last_processed + 1, n._my_servers, gaps);
        pq.push(m);
        for(unsigned int j = 0; j < n._used_gaps.size(); j++){
            for(unsigned int k = 0; k < n._used_gaps[j].size(); k++){
                if(n._used_gaps[j][k]  > server_list[n._last_processed +1].size){
                    for(int i = 0; i < P; ++i){
                        Server s = server_list[n._last_processed +1];
                        s.row = j;
                        s.n_gap = k;
                        s.pool = i;
                        vector<Server> my_servers = n._my_servers;
                        my_servers.push_back(s);
                        pq.push(Node(n._last_processed +1 , my_servers, n._used_gaps));
                    }
                }
            }
        }
    }
}

int main (){
    cin >> R >> S >> U >> P >> M;
    vvi unavailable(R, vector<int>());
    gaps = vvi(R);
    for(int i=0; i<U; i++) {
        int x, y;
        cin >> x >> y;
        unavailable[x].push_back(y);
    }
    for(int i = 0; i<R; i++){
        unavailable[i].push_back(-1);
        unavailable[i].push_back(S);
        sort(unavailable[i].begin(), unavailable[i].end());
        for(unsigned int j = 1; j < unavailable[i].size(); j++)
            gaps[i].push_back(unavailable[i][j]- unavailable[i][j-1] -1);
    }
    // size, capacity
    server_list = vector<Server>(M);
    for(int i=0; i<M; i++) {
        cin >> server_list[i].size >> server_list[i].cap;
        server_list[i].index = i;
    }
    sort(server_list.begin(), server_list.end(), [](const Server & a, const Server & b){return a.size > b.size;});
    a_star();
    return 0;
}
