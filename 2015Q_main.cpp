#include<vector>
#include<iostream>
#include<random>
#include<algorithm>
#include<utility>
#include<cassert>
#include<cstdio>

using namespace std;

bool unavailable[1000][1000] = {false};

#define SEED 31338

struct Gap {
	int start;
	int length;
	// The indices in server_list
	vector<int> servers;
};

struct Server {
	int size;
	int capacity;
	int row;
	int gap_index;
	int i_in_server_list;
	int pool;
};

bool gap_order_length(Gap &a, Gap &b) { return a.length < b.length; }

void fill_gap(Gap &g, vector<Server> &vs, vector<Server> &vs_orig, int gap_index, int row) {
	int C = g.length;
	int N = vs.size();
	int A[N+1][C+1];
	int comes_from[N+1][C+1];

    for (int j = 0; j <= C; j++) {
        A[0][j] = 0;
		comes_from[0][j] = 0;
	}
    for (int i = 1; i <= N; i++) {
        A[i][0] = 0;
		comes_from[i][0] = 0;
        for (int j = 1; j <= C; j++) {
			comes_from[i][j] = 0;
            A[i][j] = A[i-1][j];
            if (vs[i-1].size <= j) {
                int r = A[i-1][j-vs[i-1].size] + vs[i-1].capacity;
				if(A[i][j] <= r) {
					comes_from[i][j] = i;
					A[i][j] = r;
				}
            }
        }
    }

	vector<int> servers_to_delete;
	int i=N, j=C;
	while(i>0 && j>0) {
		if(comes_from[i][j] == 0) {
			i -= 1;
		} else {
			servers_to_delete.push_back(comes_from[i][j]-1);
			j -= vs[comes_from[i][j]-1].size;
			i -= 1;
		}
	}
//	cout << "DELETE: ";
//	int totsize = 0;
	for(auto it=servers_to_delete.begin(); it != servers_to_delete.end(); it++) {
		if((it+1) != servers_to_delete.end())
			assert(*(it) > *(it+1));
//		cout << *it << ", ";
		vs_orig[vs[*it].i_in_server_list].gap_index = gap_index;
		vs_orig[vs[*it].i_in_server_list].row = row;
		g.servers.push_back(vs[*it].i_in_server_list);
//		totsize += vs[*it].size;
		vs.erase(vs.begin()+ (*it));
	}
//	cout << "Filled " << totsize << " out of " << g.length << endl;
//	cout << endl;
}

void print_sol(const vector<Server> &server_list,
			   const vector<vector<Gap> >&gaps_per_row) {
	for (int i = 0; i < server_list.size(); i++) {
		if(server_list[i].row == -1) {
			cout << "x\n";
			continue;
		}
		cout << server_list[i].row << " ";
		Gap gap = gaps_per_row[server_list[i].row][server_list[i].gap_index];
		int sum = gap.start;
		for (int j = 0; j < gap.servers.size(); j++){
			if(gap.servers[j] == i){ //we have found the server
				cout << sum << " " << server_list[i].pool << endl;
				break;
			}
			sum += server_list[gap.servers[j]].size;
		}
	}
}

void print_server_configuration(const vector<Server> &server_list,
								const vector<vector<Gap> > &gaps_per_row, const int R, const int S) {
	int outcome[R+1][S+1];
	fill(&outcome[0][0], &outcome[0][0] + sizeof(outcome)/sizeof(outcome[0][0]), 0);
	char whatprint[] = {'#', '(', ')', 'O'};

	for(int i=0; i<R; i++) {
		for(int j=0; j<S; j++)
			outcome[i][j] = 0;
		int capacity = 0;
		for(auto gap: gaps_per_row[i]) {
			int s = gap.start;
			for(auto serv_i: gap.servers) {
				if(server_list[serv_i].size == 1) {
					outcome[i][s] = -4;
				} else {
					outcome[i][s] = -2;
					s += server_list[serv_i].size - 1;
					outcome[i][s] = -3;
				}
				s++;
				capacity += server_list[serv_i].capacity;
			}
			outcome[i][gap.start + gap.length] = -1;
		}
		for(int j=0; j<S; j++) {
			if(outcome[i][j] < 0) {
				cout << whatprint[-outcome[i][j]-1];
			} else {
				cout << ' ';
			}
		}
		cout << " " << capacity << endl;
	}
}


int main() {
	int R, S, U, P, M;
	cin >> R >> S >> U >> P >> M;
	for(int i=0; i<U; i++) {
		int y, x;
		cin >> y >> x;
		unavailable[y][x] = true;
	}
	// size, capacity
	vector<Server> server_list(M);
	for(int i=0; i<M; i++) {
		cin >> server_list[i].size >> server_list[i].capacity;
		server_list[i].row = server_list[i].gap_index = -1;
		server_list[i].i_in_server_list = i;
		server_list[i].pool = -1;
	}

	// For each row, a list of gaps. Each gap is a pair <starting slot, gap length, servers that belong there>
	vector<vector<Gap> > gaps_per_row(R);
	for(int i=0; i<R; i++) {
		Gap g;
		g.length = 0;
		g.start = 0;
		for(int j=0; j<=S; j++) {
			if(unavailable[i][j] || j==S) {
				if(g.length > 0) {
					gaps_per_row[i].push_back(g);
				}
				g.length = 0;
				g.start = j+1;
			} else {
				g.length++;
			}
		}
		sort(gaps_per_row[i].begin(), gaps_per_row[i].end(), gap_order_length);
	}

	// Next gap to fill in every row
	vector<int> next_gap_to_fill_row(R, 0);
	vector<int> this_round_order(R);
	for(int i=0; i<R; i++) this_round_order[i] = i;
	bool finished_updating = false;

	vector<Server> remaining_servers = server_list;
	while(!finished_updating) {
		std::shuffle(this_round_order.begin(), this_round_order.end(),
				std::default_random_engine(SEED));
		finished_updating = true;
		for(int _i=0; _i<R; _i++) {
			int i = this_round_order[_i];
			// If the next gap to fill is larger than the size skip this
			if(next_gap_to_fill_row[i] == (int)gaps_per_row[i].size())
				continue;
//			cerr << "Filling gap " << next_gap_to_fill_row[i] << " in row " << i << endl;
			finished_updating = false;

			fill_gap(gaps_per_row[i][next_gap_to_fill_row[i]], remaining_servers, server_list, next_gap_to_fill_row[i], i);

			next_gap_to_fill_row[i]++;
		}
	}

	print_sol(server_list, gaps_per_row);
	print_server_configuration(server_list, gaps_per_row, R, S);
	return 0;
}
