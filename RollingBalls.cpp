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
const int BEAM_DEPTH = 2;

// 高さ
int g_height;
// 横幅
int g_width;
// ボールの数
int g_total_ball_count;

/**
 * 数値から文字列へ
 * @param number 数値
 * @return numberを文字列化したもの
 */
string int2string(int number){
  stringstream ss; 
  ss << number;
  return ss.str();
}

/**
 * 文字を数値に
 * @param ch '0'-'9'のいずれかの文字
 * @return 数値
 */
inline int char2int(char ch){
  return ch - '0';
}

// 座標情報
struct COORD {
  int y;
  int x;

  COORD(int y, int x){
    this->y = y;
    this->x = x;
  }
};

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

  QUERY(int y = UNKNOWN, int x = UNKNOWN, int direct = UNKNOWN){
    this->y = y;
    this->x = x;
    this->direct = direct;
  }

  string to_s(){
    string str = "";
    str += int2string(this->y) + " ";
    str += int2string(this->x) + " ";
    str += int2string(this->direct);

    return str;
  }
};

struct NODE {
  QUERY query;                // ボールの操作
  vector< vector<int> > maze; // 盤面
  double eval;                // 評価値
  ll hash;                    // 盤面のハッシュ値

  bool operator >(const NODE &e) const{
    return eval < e.eval;
  }   
};

// 迷路
vector< vector<int> > g_maze;
// 保存用の迷路
vector< vector<int> > g_temp_maze;
// 目標の盤面
vector< vector<int> > g_target;
// 評価用の盤面
vector< vector<int> > g_eval_field;

// zoblish作成用
ll g_zoblish_field[MAX_HEIGHT][MAX_WIDTH][MAX_STATUS];

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

class RollingBalls {
  public:
    void init(vector<string> start, vector<string> target){
      g_height = start.size();
      g_width = start[0].size();

      init_zoblish_field();
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

      for(int i = 0; i < g_total_ball_count * 20; i++){
        QUERY query = get_best_query();
        roll(query.y, query.x, query.direct);

        query_list.push_back(query.to_s());
      }

      fprintf(stderr,"\n");
      fprintf(stderr,"current score = %4.2f\n", get_score());

      return query_list;
    }

    /**
     * 現在の盤面から一番ベストなボールの操作を取得する
     * @return query ボールの操作クエリ
     */
    QUERY get_best_query(){
      map<ll, bool> check_list;
      // 一番最初のハッシュを取得(後はこれに対して差分更新)
      ll root_hash = get_zoblish_hash();
      // 一番最初の盤面を保存
      vector< vector<int> > g_root_maze = g_maze;

      NODE best_node;
      double max_eval = INT_MIN;

      // 評価する盤面のキュー
      queue<NODE> node_queue;

      // rootなノードを作成
      NODE root_node = create_node();
      root_node.hash = root_hash;

      // 初期のキューに追加
      node_queue.push(root_node);

      check_list[root_hash] = true;

      for(int depth = 0; depth < BEAM_DEPTH; depth++){
        priority_queue< NODE, vector<NODE>, greater<NODE> > pque;
        //fprintf(stderr,"depth = %d, queue size = %lu\n", depth, node_queue.size());

        // 候補の盤面が空になるまで繰り返す
        while(!node_queue.empty()){
          // 親の盤面を取得
          NODE parent = node_queue.front(); node_queue.pop();

          // ボールの一覧を取得
          vector<BALL> ball_list = get_ball_list();

          // 全てのボールに対して処理を行う
          for(int ball_id = 0; ball_id < g_total_ball_count; ball_id++){
            BALL ball = ball_list[ball_id];

            // 4方向にコロコロ
            for(int direct = 0; direct < 4; direct++){
              COORD coord = roll_ball(ball.y, ball.x, direct);

              // ボールが1マスも進んでいない場合は処理を飛ばす
              if(coord.y == ball.y && coord.x == ball.x) continue;
              //fprintf(stderr,"(%d, %d, %d) corocoro... to (%d, %d)\n", ball.y, ball.x, direct, coord.y, coord.x);

              // ボールをコロコロ
              swap(g_maze[ball.y][ball.x], g_maze[coord.y][coord.x]);
              // ハッシュ値を再計算
              ll new_hash = update_zoblish_hash(parent.hash, ball.y, ball.x, ball.color, coord.y, coord.x, ball.color);

              // 既に調べた盤面以外は評価を行わない
              if(!check_list[new_hash]){
                check_list[new_hash] = true;

                // 子ノードを作成
                NODE child = create_node();
                child.hash = new_hash;
                child.eval = get_score();

                // 初期の探索の時はクエリを作成 
                if(depth == 0){
                  child.query = QUERY(ball.y, ball.x, direct);

                // それ以外は親のクエリを引き継ぐ
                }else{
                  child.query = parent.query;
                }

                // 候補に追加
                pque.push(child);
              }else{
                //fprintf(stderr,"already checked...\n");
              }

              // 再度ボールをコロコロ(2回swapさせることで元の盤面に戻す)
              swap(g_maze[ball.y][ball.x], g_maze[coord.y][coord.x]);
            }
          }
        }

        // ビーム幅の数だけ盤面を残す
        for(int i = 0; i < BEAM_WIDTH && !pque.empty(); i++){
          NODE node = pque.top(); pque.pop();
          node_queue.push(node);

          // 探索中に一番評価値が高いやつを残す
          if(max_eval < node.eval){
            max_eval = node.eval;
            best_node = node;
          }
        }
      }

      // 元に戻す
      g_maze = g_root_maze;

      assert(max_eval != INT_MIN);

      return best_node.query;
    }

