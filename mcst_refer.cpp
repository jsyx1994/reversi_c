/*
  0 1 2 . . .
  - - - - - - > x
0|
1|
2|
.|
.|
.|
 v
 y
*/
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <math.h>
#include <utility>
#include <map>
#include "jsoncpp/json.h"
using namespace std;

typedef int _GridType;
typedef int _Direction;
typedef int _Color;
typedef pair<int,int> _Coordinate;
const int SideLength=8;
const _Color White=-1,Black=1;
const _GridType WhiteStone=-1,Empty=0,BlackStone=1;
const _Direction North=0,NorthEast=1,East=2,SouthEast=3,South=4,SouthWest=5,West=6,NorthWest=7;
const int dx[]={0,1,1,1,0,-1,-1,-1},dy[]={-1,-1,0,1,1,1,0,-1};
const clock_t t1=clock();
const clock_t TIMELIMIT=0.9*CLOCKS_PER_SEC;
const double UCBC=0.5;
const int Threshold=60;
const int RankofPos[SideLength][SideLength] =
{
	30, 5,20,15,15,20, 5,30,
     5, 3,10,11,11,10, 3, 5,
    20,10,12,12,12,12,10,20,
    15,11,12,10,10,12,11,15,
    15,11,12,10,10,12,11,15,
    20,10,12,12,12,12,10,20,
     5, 3,10,11,11,10, 3, 5,
	30, 5,20,15,15,20, 5,30
};

class FieldCache
{
public:
	FieldCache()
	{
		for(int i=0;i<SideLength;i++)
		{
			memset(board[i],Empty,sizeof(board[i]));
		}
		ColorToPlay=Black;
		StoneNum[0]=StoneNum[1]=0;
		pre_node=NULL;
		visitcount=totalrank=0;
	}
	_GridType board[SideLength][SideLength];
	int StoneNum[2];
	_Color ColorToPlay;
	FieldCache* pre_node;
	map<_Coordinate,FieldCache*> next_node;
	int visitcount,totalrank;
	double value() const
	{
		if(visitcount==0) return -100;
		return (double)totalrank/visitcount;
	}
};
class Othello
{
	_GridType board[SideLength][SideLength];
	int StoneNum[2];//White and Black
	_Color ColorToPlay,MyColor;
	
	void BackUp(FieldCache& cache)
	{
		for(int i=0;i<SideLength;i++)
		{
			memcpy(cache.board[i],board[i],sizeof(board[i]));
		}
		cache.ColorToPlay=ColorToPlay;
		cache.StoneNum[0]=StoneNum[0];
		cache.StoneNum[1]=StoneNum[1];
	}
	void Recovery(const FieldCache& cache)
	{
		for(int i=0;i<SideLength;i++)
		{
			memcpy(board[i],cache.board[i],sizeof(board[i]));
		}
		ColorToPlay=cache.ColorToPlay;
		StoneNum[0]=cache.StoneNum[0];
		StoneNum[1]=cache.StoneNum[1];
	}
	bool isEnd()
	{
		if(StoneNum[0]+StoneNum[1]>=SideLength*SideLength) return true;
		for(int i=0;i<SideLength;i++)
		{
			for(int j=0;j<SideLength;j++)
			{
				if(Available(make_pair(i,j),Black)||Available(make_pair(i,j),White)) return false;
			}
		}
		return true;
	}
	int EndJudge(_Color color)
	{
		if(StoneNum[(color+1)/2]>StoneNum[(1-color)/2]) return 1;
		if(StoneNum[(color+1)/2]==StoneNum[(1-color)/2]) return 0;
		if(StoneNum[(color+1)/2]<StoneNum[(1-color)/2]) return -1;
	}
	double UCB(const FieldCache* current_node,const FieldCache* next_node)
	{
		if(next_node->visitcount==0) return -100;
		return next_node->value()+UCBC*sqrt(log(current_node->visitcount)/next_node->visitcount);
	}
	
