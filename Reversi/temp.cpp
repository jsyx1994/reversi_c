#include <iostream>
#include <string>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include "jsoncpp/json.h" // C++编译时默认包含此库
using namespace std;

class MctsNode;
using coordinate = pair<int, int>;
using BoardType = int[8][8];
using BoardTypePtr = int(*)[8];
using ChildZone = vector<MctsNode*>;

#define White -1
#define Black 1
#define Empty 0
#define INF 1e8
#define TIME_LIMIT 0.98
//ucb
const double C = 2;
string argv1;

double START,END;//控制时间

//sigmoid 函数
double sigmoid(double x)
{
    return (1.0)/( 1 + exp(-x));
}

class MctsNode
{
public:
    MctsNode(){}
    ~MctsNode(){}
    MctsNode(int color);
    inline void setColor(int color);
    inline int getColor();
    inline void addVisit();
    inline double getVisit();
    inline void updateWins(double reward);
    inline double getWins();
    inline void copyState(BoardTypePtr source);
    inline void recoveryState(BoardTypePtr destination);
    inline BoardTypePtr getState();
    inline void addChild(class MctsNode* child);
    inline unsigned long hasChild();
    inline ChildZone getChildren();
    inline MctsNode* getParent();
    inline double getWeight();
    inline void setPrevAction(int xPos,int yPos);
    inline coordinate getPrevAction();
    inline void deltaBlackPiece(int number);
    inline void deltaWhitePiece(int number);
    inline void setPiece(int black , int white );
    inline int getBlackPieceCount();
    inline int getWhitePieceCount();
    class MctsNode *parent;
private:
    int stateColor;
    double visitTime = 0;
    double wins = 0;
    BoardType state = {0};
    
    ChildZone children;
    coordinate prevAction;
    int blackPieceCount;
    int whitePieceCount;
};
MctsNode::MctsNode(int color)
{
    stateColor = color;
}
inline void MctsNode::setColor(int color)
{
    stateColor = color;
}
inline int MctsNode::getColor()
{
    return stateColor;
}
inline void MctsNode::addVisit()
{
    ++visitTime;
}
inline double MctsNode::getVisit()
{
    return visitTime;
}
inline void MctsNode::updateWins(double reward)
{
    wins += reward;
}
inline double MctsNode::getWins()
{
    return wins;
}
inline void MctsNode::copyState(BoardTypePtr source)
{
    memcpy(state, source, sizeof(state));
}
inline void MctsNode::recoveryState(BoardTypePtr destination)
{
    memcpy(destination, state, sizeof(state));
}
inline BoardTypePtr MctsNode::getState()
{
    return state;
}

