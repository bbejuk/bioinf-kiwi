#include <vector>
#include <map>
#include <iostream>
#include <typeinfo>
#include <random>
#include <algorithm>

#include "monteCarlo.h"

using namespace std;

map<float, vector<vector<tuple<string, int> > > > monteCarloWrapper(vector<string> keysCR, map<string, map<string, vector<vector<float> > > > groupedCR, vector<string> keysRR,
                                                                    map<string, map<string, vector<vector<float> > > > groupedRR, int maxDepth, int nTimes) {

    map<float, vector<vector<tuple<string, int> > > > pathsMap;
    vector<vector<tuple<string, int> > > paths;

    float extensionSide = 0.0;

    for( auto key : keysCR) {
        paths = monteCarlo(key, extensionSide, keysCR, groupedCR, keysRR, groupedRR, maxDepth, nTimes);
        for( auto path : paths) {
            pathsMap[extensionSide].push_back(path);
        }
    }

    extensionSide = 1.0;
    for( auto key : keysCR) {
        paths = monteCarlo(key, extensionSide, keysCR, groupedCR, keysRR, groupedRR, maxDepth, nTimes);
        for( auto path : paths) {
            pathsMap[extensionSide].push_back(path);
        }
    }

    return pathsMap;
}

vector<vector<tuple<string, int> > > monteCarlo(string start, float side, vector<string> keysCR, map<string, map<string, vector<vector<float> > > > groupedCR,
                        vector<string> keysRR, map<string, map<string, vector<vector<float> > > > groupedRR, int maxDepth, int nTimes){

    vector<vector<tuple<string, int> > > paths;
    vector<tuple<string, int> > path;
    // int nTimes = 1000;
    int nGoals = 0;
    for( int i = 0; i < nTimes; i++){
        cout << "Trial:" << i << endl;
        path = mcSearch(start, side, keysCR, groupedCR, keysRR, groupedRR, maxDepth);
        // vector<tuple<string, int> >::iterator row;
        // vector<string>::iterator col;
        // for (row = paths.begin(); row != paths.end(); row++) {
            // cout << paths[row][0] << '\n';
            // allPaths.push_back(paths[row]);
            // for (col = row->begin(); col != row->end(); col++) {
                // do stuff ...
            // }
        // }
        if( path.size() > 1){
            nGoals += 1;
            paths.push_back(path);
        }
        cout << "nGoals: " << nGoals << endl;
        // string read;
        // int number;
        // for( auto element : path) {
            // tie(read, number) = element;
            // cout << read << " " << number << '\n';
        // }
    }

    return paths;
}


vector<tuple<string, int> > mcSearch(string start, float side, vector<string> keysCR, map<string, map<string, vector<vector<float> > > > groupedCR,
                        vector<string> keysRR, map<string, map<string, vector<vector<float> > > > groupedRR, int maxDepth) {

    vector<tuple<string, int> > path;
    vector<string> stack;
    string read;
    int number;

    path.push_back(make_tuple(start, -1));

    tie(read, number) = getMCReadForContig(start, side, groupedCR);
    // cout << read << ' ' << number << endl;
    path.push_back(make_tuple(read, number));

    stack.push_back(read);
    while( !stack.empty()) {

        string currentTarget = stack.back();
        stack.pop_back();
        // cout << path.size() << endl;
        if( path.size() <= maxDepth) {
            if ( std::find(keysCR.begin(), keysCR.end(), currentTarget) != keysCR.end() ) {
                // path.push_back(make_tuple(currentTarget, -1));
                return path;
            }
            else {
                tie(read, number) = getMCReadForRead(currentTarget, side, start, groupedCR, groupedRR);
                if( number == -2) {
                    break;
                }
                // cout << "Nasel sam:" << currentTarget << ' ' << read << number << endl;
                path.push_back(make_tuple(read, number));
                stack.push_back(read);
            }
        }
    }
    path = {make_tuple("", -2)};
    return path;
}

tuple<string, int> getMCReadForContig(string contig, float side, map<string, map<string, vector<vector<float> > > > groupedCR) {

    float sum = 0.0;

    for( auto query : groupedCR[contig]){
        for( int i = 0; i < query.second.size(); i++) {
            // cout << i << endl;
            // pick ES
            if( side == query.second[i][0]){
                if( query.second[i][1] > 0) {
                    sum += query.second[i][1];
                    // break;
                }
            }
        }
    }

    float offset = 0.0;
    random_device rand_dev;
    mt19937 generator(rand_dev());
    uniform_int_distribution<int> distr(0, sum);
    int pick = distr(generator);

    for( auto query : groupedCR[contig]){
        for( int i = 0; i < query.second.size(); i++) {
            // pick ES
            if( side == query.second[i][0]){
                if( query.second[i][1] > 0) {
                    offset += query.second[i][1];
                    if( offset > pick) {
                        return make_tuple(query.first, i);
                    }
                }
            }
        }
    }

    return make_tuple("", -1);
}

tuple<string, int> getMCReadForRead(string read, float side, string startContig, map<string, map<string, vector<vector<float> > > > groupedCR, map<string, map<string, vector<vector<float> > > > groupedRR) {

    // try to find contig == goal
    for( auto target : groupedCR) {
        if( target.first == startContig) {
            continue;
        }
        for( auto query : target.second){
            if( query.first != read) {
                continue;
            }
            for( int i = 0; i < query.second.size(); i++) {
                // if extends from other side
                if( side != query.second[i][0]){
                    // cout << "GOAL FOUND!" << target.first << endl;
                    return make_tuple(target.first, -1);
                }
            }
        }
    }


    float sum = 0.0;

    for( auto query : groupedRR[read]){
        for( int i = 0; i < query.second.size(); i++) {
            // pick ES
            if( side == query.second[i][0]){
                if( query.second[i][1] > 0) {
                    sum += query.second[i][1];
                    // break;
                }
            }
        }
    }

    float offset = 0.0;
    random_device rand_dev;
    mt19937 generator(rand_dev());
    uniform_int_distribution<int> distr(0, sum);
    int pick = distr(generator);

    for( auto query : groupedRR[read]){
        for( int i = 0; i < query.second.size(); i++) {
            // cout << read << query.first << endl;
            // cout << side << " " << query.second[i][0] << endl;
            if( side == query.second[i][0]){
                if( query.second[i][1] > 0) {
                    offset += query.second[i][1];
                    if( offset > pick) {
                        // cout << "Read found!" << query.first << endl;
                        return make_tuple(query.first, i);
                    }
                }
            }
        }
    }

    return make_tuple("", -2);
}
