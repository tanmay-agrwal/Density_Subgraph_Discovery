#include <bits/stdc++.h>
using namespace std;

// CoreExact: exact algorithm for h-clique densest subgraph (CDS)
// Implements Algorithm 3 (k-clique-core decomposition) and Algorithm 4 (CoreExact) with Algorithm 1 flow-based search

class Graph {
public:
    uint32_t num_vertices;
    vector<vector<uint32_t>> neighbors_list;
    Graph(uint32_t n=0): num_vertices(n), neighbors_list(n) {}
    void insert_edge(uint32_t u, uint32_t v) {
        if (u >= num_vertices || v >= num_vertices) return;
        neighbors_list[u].push_back(v);
        neighbors_list[v].push_back(u);
    }
    static Graph load_edge_list(const string &filename) {
        ifstream in(filename);
        if (!in) { cerr<<"Cannot open "<<filename<<"\n"; exit(1); }
        vector<pair<uint32_t,uint32_t>> edges;
        string line;
        uint32_t u,v, max_id = 0;
        while (getline(in,line)) {
            if (line.empty()||line[0]=='#') continue;
            istringstream iss(line);
            if (!(iss>>u>>v)) continue;
            edges.emplace_back(u,v);
            max_id = max(max_id, max(u,v));
        }
        Graph G(max_id+1);
        for (auto &e:edges) G.insert_edge(e.first,e.second);
        return G;
    }
};

// Helper function to find k-cliques
void find_k_cliques(const Graph &G, uint32_t k, vector<vector<uint32_t>> &clique_list) {
    uint32_t n = G.num_vertices;
    deque<uint32_t> current_clique;
    function<void(vector<uint32_t>&)> dfs_search = [&](vector<uint32_t> &candidates) {
        if (current_clique.size() == k) {
            clique_list.push_back(vector<uint32_t>(current_clique.begin(), current_clique.end()));
            return;
        }
        while (!candidates.empty()) {
            uint32_t v = candidates.back();
            candidates.pop_back();
            vector<uint32_t> next_candidates;
            for (uint32_t u: candidates) {
                if (find(G.neighbors_list[v].begin(), G.neighbors_list[v].end(), u) != G.neighbors_list[v].end())
                    next_candidates.push_back(u);
            }
            current_clique.push_back(v);
            dfs_search(next_candidates);
            current_clique.pop_back();
        }
    };
    for (uint32_t v=0; v<n; ++v) {
        vector<uint32_t> initial_candidates;
        for (uint32_t u: G.neighbors_list[v]) if (u>v) initial_candidates.push_back(u);
        current_clique = {v};
        dfs_search(initial_candidates);
    }
}

// Algorithm 3: compute (k,Ψ)-core numbers via h-clique enumeration
vector<uint32_t> calculate_clique_core(const Graph &G, uint32_t h) {
    uint32_t n = G.num_vertices;
    vector<vector<uint32_t>> cliques;
    find_k_cliques(G, h, cliques);
    uint32_t m = cliques.size();
    vector<uint32_t> degree(n,0);
    vector<vector<uint32_t>> incidence(n);
    for (uint32_t i=0;i<m;++i) {
        for (uint32_t v: cliques[i]) {
            degree[v]++;
            incidence[v].push_back(i);
        }
    }
    uint32_t max_degree = *max_element(degree.begin(), degree.end());
    vector<deque<uint32_t>> buckets(max_degree+1);
    for (uint32_t v=0;v<n;++v) buckets[degree[v]].push_back(v);
    vector<uint32_t> core_values(n,0);
    vector<bool> is_removed(n,false);
    uint32_t removed_count=0;
    for (uint32_t d=0;d<=max_degree && removed_count<n;++d) {
        while (!buckets[d].empty()) {
            uint32_t v = buckets[d].front();
            buckets[d].pop_front();
            if (is_removed[v]||degree[v]!=d) continue;
            is_removed[v]=true;
            core_values[v]=d;
            removed_count++;
            for (uint32_t ci: incidence[v]) for (uint32_t u: cliques[ci]) {
                if (!is_removed[u] && degree[u]>d) {
                    uint32_t du = degree[u];
                    degree[u]--;
                    buckets[du-1].push_back(u);
                }
            }
        }
    }
    return core_values;
}

