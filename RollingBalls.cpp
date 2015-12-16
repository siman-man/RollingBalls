#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <time.h>
#include <string>
#include <string.h>
#include <sstream>
#include <cassert>
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

const int DY[4] = {0, 1, 0, -1};
const int DX[4] = {-1, 0, 1, 0};

const int MAX_HEIGHT = 60;
const int MAX_WIDTH = 60;
const int MAX_STATUS = 12;

// ビーム幅
const int BEAM_WIDTH = 100;
// 探索の深さ
const int BEAM_DEPTH = 3;

// 高さ
int g_height;
// 横幅
int g_width;
// ボールの数
int g_total_ball_count;

string int2string(int number){
  stringstream ss; 
  ss << number;
  return ss.str();
}

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

// クエリ情報
struct QUERY {
  int y;
  int x;
  int direct;

  string to_s(){
    string str = "";
    str += int2string(this->y) + " ";
    str += int2string(this->x) + " ";
    str += int2string(this->direct);

    return str;
  }
};

// 迷路
vector< vector<int> > g_maze;
// 保存用の迷路
vector< vector<int> > g_temp_maze;
// 目標の盤面
vector< vector<int> > g_target;

// zoblish作成用
ll g_zoblish_field[MAX_HEIGHT][MAX_WIDTH][MAX_STATUS];

inline int char2int(char ch){
  return ch - '0';
}


unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

struct NODE {
  string operation;           // ボールの操作
  vector< vector<int> > maze; // 盤面
  double eval;                // 評価値
  ll hash;                    // 盤面のハッシュ値

  bool operator >(const NODE &e) const{
    return eval < e.eval;
  }   
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
     * @param start 初期盤面
     */
    void init_maze(vector<string> start){
      g_maze = vector< vector<int> >(g_height, vector<int>(g_width));
      g_total_ball_count = 0;

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
            g_total_ball_count += 1;
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
     * @param start 初期盤面
     * @param target 目標とする盤面
     * @return ボールの操作クエリの一覧
     */
    vector<string> restorePattern(vector<string> start, vector<string> target){
      vector<string> query_list;

      init(start, target);

      for(int i = 0; i < 20; i++){
        string query = get_best_query();
        query_list.push_back(query);
      }

      return query_list;
    }

    /**
     * 現在の盤面から一番ベストなボールの操作を取得する
     * @return query ボールの操作クエリ
     */
    string get_best_query(){
      string query;
      map<ll, bool> check_list;

      for(int depth = 0; depth < BEAM_DEPTH; depth++){
        // ボールの一覧を取得
        vector<BALL> ball_list = get_ball_list();

        // 現状の迷路を保存
        save_maze();

        priority_queue< NODE, vector<NODE>, greater<NODE> > pque;

        // 全てのボールに対して、各方向に転がす
        for(int ball_id = 0; ball_id < g_total_ball_count; ball_id++){
          BALL ball = ball_list[ball_id];

          for(int direct = 0; direct < 4; direct++){
            if(roll_ball(ball.y, ball.x, direct)){
              ll hash = get_zoblish_hash();

              // 既に調べた盤面以外は評価を行わない
              if(!check_list[hash]){
                check_list[hash] = true;
              }
              rollback_maze();
            }
          }
        }
      }

      return query;
    }

    /**
     * ボールを転がす
     * @param y y座標
     * @param x x座標
     * @param direct 転がす方向
     * @return (true: 1マス以上進んだ, false: 全く動かない)
     */
    bool roll_ball(int y, int x, int direct){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      while(is_inside(ny, nx) && g_maze[ny][nx] == WALL){
        ny += DY[direct];
        nx += DX[direct];
      }
      ny -= DY[direct];
      nx -= DX[direct];

      assert(g_maze[ny][nx] == EMPTY);

      if(ny == y && nx == x) return false;

      swap(g_maze[ny][nx], g_maze[y][x]);

      return true;
    }

    /**
     * スコアの取得を行う
     * @return score スコア
     */
    double get_score(){
      double score = 0.0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_target[y][x] != WALL && g_target[y][x] != EMPTY){
            if(g_maze[y][x] == g_target[y][x]){
              score += 1.0;
            }else if(g_maze[y][x] != WALL && g_maze[y][x] != EMPTY){
              score += 0.5;
            }
          }
        }
      }

      return score/g_total_ball_count;
    }

    /**
     * ボールの操作コマンドを生成
     * @param y y座標
     * @param x x座標
     * @param direct 転がす方向
     */
    string create_query(int y, int x, int direct){
      string query = "";
      query += int2string(y) + " ";
      query += int2string(x) + " ";
      query += int2string(direct);

      return query;
    }

    /**
     * フィールドの内側かどうかを判定
     * @param y y座標
     * @param x x座標
     * @return (true: 内側, false: 外側)
     */
    inline bool is_inside(int y, int x){
      return (0 <= y && y < g_height && 0 <= x && x < g_width);
    }

    /**
     * フィールドの外側かどうかを判定
     * @param y y座標
     * @param x x座標
     * @return (true: 外側, false: 内側)
     */
    inline bool is_outside(int y, int x){
      return (y < 0 || g_height <= y || x < 0 || g_width <= x);
    }

    /**
     * 迷路を保存
     */
    void save_maze(){
      g_temp_maze = g_maze;
    }

    /**
     * 保存してた迷路を戻す
     */
    void rollback_maze(){
      g_maze = g_temp_maze;
    }

    /**
     * zoblish hash用の値を初期化する
     */
    void init_zoblish_field(){
      for(int y = 0; y < MAX_HEIGHT; y++){
        for(int x = 0; x < MAX_WIDTH; x++){
          for(int status = 0; status < MAX_STATUS; status++){
            g_zoblish_field[y][x][status] = xor128();
          }
        }
      }
    }

    /**
     * zoblish hashを作成
     */
    ll get_zoblish_hash(){
      ll hash = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int status = g_maze[y][x];
          hash ^= g_zoblish_field[y][x][status];
        }
      }

      return hash;
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
