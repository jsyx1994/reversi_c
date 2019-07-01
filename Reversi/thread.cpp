//
//  Resersi.cpp
//  Reversi
//
//  Created by mro on 11/1/17.
//  Copyright © 2017 mro. All rights reserved.
//

// 黑白棋（Reversi）样例程序
// 随机策略
// 作者：zhouhy
// 游戏信息：http://www.botzone.org/games#Reversi
/*
 #include <iostream>
 #include <string>
 #include <cstdlib>
 #include <ctime>
 #include <fstream>
 #include <vector>
 #include <cmath>
 #include <algorithm>
 #include <set>
 #include <thread>
 #include "jsoncpp/json.h" // C++编译时默认包含此库
 
 using namespace std;
 //定义节点的各属性
 typedef struct node
 {
 int total=0;
 int win=0;
 int value;
 struct nod * next;
 }Node;
 
 typedef struct
 {
 double value;
 int index;
 int (*possiblePos)[64][2] ;
 }thread_data;
 
 int Black = 1,White = -1,Empty = 0;
 int Simutimes = 10000;
 int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
 // 先x后y，记录棋盘状态
 vector<vector<int>> Board;
 int blackPieceCount = 2, whitePieceCount = 2;
 
 
 // 向Direction方向改动坐标，并返回是否越界
 //根据八个方向改动，右下的为方向0，顺时针依次为1，2，3，4，5，6，7
 inline bool MoveStep(int &x, int &y, int Direction)
 {
 if (Direction == 0 || Direction == 6 || Direction == 7)
 x++;
 if (Direction == 0 || Direction == 1 || Direction == 2)
 y++;
 if (Direction == 2 || Direction == 3 || Direction == 4)
 x--;
 if (Direction == 4 || Direction == 5 || Direction == 6)
 y--;
 if (x < 0 || x > 7 || y < 0 || y > 7)
 return false;
 return true;
 }
 bool ProcStep(int xPos, int yPos, int color, bool checkOnly = false)
 {
 int effectivePoints[8][2];
 int dir, x, y, currCount;
 bool isValidMove = false;
 
 //该点已被下过，为不合法落点，返回false
 if (Board[xPos][yPos] != Empty)
 return false;
 
 //以x,y为中心，朝8个方向遍历，查看是否有夹在中间的棋子。currCount 表示所夹棋子数
 for (dir = 0; dir < 8; dir++)
 {
 x = xPos;
 y = yPos;
 currCount = 0;
 //朝dir方向一直走
 while (1)
 {
 if (!MoveStep(x, y, dir))
 {
 currCount = 0;
 break;
 }
 //如果朝这个dir这个方向一直走，若有有对方的棋子，则为潜在的落子点。
 if (Board[x][y] == -color)
 {
 currCount++;
 effectivePoints[currCount][0] = x;
 effectivePoints[currCount][1] = y;
 }
 //如果这条直线的方向上存在一个点为空点，则无法落子，清空计数
 else if (Board[x][y] == Empty)
 {
 currCount = 0;
 break;
 }
 //如果遇到自己的棋子,则已经找出给定方向所夹的棋子，跳出
 else
 {
 break;
 }
 }
 if (currCount != 0)
 {
 isValidMove = true;
 if (checkOnly)
 return true;
 if (color == Black)
 {
 blackPieceCount += currCount;
 whitePieceCount -= currCount;
 }
 else
 {
 whitePieceCount += currCount;
 blackPieceCount -= currCount;
 }
 //翻转该方向的棋子
 while (currCount > 0)
 {
 x = effectivePoints[currCount][0];
 y = effectivePoints[currCount][1];
 Board[x][y] *= -1;
 currCount--;
 }
 }
 }
 if (isValidMove)
 {
 Board[xPos][yPos] = color;
 if (color == Black)
 blackPieceCount++;
 else
 whitePieceCount++;
 return true;
 }
 else
 return false;
 }
 
 // 检查color方有无合法棋步
 bool CheckIfHasValidMove(int color)
 {
 int x, y;
 for (y = 0; y < 8; y++)
 for (x = 0; x < 8; x++)
 if (ProcStep(x, y, color, true))
 return true;
 return false;
 }
 
 void getInputRange(int possiblePos[64][2],int *posCount,int color)
 {
 int x,y;
 for (y = 0; y < 8; y++)
 for (x = 0; x < 8; x++)
 if (ProcStep(x, y, color, true))
 {
 possiblePos[(*posCount)][0] = x;
 possiblePos[(*posCount)++][1] = y;
 }
 }
 
 
 class Montecarlo
 {
 public:
 Montecarlo(vector<vector<int>> Board)
 {
 Initiate();
 pair<int,int > Go = Selection();
 Deside(Go.first,Go.second);
 }
 // 在坐标处落子，检查是否合法或模拟落子
 
 pair<int, int> Selection()
 {
 // 找出当前合法落子点(个数：posCount;点对：possiblePos)
 int possiblePos[64][2], posCount = 0;
 vector<double> playOutValue;
 //getInputRange(possiblePos, &posCount,currBotColor);
 //srand(unsigned(time(NULL)));
 //choice = rand() % posCount;
 //return make_pair(possiblePos[choice][0],possiblePos[choice][1]);
 if (endOfGame())
 {
 //something to do
 }
 Expansion(possiblePos, &posCount);
 
 int resultX, resultY;
 if (posCount > 0)
 {
 //srand(unsigned(time(NULL)));
 //choice = rand() % posCount;
 //resultX = possiblePos[choice][0];
 //resultY = possiblePos[choice][1];
 
 //playOutValue.resize(posCount);
 int maxValueId= 0;
 double maxValue = -1000;
 
 //选择一个最大可能赢的节点
 
 
 void* status;
 vector<pthread_t> tids(posCount+1);
 vector<thread_data> compareMax(posCount+1);
 //##加入posCount个线程(0-posCount)
 for(int i = 0;i<posCount;++i)
 {
 
 //开始模拟
 thread_data td;
 td.index = i;
 compareMax.push_back(td);
 compareMax[i].possiblePos = &possiblePos;
 //创建线程
 pthread_create(&tids[i], NULL, Simulation, (void *) &(compareMax.at(i)));
 }
 for (int i= 0; i<posCount; ++i)
 {
 pthread_join(tids[i], &status);
 }
 maxValue = compareMax[0].value;
 maxValueId = 0;
 for (int i = 1;i < posCount;++i)
 {
 //cout << compareMax[i].value<<endl;
 if (compareMax[i].value > maxValueId)
 {
 maxValueId = i;
 maxValue = compareMax[i].value;
 }
 }
 
 resultX = possiblePos[maxValueId][0];
 resultY = possiblePos[maxValueId][1];
 }
 else
 {
 resultX = -1;
 resultY = -1;
 }
 return make_pair(resultX, resultY);
 }
 
 void Expansion(int possiblePos[64][2],int *posCount)
 {
 getInputRange(possiblePos,posCount,currBotColor);
 }
 //##
 static void* Simulation(void *tdata)//int i,int possiblePos[64][2])//模拟当前层的第i个分支，下在该点上的情况
 {
 srand(unsigned(time(NULL)));
 thread_data* td = (thread_data*) tdata;
 vector<vector<int>> cpBoard(Board);
 int cpBlackPieceCount(blackPieceCount),cpWhitePieceCount(whitePieceCount);        //自己模拟选择下在该点上
 int x = (*td->possiblePos)[td->index][0];
 int y = (*td->possiblePos)[td->index][1];
 ProcStep(x, y, currBotColor);
 int win  = 0;
 for (int j = 1;j <=Simutimes;++j)
 {
 //模拟前拷贝棋盘到备用棋盘，当前黑白子数，以免局面受影响;
 vector<vector<int>> copyBoard(Board.begin(),Board.end());
 int copyBlackPieceCount(blackPieceCount),copyWhitePieceCount(whitePieceCount);
 //下一步的起始为对方走 －currBotColor
 //cout << playout(-currBotColor)<<endl;
 win += playout(-currBotColor);
 //还原棋谱                还原子数
 Board.assign(copyBoard.begin(), copyBoard.end());
 blackPieceCount = copyBlackPieceCount;   whitePieceCount = copyWhitePieceCount;
 }
 //cout << win <<endl;
 Board.assign(cpBoard.begin(), cpBoard.end());
 blackPieceCount = cpBlackPieceCount;  whitePieceCount = cpWhitePieceCount;
 td->value = 1.0*win/Simutimes;
 return NULL;
 }
 
 static int playout(int color)//返回输或赢：输，平0/赢1
 {
 while(!endOfGame())
 {
 int possiblePos[64][2], posCount = 0, choice;
 int simuX,simuY;
 getInputRange(possiblePos, &posCount,color);
 //当前执子方有可走步，游戏没有结束
 if (posCount >0)
 {
 choice = rand() % posCount;
 //cout << choice << endl;
 simuX = possiblePos[choice][0];
 simuY = possiblePos[choice][1];
 //cout << simuX << simuY <<endl;
 ProcStep(simuX, simuY, color);
 }
 else //什么都不做,并交换执子
 {
 }
 //交换执子
 color *= -1;
 }
 //游戏结束统计各方子数，查看当前执子currBotColor是否胜利
 
 //赢的一方是否为当前执子方？
 //cout << whitePieceCount << " " <<blackPieceCount << " " << currBotColor << " " << ((whitePieceCount > blackPieceCount ? White : Black) == currBotColor ?  1 : 0) << endl;
 return ((whitePieceCount > blackPieceCount ? White : Black) == currBotColor ?  1 : 0);
 }
 void Backpropagation(){}
 
 static bool endOfGame()//棋盘已满，或者两方都无法移动时，游戏结束；否则没有结束
 {
 if (blackPieceCount + whitePieceCount == 64) return true;
 if ((!CheckIfHasValidMove(White)) && (!CheckIfHasValidMove(Black))) return true;
 return false;
 }
 void Initiate()
 {
 int x, y;
 Board.resize(8);
 for(int i = 0;i <8;++i)
 {
 for(int j = 0;j <8;++j)
 {
 Board[i].push_back(0);
 }
 }
 // 初始化棋盘
 Board[3][4] = Board[4][3] = Black;  //|白|黑|
 Board[3][3] = Board[4][4] = White; //|黑|白|
 
 // 读入JSON
 string str;
 getline(cin, str);
 Json::Reader reader;
 Json::Value input;
 reader.parse(str, input);
 
 // 分析自己收到的输入和自己过往的输出，并恢复状态
 int turnID = input["responses"].size();
 currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? Black : White; // 第一回合收到坐标是-1, -1，说明我是黑方
 
 //currBotColor = Black;
 
 for (int i = 0; i < turnID; i++)
 {
 // 根据这些输入输出逐渐恢复状态到当前回合
 x = input["requests"][i]["x"].asInt();
 y = input["requests"][i]["y"].asInt();
 if (x >= 0)
 ProcStep(x, y, -currBotColor); // 模拟对方落子
 x = input["responses"][i]["x"].asInt();
 y = input["responses"][i]["y"].asInt();
 if (x >= 0)
 ProcStep(x, y, currBotColor); // 模拟己方落子
 }
 
 // 看看自己本回合输入
 x = input["requests"][turnID]["x"].asInt();
 y = input["requests"][turnID]["y"].asInt();
 if (x >= 0)
 ProcStep(x, y, -currBotColor); // 模拟对方落子
 
 }
 
 void Deside(int resultX,int resultY)
 {
 Json::Value ret;
 ret["response"]["x"] = resultX;
 ret["response"]["y"] = resultY;
 Json::FastWriter writer;
 cout << writer.write(ret) << endl;
 }
 
 
 };
 int main()
 {
 
 // 做出决策（你只需修改以下部分）
 
 Montecarlo AI(Board);
 
 // 决策结束，输出结果（你只需修改以上部分）
 
 
 return 0;
 }*/