// Dinic max-flow implementation
struct FlowEdge{int to; int64_t capacity; int reverse_index;};
struct DinicFlow{
    int node_count, source, sink;
    vector<vector<FlowEdge>> flow_graph;
    vector<int> levels, pointers;
    DinicFlow(int N, int s, int t):node_count(N),source(s),sink(t),flow_graph(N),levels(N),pointers(N){}
    void add_flow_edge(int u,int v,int64_t c) {
        flow_graph[u].push_back({v,c,(int)flow_graph[v].size()});
        flow_graph[v].push_back({u,0,(int)flow_graph[u].size()-1});
    }
    bool build_levels() {
        fill(levels.begin(),levels.end(),-1);
        queue<int>q; q.push(source); levels[source]=0;
        while(!q.empty()){
            int u=q.front();q.pop();
            for(auto &e:flow_graph[u]) if(e.capacity>0 && levels[e.to]<0) {
                levels[e.to]=levels[u]+1; q.push(e.to);
            }
        }
        return levels[sink]>=0;
    }
    int64_t push_flow(int u,int64_t flow) {
        if (!flow||u==sink) return flow;
        for (int &cid=pointers[u]; cid<flow_graph[u].size(); ++cid) {
            auto &e = flow_graph[u][cid];
            if (e.capacity>0 && levels[e.to]==levels[u]+1) {
                int64_t pushed = push_flow(e.to, min(flow,e.capacity));
                if (pushed) {
                    e.capacity-=pushed;
                    flow_graph[e.to][e.reverse_index].capacity+=pushed;
                    return pushed;
                }
            }
        }
        return 0;
    }
    int64_t compute_maxflow() {
        int64_t total_flow=0;
        while(build_levels()){
            fill(pointers.begin(),pointers.end(),0);
            while(int64_t pushed=push_flow(source,INT64_MAX)) total_flow+=pushed;
        }
        return total_flow;
    }
};

