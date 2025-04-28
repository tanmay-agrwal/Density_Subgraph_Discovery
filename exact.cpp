#include <bits/stdc++.h>
using namespace std;

// Implementation of Algorithm 1: Exact (flow-based binary search) for h-clique densest subgraph
// Modified data structures and data types: set for adjacency, separate vectors for flow edges, set for cliques,
// size_t for indices, long for capacities, long double for floating-point, vector<bool> for visited

class Graph {
public:
    size_t n;
    vector<set<int>> adjacencyList;
    Graph(size_t _n=0): n(_n), adjacencyList(n) {}
    void addEdge(int u, int v) {
        adjacencyList[u].insert(v);
        adjacencyList[v].insert(u);
    }
    static Graph readEdgeList(const string &fname) {
        ifstream in(fname);
        if (!in) { cerr << "Cannot open " << fname << "\n"; exit(1); }
        int u, v;
        size_t maxid = 0; // Use size_t for maxid
        vector<pair<int,int>> edges;
        string line;
        while (getline(in, line)) {
            if (line.empty() || line[0]=='#') continue;
            istringstream iss(line);
            if (!(iss>>u>>v)) continue;
            edges.emplace_back(u,v);
            maxid = max(maxid, static_cast<size_t>(max(u,v)));
        }
        Graph G(maxid+1);
        for (auto &e: edges) G.addEdge(e.first, e.second);
        return G;
    }
};

// Dinic max-flow implementation with separate vectors for edge data
class Dinic {
public:
    size_t N, s, t;
    vector<vector<int>> to, rev;
    vector<vector<long>> cap; // Changed to long from long long
    vector<int> level, ptr;
    Dinic(size_t _N, size_t _s, size_t _t): N(_N), s(_s), t(_t), to(N), cap(N), rev(N), level(N), ptr(N) {}
    void addEdge(int u, int v, long c) {
        to[u].push_back(v);
        cap[u].push_back(c);
        rev[u].push_back(to[v].size());
        to[v].push_back(u);
        cap[v].push_back(0);
        rev[v].push_back(to[u].size()-1);
    }
    bool bfs() {
        fill(level.begin(), level.end(), -1);
        queue<size_t> q;
        q.push(s);
        level[s] = 0;
        while (!q.empty()) {
            size_t u = q.front();
            q.pop();
            for (size_t i = 0; i < to[u].size(); ++i) {
                if (cap[u][i] > 0 && level[to[u][i]] < 0) {
                    level[to[u][i]] = level[u] + 1;
                    q.push(to[u][i]);
                }
            }
        }
        return level[t] >= 0;
    }
    long dfs(size_t u, long f) {
        if (!f || u == t) return f;
        for (int &cid = ptr[u]; cid < static_cast<int>(to[u].size()); ++cid) {
            if (cap[u][cid] > 0 && level[to[u][cid]] == level[u] + 1) {
                long pushed = dfs(to[u][cid], min(f, cap[u][cid]));
                if (pushed) {
                    cap[u][cid] -= pushed;
                    cap[to[u][cid]][rev[u][cid]] += pushed;
                    return pushed;
                }
            }
        }
        return 0;
    }
    long maxflow() {
        long flow = 0;
        while (bfs()) {
            fill(ptr.begin(), ptr.end(), 0);
            while (long pushed = dfs(s, LONG_MAX))
                flow += pushed;
        }
        return flow;
    }
};

