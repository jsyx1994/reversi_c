//
//  calcProb.cpp
//  Reversi
//
//  Created by mro on 11/22/17.
//  Copyright © 2017 mro. All rights reserved.
//
/*
#include <iostream>
using namespace std;
double Weight[8][8] =
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
int main()
{
    double ave = 0;
    for(int i = 0;i<8;++i)
    {
        for(int j = 1;j<9;++j)
        {
            ave += Weight[i][j];
        }
    }
    for(int i = 0;i<8;++i)
    {
        for(int j = 0;j<9;++j)
        {
            Weight[i][j] /= ave;
        }
    }
    for(int i = 0;i<8;++i)
    {
        for(int j = 0;j<8;++j)
        {
            cout << Weight[i][j] << ",";
        }
        cout << endl;
    }
    return 0;
}*/