int main(int argc,char*argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (argc!=3) { cerr<<"Usage: "<<argv[0]<<" <edge-file> <h>\n"; return 1; }
    Graph G = Graph::load_edge_list(argv[1]);
    uint32_t h = stoi(argv[2]); if (h<2) { cerr<<"h>=2 required\n"; return 1; }

    // 1. Compute core numbers
    auto core_values = calculate_clique_core(G, h);
    uint32_t k_max = *max_element(core_values.begin(), core_values.end());

    // 2. Extract (k_max,Ψ)-core vertices
    vector<uint32_t> core_vertices;
    vector<int> vertex_id(G.num_vertices, -1);
    for (uint32_t v=0; v<G.num_vertices; ++v) if (core_values[v]>=k_max) {
        vertex_id[v]=core_vertices.size();
        core_vertices.push_back(v);
    }
    uint32_t core_size = core_vertices.size();
    Graph core_graph(core_size);
    for (uint32_t i=0;i<core_size;++i) {
        for (uint32_t u: G.neighbors_list[core_vertices[i]]) if (vertex_id[u]>=0 && vertex_id[u]>i)
            core_graph.insert_edge(i,vertex_id[u]);
    }

    // 3. Enumerate (h-1)-cliques in core_graph
    vector<vector<uint32_t>> cliques_hm1;
    find_k_cliques(core_graph, h-1, cliques_hm1);
    uint32_t num_cliques = cliques_hm1.size();

    // 4. Binary search
    double lower_bound = (double)k_max / h, upper_bound = k_max;
    vector<uint32_t> densest_subgraph = core_vertices;
    while (upper_bound - lower_bound > 1.0/(core_size*(core_size-1))) {
        double alpha = (lower_bound + upper_bound)/2;
        int network_size = 1 + core_size + num_cliques + 1;
        int source=0, sink=network_size-1;
        DinicFlow dinic(network_size,source,sink);
        vector<int64_t> clique_degrees(core_size,0);
        int64_t total_clique_degree=0;
        // compute clique-degree in core_graph
        for (uint32_t i=0;i<num_cliques;++i) for (uint32_t v: cliques_hm1[i]) clique_degrees[v]++;
        for (uint32_t v=0;v<core_size;++v) {
            dinic.add_flow_edge(source,1+v,clique_degrees[v]);
            total_clique_degree+=clique_degrees[v];
        }
        // v->t
        for (uint32_t v=0;v<core_size;++v) dinic.add_flow_edge(1+v,1+core_size+num_cliques,(int64_t)(alpha*h));
        // clique->verts
        for (uint32_t i=0;i<num_cliques;++i) {
            int clique_id = 1+core_size+i;
            for (uint32_t v: cliques_hm1[i]) dinic.add_flow_edge(clique_id,1+v,INT64_MAX);
        }
        // verts->clique
        for (uint32_t i=0;i<num_cliques;++i) {
            int clique_id = 1+core_size+i;
            for (uint32_t v=0;v<core_size;++v) {
                bool in_clique=false;
                for (uint32_t u2: cliques_hm1[i]) if (u2==v) { in_clique=true; break; }
                if (in_clique) continue;
                bool is_connected=true;
                for (uint32_t u2: cliques_hm1[i]) {
                    if (find(core_graph.neighbors_list[v].begin(),core_graph.neighbors_list[v].end(),u2)==core_graph.neighbors_list[v].end()) {
                        is_connected=false;
                        break;
                    }
                }
                if (is_connected) dinic.add_flow_edge(1+v,clique_id,1);
            }
        }
        int64_t flow_value = dinic.compute_maxflow();
        if (flow_value < total_clique_degree) {
            // extract reachable vertices
            vector<char> visited(network_size,0);
            queue<int>q; visited[source]=1; q.push(source);
            while(!q.empty()){
                int x=q.front();q.pop();
                for (auto &e: dinic.flow_graph[x]) if (e.capacity>0 && !visited[e.to]) {
                    visited[e.to]=1;
                    q.push(e.to);
                }
            }
            vector<uint32_t> candidate_vertices;
            for (uint32_t v=0;v<core_size;++v) if (visited[1+v]) candidate_vertices.push_back(core_vertices[v]);
            densest_subgraph=candidate_vertices;
            lower_bound=alpha;
        } else {
            upper_bound=alpha;
        }
    }

    // 5. Output
    cout << "Densest subgraph size = " << densest_subgraph.size() <<endl<< " Vertices:";
    for (uint32_t v : densest_subgraph) cout << " " << v;
    cout <<endl;

    // Compute and print densities for verification
    // Build induced subgraph S on 'densest_subgraph'
    uint32_t subgraph_size = densest_subgraph.size();
    unordered_set<uint32_t> in_subgraph(densest_subgraph.begin(), densest_subgraph.end());
    Graph subgraph(subgraph_size);
    // map old to new indices
    unordered_map<uint32_t,uint32_t> index_map;
    for (uint32_t i=0; i<subgraph_size; ++i) index_map[densest_subgraph[i]] = i;
    int64_t edge_count = 0;
    for (uint32_t old_v : densest_subgraph) {
        uint32_t v = index_map[old_v];
        for (uint32_t neighbor : G.neighbors_list[old_v]) {
            if (in_subgraph.count(neighbor)) {
                uint32_t u = index_map[neighbor];
                if (u > v) {
                    subgraph.insert_edge(v, u);
                    edge_count++;
                }
            }
        }
    }
    double edge_density = (double)edge_count / subgraph_size;
    cout << "Edge-density = " << edge_density <<endl;

    // Count h-cliques in subgraph
    vector<vector<uint32_t>> subgraph_cliques;
    find_k_cliques(subgraph, h, subgraph_cliques);
    int64_t clique_count = subgraph_cliques.size();
    double clique_density = (double)clique_count / subgraph_size;
    cout << h << "-clique-density = " << clique_density << " (" << clique_count << " cliques over " << subgraph_size << " vertices)"<<endl;

    return 0;
}