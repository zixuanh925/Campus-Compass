#include "CampusCompass.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <limits>
#include <sstream>
#include <string>

using namespace std;

// ===== Constructor ===== //
CampusCompass::CampusCompass() {
}

// ===== Output ===== //
string CampusCompass::getOutput() {
    string result = outputBuf_.str();
    outputBuf_.str("");
    outputBuf_.clear();
    return result;
}

// ===== Graph helpers ===== //
void CampusCompass::addLocation(int id, const string &name) {
    locationName_[id] = name;
    if (adj_.find(id) == adj_.end())
        adj_[id] = {};
}

bool CampusCompass::addEdge(int u, int v, int weight) {
    if (!locationExists(u) || !locationExists(v)) return false;
    adj_[u].push_back({v, weight, false});
    adj_[v].push_back({u, weight, false});
    return true;
}

Edge* CampusCompass::findEdge(int u, int v) {
    auto it = adj_.find(u);
    if (it == adj_.end()) return nullptr;
    for (auto &e : it->second)
        if (e.to == v) return &e;
    return nullptr;
}

const Edge* CampusCompass::findEdge(int u, int v) const {
    auto it = adj_.find(u);
    if (it == adj_.end()) return nullptr;
    for (const auto &e : it->second)
        if (e.to == v) return &e;
    return nullptr;
}

bool CampusCompass::locationExists(int id) const {
    return locationName_.count(id) > 0;
}

// ── Dijkstra (open edges only) ────────────────────────────────

pair<unordered_map<int,int>, unordered_map<int,int>>
CampusCompass::dijkstra(int src) const {
    const int INF = numeric_limits<int>::max();
    unordered_map<int,int> dist, prev;
    for (const auto &kv : locationName_) {
        dist[kv.first] = INF;
        prev[kv.first] = -1;
    }
    dist[src] = 0;

    using P = pair<int,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        auto it = adj_.find(u);
        if (it == adj_.end()) continue;
        for (const auto &e : it->second) {
            if (e.closed) continue;
            if (dist[u] == INF) continue;
            int nd = dist[u] + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }
    return {dist, prev};
}

int CampusCompass::shortestPath(int src, int dst) const {
    if (!locationExists(src) || !locationExists(dst)) return -1;
    auto [dist, prev] = dijkstra(src);
    int d = dist[dst];
    return (d == numeric_limits<int>::max()) ? -1 : d;
}

vector<int> CampusCompass::shortestPathNodes(int src, int dst) const {
    auto [dist, prev] = dijkstra(src);
    if (dist[dst] == numeric_limits<int>::max()) return {};
    vector<int> path;
    for (int cur = dst; cur != -1; cur = prev[cur])
        path.push_back(cur);
    reverse(path.begin(), path.end());
    return path;
}

bool CampusCompass::graphIsConnected(int src, int dst) const {
    auto [dist, prev] = dijkstra(src);
    return dist.count(dst) && dist[dst] != numeric_limits<int>::max();
}

// ── Prim's MST on induced sub-graph ──────────────────────────

int CampusCompass::mstCost(const set<int> &nodes) const {
    if (nodes.size() <= 1) return 0;

    unordered_map<int,int>  key;
    unordered_map<int,bool> inMST;
    for (int n : nodes) { key[n] = numeric_limits<int>::max(); inMST[n] = false; }

    int start = *nodes.begin();
    key[start] = 0;

    using P = pair<int,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push({0, start});

    int total = 0, count = 0;
    while (!pq.empty()) {
        auto [cost, u] = pq.top(); pq.pop();
        if (inMST[u]) continue;
        inMST[u] = true;
        total += cost;
        ++count;

        auto it = adj_.find(u);
        if (it == adj_.end()) continue;
        for (const auto &e : it->second) {
            if (!nodes.count(e.to)) continue;
            if (!inMST[e.to] && e.weight < key[e.to]) {
                key[e.to] = e.weight;
                pq.push({e.weight, e.to});
            }
        }
    }
    return (count == (int)nodes.size()) ? total : -1;
}

// ===== Validation helpers ===== //
bool CampusCompass::validName(const string &name) {
    if (name.empty()) return false;
    for (char c : name)
        if (!isalpha(c) && c != ' ') return false;
    return true;
}

bool CampusCompass::validUFID(int id) {
    return id >= 10000000 && id <= 99999999;
}

bool CampusCompass::validClassCode(const string &code) {
    if (code.size() != 7) return false;
    for (int i = 0; i < 3; ++i) if (!isupper(code[i])) return false;
    for (int i = 3; i < 7; ++i) if (!isdigit(code[i])) return false;
    return true;
}

bool CampusCompass::classCodeExists(const string &code) const {
    return classes_.count(code) > 0;
}

bool CampusCompass::studentExists(int id) const {
    return students_.count(id) > 0;
}

void CampusCompass::pruneStudentIfEmpty(int id) {
    auto it = students_.find(id);
    if (it != students_.end() && it->second.classCodes.empty())
        students_.erase(it);
}

