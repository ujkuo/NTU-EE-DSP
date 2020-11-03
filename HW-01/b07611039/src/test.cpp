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

const int STATE_NUM = 6;
const int OBSERVATION_NUM = 6;
const int SEQ_NUM = 50;

double test(string str, HMM model)
{
    double delta[STATE_NUM][SEQ_NUM] = {0.0};
    for(int i = 0; i < STATE_NUM; i++)
        delta[i][0] = model.initial[i] * model.observation[str[0] - 65][i];

    double max_p = 0.0;
    for(int i = 1; i < SEQ_NUM; i++)
    {
        for(int j = 0; j < STATE_NUM; j++)
        {
            for(int k = 0; k < STATE_NUM; k++)
            {
                double temp = delta[k][i-1] * model.transition[k][j];
                if(temp > delta[j][i])
                    delta[j][i] = temp;
            }
            delta[j][i] *= model.observation[str[i] - 65][j];
        }

        for(int i = 0; i < STATE_NUM; i++)
            if(delta[i][SEQ_NUM - 1] > max_p)
                max_p = delta[i][SEQ_NUM - 1];
    }
    return max_p;
}

// input format: ./test <models_list_path> <seq_path> <output_result_path>
int main(int argc, char* argv[])
{
    char *model_list = argv[1];
    char *seq_model = argv[2];
    char *result = argv[3];

    HMM hmms[5];
    load_models(model_list, hmms, 5);
    
    ifstream input_file(seq_model);
    ofstream output_file(result);
    string str;
    
    while(input_file >> str)
    {
        double max_p = 0.0;
        int max_index = 0;
        for(int i = 0; i < 5; i++)
        {
            double p = test(str, hmms[i]);
            if(p > max_p)
            {
                max_p = p;
                max_index = i;
            }
        }
        output_file << "model_0" << max_index + 1 << ".txt " << max_p << endl;
    }

    return 0;
}

