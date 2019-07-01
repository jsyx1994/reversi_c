/*
#include <Python/Python.h>

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
#define TIME_LIMIT 20
//ucb
 //{"requests":[{"x":-1,"y":-1}],"responses":[]}
const double C = 2;


double START,END;//控制时间
short last4statesForTrain[8][8][4] = {0}; //记录棋盘每个点的最后四个落子历史 -1:白，0:空，1:黑; 每次更新挤掉最后一位
short last4statesForPredict[8][8][4];
void updateLast4(int i, int j, short newData, bool forTrain = true)
{
    
    if (forTrain)
    {
        auto& refer = last4statesForTrain[i][j];
        for (int k = 3; k > 0; --k)
        {
            refer[k] = refer[k-1];
        }
        refer[0] = newData;
    }
    else
    {
        auto& refer = last4statesForPredict[i][j];
        for (int k = 3; k > 0; --k)
        {
            refer[k] = refer[k-1];
        }
        refer[0] = newData;
    }

}

//sigmoid 函数
double sigmoid(double x)
{
    return (1.0)/( 1 + exp(-x));
}

//C++ python api. vector to list
PyObject* vectorToList_Float(const vector<int> &data) {
    PyObject* listObj = PyList_New( data.size() );
    if (!listObj) throw logic_error("Unable to allocate memory for Python list");
    for (unsigned int i = 0; i < data.size(); i++) {
        PyObject *num = PyFloat_FromDouble( (double) data[i]);
        if (!num) {
            Py_DECREF(listObj);
            throw logic_error("Unable to allocate memory for Python list");
        }
        PyList_SET_ITEM(listObj, i, num);
    }
    return listObj;
}

// PyObject -> Vector
template <class T>
vector<T> listTupleToVector(PyObject* incoming) {
    vector<T> data;
    if (PyTuple_Check(incoming)) {
        for(Py_ssize_t i = 0; i < PyTuple_Size(incoming); i++) {
            PyObject *value = PyTuple_GetItem(incoming, i);
            data.push_back( PyFloat_AsDouble(value) );
        }
    } else {
        if (PyList_Check(incoming)) {
            for(Py_ssize_t i = 0; i < PyList_Size(incoming); i++) {
                PyObject *value = PyList_GetItem(incoming, i);
                data.push_back( PyFloat_AsDouble(value) );
            }
        } else {
            throw logic_error("Passed PyObject pointer was not a list or tuple!");
        }
    }
    return data;
}


//------------------------------------------------------------------------------------------------------------------------
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
    static void compare2Board2Update(MctsNode* workingNode, MctsNode* bestChild);
    class MctsNode *parent;
    
    inline double getPsa();
    inline void setPsa(double prob);

private:
    int stateColor;
    double visitTime = 0;
    double wins = 0;
    BoardType state = {0};
    
    ChildZone children;
    coordinate prevAction;
    int blackPieceCount;
    int whitePieceCount;
    
    double psa;
    
};
//------------------------------------------------------------------------------------------------------------------------
class Bot
{
public:
    Bot();
    ~Bot(){}
    inline MctsNode* getRootState();
    inline bool ProcStep(MctsNode*& node,int xPos, int yPos, int color, bool checkOnly = false, bool recovery = true);
    inline bool MoveStep(int &x, int &y, int Direction);
    inline bool CheckIfHasValidMove(MctsNode* node,int color);
    inline void RecoverHistory();
    inline void getInputRange(MctsNode* node, int possiblePos[64][2],int & posCount);
    inline void exec();
    inline void select(MctsNode*& workingNode);
    inline void expand(MctsNode*& workingNode);
    inline void simulate(MctsNode* workingNode);
    
    inline pair<vector<vector<double>>, double> CNNSPredict(MctsNode* workingNode);
    
    inline void getCandidatePos(MctsNode*& node , vector<coordinate> & candidatePos);//计算空点
    inline void backpropagate(MctsNode* node, int winner, double reward);
    inline coordinate Decide(MctsNode* root);
    inline void outputDecision(int resultX,int resultY);
    inline void outPutState(MctsNode* node);//for debug and show
    inline bool gameEnded(MctsNode* node);
    inline MctsNode* chooseChild(MctsNode* node);
    inline pair<int, double> judge(MctsNode* node);
    inline pair<double, double> calcScore(MctsNode* node);
    inline void initPythonM();
private:
    BoardType BoardCopy;
    MctsNode *rootState;
    PyObject* myFunction;
    
    PyObject* myModule;
    PyObject* initPredict;
    PyObject* np;
    PyObject* sess;
    PyObject* feed_x;
    PyObject* model_forward_z;
    PyObject* predict;
    
};


//------------------------------------------------------------------------------------------------------------------------
class CNN
{
public:
    const int batch_size = 65;
    //short tempStates[9];
    // CNN();
    //~CNN();
    static vector<vector<double>> matrix(vector<double> prob);
    static vector<int> calcInput(int currColor, bool forTrain = true); // return sequence of feature
    inline void writeInput(int currColor);
    inline void writeOutput(ChildZone cz , Bot& bot);
    
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
        double Psa = 0.75 * psa + 0.25 * rand() / RAND_MAX;
        double weight = aveReward + 10 * Psa * sqrt(totalVisit)/(visitTime);
        //double weight = aveReward + C * sqrt(log(totalVisit/visitTime));
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
void MctsNode::compare2Board2Update(MctsNode* workingNode, MctsNode* bestChild)
{
    BoardTypePtr currentBoard = workingNode->getState();//
    BoardTypePtr newBoard = bestChild->getState();//
    for (int i = 0; i < 8; ++i)
    {
        for (int j= 0 ; j < 8; ++j)
        {
            int newdata = newBoard[i][j];
            if (currentBoard[i][j] != newdata)
            {
                updateLast4(i, j,newdata,false);
            }
        }
    }
}


inline double MctsNode::getPsa() {
    return psa;
}


inline void MctsNode::setPsa(double prob) {
    psa = prob;
}
//----------------------------------------------------------------------------------------------------------------------------------


//初始化
Bot::Bot()
{
    srand(unsigned(time(NULL)));
    
    
    initPythonM();
    
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
        //every time from root state to update

        memcpy(last4statesForPredict, last4statesForTrain, sizeof(last4statesForTrain));
        
        //move to the position needing predict, update last4statesForPredict when choosing a child. implement in chooseChild function
        select(workingNode);
        //cout << endl;
        
        //CNNSPredict(workingNode);
        //simulate(workingNode);
        workingNode = rootState;
    }
    
    coordinate c = Decide(rootState);
    outputDecision(c.first, c.second);
}
inline void Bot::select(MctsNode*& workingNode)
{
    //repeat
    while (workingNode->hasChild())
    {
        MctsNode* bestchild = chooseChild(workingNode);
        
        //every time select a child, compare current position to the new position,update last4
        MctsNode::compare2Board2Update(workingNode, bestchild);
        
        workingNode = bestchild;
        
        
        
    }
    
    if (gameEnded(workingNode))
    {
        //Ultimate true reward
        pair<int, double> winnerAndReward = judge(workingNode);
        backpropagate(workingNode, winnerAndReward.first, winnerAndReward.second);
        //outPutState(workingNode);
        return ;
    }
    //已无孩子，对当前节点进行扩展并且预测局面下的赢家。
    expand(workingNode);
    //与之前不同，扩展时已完成预测和回传，不要再选择当前最佳孩子
    //MctsNode* bestchild = chooseChild(workingNode);
    
    //every time select a child, compare current position to the new position,update last4
    //MctsNode::compare2Board2Update(workingNode, bestchild);
    
    //workingNode = bestchild;

    outPutState(workingNode);
    
}

//expand: set color,borad ... all state needed
inline void Bot::expand(MctsNode*& workingNode)
{
    //###此处加入预测部分，得到各点概率和赢家（胜率）。###
    pair<vector<vector<double>>, double> proWinner;
    proWinner = CNNSPredict(workingNode);
    vector<vector<double>> prob = proWinner.first;
    double w = proWinner.second;
    int winner = (w > 0.5)?Black:White;

    //计算目前节点的胜率
    int color = workingNode->getColor();
    double maxWinningRate = max(w,1-w);
    
    //目前节点的胜率作为reward传递
    backpropagate(workingNode, winner, maxWinningRate);
    
    int possiblePos[64][2],posCount = 0;
    getInputRange(workingNode, possiblePos, posCount);
    if (posCount > 0)
    {
        //计算可能落点的概率之和以重新部署概率，可以考虑使用softmax
        double proSum = 0;
        for (int i = 0; i < posCount; ++i)
        {
            int x,y;
            x = possiblePos[i][0];
            y = possiblePos[i][1];
            proSum += prob[x][y];
        }
        for (int i = 0; i < posCount; ++i)
        {
            
            //###为每个可能的MCTSNode加入相应的选择的概率 兀(s,an) ，胜率用于更新reward？###
            MctsNode *node = new MctsNode;
            node->parent = workingNode;
            int x =  possiblePos[i][0], y = possiblePos[i][1];
            

            // 重新部署策略选择概率
            node->setPsa(prob[x][y]/proSum);
            
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
        
        //！！！！待考
        //only have one choice to go,so 1
        node->setPsa(1);
        
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
    const int round =64;
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

inline pair<vector<vector<double>>, double> Bot::CNNSPredict(MctsNode* workingNode)
{
    int color = workingNode->getColor();
    
    vector<int> data = CNN::calcInput(color,false);

    
    PyObject* predictInput = vectorToList_Float(data);
    //PyObject_Print(predictInput,stdout,0);
    
    PyObject* np_asarray = PyObject_GetAttrString(np, "asarray");
    PyObject* np_float32 = PyObject_GetAttrString(np, "float32");
    PyObject* npPredictInput = PyObject_CallFunctionObjArgs(np_asarray,predictInput,np_float32,NULL);
    PyObject* reshape = PyObject_GetAttrString(npPredictInput, "reshape");
    long xd = 64, yd = 9;
    PyObject* shape = PyTuple_Pack(2,PyLong_FromLong(xd),PyLong_FromLong(yd));
    //PyObject_Print(shape,stdout,0);
    npPredictInput = PyObject_CallFunctionObjArgs(reshape,shape,NULL);

    PyObject* feed_dict = PyDict_New();
    int suss = PyDict_SetItem(feed_dict,feed_x,npPredictInput);
    //PyObject_Print(feed_dict,stdout,0);
    //cout << suss<<endl;
    
    double s = clock();
    PyObject* myResult = PyObject_CallFunctionObjArgs(predict,model_forward_z,feed_dict,NULL);
    PyObject_Print(myResult, stdout, 0);
    cout << endl;
    double e = clock();
    //cout << ((e -s)/CLOCKS_PER_SEC) << endl;
    
    
    
    char keyProb[] = "probability";
    char keyWinner[] = "winner";
    PyObject* key_prob =  Py_BuildValue("s",keyProb);
    PyObject* key_winner = Py_BuildValue("s",keyWinner);
    //PyObject_Print(key_prob, stdout, 0);
    
    
    //acquire prodiction values;
    PyObject* prob = PyDict_GetItem(myResult,key_prob);
    PyObject* winner = PyDict_GetItem(myResult, key_winner);
    //PyObject_Print(prob, stdout, 0);
    
    vector<double> probablity = listTupleToVector<double>(prob);
    vector<vector<double>> probablityMatrix = CNN::matrix(probablity);
    //cout <<c << endl;
    double w = PyFloat_AsDouble(winner);
    cout << w << endl;

    return make_pair(probablityMatrix, w);
    
    int Winner = (w > 0.5)?Black:White;
    double reward =  1;
    backpropagate(workingNode, Winner, reward);

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
                node->updateWins(reward);
            else
                node->updateWins(1-reward);
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
        if (r->getVisit() > bestchild->getVisit())
        {
            bestchild = r;
        }
    }
    return bestchild->getPrevAction();
}

inline void Bot::outPutState(MctsNode* node)
{
    BoardTypePtr state = node->getState();
    cout << "   ";
    for (int i =0 ; i < 8; ++i)
    {
        cout << i << "  ";
    }
    cout << endl;
    for (int i = 0; i < 8; ++i)
    {
        cout << i << "  ";
        for (int j = 0; j < 8; ++j)
        {
            if (state[j][i] == Black)
            {
                cout << "●  ";
            }
            else if (state[j][i] == White)
            {
                cout << "○  ";
            }
            else
                cout << "·  ";
        }
        cout << endl;
    }
    cout << endl;
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
    //step down : Train = false
    
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
inline bool Bot::ProcStep(MctsNode*& node,int xPos, int yPos, int color, bool checkOnly ,bool usedInsimulate)
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
                //将改变的数据记录到最后四个状态上
                if (!usedInsimulate)
                    updateLast4(x, y, Board[x][y]);
                
                currCount--;
            }
        }
    }
    //处理当前落点对棋盘的改变
    if (isValidMove)
    {
        Board[xPos][yPos] = color;
        if (!usedInsimulate)
            updateLast4(xPos, yPos, color);
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
    
    updateLast4(3, 4, Black);
    updateLast4(4, 3, Black);
    updateLast4(3, 3, White);
    updateLast4(4, 4, White);

    // 读入JSON
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input;
    reader.parse(str, input);
    
    // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID = input["responses"].size();
    currBotColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? Black : White; // 第一回合收到坐标是-1, -1，说明我是黑方
    
    //将第一个局面看成对方留给你的局面
    rootState->setColor(currBotColor);
    
    for (int i = 0; i < turnID; i++)
    {
        // 根据这些输入输出逐渐恢复状态到当前回合
        x = input["requests"][i]["x"].asInt();
        y = input["requests"][i]["y"].asInt();
        if (x >= 0)
            ProcStep(rootState, x, y, -currBotColor, false, false); // 模拟对方落子
        
        //outPutState(rootState);
        x = input["responses"][i]["x"].asInt();
        y = input["responses"][i]["y"].asInt();
        if (x >= 0)
            ProcStep(rootState, x, y, currBotColor, false, false); // 模拟己方落子
        
    }
    // 看看自己本回合输入
    x = input["requests"][turnID]["x"].asInt();
    y = input["requests"][turnID]["y"].asInt();
    if (x >= 0)
        ProcStep(rootState, x, y, -currBotColor, false, false); // 模拟对方落子
    cout << "root state:";
    outPutState(rootState);
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
inline void Bot::initPythonM()
{

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("sys.path.append('/Users/mro/Desktop/LinuxCNN_reversi/')");
    
    myModule = PyImport_ImportModule("low_level_CNNReversi");
    initPredict = PyObject_GetAttrString(myModule, "init_model_predict");
    np = PyObject_GetAttrString(myModule, "np");
    sess = PyObject_GetAttrString(myModule, "sess");
    
    //need to store for coming using
    predict = PyObject_GetAttrString(myModule, "predict_test");
    PyObject* x_zReturned = PyObject_CallObject(initPredict, NULL);
    feed_x = PyTuple_GetItem(x_zReturned, 0);
    model_forward_z = PyTuple_GetItem(x_zReturned, 1);
}
//------------------------------------------------------------------------------------------------------------------------

//new
 vector<int> CNN::calcInput(int currColor,bool forTrain)
{
    vector<int> a;
    if (forTrain)
    {
        for (int i = 0; i < 8; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                for (int k = 0; k < 4; ++k) //自己的情况
                {
                    if (last4statesForTrain[i][j][k] != currColor)
                    {
                        a.push_back(0);
                    }
                    else
                    {
                        a.push_back(1);
                    }
                    
                }
                for (int k = 0; k < 4; ++k )//对方的情况
                {
                    if (last4statesForTrain[i][j][k] != -currColor)
                    {
                        a.push_back(0);
                    }
                    else
                    {
                        a.push_back(1);
                    }
                }
                a.push_back(((currColor == Black)?1:0));
                
            }
        }
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                for (int k = 0; k < 4; ++k) //自己的情况
                {
                    if (last4statesForPredict[i][j][k] != currColor)
                    {
                        a.push_back(0);
                    }
                    else
                    {
                        a.push_back(1);
                    }
                    
                }
                for (int k = 0; k < 4; ++k )//对方的情况
                {
                    if (last4statesForPredict[i][j][k] != -currColor)
                    {
                        a.push_back(0);
                    }
                    else
                    {
                        a.push_back(1);
                    }
                }
                a.push_back(((currColor == Black)?1:0));
                
            }
        }

    }

    return a;
}

 vector<vector<double>> CNN::matrix(vector<double> prob)
{
    vector<vector<double>> probMatrix;
    probMatrix.resize(8);
    for (auto &r : probMatrix)
    {
        r.resize(8);
    }
    int i = -1;
    for (auto &out : probMatrix) {
        for (auto &in : out) {
            in = prob[++i];
        }
    }
    return probMatrix;
}

inline void CNN::writeInput(int currColor)
{
    ofstream out("/model/cnnInput",ofstream::app);
    
    vector<int> features = calcInput(currColor);
    for (unsigned int i = 0 ; i < features.size(); ++i) {
        if ((i % 9 == 0) && (i!=0) )
            out << endl;
        out << features.at(i) << ' ';
    }
    out << endl;

}
inline void CNN::writeOutput(ChildZone cz, Bot& bot)
{
    double outBoard[8][8] = {0};
    double sum = 0;
    ofstream out("/model/cnnOut",ofstream::app);
    for (auto &r : cz)
    {
        coordinate p = r->getPrevAction();
        int x = p.first;
        int y = p.second;
        outBoard[x][y] = r->getWins() / r->getVisit();
        sum += outBoard[x][y];
    }
    for (auto &r : cz)
    {
        coordinate p = r->getPrevAction();
        int x = p.first;
        int y = p.second;
        if (sum == 0) //root状态下的所有孩子都没有赢过
            outBoard[x][y] = 0;
        else
            outBoard[x][y] /= sum;
   }
    for(int i = 0; i < 8 ; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            out << outBoard[i][j] << ' ';
        }
    }
    out << endl;
}
int main()
{
    //updateLast4(1, 1, 2);
    //updateLast4(1, 1, 1);
    
    CNN cnn;//进行神经网络的相关处理
    
    //Py_Initialize();
    
    Py_Initialize();
    Bot bot;
    bot.exec();
    
    Py_Finalize();

    
    cnn.writeInput((bot.getRootState())->getColor());
    cnn.writeOutput(bot.getRootState()->getChildren(),bot);
    return 0;
}*/