inline void MctsNode::addChild(class MctsNode* child)
{
    children.push_back(child);
}
inline unsigned long MctsNode::hasChild()
{
    return children.size();
}
inline ChildZone MctsNode::getChildren()
{
    return children;
}
inline MctsNode* MctsNode::getParent()
{
    return parent;
}
//#remain to code
inline double MctsNode::getWeight()
{
    if (visitTime == 0)
        return INF;
    else
    {
        double aveReward = wins/visitTime; //#待考
        double totalVisit = parent->visitTime;
        double weight = aveReward + C * sqrt(log(totalVisit/visitTime));
        return weight;
    }
}
inline void MctsNode::setPrevAction(int xPos, int yPos)
{
    prevAction = make_pair(xPos, yPos);
}
inline coordinate MctsNode::getPrevAction()
{
    return prevAction;
}
inline void MctsNode::deltaBlackPiece(int number)
{
    blackPieceCount += number;
}
inline void MctsNode::deltaWhitePiece(int number)
{
    whitePieceCount += number;
}
inline void MctsNode::setPiece(int black , int white )
{
    blackPieceCount = black;
    whitePieceCount = white;
}
inline int MctsNode::getBlackPieceCount()
{
    return blackPieceCount;
}
inline int MctsNode::getWhitePieceCount()
{
    return whitePieceCount;
}
//----------------------------------------------------------------------------------------------------------------------------------
class Bot
{
public:
    Bot();
    ~Bot(){}
    inline MctsNode* getRootState();
    inline bool ProcStep(MctsNode*& node,int xPos, int yPos, int color, bool checkOnly = false);
    inline bool MoveStep(int &x, int &y, int Direction);
    inline bool CheckIfHasValidMove(MctsNode* node,int color);
    inline void RecoverHistory();
    inline void getInputRange(MctsNode* node, int possiblePos[64][2],int & posCount);
    inline void exec();
    inline void select(MctsNode*& workingNode);
    inline void expand(MctsNode*& workingNode);
    inline void simulate(MctsNode* workingNode);
    inline void getCandidatePos(MctsNode*& node , vector<coordinate> & candidatePos);//计算空点
    inline void backpropagate(MctsNode* node, int winner, double reward);
    inline coordinate Decide(MctsNode* root);
    inline void outputDecision(int resultX,int resultY);
    inline void outPutState(MctsNode* node);//for debug and show
    inline bool gameEnded(MctsNode* node);
    inline MctsNode* chooseChild(MctsNode* node);
    inline pair<int, double> judge(MctsNode* node);
private:
    BoardType BoardCopy;
    MctsNode *rootState;
};
//初始化
Bot::Bot()
{
    srand(unsigned(time(NULL)));
    START = clock();
    rootState = new MctsNode;
    //根状态没有父节点
    rootState->parent = nullptr;
    rootState->setPiece(2, 2);
    RecoverHistory();
    //outPutState(rootState);
    //rootState->setColor(currBotColor);再recoverHistory中设置
    //rootState->setPrevAction(-1, -1);
    
}
inline MctsNode* Bot::getRootState()
{
    return rootState;
}
inline void Bot::exec()
{
    //cout << rootState->hasChild();
    //outPutState(rootState);
    MctsNode *workingNode = rootState;
    for (END = clock(); (double)(END-START) / (CLOCKS_PER_SEC) < TIME_LIMIT; END = clock())
    {
        select(workingNode);
        //cout << endl;
        simulate(workingNode);
        workingNode = rootState;
    }
    
    coordinate c = Decide(rootState);
    outputDecision(c.first, c.second);
}
inline void Bot::select(MctsNode*& workingNode)
{
//    srand(unsigned(time(NULL)));
    while (workingNode->hasChild())
    {
        double chance = 1.0 * rand() / RAND_MAX;
//        cout << chance << endl;
        if (chance > 0.25)
        {
            MctsNode* bestchild = chooseChild(workingNode);
            workingNode = bestchild;
//            cout << "normal" << endl;
        }
        else{
            double totalVisit = workingNode->getVisit();
            auto children = workingNode->getChildren();
            double p = 0;
            vector<double> prob;
            prob.push_back(p);
            for (auto child: children)
            {
                p = p + child->getVisit()/totalVisit;
                prob.push_back(p);
            }
            double c = rand() / RAND_MAX;
            unsigned long k;
            for (unsigned long i = 1; i < children.size(); ++i)
            {
                if (c >= prob[i-1] && c < prob[i])
                {
                    k = i;
                    break;
                }
            }
            workingNode = children[k-1];
//            cout << "abnormal" << endl;
        }
    }
    //已无孩子
    if (gameEnded(workingNode))
        return ;
    expand(workingNode);
    //扩展完成，选择当前最佳孩子
    workingNode = chooseChild(workingNode);
    //outPutState(workingNode);
    
}
inline void Bot::expand(MctsNode*& workingNode)
{
    int possiblePos[64][2],posCount = 0;
    getInputRange(workingNode, possiblePos, posCount);
    if (posCount > 0)
    {
        for (int i = 0; i < posCount; ++i)
        {
            MctsNode *node = new MctsNode;
            node->parent = workingNode;
            int x =  possiblePos[i][0], y = possiblePos[i][1];
            node->setPrevAction(x,y);
            int color = workingNode->getColor();
            node->setColor(-color);//#待考
            node->copyState(workingNode->getState());
            node->setPiece(workingNode->getBlackPieceCount(), workingNode->getWhitePieceCount());
            ProcStep(node, x, y, color);//#待考
            //outPutState(node);
            workingNode->addChild(node);
        }
    }
    //无法再扩展，可能扩展出了一个哑节点
    else
    {
        MctsNode *node = new MctsNode;
        node->parent = workingNode;
        node->setPrevAction(-1, -1);
        int color = workingNode->getColor();
        node->setColor(-color);
        node->copyState(workingNode->getState());
        node->setPiece(workingNode->getBlackPieceCount(), workingNode->getWhitePieceCount());
        //不进行落子，直接将局面留给敌方
        //cout << "!" <<endl;
        workingNode->addChild(node);
        
    }
}
inline void Bot::getCandidatePos(MctsNode*& node , vector<coordinate> & candidatePos)
{
    BoardTypePtr state = node->getState();
    //outPutState(node);
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0 ; j < 8; ++j)
        {
            if (state[i][j] == Empty)
            {
                coordinate coord = make_pair(i, j);
                candidatePos.push_back(coord);
            }
        }
    }
    //random_shuffle(candidatePos.begin(), candidatePos.end());
}
inline void Bot::simulate(MctsNode* workingNode)
{
    MctsNode simuState(*workingNode);
    MctsNode *simuNode = &simuState;
    int simuColor = workingNode->getColor();
    
    while (!gameEnded(simuNode))
    {
        int threashold = 200;
        int simuX,simuY;
        while (threashold-- )
        {
            simuX = rand() % 8;
            simuY = rand() % 8;
            if (ProcStep(simuNode, simuX, simuY, simuColor))
            {
                break;
            }
        }
        if (threashold == 0)
        {
            int possiblePos[64][2], posCount = 0, choice;
            getInputRange(simuNode, possiblePos, posCount);
            if (posCount > 0)
            {
                choice = rand() % posCount;
                simuX = possiblePos[choice][0];
                simuY = possiblePos[choice][1];
                ProcStep(simuNode, simuX, simuY, simuColor);
                //outPutState(workingNode);
                //cout << endl;
            }
            else
            {
                //pass
            }
        }
        
        simuColor *= -1;
        simuNode->setColor(simuColor);
    }
    pair<int, double> winnerAndReward;
    winnerAndReward = judge(simuNode);
    backpropagate(workingNode, winnerAndReward.first, winnerAndReward.second);
}
inline void Bot::backpropagate(MctsNode* node, int winner , double reward)
{
    while (node)
    {
        node->addVisit();
        if (winner != Empty)
        {
            //getColor所得到的颜色为节点状态下需要进行决策的一方的颜色，而到达这个状态的原因是因为对方的动作。
            //因此，如果计算出了赢方，实际上应该更新使之然的一方的颜色
            if (node->getColor() == winner)
                node->updateWins(-reward);
            else
                node->updateWins(+reward);
        }
        node = node->parent;
    }
}
inline coordinate Bot::Decide(MctsNode* root)
{
    //robost
    ChildZone childzone = root->getChildren();
    auto bestchild = *childzone.cbegin();
    for (auto &r : childzone)
    {
        if (r->getWins()/ r->getVisit() > bestchild->getWins() / bestchild->getVisit())
        {
            bestchild = r;
        }
    }
    return bestchild->getPrevAction();
}

