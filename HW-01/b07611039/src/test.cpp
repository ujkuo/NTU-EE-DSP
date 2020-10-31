/*
 * Version 
 * Author: ujkuo
 * GitHub: github.com/ujkuo
 * Copyleft (C) 2020 ujkuo all rights reversed
 *
 */

#include "hmm.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

const int state_num = 6;
const int observation_num = 6;
const int seq_num = 50;

double test(string str, HMM model)
{
    double delta[state_num][seq_num] = {0.0};
    double max_p = 0.0;
    for(int i = 0; i < state_num; i++)
        delta[0][i] = model.initial[i] * model.observation[str[0]-'A'][i];

    for(int i = 1; i < seq_num; i++)
    {
        int observation_now = str[i] - 'A';
        double d = 0.0;
        for(int j = 0; j < state_num; j++)
        {
            for(int k = 0; k < state_num; k++)
                delta[i][j] = max(delta[i][j], delta[i-1][k] * model.transition[k][j]);
            delta[i][j] *= model.observation[observation_now][i];
        }
    }
    return max_p;
}

int main(int argc, char* argv[])
{
    char *model_list = argv[1];
    char *seq_model = argv[2];
    char *model_name = argv[3];

    HMM hmms[5];
    load_models(model_list, hmms, 5);
    
    ifstream input_file(seq_model);
    ofstream output_file(model_name);
    string str;

    double max_p = 0.0;
    int max_index = 0;
    while(input_file >> str)
    {
        for(int i = 0; i < 5; i++)
        {
            double p = test(str, hmms[i]);
            if(p > max_p)
            {
                max_p = p;
                max_index = i;
            }
        }
        output_file << "model_o" << max_index + 1 << ".txt" << max_p << endl;
    }

    return 0;
}

