/*
#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "CampusCompass.h"

using namespace std;

// ===== Helpers ===== //
static CampusCompass makeCC() {
    CampusCompass cc;
    cc.ParseCSV("data/edges.csv", "data/classes.csv");
    return cc;
}

static string run(CampusCompass &cc, const string &cmd) {
    cc.ParseCommand(cmd);
    return cc.getOutput();
}

// ===== Tests ===== //
TEST_CASE("Incorrect commands print unsuccessful", "[invalid]") {
    auto cc = makeCC();

    // Name contains a digit — invalid
    REQUIRE(run(cc, R"(insert "A11y" 45679999 1 1 COP3530)") == "unsuccessful\n");

    // UFID is only 7 digits — must be exactly 8
    REQUIRE(run(cc, R"(insert "Alice" 9999999 1 1 COP3530)") == "unsuccessful\n");

    // Class code uses lowercase letters — must be 3 uppercase + 4 digits
    REQUIRE(run(cc, R"(insert "Bob" 10000001 1 1 cop3530)") == "unsuccessful\n");

    // N says 3 classes but only 2 are provided
    REQUIRE(run(cc, R"(insert "Carol" 10000002 1 3 COP3530 MAC2311)") == "unsuccessful\n");

    // Completely misspelled command name
    REQUIRE(run(cc, "addStudent \"Dave\" 10000003 1 1 COP3530") == "unsuccessful\n");
}

TEST_CASE("Edge cases for various functions", "[edge]") {
    auto cc = makeCC();

    // Edge case 1: remove a student that does not exist
    REQUIRE(run(cc, "remove 99999999") == "unsuccessful\n");

    // Edge case 2: dropClass on the student's only class removes the student.
    run(cc, R"(insert "Eve" 10000010 1 1 COP3530)");
    REQUIRE(run(cc, "dropClass 10000010 COP3530") == "successful\n");
    REQUIRE(run(cc, "remove 10000010") == "unsuccessful\n");

    // Edge case 3: replaceClass fails when the student already has the new class
    run(cc, R"(insert "Frank" 10000011 1 2 COP3530 MAC2311)");
    REQUIRE(run(cc, "replaceClass 10000011 COP3530 MAC2311") == "unsuccessful\n");
}

TEST_CASE("dropClass, removeClass, remove, and replaceClass commands", "[commands]") {
    auto cc = makeCC();

    run(cc, R"(insert "Grace" 20000001 1 2 COP3530 MAC2311)");
    run(cc, R"(insert "Henry" 20000002 1 2 COP3530 CDA3101)");
    run(cc, R"(insert "Iris"  20000003 1 1 COP3530)");

    // dropClass: Grace drops COP3530, keeps MAC2311
    REQUIRE(run(cc, "dropClass 20000001 COP3530") == "successful\n");
    // dropClass nonexistent class for that student fails
    REQUIRE(run(cc, "dropClass 20000001 COP3530") == "unsuccessful\n");

    // replaceClass: Henry swaps CDA3101 for MAC2311
    REQUIRE(run(cc, "replaceClass 20000002 CDA3101 MAC2311") == "successful\n");
    // replaceClass with a class code not in the CSV fails
    REQUIRE(run(cc, "replaceClass 20000002 COP3530 ZZZ9999") == "unsuccessful\n");

    // remove: Iris is removed successfully
    REQUIRE(run(cc, "remove 20000003") == "successful\n");
    // remove same student again fails
    REQUIRE(run(cc, "remove 20000003") == "unsuccessful\n");

    // removeClass COP3530: only Henry still has it (Grace dropped it, Iris removed)
    REQUIRE(run(cc, "removeClass COP3530") == "1\n");
    // removeClass on a code no one is enrolled in fails
    REQUIRE(run(cc, "removeClass COP3530") == "unsuccessful\n");
}

TEST_CASE("printShortestEdges: reachable then blocked by edge closures", "[path]") {
    auto cc = makeCC();

    run(cc, R"(insert "Jake" 30000001 1 1 IDS2935)");

    // Before any closures: Jake can reach node 7
    string before = run(cc, "printShortestEdges 30000001");
    // Output must contain the class code and a positive time (13)
    REQUIRE(before.find("IDS2935") != string::npos);
    REQUIRE(before.find("13") != string::npos);
    REQUIRE(before.find("-1") == string::npos);

    // Close all four edges adjacent to node 7
    run(cc, "toggleEdgesClosure 4 5 7 7 8 43 7 49 7");

    // After closures: node 7 is isolated, time must be -1
    string after = run(cc, "printShortestEdges 30000001");
    REQUIRE(after.find("IDS2935") != string::npos);
    REQUIRE(after.find("-1") != string::npos);
}
*/
