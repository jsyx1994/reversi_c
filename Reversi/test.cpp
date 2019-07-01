//
//  Reversi.cpp
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
#include <map>
#include <limits.h>
#include "jsoncpp/json.h" // C++编译时默认包含此库

using namespace std;
//定义节点的各属性
int Weight[8][8] =
{
    24, 4,14,12,12,14, 4,24,
    4, 3, 8, 6, 6, 8, 3, 4,
    14, 8, 4, 2, 2, 4, 8,14,
    12, 6, 2, 0, 0, 2, 6,12,
    12, 6, 2, 0, 0, 2, 6,12,
    14, 8, 4, 2, 2, 4, 8,14,
    4, 3, 8, 6, 6, 8, 3, 4,
    24, 4,14,12,12,14, 4,24,
};

int Black = 1,White = -1,Empty = 0;
int Simutimes;
int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
// 先x后y，记录棋盘状态
vector<vector<int>> Board;
int blackPieceCount = 2, whitePieceCount = 2;
clock_t start,end;
double timeLimit = 0.99;
class Mct
{
public:
    double win = 0;
    double visitedTime = 0;
    double _x;
    double weight = MAXFLOAT;
    class Mct* parent;
    pair<int, int> coord;
    set<Mct*> childZone;
};
Mct root;
vector<Mct*> candiNodeList;

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
    Mct* current_node;
    Montecarlo(vector<vector<int>> Board)
    {
        Initiate();
        pair<int,int > Go = Selection();
        Deside(Go.first,Go.second);
    }
    // 在坐标处落子，检查是否合法或模拟落子
    Mct * ucbChoose()
    {
        Mct* maxValueNode = *(candiNodeList.begin());
        for(auto it = candiNodeList.begin()+1;it != candiNodeList.end();++it)
        {
            if ((*it)->weight > maxValueNode->weight)
            {
                maxValueNode = (*it);
            }
        }
        return maxValueNode;
    }
    
    pair<int, int> Selection()
    {
        
        // 找出合法落子点(个数：posCount;点对：possiblePos)
        int possiblePos[64][2], posCount = 0;
        vector<double> playOutValue;
        //getInputRange(possiblePos, &posCount,currBotColor);
        // srand(unsigned(time(NULL)));
        // choice = rand() % posCount;
        //return make_pair(possiblePos[choice][0],possiblePos[choice][1]);
        
        
        if (endOfGame())
        {
            //something to do
        }
        clock_t st=clock();
        clock_t ed;
        for (ed = clock(); double(ed-st)/CLOCKS_PER_SEC < timeLimit; ed = clock())
        {
            vector<vector<int>> cpBoard(Board);
            int cpBlackPieceCount(blackPieceCount),cpWhitePieceCount(whitePieceCount);
            
            int simuColor = currBotColor;
            //备份指针，以便模拟后能够指回根
            Mct* rewindNode = &root;
            
            //有孩子，Max ucb走子,直到无子可走;
            while(current_node->childZone.size()>0)
            {
                Mct * bestNode = ucbChoose();
                //模拟走子
                ProcStep(bestNode->coord.first, bestNode->coord.second, simuColor);
                current_node = bestNode;
                simuColor *=-1;
            }
            //没有孩子，扩展……
            if(current_node->childZone.size() == 0)
            {
                Expansion(possiblePos, &posCount);
                //有子可下
                if (posCount > 0)
                {
                    Mct* choosedNode = ucbChoose();
                    choosedNode->parent = current_node;
                    current_node->childZone.insert(choosedNode);
                    
                }
                else//无子可下，该节点的模拟棋颜色改变
                {
                    simuColor *=-1;
                }
            }
            
            //随机模拟一次
            int winOrLoss = playout(simuColor);
            Backpropagation();
        }
        
        
        
        
        timeLimit /= posCount+1;
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
            for(int i = 0;i<posCount;++i)
            {
                
                //开始模拟
                double value = Simulation(i,possiblePos);
                //cout << value<<endl;
                //选择具有最大价值的节点 ##MDF
                if(value > maxValue)
                {
                    maxValueId = i;
                    maxValue = value;
                }
                
                //playOutValue.push_back(value);
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
    
    void Expansion(int possiblePos[64][2],int *posCount,int color = currBotColor)
    {
        getInputRange(possiblePos,posCount,color);
        //加入待选列表中
        for (int i = 0;i < *posCount;++i)
        {
            Mct* node = new Mct;
            node->coord = make_pair(possiblePos[i][0], possiblePos[i][1]);
            candiNodeList.push_back(node);
        }
        
        
    }
    
    double Simulation(int i,int possiblePos[64][2])//模拟当前层的第i个分支，下在该点上的情况
    {
        srand(unsigned(time(NULL)));
        vector<vector<int>> cpBoard(Board);
        int cpBlackPieceCount(blackPieceCount),cpWhitePieceCount(whitePieceCount);        //自己模拟选择下在该点上
        int x = possiblePos[i][0];
        int y = possiblePos[i][1];
        ProcStep(x, y, currBotColor);
        int win  = 0;
        Simutimes = 0;
        start = clock();
        for (auto end = clock();double(end-start)/CLOCKS_PER_SEC<timeLimit;++Simutimes)
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
            end = clock();
        }
        //cout << win <<endl;
        Board.assign(cpBoard.begin(), cpBoard.end());
        blackPieceCount = cpBlackPieceCount;  whitePieceCount = cpWhitePieceCount;
        //cout << Simutimes <<endl;
        return 1.0*win/Simutimes;
    }
    
    int playout(int color)//返回输或赢：输，平0/赢1
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
    
    bool endOfGame()//棋盘已满，或者两方都无法移动时，游戏结束；否则没有结束
    {
        if (blackPieceCount + whitePieceCount == 64) return true;
        if ((!CheckIfHasValidMove(White)) && (!CheckIfHasValidMove(Black))) return true;
        return false;
    }
    void Initiate()
    {
        int x, y;
        //初始化根节点
        root.parent = nullptr;
        //初始化当前节点
        current_node = &root;
        
        Board.resize(8);
        for(int i = 0;i <8;++i)
        {
            Board[i].assign(8, 0);
            // cout << Board[i][4]<<endl;
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