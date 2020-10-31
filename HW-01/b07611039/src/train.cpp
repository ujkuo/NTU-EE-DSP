/*
 * Version 
 * Author: ujkuo
 * GitHub: github.com/ujkuo
 * Copyleft (C) 2020 ujkuo all rights reversed
 *
 */

#include "hmm.h" // use the -Iinc command in makefile can include header files in the different path
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

HMM model;
vector< vector<int> > data;
const int state_num = 6;
const int observation_num = 6;
const int seq_num = 50;

/*
 * [TODO]
 * add parameters
 * design the algorithm
 * set debugger
 */


void training_hmm(int iteration)
{
    for(int i = 0; i < iteration; i++)
    {
        double observation_new[observation_num][state_num] = {0.0};
        double gamma_new[state_num][seq_num] = {0.0};
        double epsilon_new[state_num][state_num] = {0.0};

        for(int j = 0; j < data.size(); j++)
        {
            double alpha[state_num][seq_num] = {0.0};
            double betta[state_num][seq_num] = {0.0};
            double gamma[state_num][seq_num] = {0.0};
            double epsilon[seq_num - 1][state_num][state_num];

            /* calculate alpha */
            for(int k = 0; k < state_num; k++)
                alpha[k][0] = model.initial[k] * model.observation[data[j][0]][k];

            for(int k = 1; k < seq_num; k++)
                for(int m = 0; m < state_num; m++)
                {
                    for(int n = 0; n < state_num; n++)
                        alpha[m][k] += alpha[n][k-1] * model.transition[n][m];
                    alpha[m][k] *= model.observation[data[j][k]][m];
                }

            /* calculate betta */
            for(int k = 0; k < state_num; k++)
                betta[k][seq_num - 1] = 1.0;
            for(int k = state_num - 2; k >= 0; k--)
                for(int m = 0; m < state_num; m++)
                    for(int n = 0; n < state_num; n++)
                        betta[m][k] += model.transition[m][n] * model.observation[data[j][k+1]][n] * betta[n][k+1];

            /* calculate gamma */
            for(int k = 0; k < state_num; k++)
            {
                double sum = 0.0;
                for(int m = 0; m < seq_num; m++)
                    sum += alpha[k][m] * betta[k][m];
                for(int m = 0; m < seq_num; m++)
                    gamma[k][m] = alpha[k][m] * betta[k][m] / sum;
            }

            /* calculate epsilon */
            for(int k = 0; k < seq_num-1; k++)
            {
                double sum = 0.0;
                for(int m = 0; m < state_num; m++)
                    for(int n = 0; n < state_num; n++)
                        sum += alpha[m][k] * model.transition[m][n] * model.observation[data[j][k+1]][n] * betta[n][k+1];

                for(int m = 0; m < state_num; m++)
                    for(int n = 0; n < state_num; n++)
                        epsilon[k][m][n] = alpha[m][k] * model.transition[m][n] * model.observation[data[j][k+1]][n] * betta[n][k+1] / sum;
            }
            
            /* update */
            for(int k = 0; k < seq_num; k++)
                for(int m = 0; m < state_num; m++)
                    observation_new[data[j][k]][i] += gamma[m][k];

            for(int k = 0; k < state_num; k++)
                for(int m = 0; m < seq_num; m++)
                    gamma_new[k][m] += gamma[k][m];

            for(int k = 0; k < seq_num; k++)
                for(int m = 0; m < state_num; m++)
                    for(int n = 0; n < state_num; n++)
                        epsilon_new[m][n] += epsilon[k][m][n];
        }

        /* re-write model parameters */
        for(int j = 0; j < state_num; j++)
            model.initial[j] = gamma_new[j][0] / data.size();
        
        double temp[state_num] = {0.0};
        for(int j = 0; j < state_num; j++)
            for(int k = 0; k < seq_num-1; k++)
                temp[j] += gamma_new[k][j];
        for(int j = 0; j < state_num; j++)
            for(int k = 0; k < state_num; k++)
                model.transition[j][k] = epsilon_new[j][k] / temp[j];

        for(int j = 0; j < state_num; j++)
            temp[i] += gamma_new[j][seq_num - 1];
        for(int j = 0; j < state_num; j++)
            for(int k = 0; k < observation_num; k++)
                model.observation[k][j] = observation_new[k][i] / temp[j];
    }

}



void printer()
{
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            int a = i + j;
}


/* Input format: ./train iteration model_init.txt seq_model_01.txt model_01.txt */

int main(int argc, char *argv[])
{
    /* input argument command line */
    int iteration = strtol(argv[1], NULL, 10);
    cout << iteration;
    char *model_init = argv[2];
    char *seq_model = argv[3];
    char *model_name = argv[4];

    loadHMM(&model, model_init);

    /* read input file */
    ifstream input_file(seq_model);
    string str;
    while(input_file >> str)
    {
        // cout << str;
        vector<int> temp;
        for (int i = 0; i < str.length(); i++)
            temp.push_back(str[i] - 'A');
        data.push_back(temp);
    }

    FILE *saved_path = open_or_die( argv[4], "w"  );
    dumpHMM(saved_path, &model);
    
    return 0;
}