	pair<_Coordinate,bool> NextPos(const _Coordinate& pos,_Direction dir)
	{
		if(dir<0||dir>7) return make_pair(pos,false);
		_Coordinate np=pos;
		np.first+=dx[dir];
		np.second+=dy[dir];
		if(np.first<0||np.first>=SideLength||np.second<0||np.second>=SideLength) return make_pair(pos,false);
		return make_pair(np,true);
	}
	bool Available(const _Coordinate& pos,_Color color)
	{
		if(board[pos.second][pos.first]!=Empty) return false;
		for(_Direction dir=North;dir<=NorthWest;dir++)
		{
			if(Available(pos,dir,color)) return true;
		}
		return false;
	}
	bool Available(const _Coordinate& pos,_Direction dir,_Color color)
	{
		if(board[pos.second][pos.first]!=Empty) return false;
		pair<_Coordinate,bool> res=NextPos(pos,dir);
		if(!res.second) return false;
		_Coordinate np=res.first;
		_GridType temp=(color==White?WhiteStone:BlackStone);
	    if(board[np.second][np.first]==temp||board[np.second][np.first]==Empty) return false;
		//该方向相邻子为同色或空 
        do
        {
        	res=NextPos(res.first,dir);
        	if(!res.second) return false;
        	np=res.first;
        	if(board[np.second][np.first]==temp) return true;
        	if(board[np.second][np.first]==Empty) return false;
        }while(res.second);
        return false;
	}
	bool PutStone(_Coordinate pos,_Color color)
	{
		if(pos==make_pair(-1,-1)) return true;
		_GridType temp=(color==White?WhiteStone:BlackStone);
		pair<_Coordinate,bool> res;
		_Coordinate np;
		bool flag=false;
		for(_Direction dir=North;dir<=NorthWest;dir++)
		{
			if(Available(pos,dir,color))
			{
				res=NextPos(pos,dir);
				np=res.first;
				while(board[np.second][np.first]!=temp)
				{
					board[np.second][np.first]=temp;
					res=NextPos(np,dir);
					np=res.first;
					StoneNum[(color+1)/2]++;
					StoneNum[(1-color)/2]--;
				}
				flag=true;
			}
		}
		if(flag)
		{
			board[pos.second][pos.first]=temp;
			StoneNum[(color+1)/2]++;
			return true;
		}
		else return false;
	}
	
