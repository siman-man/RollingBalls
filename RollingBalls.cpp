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
const char WALL = 10;
const char EMPTY = 11;

const int DY[4] = {0, 1, 0, -1};
const int DX[4] = {-1, 0, 1, 0};

// 最大の縦幅
const int MAX_HEIGHT = 60;
// 最大の横幅
const int MAX_WIDTH = 60;
// 取りうる状態数(zoblishで使う)
const int MAX_STATUS = 12;

// ビーム幅
const int BEAM_WIDTH = 100;
// 探索の深さ
const int BEAM_DEPTH = 2;
// 探索回数
ll g_search_count = 0;

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
 * z座標の取得
 * @param y y座標
 * @param x x座標
 * @return 1次元にした結果の座標
 */
inline int getZ(int y, int x){
  return (y * MAX_HEIGHT + x);
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
  int roll_count;

  BALL(int y = UNKNOWN, int x = UNKNOWN, int color = UNKNOWN){
    this->y = y;
    this->x = x;
    this->color = color;
    this->roll_count = 0;
  }
};

// クエリ情報
struct QUERY {
  int ball_id;
  int y;
  int x;
  int direct;

  QUERY(int ball_id = UNKNOWN, int y = UNKNOWN, int x = UNKNOWN, int direct = UNKNOWN){
    this->ball_id = ball_id;
    this->y = y;
    this->x = x;
    this->direct = direct;
  }
};

struct NODE {
  QUERY query;                      // ボールの操作
  char maze[MAX_HEIGHT][MAX_WIDTH]; // 盤面
  int eval;                      // 評価値
  int score;                     // スコア
  ll hash;                          // 盤面のハッシュ値

  bool operator >(const NODE &e) const{
    return score + eval < e.score + e.eval;
  }   
};

// 迷路
char g_maze[MAX_HEIGHT][MAX_WIDTH];
// 保存用の迷路
char g_temp_maze[MAX_HEIGHT][MAX_WIDTH];
// 一番最初の盤面を保存する用
char g_origin_maze[MAX_HEIGHT][MAX_WIDTH];
// 目標の盤面
vector< vector<int> > g_target;
// 評価用の盤面
vector< vector<int> > g_eval_field;
// ボールのリスト
vector<BALL> g_ball_list;
// クエリのリスト
vector<QUERY> g_query_list;

// zoblish作成用
ll g_zoblish_field[MAX_HEIGHT][MAX_WIDTH][MAX_STATUS];

// 乱数生成
unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

class RollingBalls {
  public:
    /**
     * 初期化関数
     * @param start 初期盤面
     * @param target 目標盤面
     */
    void init(vector<string> start, vector<string> target){
      g_height = start.size();
      g_width = start[0].size();

      init_zoblish_field();
      init_maze(start);
      init_target(target);
      // 評価値盤面の更新
      update_eval_field();
    }

