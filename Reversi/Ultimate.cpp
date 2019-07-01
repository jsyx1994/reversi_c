/*#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include <map>
#include <list>
#include "jsoncpp/json.h" // C++编译时默认包含此库
using namespace std;

#define White -1
#define Black 1
#define Empty 0
#define LimitZero 0.0000001
typedef pair<int, int> coordinate;

class Piece;
int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
int gridInfo[8][8] = { 0 }; // 先x后y，记录棋盘状态
int blackPieceCount = 2, whitePieceCount = 2;
int antiDirection[8] = {4,5,6,7,0,1,2,3};
map<coordinate,Piece> pieceLink;

class Piece
{
public:
    Piece(){}
    ~Piece(){}
    int getColor()
    {
        return gridInfo[xPos][yPos];
    }
    void removeChi(int direction)
    {
        chi.erase(direction);
    }
    bool isHasChi()
    {
        return chi.size();
    }
    unordered_set<int> getChi()
    {
        return chi;
    }
private:
    //定义子的气，初始化8面都有气
    unordered_set<int> chi{0,1,2,3,4,5,6,7};
    int xPos;
    int yPos;
    int color;
};

class Location
{
public:
    Location()
    {
        whitePieceLoc.insert("33");
        whitePieceLoc.insert("44");
        blackPieceLoc.insert("43");
        blackPieceLoc.insert("34");
    }
    ~Location(){}
    //0x,1y
    //int whitePieceLoc[64][2];
    //int blackPieceLoc[64][2];
    unordered_set<string> whitePieceLoc;
    unordered_set<string> blackPieceLoc;
    
}loc;
//Location loc;

class MctsNode
{
public:
    MctsNode(){}
    ~MctsNode(){}
    int stateColor;
    double visitTime = LimitZero;
    double reward;
    
};
// 向Direction方向改动坐标，并返回是否越界
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

// 在坐标处落子，检查是否合法或模拟落子
bool ProcStep(int xPos, int yPos, int color, bool checkOnly = false)
{
    int effectivePoints[8][2];
    int dir, x, y, currCount;
    bool isValidMove = false;
    if (gridInfo[xPos][yPos] != 0)
        return false;
    for (dir = 0; dir < 8; dir++)
    {
        x = xPos;
        y = yPos;
        currCount = 0;
        while (1)
        {
            //探索已越界，但是仍然没有完成“探索”，结束探索
            if (!MoveStep(x, y, dir))
            {
                currCount = 0;
                break;
            }
            //遇到对方棋子
            if (gridInfo[x][y] == -color)
            {
                currCount++;
                effectivePoints[currCount][0] = x;
                effectivePoints[currCount][1] = y;
            }
            //该方向探索遇到空点，无效的探索
            else if (gridInfo[x][y] == Empty)
            {
                currCount = 0;
                break;
            }
            //遇到自己的棋子，当前方向上的探索结束，可以吃掉对方的棋
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
            while (currCount > 0)
            {
                x = effectivePoints[currCount][0];
                y = effectivePoints[currCount][1];
                
                //转换为字符串，便于hash
                char sx = '0' + x, sy = '0' + y;
                string s = "";
                s.push_back(sx);
                s.push_back(sy);
                
                gridInfo[x][y] *= -1;
                currCount--;
                if (color == Black)
                {
                    loc.blackPieceLoc.insert(s);
                    loc.whitePieceLoc.erase(s);
                }
                else
                {
                    loc.blackPieceLoc.erase(s);
                    loc.whitePieceLoc.insert(s);
                }
            }
        }
    }
    //处理当前落点对棋盘的改变
    if (isValidMove)
    {
        gridInfo[xPos][yPos] = color;
        Piece *p = new Piece;
        for (int dir = 0; dir < 8; ++dir)
        {
            int x = xPos,y = yPos;
            MoveStep(x, y, dir);
            if (gridInfo[x][y] != Empty)
            {
                p->removeChi(dir);
                pieceLink[make_pair(x, y)].removeChi(antiDirection[dir]);
            }
        }
        pieceLink[make_pair(xPos, yPos)] = *p;
        
        char sx = '0' + xPos, sy = '0' + yPos;
        string s = "";
        s.push_back(sx);
        s.push_back(sy);
        
        if (color == Black)
        {
            loc.blackPieceLoc.insert(s);
            blackPieceCount++;
        }
        else
        {
            loc.whitePieceLoc.insert(s);
            whitePieceCount++;
        }
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

void RecoverHistory()
{
    int x, y;
    
    // 初始化棋盘
    gridInfo[3][4] = gridInfo[4][3] = Black;  //|白|黑|
    gridInfo[3][3] = gridInfo[4][4] = White; //|黑|白|
    
    //初始化pieceLink
    Piece *p1 = new Piece, *p2 = new Piece, *p3 = new Piece, *p4 = new Piece;
    p1->removeChi(7),p1->removeChi(0),p1->removeChi(1),pieceLink[make_pair(3, 3)] = *p1;
    p2->removeChi(1),p2->removeChi(2),p2->removeChi(3),pieceLink[make_pair(4, 3)] = *p2;
    p2->removeChi(5),p2->removeChi(6),p2->removeChi(7),pieceLink[make_pair(3, 4)] = *p3;
    p4->removeChi(3),p2->removeChi(4),p2->removeChi(5),pieceLink[make_pair(4, 4)] = *p4;
    
    // 读入JSON
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    
    // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID = input["responses"].size();
    currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? Black : White; // 第一回合收到坐标是-1, -1，说明我是黑方
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
void getInputRange(int possiblePos[64][2],int & posCount)
{
    int x,y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            if (gridInfo[x][y]== Empty && ProcStep(x, y, currBotColor, true))
            {
                possiblePos[posCount][0] = x;
                possiblePos[posCount++][1] = y;
            }
}
void getInputRange(int possiblePos[64][2],int & posCount,Location &loc,int color = currBotColor)
{
    int visited[8][8] = {0};
    if (color == Black)
    {
        for (auto it = loc.whitePieceLoc.begin(); it != loc.whitePieceLoc.end(); ++it)
        {
            int xPos = (*it)[0]- '0' ,yPos = (*it)[1]- '0';
            //算法，更新时不把
            //if (gridInfo[xPos][yPos] == Black)
            //可落子已定在对方位置的附近
            Piece p = pieceLink[make_pair(xPos, yPos)];
            unordered_set<int> chi = p.getChi();
            for (auto it = chi.begin(); it !=chi.end(); ++it)
            {
                int x = xPos, y = yPos;
                if(MoveStep(x, y, *it))
                {
                    
                    if (visited[x][y])
                        continue;
                    else
                    {
                        visited[x][y] = 1;
                    }
                    if(ProcStep(x, y, Black, true))
                    {
                        possiblePos[posCount][0] = x;
                        possiblePos[posCount++][1] = y;
                    }
                }

            }
        }
    }
    else
    {
        for (auto it = loc.blackPieceLoc.begin(); it != loc.blackPieceLoc.end(); ++it)
        {
            int xPos = (*it)[0]- '0' ,yPos = (*it)[1]- '0';

            Piece p = pieceLink[make_pair(xPos, yPos)];
            unordered_set<int> chi = p.getChi();
            for (auto it = chi.begin(); it !=chi.end(); ++it)
            {
                int x = xPos, y = yPos;
                if(MoveStep(x, y, *it))
                {
                    
                    if (visited[x][y])
                        continue;
                    else
                    {
                        //visited[x][y] = 1;
                    }
                    if(ProcStep(x, y, Black, true))
                    {
                        possiblePos[posCount][0] = x;
                        possiblePos[posCount++][1] = y;
                    }
                }
                
            }

        }

    }
}
void Decide(int resultX,int resultY)
{
    
    Json::Value ret;
    ret["response"]["x"] = resultX;
    ret["response"]["y"] = resultY;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    
}

int main()
{
    
    RecoverHistory();
    // 做出决策（你只需修改以下部分）
    int possiblePos[64][2],posCount=0,choice;
    clock_t start,end;
    start = clock();
    for(int i = 1; i < 10000;++i)
    {
        posCount = 0;
        getInputRange(possiblePos,posCount,loc);
    }
    end = clock();
    cout << (double(end-start))/CLOCKS_PER_SEC;
    int resultX, resultY;
    if (posCount > 0)
    {
        srand(unsigned(time(NULL)));
        choice = rand() % posCount;
        resultX = possiblePos[choice][0];
        resultY = possiblePos[choice][1];
    }
    else
    {
        resultX = -1;
        resultY = -1;
    }
    Decide(resultX, resultY);
    // 决策结束，输出结果（你只需修改以上部分）

    return 0;
}*/