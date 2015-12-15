# RollingBalls

Problem Statement
問題文
You are given a maze. Each cell of the maze contains a wall, an empty cell or a ball of some color. 
あなたには迷路が与えられます。各セルには壁や空マス、または色のついたボールが入っています。

You are given the starting configuration of the maze. You want to move the balls into another, target configuration. 
あなたにはこのこれらの要素で構築された迷路が与えられ。                あなたが他の場所にボールを移動させたい場合。
To do this, you can roll balls, one at a time, in straight lines (horizontal or vertical). Once the ball starts rolling, 
あなたは1度に一回ボールを直線的に転がすことが出来る。 ボールが転がり始めると。
it stops only when it hits a wall or another ball (which does NOT start moving). Balls can't roll out of the maze, 
壁か他のボールにぶつかるまでは止まりません。 ボールは迷路の外には出ません。
so they stop when they hit a border of the maze. Each rolls starts only after the previously rolling ball has stopped, 
迷路の端っこにぶつかった時にも停止します。
so at any time there is at most one rolling ball in the maze. 
最低でも1回は転がります。

You have to do at most 20 * (number of balls in the maze) rolls. After all balls stop moving, their configuration must be as 
あなたは最大で20 * (迷路の中のボールの数)回ボールを転がすことが出来ます。 全てのボールが停止したあと、ターゲットに一番近くします。
close to the target one as possible.

Implementation Details
実装の詳細について
Your code should implement one method restorePattern(vector <string> start, vector <string> target). 
あなたはrestorePatternメソッドを実装する必要があります
start and target give the starting and the target configurations of the maze, formatted as follows: 
character j of element i represents cell in row i and column j. '#' is a wall, '.' is an empty cell, '0'..'9' are 
'#'は壁、'.'は空のマス、'0'から'9'の文字はボールの色を表します。
balls of various colors. Walls are located in the same cells in start and target, the only difference is 
in the positions of the balls. The set of balls' colors will also be the same. 

Your return from this method should be a vector <string>, containing at most 20 * (number of balls in the maze) elements. 
Each element of your return should be formatted as "R C D", where R and C are row and column coordinates of the ball you want to roll, 
and D is the direction of roll: 0 decreases column, 1 increases row, 2 increases column and 3 decreases row. 
(See visualizer for implementation of roll directions.)

Scoring
スコアについて
Your score for a test case will be calculated as follows. For each ball in target configuration you get 1 point 
あなたのスコアは次のように計算されます。                              各ターゲットの中にあるボールについて1ptが与えれれます。
if the end configuration has a ball of the same color in this cell, or else 0.5 points if it has a ball of different 
そうでないボールが中に入っていた場合は0.5ptが与えられます。
color in this cell. The score is the sum of points for each ball in target congifuration, divided by the total number of 
スコアはこれの合計値をボールの合計数で割った値となります。
balls in the maze. If your return contains an invalid roll or too many rolls, your score for this test case will be 0. 
もしあなたが必要以上にボールを転がしたりした場合は0点です。
Your overall score will be a sum of your scores for individual test cases.
あなたの最終的なスコアは各テストケースのスコアの合計値となります。