// ===== ParseCSV ===== //
bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    // ── edges.csv ─────────────────────────────────────────────
    {
        ifstream f(edges_filepath);
        if (!f.is_open()) return false;
        string line;
        getline(f, line); // skip header
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            stringstream ss(line);
            string tok;
            vector<string> fields;
            while (getline(ss, tok, ',')) fields.push_back(tok);
            if (fields.size() < 5) continue;
            int id1    = stoi(fields[0]);
            int id2    = stoi(fields[1]);
            string n1  = fields[2];
            string n2  = fields[3];
            int weight = stoi(fields[4]);
            addLocation(id1, n1);
            addLocation(id2, n2);
            addEdge(id1, id2, weight);
        }
    }

    // ── classes.csv ───────────────────────────────────────────
    {
        ifstream f(classes_filepath);
        if (!f.is_open()) return false;
        string line;
        getline(f, line); // skip header
        while (getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            stringstream ss(line);
            string tok;
            vector<string> fields;
            while (getline(ss, tok, ',')) fields.push_back(tok);
            if (fields.size() < 4) continue;
            ClassInfo ci;
            ci.code       = fields[0];
            ci.locationId = stoi(fields[1]);
            ci.startTime  = fields[2];
            ci.endTime    = fields[3];
            classes_[ci.code] = ci;
        }
    }
    return true;
}

// ===== Quoted-name parser ===== //
bool CampusCompass::parseQuotedName(istringstream &ss, string &name) {
    ss >> ws;
    if (ss.peek() != '"') return false;
    ss.get(); // consume opening quote
    return static_cast<bool>(getline(ss, name, '"'));
}

// ===== Command handlers ===== //
// insert
void CampusCompass::cmdInsert(istringstream &ss) {
    string name;
    if (!parseQuotedName(ss, name)) { outputBuf_ << "unsuccessful\n"; return; }

    int ufid, residenceId, n;
    if (!(ss >> ufid >> residenceId >> n)) { outputBuf_ << "unsuccessful\n"; return; }

    vector<string> codes;
    for (int i = 0; i < n; ++i) {
        string c;
        if (!(ss >> c)) { outputBuf_ << "unsuccessful\n"; return; }
        codes.push_back(c);
    }
    if (static_cast<int>(codes.size()) != n) { outputBuf_ << "unsuccessful\n"; return; }

    // Validate
    if (!validName(name) || !validUFID(ufid))    { outputBuf_ << "unsuccessful\n"; return; }
    if (studentExists(ufid))                      { outputBuf_ << "unsuccessful\n"; return; }
    if (!locationExists(residenceId))             { outputBuf_ << "unsuccessful\n"; return; }
    if (codes.empty() || (int)codes.size() > 6)  { outputBuf_ << "unsuccessful\n"; return; }

    for (const auto &c : codes) {
        if (!validClassCode(c) || !classCodeExists(c)) { outputBuf_ << "unsuccessful\n"; return; }
    }
    // Duplicate class codes?
    set<string> unique(codes.begin(), codes.end());
    if (unique.size() != codes.size()) { outputBuf_ << "unsuccessful\n"; return; }

    Student s{name, ufid, residenceId, codes};
    students_[ufid] = s;
    outputBuf_ << "successful\n";
}

// remove
void CampusCompass::cmdRemove(istringstream &ss) {
    int ufid;
    if (!(ss >> ufid)) { outputBuf_ << "unsuccessful\n"; return; }
    auto it = students_.find(ufid);
    if (it == students_.end()) { outputBuf_ << "unsuccessful\n"; return; }
    students_.erase(it);
    outputBuf_ << "successful\n";
}

// dropClass
void CampusCompass::cmdDropClass(istringstream &ss) {
    int ufid; string code;
    if (!(ss >> ufid >> code)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!validClassCode(code) || !classCodeExists(code)) { outputBuf_ << "unsuccessful\n"; return; }

    auto it = students_.find(ufid);
    if (it == students_.end()) { outputBuf_ << "unsuccessful\n"; return; }

    auto &codes = it->second.classCodes;
    auto pos = find(codes.begin(), codes.end(), code);
    if (pos == codes.end()) { outputBuf_ << "unsuccessful\n"; return; }

    codes.erase(pos);
    pruneStudentIfEmpty(ufid);
    outputBuf_ << "successful\n";
}

// replaceClass
void CampusCompass::cmdReplaceClass(istringstream &ss) {
    int ufid; string c1, c2;
    if (!(ss >> ufid >> c1 >> c2)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!validClassCode(c1) || !validClassCode(c2)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!classCodeExists(c2)) { outputBuf_ << "unsuccessful\n"; return; }

    auto it = students_.find(ufid);
    if (it == students_.end()) { outputBuf_ << "unsuccessful\n"; return; }

    auto &codes = it->second.classCodes;
    auto posOld = find(codes.begin(), codes.end(), c1);
    if (posOld == codes.end()) { outputBuf_ << "unsuccessful\n"; return; }
    if (find(codes.begin(), codes.end(), c2) != codes.end()) { outputBuf_ << "unsuccessful\n"; return; }

    *posOld = c2;
    outputBuf_ << "successful\n";
}

