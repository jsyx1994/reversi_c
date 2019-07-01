
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
 {"requests":[{"x":-1,"y":-1}],"responses":[]}
 
 {"requests":[{"x":4,"y":5},{"x":2,"y":2},{"x":2,"y":3},{"x":5,"y":2},{"x":5,"y":4},{"x":7,"y":3},{"x":3,"y":6},{"x":4,"y":6},{"x":7,"y":4},{"x":5,"y":6},{"x":4,"y":1},{"x":5,"y":0},{"x":2,"y":7},{"x":3,"y":1},{"x":0,"y":2},{"x":6,"y":5},{"x":1,"y":4},{"x":6,"y":2},{"x":3,"y":7},{"x":6,"y":7},{"x":0,"y":4},{"x":3,"y":0},{"x":1,"y":6},{"x":1,"y":0},{"x":6,"y":6},{"x":7,"y":6},{"x":1,"y":7},{"x":0,"y":0},{"x":0,"y":6},{"x":7,"y":1}],"responses":[{"x":5,"y":3},{"x":2,"y":5},{"x":3,"y":5},{"x":5,"y":5},{"x":6,"y":3},{"x":7,"y":2},{"x":2,"y":4},{"x":5,"y":7},{"x":1,"y":2},{"x":5,"y":1},{"x":3,"y":2},{"x":4,"y":7},{"x":7,"y":5},{"x":4,"y":2},{"x":6,"y":4},{"x":2,"y":1},{"x":2,"y":0},{"x":4,"y":0},{"x":0,"y":5},{"x":0,"y":3},{"x":0,"y":1},{"x":1,"y":5},{"x":2,"y":6},{"x":6,"y":1},{"x":7,"y":7},{"x":1,"y":1},{"x":0,"y":7},{"x":1,"y":3},{"x":-1,"y":-1}]}

 
 {"requests":[{"x":-1,"y":-1},{"x":3,"y":5},{"x":6,"y":4},{"x":5,"y":2},{"x":4,"y":6},{"x":4,"y":5},{"x":2,"y":4},{"x":7,"y":4},{"x":4,"y":2},{"x":2,"y":6},{"x":7,"y":2},{"x":7,"y":3}],"responses":[{"x":5,"y":4},{"x":2,"y":2},{"x":6,"y":5},{"x":5,"y":5},{"x":5,"y":6},{"x":2,"y":5},{"x":6,"y":3},{"x":6,"y":2},{"x":5,"y":3},{"x":1,"y":5},{"x":6,"y":1}}

#define MAXFLOAT 100000
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
#include <sstream>
#include "jsoncpp/json.h" // C++编译时默认包含此库
using namespace std;
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
map<string,pair<int,int>> data;
int Black = 1,White = -1,Empty = 0;
int Simutimes;
int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
// 先x后y，记录棋盘状态
vector<vector<int>> Board;
int blackPieceCount = 2, whitePieceCount = 2;
clock_t start,end;
double timeLimit = 5;
int turnID;
string argv1;
string currentState;
class Mct
{
public:
    double win = 0; //
    double visitedTime = 0; //该节点访问次数
    double weight = MAXFLOAT; //该节点的权重，用于扩展和选择节点
    class Mct *parent; //该节点的父亲节点
    pair<int, int> coord; //该节点对应的落子x,y坐标
    set<Mct*> childZone; //该节点的孩子圈
    //double ucb1TunedFirstPart=0;
};
Mct root;

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
void saveState(int resultX,int resultY)
{
    ofstream ofs;
    ofs.open("/Users/mro/Desktop/data",ofstream::app);
    ofs << "data[\"";
    for (int i = 0; i < 8; ++i)
    {
        for(int j = 0; j <8; ++j)
        {
            ofs << Board[i][j];
        }
    }
    ofs << "\"]=make_pair(" <<resultX << ',' << resultY << ')' <<endl;
}
class Montecarlo
{
public:
    Mct* current_node;
    Montecarlo(vector<vector<int>> &Board)
    {
        Initiate();
        clock_t start = clock();
        //可移出循环？
        vector<vector<int>> cpBoard(Board);
        int cpBlackPieceCount(blackPieceCount),cpWhitePieceCount(whitePieceCount);
        Mct* rewindNode = current_node;
        for(auto end = clock(); double(end-start)/CLOCKS_PER_SEC < timeLimit;end=clock())
        {
            int simuColor = currBotColor;
            Mct* selectedNode = TreePolicy(simuColor);
            
            //ProcStep(selectedNode->coord.first, selectedNode->coord.second, simuColor);
            double winOrLoss = DefaultPolicy(selectedNode,simuColor);
            //cout << winOrLoss << endl;
            Backpropagation(selectedNode,winOrLoss);
            
            current_node = rewindNode;
            Board.assign(cpBoard.begin(), cpBoard.end());
            blackPieceCount = cpBlackPieceCount;  whitePieceCount = cpWhitePieceCount;
        }
        //cout << current_node->visitedTimes<<endl;
        Deside();
    }
    
    //proc until end
    double DefaultPolicy(Mct* &node,int color)
    {
        
        double winOrLoss = playout(color);
        //cout << winOrLoss << endl;
        return winOrLoss;
    }
    
    Mct* TreePolicy(int &simuColor)
    {
        int possiblePos[64][2],posCount=0;
        while (current_node->childZone.size() > 0)
        {
            if(endOfGame())
            {
                return current_node;
            }
            Mct* bestChild = ucbChoose(simuColor);
            //模拟走子
            ProcStep(bestChild->coord.first, bestChild->coord.second, simuColor);
            current_node = bestChild;
            //每走一步都要交换落子颜色
            simuColor *= -1;
        }
        //选择到叶子节点的时候发现所有的节点的权重都非无穷大，说明孩子们都长大了，孩子要生孩子了。
        if (current_node->weight != MAXFLOAT && !endOfGame())
        {
            
            Expansion(possiblePos, &posCount,simuColor);
            //如果没有扩展出孩子,并且非终局，说明该色棋子没有可下点，直接交换颜色，使用DefaultPolicy
            if (posCount == 0)
            {
                simuColor *= -1;
                return current_node;
            }
            else
            {
                //选择一个肚子最大的孩子
                Mct* pregnantOne = ucbChoose(simuColor);
                ProcStep(pregnantOne->coord.first, pregnantOne->coord.second, simuColor);
                simuColor *= -1;
                //准备进入模拟阶段
                return pregnantOne;
            }
        }
        
        return current_node;
    }
    
    Mct* ucbChoose(int color)
    {
        auto it = current_node->childZone.begin();
        Mct* maxValueNode = *it;
        for(auto iter = it;iter != current_node->childZone.end();++iter)
        {
            if((*iter)->visitedTime!=0)
            {
                double Qv_ = (*iter)->win;
                double Nv_ = (*iter)->visitedTime;
                double Nv = (*iter)->parent->visitedTime;
                double Xj;
                if (color == currBotColor)
                {
                    Xj = Qv_/Nv_;
                }
                else Xj =  1-Qv_/Nv_;
                
                //double Vj = 1.0/Nv_ * (*iter)->ucb1TunedFirstPart - Xj*Xj + sqrt(2 * log(Nv) / Nv_);
                //cout <<Vj <<endl;
                
                (*iter)->weight =  Xj +  sqrt(2*log(Nv) / Nv_); //* min(0.25,Vj));
            }
            if ((*iter)->weight > maxValueNode->weight)
            {
                maxValueNode = (*iter);
            }
        }
        return maxValueNode;
    }
    
    //何时扩展？
    void Expansion(int possiblePos[64][2],int *posCount,int color = currBotColor)
    {
        getInputRange(possiblePos,posCount,color);
        for (int i = 0;i < *posCount;++i)
        {
            Mct* node = new Mct;
            node->coord = make_pair(possiblePos[i][0], possiblePos[i][1]);
            node->parent = current_node;
            //node加入该父亲的孩子圈
            current_node->childZone.insert(node);
            
        }
        
    }
    
    
    double playout(int color)//返回输或赢：输，平0/赢1
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
        //cout << whitePieceCount << " " <<blackPieceCount << " " << currBotColor << " " << ((whitePieceCount > blackPieceCount ? White : Black) == currBotColor ?  1 : 0) << endl;1
        //return ((whitePieceCount > blackPieceCount ? White : Black) == currBotColor ?  1 : -1);
        
        if (currBotColor == Black)
        {
            //return (blackPieceCount-whitePieceCount)*1.0/64;
            
            if (blackPieceCount > whitePieceCount)
            {
                return min(1.0,0.5 + (blackPieceCount - whitePieceCount) * 1.0 / blackPieceCount);
            }
            else if(blackPieceCount == whitePieceCount)
            {
                return 0.5;
            }
            else return max(0.0,0.5 - (whitePieceCount - blackPieceCount) * 1.0 / whitePieceCount);
        }
        else
        {
            //return (whitePieceCount-blackPieceCount)*1.0/64;
            
            if (whitePieceCount > blackPieceCount)
            {
                return min(1.0, 0.5 + (whitePieceCount - blackPieceCount) * 1.0 / whitePieceCount);
            }
            
            else if (whitePieceCount == blackPieceCount)
            {
                return 0.5;
            }
            else return max(0.0, 0.5 - (blackPieceCount - whitePieceCount) * 1.0 /blackPieceCount);
        }
    }
    void Backpropagation(Mct* &node,double winOrLoss)
    {
        while (node != nullptr)
        {
            node->visitedTime++;
            node->win += winOrLoss;
            //node->ucb1TunedFirstPart += pow((node->win) / (node->visitedTime),2);
            node = node->parent;
        }
    }
    
    bool endOfGame()//棋盘已满，或者两方都无法移动时，游戏结束；否则没有结束
    {
        if (blackPieceCount + whitePieceCount == 64) return true;
        if ((!CheckIfHasValidMove(White)) && (!CheckIfHasValidMove(Black))) return true;
        return false;
    }
    void Initiate()
    {
        int x, y;
        srand(unsigned(time(NULL))+rand());
        cout << "entered" <<endl;
        //初始化根节点
        root.parent = nullptr;
        //根节点初始化时特殊对待，因为要扩展的必要性
        root.weight = -MAXFLOAT;
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
        
        str = argv1;
        //getline(cin, str);
        Json::Reader reader;
        Json::Value input;
        reader.parse(str, input);
        
        // 分析自己收到的输入和自己过往的输出，并恢复状态
        turnID = input["responses"].size();
        //cout << turnID <<endl;
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
    
    void Deside()
    {
        int resultX,resultY;
        
        if (!CheckIfHasValidMove(currBotColor))
        {
            resultX = -1;
            resultY = -1;
        }
        else
        {
            Mct *choice = *root.childZone.begin();
            double value = -MAXFLOAT;
            for (auto it = root.childZone.begin(); it != root.childZone.end(); ++it)
            {
                Mct* node = *it;
                double estimate = node->visitedTime;
                if (estimate > value)
                {
                    value = estimate;
                    choice = node;
                }
            }
            resultX = choice->coord.first;
            resultY = choice->coord.second;
            
            //cout <<choice->visitedTime;
        }
        Json::Value ret;
        ret["response"]["x"] = resultX;
        ret["response"]["y"] = resultY;
        Json::FastWriter writer;
        cout << writer.write(ret) << endl;
        saveState(resultX,resultY);
    }
};
int main(int argc,char **argv)
{
    //data["000000000000000000000000000-110000001-1000000000000000000000000000"]=make_pair(2,4);
    //cout << (currentState.compare "000000000000000000000000000-110000001-1000000000000000000000000000")<<endl;

    // 做出决策（你只需修改以下部分）
    argv1 = argv[1];
    cout << argv1 <<endl;
    Montecarlo AI(Board);
    
   // pair<int,int> result = data.at(currentState);
   // cout << result.first << ' ' <<  result.second <<endl;
    // 决策结束，输出结果（你只需修改以上部分）
    return 0;
}