inline MctsNode* Bot::chooseChild(MctsNode* node)
{
    double MAX = -INF;
    ChildZone childzone = node->getChildren();
    int randomChoice = rand() % (childzone.size());
    MctsNode* bestchild = *(childzone.begin() + randomChoice);
    for (auto &r:childzone)
    {
        double Weight = r->getWeight();
        if (Weight > MAX)
        {
            bestchild = r;
            MAX = Weight;
        }
    }
    return bestchild;
}
inline pair<int, double> Bot::judge(MctsNode* node)
{
    int blackPieceCount,whitePieceCount;
    blackPieceCount = node->getBlackPieceCount();
    whitePieceCount = node->getWhitePieceCount();
    //double delta = fabs(double(blackPieceCount - whitePieceCount));
    //double reward = 2 * (sqrt(delta) / 8);
    double reward = 1;
    
    if (blackPieceCount == whitePieceCount)
        return make_pair(Empty,0);
    if (blackPieceCount > whitePieceCount)
        return make_pair(Black, reward);
    
    return make_pair(White, reward);
    
}
inline bool Bot::gameEnded(MctsNode* node)
{
    
    if (node->getBlackPieceCount() + node->getWhitePieceCount() >= 64) return true;
    if ((!CheckIfHasValidMove(node,White)) && (!CheckIfHasValidMove(node,Black))) return true;
    return false;
}
inline bool Bot::ProcStep(MctsNode*& node,int xPos, int yPos, int color, bool checkOnly)
{
    int effectivePoints[8][2];
    int dir, x, y, currCount;
    bool isValidMove = false;
    
    BoardTypePtr Board = node->getState();
    if (Board[xPos][yPos] != Empty)
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
            if (Board[x][y] == -color)
            {
                currCount++;
                effectivePoints[currCount][0] = x;
                effectivePoints[currCount][1] = y;
            }
            //该方向探索遇到空点，无效的探索
            else if (Board[x][y] == Empty)
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
                node->deltaBlackPiece(+currCount);
                node->deltaWhitePiece(-currCount);
            }
            else
            {
                node->deltaBlackPiece(-currCount);
                node->deltaWhitePiece(+currCount);
            }
            while (currCount > 0)
            {
                x = effectivePoints[currCount][0];
                y = effectivePoints[currCount][1];
                
                
                Board[x][y] *= -1;
                currCount--;
            }
        }
    }
    //处理当前落点对棋盘的改变
    if (isValidMove)
    {
        Board[xPos][yPos] = color;
        if (color == Black)
        {
            node->deltaBlackPiece(+1);
        }
        else
        {
            node->deltaWhitePiece(+1);
        }
        return true;
    }
    else
        return false;
}
// 向Direction方向改动坐标，并返回是否越界
inline bool Bot::MoveStep(int &x, int &y, int Direction)
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

