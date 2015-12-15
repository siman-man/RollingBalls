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

const int UNKNOWN = -1;
const int WALL = 10;
const int EMPTY = 11;

// 高さ
int g_height;
// 横幅
int g_width;

// ボールの情報
struct BALL {
  int y;
  int x;
  int color;

  BALL(int y = UNKNOWN, int x = UNKNOWN, int color = UNKNOWN){
    this->y = y;
    this->x = x;
    this->color = color;
  }
};

// 迷路
vector< vector<int> > g_maze;

// 目標
vector< vector<int> > g_target;

inline int char2int(char ch){
  return ch - '0';
}

string int2string(int number){
  stringstream ss; 
  ss << number;
  return ss.str();
}

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

struct NODE {
  string operation; // ボールの操作
  NODE *parent;     // 親のノード
  ll hash;          // 盤面のハッシュ値
};

class RollingBalls {
  public:
    void init(vector<string> start, vector<string> target){
      g_height = start.size();
      g_width = start[0].size();

      init_maze(start);
      init_target(target);
    }

    /**
     * mazeの初期化
     */
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

    /**
     * ボールのリストを作成
     */
    vector<BALL> get_ball_list(){
      vector<BALL> ball_list;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];

          if(0 <= color && color <= 9){
            ball_list.push_back(BALL(y, x, color));
          }
        }
      }

      return ball_list;
    }

    /**
     * targetフィールドの初期化
     */
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

    /**
     * ソルバー
     */
    vector<string> restorePattern(vector<string> start, vector<string> target){
      vector<string> result;

      init(start, target);

      return result;
    }

    /**
     * ボールを転がす
     */
    void roll_ball(int y, int x, int direct){
    }

    /**
     * ボールの操作コマンドを生成
     */
    string create_query(int y, int x, int direct){
      string query = "";
      query += int2string(y) + " ";
      query += int2string(x) + " ";
      query += int2string(direct);

      return query;
    }
};

int main(){
  int h;
  string str;
  vector<string> start, target;
  cin >> h;
  for(int i=0;i<h;i++){cin >> str;start.push_back(str);}
  for(int i=0;i<h;i++){cin >> str;target.push_back(str);}
  RollingBalls rb;
  vector<string> ret = rb.restorePattern(start, target);
  cout << ret.size() << endl;
  for(int i=0;i<ret.size();i++){cout << ret[i] << endl;}
  return 0;
}