// Enumerate all (h-1)-cliques in G, store in a set
void enumCliques(const Graph &G, int h1, set<vector<int>> &out) {
    size_t n = G.n;
    vector<int> clique;
    function<void(vector<int>&)> dfs = [&](vector<int> &cand) {
        if (clique.size() == static_cast<size_t>(h1)) {
            out.insert(clique);
            return;
        }
        while (!cand.empty()) {
            int v = cand.back();
            cand.pop_back();
            vector<int> nxt;
            for (int u : cand) {
                if (G.adjacencyList[v].count(u))
                    nxt.push_back(u);
            }
            clique.push_back(v);
            dfs(nxt);
            clique.pop_back();
        }
    };
    for (size_t v = 0; v < n; ++v) {
        vector<int> cand;
        for (int u : G.adjacencyList[v]) if (u > static_cast<int>(v)) cand.push_back(u);
        clique = {static_cast<int>(v)};
        dfs(cand);
    }
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <edge-file> <h>\n";
        return 1;
    }

    // Input
    Graph G = Graph::readEdgeList(argv[1]);
    int h = stoi(argv[2]);
    if (h < 2) {
        cerr << "h must be >= 2\n";
        return 1;
    }
    size_t n = G.n;

    // Precompute all (h-1)-cliques
    set<vector<int>> hm1;
    enumCliques(G, h-1, hm1);
    size_t H = hm1.size();

    // Binary search on density alpha
    long double l = 0, u = 0; // Changed to long double
    // upper bound = max clique-degree
    vector<long> cliqueDeg(n, 0); // Changed to long
    for (const auto &clique : hm1) {
        for (int v : clique) cliqueDeg[v]++;
    }
    u = static_cast<long double>(*max_element(cliqueDeg.begin(), cliqueDeg.end()));

    pair<vector<int>, long double> best = {{}, 0.0}; // Changed to long double
    long double eps = 1.0L / (n * (n - 1)); // Changed to long double
    while (u - l > eps) {
        long double alpha = (l + u) / 2; // Changed to long double

        // Build flow network
        size_t N = 1 + n + H + 1;
        size_t s = 0, t = N - 1;
        Dinic din(N, s, t);
        // s->v with cap cliqueDeg[v]
        for (size_t v = 0; v < n; ++v) din.addEdge(s, static_cast<int>(v + 1), cliqueDeg[v]); // Corrected
        // v->t with cap alpha*h
        for (size_t v = 0; v < n; ++v) din.addEdge(static_cast<int>(v + 1), static_cast<int>(n + H + 1), static_cast<long>(alpha * h));
        // (h-1)-clique nodes: index 1+n ... 1+n+H-1
        size_t i = 0;
        for (const auto &clique : hm1) {
            size_t cid = 1 + n + i;
            for (int v : clique) din.addEdge(static_cast<int>(cid), static_cast<int>(v + 1), LONG_MAX);
            ++i;
        }
        for (size_t v = 0; v < n; ++v) {
            i = 0;
            for (const auto &clique : hm1) {
                size_t cid = 1 + n + i;
                // Check if v completes clique
                bool inClique = false;
                for (int u2 : clique) if (u2 == static_cast<int>(v)) { inClique = true; break; }
                if (inClique) { ++i; continue; }
                // v adjacent to all in clique?
                bool ok = true;
                for (int u2 : clique) {
                    if (!G.adjacencyList[v].count(u2)) { ok = false; break; }
                }
                if (ok) din.addEdge(static_cast<int>(v + 1), static_cast<int>(cid), 1);
                ++i;
            }
        }
        // Compute flow and min-cut
        long flow = din.maxflow();
        // Total cap from source = sum cliqueDeg[v]
        long total = accumulate(cliqueDeg.begin(), cliqueDeg.end(), 0L);

        if (flow < total) {
            // Feasible: extract S side
            vector<bool> vis(N, false); // Changed to vector<bool>
            queue<size_t> q;
            vis[s] = true;
            q.push(s);
            while (!q.empty()) {
                size_t x = q.front();
                q.pop();
                for (size_t j = 0; j < din.to[x].size(); ++j) {
                    if (din.cap[x][j] > 0 && !vis[din.to[x][j]]) {
                        vis[din.to[x][j]] = true;
                        q.push(din.to[x][j]);
                    }
                }
            }
            vector<int> sub;
            for (size_t v = 0; v < n; ++v) if (vis[v + 1]) sub.push_back(static_cast<int>(v));
            best = {sub, alpha};
            l = alpha;
        } else {
            u = alpha;
        }
    }

    // Output the vertex list of the densest subgraph
    cout << "Densest subgraph size = " << best.first.size() << endl << " Vertices:";
    for (int v : best.first) cout << " " << v;
    cout << endl;

    size_t sz = best.first.size();
    // Compute edge-density of the returned subgraph
    unordered_set<int> inSub(best.first.begin(), best.first.end());
    long edgeCount = 0;
    for (int v : best.first) {
        for (int u : G.adjacencyList[v]) {
            if (inSub.count(u) && u > v) {
                edgeCount++;
            }
        }
    }
    long double edgeDensity = static_cast<long double>(edgeCount) / sz; // Changed to long double
    cout << "Edge-density = " << edgeDensity << endl;

    // Compute h-clique-density of the returned subgraph
    // Build induced subgraph S with remapped vertices
    Graph S(sz);
    unordered_map<int, int> rmap;
    for (size_t i = 0; i < sz; ++i) rmap[best.first[i]] = static_cast<int>(i);
    for (int v : best.first) {
        for (int u : G.adjacencyList[v]) {
            if (inSub.count(u) && rmap[u] > rmap[v]) {
                S.addEdge(rmap[v], rmap[u]);
            }
        }
    }
    set<vector<int>> subCliques;
    enumCliques(S, h, subCliques);
    long cCount = subCliques.size();
    long double cliqueDensity = static_cast<long double>(cCount) / sz; // Changed to long double
    cout << h << "-clique-density = " << cliqueDensity
         << " (" << cCount << " cliques over " << sz << " vertices) " << endl;

    return 0;
}
