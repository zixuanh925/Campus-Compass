#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <sstream>

using namespace std;

// ===== Internal structure ===== //
struct Edge {
    int to;
    int weight;
    bool closed = false;
};

struct ClassInfo {
    string code;
    int locationId;
    string startTime;
    string endTime;
};

struct Student {
    string name;
    int id;
    int residenceId;
    vector<string> classCodes;
};


class CampusCompass {
private:
    // Graph (adjacency list)
    unordered_map<int, vector<Edge>> adj_;
    unordered_map<int, string> locationName_;

    // Domain data
    map<string, ClassInfo> classes_;   // classCode -> ClassInfo
    map<int, Student> students_;  // studentId -> Student

    // Output
    ostringstream outputBuf_;

    // Graph helpers
    void addLocation(int id, const string &name);
    bool addEdge(int u, int v, int weight);
    Edge* findEdge(int u, int v);
    const Edge* findEdge(int u, int v) const;
    bool locationExists(int id) const;

    pair<unordered_map<int,int>, unordered_map<int,int>> dijkstra(int src) const;
    int shortestPath(int src, int dst) const;
    vector<int> shortestPathNodes(int src, int dst) const;
    bool graphIsConnected(int src, int dst) const;
    int mstCost(const set<int> &nodes) const;

    // Student helpers
    void pruneStudentIfEmpty(int id);

    // Validation helpers
    static bool validName(const string &name);
    static bool validUFID(int id);
    static bool validClassCode(const string &code);
    bool classCodeExists(const string &code) const;
    bool studentExists(int id) const;

    // Command handlers
    void cmdInsert(istringstream &ss);
    void cmdRemove(istringstream &ss);
    void cmdDropClass(istringstream &ss);
    void cmdReplaceClass(istringstream &ss);
    void cmdRemoveClass(istringstream &ss);
    void cmdToggleEdgesClosure(istringstream &ss);
    void cmdCheckEdgeStatus(istringstream &ss);
    void cmdIsConnected(istringstream &ss);
    void cmdPrintShortestEdges(istringstream &ss);
    void cmdPrintStudentZone(istringstream &ss);

    static bool parseQuotedName(istringstream &ss, string &name);

public:
    CampusCompass();

    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);

    // Parses and executes one command line; returns true if command was valid.
    bool ParseCommand(const string &command);

    // Returns (and clears) all output produced since last call.
    string getOutput();
};