// 检查color方有无合法棋步
inline bool Bot::CheckIfHasValidMove(MctsNode* node, int color)
{
    int x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            if (ProcStep(node, x, y, color, true))
                return true;
    return false;
}

inline void Bot::RecoverHistory()
{
    int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
    int x, y;
    BoardTypePtr gridInfo = rootState->getState();
    //int blackPieceCount = 2, whitePieceCount = 2;
    
    // 初始化棋盘
    gridInfo[3][4] = gridInfo[4][3] = Black;  //|白|黑|
    gridInfo[3][3] = gridInfo[4][4] = White; //|黑|白|
    
    // 读入JSON
    string str;
    getline(cin, str);
    //str = argv1;
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    
    // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID = input["responses"].size();
    currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? Black : White; // 第一回合收到坐标是-1, -1，说明我是黑方
    rootState->setColor(currBotColor);
    
    for (int i = 0; i < turnID; i++)
    {
        // 根据这些输入输出逐渐恢复状态到当前回合
        x = input["requests"][i]["x"].asInt();
        y = input["requests"][i]["y"].asInt();
        if (x >= 0)
            ProcStep(rootState, x, y, -currBotColor,false); // 模拟对方落子
        
        //outPutState(rootState);
        x = input["responses"][i]["x"].asInt();
        y = input["responses"][i]["y"].asInt();
        if (x >= 0)
            ProcStep(rootState, x, y, currBotColor,false); // 模拟己方落子
        //outPutState(rootState);
    }
    
    // 看看自己本回合输入
    x = input["requests"][turnID]["x"].asInt();
    y = input["requests"][turnID]["y"].asInt();
    if (x >= 0)
        ProcStep(rootState, x, y, -currBotColor,false); // 模拟对方落子
}
inline void Bot::getInputRange(MctsNode* node,int possiblePos[64][2],int & posCount)
{
    int x,y;
    int color = node->getColor();
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            if (ProcStep(node, x, y, color, true))
            {
                possiblePos[posCount][0] = x;
                possiblePos[posCount++][1] = y;
            }
}

inline void Bot::outputDecision(int resultX,int resultY)
{
    
    Json::Value ret;
    ret["response"]["x"] = resultX;
    ret["response"]["y"] = resultY;
    Json::FastWriter writer;
    cout << writer.write(ret) << endl;
    
}
int main()//int argc , char **argv)
{
    
    //argv1 = argv[1];
    //cout << argv1 <<endl;
    Bot bot;
    // 做出决策（你只需修改以下部分
    
    bot.exec();
    // 决策结束，输出结果（你只需修改以上部分）
    cout<< (bot.getRootState())->getVisit();
    return 0;
}