// removeClass
void CampusCompass::cmdRemoveClass(istringstream &ss) {
    string code;
    if (!(ss >> code)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!validClassCode(code) || !classCodeExists(code)) { outputBuf_ << "unsuccessful\n"; return; }

    int count = 0;
    vector<int> toErase;
    for (auto &[sid, student] : students_) {
        auto &codes = student.classCodes;
        auto pos = find(codes.begin(), codes.end(), code);
        if (pos != codes.end()) {
            codes.erase(pos);
            ++count;
            if (codes.empty()) toErase.push_back(sid);
        }
    }
    for (int sid : toErase) students_.erase(sid);

    if (count == 0) { outputBuf_ << "unsuccessful\n"; return; }
    outputBuf_ << count << "\n";
}

// toggleEdgesClosure
void CampusCompass::cmdToggleEdgesClosure(istringstream &ss) {
    int n;
    if (!(ss >> n)) { outputBuf_ << "unsuccessful\n"; return; }
    for (int i = 0; i < n; ++i) {
        int u, v;
        if (!(ss >> u >> v)) { outputBuf_ << "unsuccessful\n"; return; }
        Edge* uv = findEdge(u, v);
        Edge* vu = findEdge(v, u);
        if (!uv || !vu) { outputBuf_ << "unsuccessful\n"; return; }
        uv->closed = !uv->closed;
        vu->closed = !vu->closed;
    }
    outputBuf_ << "successful\n";
}

// checkEdgeStatus
void CampusCompass::cmdCheckEdgeStatus(istringstream &ss) {
    int u, v;
    if (!(ss >> u >> v)) { outputBuf_ << "unsuccessful\n"; return; }
    const Edge* e = findEdge(u, v);
    if (!e)           outputBuf_ << "DNE\n";
    else if (e->closed) outputBuf_ << "closed\n";
    else              outputBuf_ << "open\n";
}

// isConnected
void CampusCompass::cmdIsConnected(istringstream &ss) {
    int u, v;
    if (!(ss >> u >> v)) { outputBuf_ << "unsuccessful\n"; return; }
    outputBuf_ << (graphIsConnected(u, v) ? "successful" : "unsuccessful") << "\n";
}

// printShortestEdges
void CampusCompass::cmdPrintShortestEdges(istringstream &ss) {
    int ufid;
    if (!(ss >> ufid)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!studentExists(ufid)) { outputBuf_ << "unsuccessful\n"; return; }

    const Student &s = students_.at(ufid);
    vector<string> codes = s.classCodes;
    sort(codes.begin(), codes.end());

    outputBuf_ << "Time For Shortest Edges: " << s.name << "\n";
    for (const auto &code : codes) {
        int t = shortestPath(s.residenceId, classes_.at(code).locationId);
        outputBuf_ << code << ": " << t << "\n";
    }
}

// printStudentZone
void CampusCompass::cmdPrintStudentZone(istringstream &ss) {
    int ufid;
    if (!(ss >> ufid)) { outputBuf_ << "unsuccessful\n"; return; }
    if (!studentExists(ufid)) { outputBuf_ << "unsuccessful\n"; return; }

    const Student &s = students_.at(ufid);

    // Step 1: collect all nodes from shortest paths to each class
    set<int> allNodes;
    allNodes.insert(s.residenceId);
    for (const auto &code : s.classCodes) {
        vector<int> path = shortestPathNodes(s.residenceId,
                                             classes_.at(code).locationId);
        for (int n : path) allNodes.insert(n);
    }

    // Step 2 & 3: MST of induced sub-graph
    int cost = mstCost(allNodes);
    outputBuf_ << "Student Zone Cost For " << s.name << ": " << cost << "\n";
}

// ===== ParseCommand ===== //
bool CampusCompass::ParseCommand(const string &command) {
    istringstream ss(command);
    string cmd;
    if (!(ss >> cmd)) { outputBuf_ << "unsuccessful\n"; return false; }

    if      (cmd == "insert")               cmdInsert(ss);
    else if (cmd == "remove")               cmdRemove(ss);
    else if (cmd == "dropClass")            cmdDropClass(ss);
    else if (cmd == "replaceClass")         cmdReplaceClass(ss);
    else if (cmd == "removeClass")          cmdRemoveClass(ss);
    else if (cmd == "toggleEdgesClosure")   cmdToggleEdgesClosure(ss);
    else if (cmd == "checkEdgeStatus")      cmdCheckEdgeStatus(ss);
    else if (cmd == "isConnected")          cmdIsConnected(ss);
    else if (cmd == "printShortestEdges")   cmdPrintShortestEdges(ss);
    else if (cmd == "printStudentZone")     cmdPrintStudentZone(ss);
    else { outputBuf_ << "unsuccessful\n"; return false; }

    return true;
}