	pair<FieldCache*,bool> Selection(FieldCache* current_node)
	{
		if(isEnd()) return make_pair(current_node,false);
		if(current_node->visitcount<Threshold) return make_pair(current_node,false);
		FieldCache* next=current_node;
		double temp,mUCB=-200;
		for(map<_Coordinate,FieldCache*>::iterator iter=current_node->next_node.begin();iter!=current_node->next_node.end();iter++)
		{
			temp=UCB(current_node,iter->second);
			if(temp>mUCB)
			{
				next=iter->second;
				mUCB=temp;
			}
		}
		Recovery(*next);
		if(next==current_node) return make_pair(next,false);
		return make_pair(next,true);
	}
	FieldCache* Expand(FieldCache* current_node)
	{
		if(isEnd()) return current_node;
		map<_Coordinate,int> alterlist;
		_Coordinate choice;
		int temp,sum=0;
		for(int i=0;i<SideLength;i++)
		{
			for(int j=0;j<SideLength;j++)
			{
				if(Available(make_pair(i,j),ColorToPlay))
				{
					alterlist.insert(make_pair(make_pair(i,j),RankofPos[i][j]));
					sum+=RankofPos[i][j];
				}
			}
		}
		if(sum==0) 
		{
			choice=make_pair(-1,-1);
		}
		else
		{
			temp=rand()%sum;
			for(map<_Coordinate,int>::iterator iter=alterlist.begin();iter!=alterlist.end();iter++)
			{
				temp-=iter->second;
				if(temp<0)
				{
					choice=iter->first;
					break;
				}
			}
		}
		map<_Coordinate,FieldCache*>::iterator it=current_node->next_node.find(choice);
		if(it!=current_node->next_node.end())
		{
			Recovery(*(it->second));
			return it->second;
		}
		FieldCache *next=new FieldCache();
		PutStone(choice,ColorToPlay);
		ColorToPlay=-ColorToPlay;
		next->pre_node=current_node;
		BackUp(*next);
		current_node->next_node.insert(make_pair(choice,next));
		return next;
	}
	void Simulation(const FieldCache* current_node,int rank[2])
	{
		map<_Coordinate,int> alterlist;
		_Coordinate choice;
		int temp,sum=0;
		while(!isEnd())
		{
			alterlist.clear();
			sum=0;
			for(int i=0;i<SideLength;i++)
			{
				for(int j=0;j<SideLength;j++)
				{
					if(Available(make_pair(i,j),ColorToPlay))
					{
						alterlist.insert(make_pair(make_pair(i,j),RankofPos[i][j]));
						sum+=RankofPos[i][j];
					}
				}
			}
			if(sum==0) 
			{
				choice=make_pair(-1,-1);
			}
			else
			{
				temp=rand()%sum;
				for(map<_Coordinate,int>::iterator iter=alterlist.begin();iter!=alterlist.end();iter++)
				{
					temp-=iter->second;
					if(temp<0)
					{
						choice=iter->first;
						break;
					}
				}
			}
			PutStone(choice,ColorToPlay);
			ColorToPlay=-ColorToPlay;
		}
		rank[0]=EndJudge(White);
		rank[1]=EndJudge(Black);
		Recovery(*current_node);
		return;
	}
	void BackPropagation(FieldCache* current_node,const int r[2])
	{
		while(current_node!=NULL)
		{
			current_node->visitcount++;
			current_node->totalrank+=r[(1-current_node->ColorToPlay)/2];
			current_node=current_node->pre_node;
		}
		return;
	}
	int MiniMax(_Color color, int alpha, int beta)
	{
		if(isEnd()) return EndJudge(color);
		FieldCache cache;
		BackUp(cache);
	    if(color==White)
	    {
	        int best=10000;
	        for(int i=0;i<SideLength;i++)
	        {
	        	for(int j=0;j<SideLength;j++)
	        	{
		            if(Available(make_pair(i,j),color))
		            {
		                PutStone(make_pair(i,j),color);
		                best=min(best,MiniMax(-color, alpha, beta));
		                beta=min(best,beta);
		                Recovery(cache);
		                if(alpha>=beta) break;//beta剪枝 
		            }
				}
	        }
	        return best;
	    }
	    else
	    {
	        int best=-10000;
	        for(int i=0;i<SideLength;i++)
	        {
	        	for(int j=0;j<SideLength;j++)
	        	{
		            if(Available(make_pair(i,j),color))
		            {
		                PutStone(make_pair(i,j),color);
		                best=max(best,MiniMax(-color, alpha, beta));
		                alpha=max(best,alpha);
		                Recovery(cache);
		                if(alpha>=beta) break;//alpha剪枝 
		            }
				}
	        }
	        return best;
	    }
	}
	
public:
	Othello()
	{
		for(int i=0;i<SideLength;i++)
		{
			memset(board[i],Empty,sizeof(board[i]));
		}
		board[SideLength/2-1][SideLength/2-1]=board[SideLength/2][SideLength/2]=WhiteStone;//|w|b|
		board[SideLength/2][SideLength/2-1]=board[SideLength/2-1][SideLength/2]=BlackStone;//|b|w|
		StoneNum[0]=StoneNum[1]=2;
		string str;
#ifndef _BOTZONE_ONLINE
		ifstream is("input.txt");
		getline(is,str);
		is.close();
#else
		getline(cin,str);
#endif
		Json::Reader reader;
		Json::Value input;
		reader.parse(str,input);
		
		int x,y;
 		MyColor = input["requests"][(Json::Value::UInt) 0]["x"].asInt() < 0 ? Black : White;
 		ColorToPlay=MyColor;
		int turnID=input["responses"].size();
		for(int i=0;i<turnID;i++)
		{
			x = input["requests"][i]["x"].asInt();
			y = input["requests"][i]["y"].asInt();
			if(x>=0)
				PutStone(make_pair(x,y),-MyColor);
			x = input["responses"][i]["x"].asInt();
			y = input["responses"][i]["y"].asInt();
			if(x>=0)
				PutStone(make_pair(x,y),MyColor);
		}
		x = input["requests"][turnID]["x"].asInt();
		y = input["requests"][turnID]["y"].asInt();
		if(x>=0)
			PutStone(make_pair(x,y),-MyColor);
			
		srand((unsigned)time(NULL)+rand());
	}
	
