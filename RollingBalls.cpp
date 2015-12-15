#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <time.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;

const int WALL = 10;
const int EMPTY = 11;

// 高さ
int g_height;
// 横幅
int g_width;

// 迷路
vector< vector<int> > g_maze;

// 目標
vector< vector<int> > g_target;

inline int char2int(char ch){
  return ch - '0';
}

class RollingBalls {
  public:
    void init(vector<string> start, vector<string> target){
      g_height = start.size();
      g_width = start[0].size();

      init_maze(start);
      init_target(target);
    }

    void init_maze(vector<string> start){
      g_maze = vector< vector<int> >(g_height, vector<int>(g_width));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          char ch = start[y][x];

          if(ch == '#'){
            g_maze[y][x] = WALL;
          }else if(ch == '.'){
            g_maze[y][x] = EMPTY;
          }else{
            int color = char2int(ch);
            g_maze[y][x] = color;
          }
        }
      }
    }

    void init_target(vector<string> target){
      g_target = vector< vector<int> >(g_height, vector<int>(g_width));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          char ch = target[y][x];

          if(ch == '#'){
            g_target[y][x] = WALL;
          }else if(ch == '.'){
            g_target[y][x] = EMPTY;
          }else{
            int color = char2int(ch);
            g_target[y][x] = color;
          }
        }
      }
    }

    vector<string> restorePattern(vector<string> start, vector<string> target){
      vector<string> result;

      init(start, target);

      return result;
    }
};

int main(){
  int h;
  string str;
  vector<string> start;
  vector<string> target;
  cin >> h;

  for(int i = 0; i < h; i++){
    cin >> str;
    start.push_back(str);
  }
  for(int i = 0; i < h; i++){
    cin >> str;
    target.push_back(str);
  }
  RollingBalls rb;
  vector<string> ret = rb.restorePattern(start, target);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