    /**
     * mazeの初期化
     * @param start 初期盤面
     */
    void init_maze(vector<string> start){
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

            g_ball_list.push_back(BALL(y, x, color));
          }
        }
      }
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

      map<ll, bool> check_list;

      for(int i = 0; i < g_total_ball_count * 20; i++){
        QUERY query = beam_search(i%g_total_ball_count);

        if(query.ball_id != UNKNOWN){
          roll(query.ball_id, query.y, query.x, query.direct);
          query_list.push_back(query2string(query));
        }
      }
      fprintf(stderr,"\n");
      fprintf(stderr,"search count = %lld\n", g_search_count);
      fprintf(stderr,"current score = %d\n", get_score());

      return query_list;
    }
    
    /**
     * ビーーーーームサーチ
     * 現在の盤面から一番ベストなボールの操作を取得する
     * @return query ボールの操作クエリ
     */
    QUERY beam_search(int start_id){
      // 同じ盤面を調べないようにハッシュ値を保存する
      map<ll, bool> check_list;
      // 一番最初のハッシュを取得(後はこれに対して差分更新)
      ll root_hash = get_zoblish_hash();
      // 一番最初の盤面を保存
      memcpy(g_origin_maze, g_maze, sizeof(g_maze));

      NODE best_node;
      int max_eval = INT_MIN;

      // 評価する盤面のキュー
      queue<NODE> node_queue;


      // rootなノードを作成
      NODE root_node = create_node();
      root_node.score = get_score();
      root_node.hash = root_hash;
      root_node.eval = get_eval();

      // 初期のキューに追加
      node_queue.push(root_node);

      // 初期盤面は飛ばすように
      check_list[root_hash] = true;

      for(int depth = 0; depth < BEAM_DEPTH; depth++){
        priority_queue< NODE, vector<NODE>, greater<NODE> > pque;
        //fprintf(stderr,"depth = %d, queue size = %lu\n", depth, node_queue.size());

        // 候補の盤面が空になるまで繰り返す
        while(!node_queue.empty()){
          // 親の盤面を取得
          NODE parent = node_queue.front(); node_queue.pop();

          //g_maze = parent.maze;
          memcpy(g_maze, parent.maze, sizeof(parent.maze));

          for(int i = 0; i < 7; i++){
            int ball_id = (start_id + i)%g_total_ball_count;
            BALL ball = g_ball_list[ball_id];

            // 4方向にコロコロ
            for(int direct = 0; direct < 4; direct++){
              COORD coord = roll_ball(ball.y, ball.x, direct);

              // ボールが1マスも進んでいない場合は処理を飛ばす
              if(coord.y == ball.y && coord.x == ball.x) continue;
              // ボールをコロコロ
              swap(g_maze[ball.y][ball.x], g_maze[coord.y][coord.x]);
              // ハッシュ値を再計算
              ll new_hash = update_zoblish_hash(parent.hash, ball.y, ball.x, ball.color, coord.y, coord.x, ball.color);

              // 既に調べた盤面以外は評価を行わない
              if(!check_list[new_hash]){
                g_search_count += 1;
                check_list[new_hash] = true;

                // 子ノードを作成
                NODE child = create_node();
                child.hash = new_hash;
                child.score = update_score(parent.score, ball.y, ball.x, coord.y, coord.x);
                child.eval = update_eval(parent.eval, ball.y, ball.x, coord.y, coord.x);

                // 初期の探索の時はクエリを作成 
                if(depth == 0){
                  child.query = QUERY(ball_id, ball.y, ball.x, direct);

                // それ以外は親のクエリを引き継ぐ
                }else{
                  child.query = parent.query;
                }

                // 候補に追加
                pque.push(child);
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
          if(max_eval < node.eval + node.score){
            max_eval = node.eval + node.score;
            best_node = node;
          }
        }
      }

      // 元に戻す
      memcpy(g_maze, g_origin_maze, sizeof(g_origin_maze));

      //assert(max_eval != INT_MIN);

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

      return COORD(ny, nx);
    }

    /**
     * 再帰的に床を滑る
     * @param y y座標
     * @param x x座標
     * @param direct 滑る方向
     */
    void slip(int y, int x, int direct, int depth){
      // 限界まで滑る
      if(depth > 20) return;

      int ny = y + DY[direct];
      int nx = x + DX[direct];

      /**
       * 床を滑る
       */
      while(is_inside(ny, nx) && g_maze[ny][nx] != WALL){
        ny += DY[direct];
        nx += DX[direct];
      }
      ny -= DY[direct];
      nx -= DX[direct];

      // 1マスも動かないなら終わり
      if(y == ny && x == nx) return;

      // 評価値を上げる
      g_eval_field[ny][nx] += 20 - depth;

      for(int nd = 0; nd < 4; nd++){
        // 逆方向には滑らない
        if(nd == ((direct+2)&3)) continue;

        if(is_outside(ny,nx) || g_maze[ny][nx] == WALL){
          slip(ny, nx, (nd+2)&3, depth+1);
        }
      }
    }

    /**
     * 実際にボールを転がす
     * @param ball_id ボールのID
     */
    void roll(int ball_id, int y, int x, int direct){
      BALL *ball = get_ball(ball_id);
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      while(is_inside(ny, nx) && g_maze[ny][nx] == EMPTY){
        ny += DY[direct];
        nx += DX[direct];
      }

      ny -= DY[direct];
      nx -= DX[direct];

      swap(g_maze[y][x], g_maze[ny][nx]);
      ball->y = ny;
      ball->x = nx;
    }

    /**
     * 指定した場所のスコアを取得
     * @param y y座標
     * @param x x座標
     */
    int get_point(int y, int x){
      int color = g_maze[y][x];
      int target_color = g_target[y][x];

      if(is_ball(color) && is_ball(target_color)){
        return (color == target_color)? 100 : 50;
      }else{
        return 0;
      }
    }

    /**
     * スコアの取得を行う
     * @return score スコア
     */
    int get_score(){
      int score = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];
          int target_color = g_target[y][x];

          if(is_ball(color) && is_ball(target_color)){
            // 色が一致していたら1pt
            if(color == target_color){
              score += 100;
            // そうでない場合は0.5pt
            }else{
              score += 50;
            }
          }
        }
      }

      return score;
    }

    /**
     * スコアの差分更新を行う
     */
    int update_score(int score, int y1, int x1, int y2, int x2){
      int s1 = g_maze[y2][x2];
      int s2 = g_target[y1][x1];
      int s3 = g_maze[y2][x2];
      int s4 = g_target[y2][x2];

      if(is_ball(s1) && is_ball(s2)){
        if(s1 == s2){
          score -= 100;
        }else{
          score -= 50;;
        }
      }

      if(is_ball(s3) && is_ball(s4)){
        if(s3 == s4){
          score += 100;
        }else{
          score += 50;
        }
      }

      return score;
    }

    /**
     * 評価値の取得を行う
     * @return eval 評価値
     */
    int get_eval(){
      int eval = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_maze[y][x];
          int point = g_eval_field[y][x];

          // 現在ボールのある場所のポイントをそのまま足す
          if(is_ball(color)){
            eval += point;
          }
        }
      }

      return eval;
    }

    /**
     * 評価値の差分更新を行う
     */
    int update_eval(int eval, int y1, int x1, int y2, int x2){
      int point1 = g_eval_field[y1][x1];
      int point2 = g_eval_field[y2][x2];

      eval += (point2 - point1);

      return eval;
    }

    /**
     * 評価用のフィールドを作成
     */
    void update_eval_field(){
      g_eval_field = vector< vector<int> >(g_height, vector<int>(g_width, 0));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int color = g_target[y][x];

          if(is_ball(color)){
            g_eval_field[y][x] += 100;
            // ゴールの周りのセルも評価値を上げる
            check_around_cell(y,x);

            for(int direct = 0; direct < 4; direct++){
              int ny = y + DY[direct];
              int nx = x + DX[direct];

              if(is_inside(ny, nx) && get_point(ny,nx) > 0) continue;

              // 壁の場合
              if(is_outside(ny,nx) || g_maze[ny][nx] == WALL){
                slip(y, x, (direct+2)&3, 0);
              }
            }
          }
        }
      }
    }

    /**
     * 周りのセルを調べて空白地点ならポイントを少し上げる
     * @param y y座標
     * @param x x座標
     */
    void check_around_cell(int y, int x){
      for(int direct = 0; direct < 4; direct++){
        int ny = y + DY[direct];
        int nx = x + DX[direct];

        if(is_inside(ny, nx) && g_maze[ny][nx] == EMPTY){
          g_eval_field[ny][nx] += 5;
        }
      }
    }

    /**
     * 掃除
     */
    void sweep(int y, int x, int direct, int point){
      int ny = y + DY[direct];
      int nx = x + DX[direct];

      while(true){
        ny += DY[direct];
        nx += DX[direct];

        if(is_inside(ny, nx) && g_maze[ny][nx] == EMPTY){
          g_eval_field[ny][nx] += point;
        }else{
          break;
        }
      }
      ny -= DY[direct];
      nx -= DX[direct];
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
     * @return NODE
     */
    inline NODE create_node(){
      NODE node;

      memcpy(node.maze, g_maze, sizeof(g_maze));

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
     * ボール情報を取得する
     * @param ball_id ボールのID
     * @return ボールの情報
     */
    inline BALL *get_ball(int ball_id){
      return &g_ball_list[ball_id];
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
     * 壁かどうかを判定する
     * @param y y座標
     * @param x x座標
     * @return (true: 壁, false: not 壁)
     */
    inline bool is_wall(int y, int x){
      return (is_outside(y, x) || g_maze[y][x] == WALL);
    }

    /**
     * 周りに壁かボールがあるかどうかを調べる
     * @param y y座標
     * @param x x座標
     * @return (true: 何かある, false: 無い)
     */
    bool is_exist_stop_object(int y, int x){
      int color = g_maze[y][x];
      return (is_wall(y,x) || is_ball(color));
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
     * @return ハッシュ値
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
    inline ll update_zoblish_hash(ll hash, int y1, int x1, int c1, int y2, int x2, int c2){

      // 移動前のボールの位置を消して
      hash ^= g_zoblish_field[y1][x1][c1];
      // 移動後のボールの位置に入れてあげる
      hash ^= g_zoblish_field[y2][x2][c2];

      return hash;
    }

    /**
     * QUERYからStringに変換
     * @param query クエリ
     * @return クエリを文字列化したもの
     */ 
    inline string query2string(QUERY &query){
      string str = "";
      str += int2string(query.y) + " ";
      str += int2string(query.x) + " ";
      str += int2string(query.direct);

      return str;
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