	_Coordinate MakeChoice()
	{
		if(StoneNum[0]+StoneNum[1]>=58)
		{
			FieldCache cache;
			_Coordinate choice=make_pair(-1,-1);
			BackUp(cache);
			if(MyColor==White)
			{
			    int result,beta=10000;
		    	for (int i = 0; i<SideLength; i++)
		    	{
		    		for(int j=0;j<SideLength;j++)
		    		{
			    		if (Available(make_pair(i,j),MyColor))//查询搜索表 
			    		{
			    			PutStone(make_pair(i,j),MyColor);//落子 
			    			result=MiniMax(-MyColor,-10000,beta);//求值 
			    			if(choice.first==-1)
			                {
			                	choice=make_pair(i,j);
			    			    beta=result;
			                } 
			    			else if(result<beta)
			    			{
			    				choice=make_pair(i,j);
			    			    beta=result;
			    			}
			    			else if(result==beta)
			    			{
			    			    if(rand()%2==0)
			    			    {
			    			    	choice=make_pair(i,j);
			    			    }
			    			}
			    			Recovery(cache);
		    			}
					}
		    	}
			}
			else
			{
			    int result,alpha=-10000;
		    	for (int i = 0; i<SideLength; i++)
		    	{
		    		for(int j=0;j<SideLength;j++)
		    		{
			    		if (Available(make_pair(i,j),MyColor))//查询搜索表 
			    		{
			    			PutStone(make_pair(i,j),MyColor);//落子 
			    			result=MiniMax(-MyColor,alpha,10000);//求值 
			    			if(choice.first==-1)
			                {
			                	choice=make_pair(i,j);
			    			    alpha=result;
			                } 
			    			else if(result>alpha)
			    			{
			                	choice=make_pair(i,j);
			    			    alpha=result;
			    			}
			    			else if(result==alpha)
			    			{
			    			    if(rand()%2==0)
			    			    {
			                		choice=make_pair(i,j);
			    			    }
			    			}
			    			Recovery(cache);
			    		}
			    	}
		    	}
			}
		}
		else
		{
			return MCTS();
		}
	}
	_Coordinate MCTS()
	{
		FieldCache *root_node,*current_node;
		root_node=new FieldCache();
		pair<FieldCache*,bool> res;
		int r[2];
		BackUp(*root_node);
		while(clock()-t1<TIMELIMIT)
		{
			current_node=root_node;
			do
			{
				res=Selection(current_node);
				current_node=res.first;
			}while(res.second);
			current_node=Expand(current_node);
			Simulation(current_node,r);
			BackPropagation(current_node,r);
			Recovery(*root_node);
		}
		_Coordinate choice=make_pair(-1,-1);
		double temp,best=-100;
#ifndef _BOTZONE_ONLINE
		cout << root_node->visitcount << endl;
#endif
		for(map<_Coordinate,FieldCache*>::iterator iter=root_node->next_node.begin();iter!=root_node->next_node.end();iter++)
		{
			temp=iter->second->value();
#ifndef _BOTZONE_ONLINE
			cout << iter->first.second << "," << iter->first.first << " " << iter->second->visitcount << " " << temp << endl;
#endif
			if(temp>best)
			{
				choice=iter->first;
				best=temp;
			}
		}
		return choice;
	}
	void DebugPrint()
	{
#ifndef _BOTZONE_ONLINE
		system("color f0");
//		system("cls");//清屏 
		cout << "我的颜色为:" << ((MyColor==White) ? "○\n" : "●\n");
		cout << "现在要落子的颜色为:" << ((ColorToPlay==White) ? "○\n" : "●\n");//提示当前要落子的颜色 
		cout << "    0   1   2   3   4   5   6   7\n";//输出列号 
		for (int i = 0; i<SideLength; i++)
		{
			if (i == 0) cout << " ┌─┬─┬─┬─┬─┬─┬─┬─┐\n" << i;//绘制棋盘，输出行号 
			else cout << " ├─┼─┼─┼─┼─┼─┼─┼─┤\n" << i;
			for (int j = 0; j<SideLength; j++)
			{
				cout << "│";
				switch (board[i][j])
	    		{
	    			case White:cout << "○"; break;
	    			case Black:cout << "●"; break;
	    			case Empty:
	    				if (Available(make_pair(j,i), ColorToPlay)) cout << "* ";//己方可能点为* 
	    				else cout << "  ";
	    				break;
	    			default:break;
				}
			}
			cout << "│\n";
			if (i == SideLength - 1) cout << " └─┴─┴─┴─┴─┴─┴─┴─┘\n";
		}
#endif
	}
};
/***************************************************************************/ 
 int main()
 {
 	_Coordinate choice;
    Othello othello;
    choice=othello.MCTS();
    othello.DebugPrint();
 	Json::Value ret;
 	ret["response"]["x"] = choice.first;
 	ret["response"]["y"] = choice.second;
 	Json::FastWriter writer;
 	cout << writer.write(ret) << endl;
 	return 0;
 }