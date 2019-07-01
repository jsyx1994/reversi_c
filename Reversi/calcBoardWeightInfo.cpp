//
//  calcBoardWeightInfo.cpp
//  Reversi
//
//  Created by mro on 11/22/17.
//  Copyright © 2017 mro. All rights reserved.
//
/*
#include <iostream>
#include <cmath>
using namespace std;
int Board[10][10];
int main()
{
    for(int i = 0;i<10;++i)
    {
        for(int j = 0;j<10;++j)
        {
            if(i==0||j==0||i==9||j==9)
                Board[i][j] = -1;
            else Board[i][j] = 0;
        }
    }
    pair<double,double> origin = make_pair(4.5, 4.5);
    for(int i = 1;i<9;++i)
    {
        for(int j = 1;j<9;++j)
        {
            Board[i][j] = pow(((i*1.0)-origin.first),2.0) + pow(((j*1.0)-origin.second), 2.0);
            if((fabs(i-1)<=1 && fabs(j-1)<=1) || (fabs(i-8)<=1 && fabs(j-8) <=1) || ((fabs(i-1)<=1 && fabs(j-8) <=1))|| (((fabs(i-8)<=1 && fabs(j-1) <=1))))
            {
                Board[i][j] = sqrt(Board[i][j]);
            }
        }
    }
    for(int i = 1;i<9;++i)
    {
        for(int j = 1;j<9;++j)
        {
            cout << Board[i][j] << ",";
        }
        cout << endl;
    }
    return 0;
}
 */
