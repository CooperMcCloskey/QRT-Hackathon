#!/usr/bin/env python3
# build.py <core.cpp> <out.cpp> : produce single-file HackerRank submission
import sys
core=open(sys.argv[1]).read().replace('#include "calib.inc"',open(sys.argv[4] if len(sys.argv)>4 else '/home/claude/p5/calib.inc').read()); lm=open('/home/claude/p5/lm.inc').read() if len(sys.argv)<4 else open(sys.argv[3]).read()
head='''#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

'''
tail='''
/*
 * Complete the 'assemble' function below.
 *
 * The function is expected to return a 2D_INTEGER_ARRAY.
 * The function accepts STRING_ARRAY pieces as parameter.
 */

vector<vector<int>> assemble(vector<string> pieces) {
    return DL::solve(pieces);
}

int main()
{
    const char* op = getenv("OUTPUT_PATH");
    ofstream ffile;
    if (op) ffile.open(op);
    ostream& fout = op ? (ostream&)ffile : (ostream&)cout;

    string pieces_count_temp;
    getline(cin, pieces_count_temp);

    int pieces_count = stoi(ltrim(rtrim(pieces_count_temp)));

    vector<string> pieces(pieces_count);

    for (int i = 0; i < pieces_count; i++) {
        string pieces_item;
        getline(cin, pieces_item);

        pieces[i] = pieces_item;
    }

    vector<vector<int>> result = assemble(pieces);

    for (size_t i = 0; i < result.size(); i++) {
        for (size_t j = 0; j < result[i].size(); j++) {
            fout << result[i][j];

            if (j != result[i].size() - 1) {
                fout << " ";
            }
        }

        if (i != result.size() - 1) {
            fout << "\\n";
        }
    }

    fout << "\\n";

    if (op) ffile.close();

    return 0;
}

string ltrim(const string &str) {
    string s(str);
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char c) { return !isspace(c); }));
    return s;
}

string rtrim(const string &str) {
    string s(str);
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !isspace(c); }).base(), s.end());
    return s;
}
'''
open(sys.argv[2],'w').write(head+lm+core+tail)