    /**
     * ボールを転がす
     * @param y y座標
     * @param x x座標
     * @param direct 転がす方向
     * @return coord 転がした後のボールの位置
     */
    COORD roll_ball(int y, int x, int direct){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      while(is_inside(ny, nx) && g_maze[ny][nx] == EMPTY){
        ny += DY[direct];
        nx += DX[direct];
      }
      ny -= DY[direct];
      nx -= DX[direct];

      if(is_inside(ny, nx)){
        return COORD(ny, nx);
      }else{
        return COORD(y, x);
      }
    }

    /**
     * 実際にボールを転がす
     */
    void roll(int y, int x, int direct){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      while(is_inside(ny, nx) && g_maze[ny][nx] == EMPTY){
        ny += DY[direct];
        nx += DX[direct];
      }

      ny -= DY[direct];
      nx -= DX[direct];

      swap(g_maze[y][x], g_maze[ny][nx]);
    }

    /**
     * スコアの取得を行う
     * @return score スコア
     */
    double get_score(){
      double score = 0.0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];
          int target_color = g_target[y][x];

          if(is_ball(color) && is_ball(target_color)){
            // 色が一致していたら1pt
            if(color == target_color){
              score += 1.0;
            // そうでない場合は0.5pt
            }else{
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
     * ノードを作成する
     */
    NODE create_node(){
      NODE node;

      node.maze = g_maze;

      return node;
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
     * ボールかどうかを判定する
     * @param color 色
     * @return (true: ボール, false: ボールじゃない)
     */
    inline bool is_ball(int color){
      return (color != WALL && color != EMPTY);
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
     * 色の着いたボールの位置だけに対して乱数をxorする
     */
    ll get_zoblish_hash(){
      ll hash = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];

          if(is_ball(color)){
            hash ^= g_zoblish_field[y][x][color];
          }
        }
      }

      return hash;
    }

    /**
     * zoblish hashの更新
     * @param y1 移動前のボールのy座標
     * @param x1 移動前のボールのx座標
     * @param c1 移動前のボールの色
     * @param y2 移動後のボールのy座標
     * @param x2 移動後のボールのx座標
     * @param c2 移動後のボールの色
     * @return 更新されたハッシュ値
     */
    ll update_zoblish_hash(ll hash, int y1, int x1, int c1, int y2, int x2, int c2){

      // 移動前のボールの位置を消して
      hash ^= g_zoblish_field[y1][x1][c1];
      // 移動後のボールの位置に入れてあげる
      hash ^= g_zoblish_field[y2][x2][c2];

      return hash;
    }

    /**
     * 迷路を表示する
     */
    void show_maze(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];

          if(color == WALL){
            fprintf(stderr,"#");
          }else if(color == EMPTY){
            fprintf(stderr,".");
          }else{
            fprintf(stderr,"%d", color);
          }
        }
        fprintf(stderr,"\n");
      }
    }

    /**
     * 目標の迷路を表示する
     */
    void show_target(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_target[y][x];

          if(color == WALL){
            fprintf(stderr,"#");
          }else if(color == EMPTY){
            fprintf(stderr,".");
          }else{
            fprintf(stderr,"%d", color);
          }
        }
        fprintf(stderr,"\n");
      }
    }
};

int main(){
  int h;
  string str;
  vector<string> start, target;
  cin >> h;
  for(int i=0;i<h;i++){cin >> str;start.push_back(str);}
  cin >> h;
  for(int i=0;i<h;i++){cin >> str;target.push_back(str);}
  RollingBalls rb;
  vector<string> ret = rb.restorePattern(start, target);
  cout << ret.size() << endl;
  for(int i=0;i<ret.size();i++){cout << ret[i] << endl;}
  return 0;
}
