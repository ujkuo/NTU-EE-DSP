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
vector< vector<int> > data; // 10000*50
const int state_num = 6;
const int observation_num = 6;
const int seq_num = 50;

/*
 * [TODO]
 * add parameters
 * design the algorithm
 * set debugger
 */


void train(int iteration)
{
    for(int i = 0; i < iteration; i++)
    {
        // cout << "iteration: " << i << endl;
        double observation_new[observation_num][state_num] = {0.0}; // 6*6
        double gamma_new[state_num][seq_num] = {0.0}; // 6*50
        double epsilon_new[state_num][state_num] = {0.0}; // 6*6
        // cout << data.size();

        for(int j = 0; j < data.size(); j++)
        {
            // cout << j << " ";
            double alpha[state_num][seq_num] = {0.0}; // 6*50
            double betta[state_num][seq_num] = {0.0}; // 6*50
            double gamma[state_num][seq_num] = {0.0}; // 6*50
            double epsilon[seq_num - 1][state_num][state_num] = {0.0}; // 49*6*6

            /* calculate alpha */
            for(int k = 0; k < state_num; k++)
            {
                alpha[k][0] = model.initial[k] * model.observation[data[j][0]][k];
                // cout << alpha[k][0];
            }

            for(int k = 1; k < seq_num; k++)
            {
                for(int m = 0; m < state_num; m++)
                {
                    for(int n = 0; n < state_num; n++)
                        alpha[m][k] += alpha[n][k-1] * model.transition[n][m];
                    alpha[m][k] *= model.observation[data[j][k]][m];
                }
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
            {
                for(int m = 0; m < seq_num; m++)
                {
                    gamma_new[k][m] += gamma[k][m]; // kill bugs
                    // cout << gamma_new[k][m] << " ";
                }
                // cout << endl;
            }

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
                temp[j] += gamma_new[j][k];
        for(int j = 0; j < state_num; j++)
            for(int k = 0; k < state_num; k++)
                model.transition[j][k] = epsilon_new[j][k] / temp[j];

        for(int j = 0; j < state_num; j++)
            temp[i] += gamma_new[j][seq_num - 1];
        for(int j = 0; j < state_num; j++)
            for(int k = 0; k < observation_num; k++)
            {
                model.observation[k][j] = observation_new[k][i] / temp[j];
                // cout << model.observation[k][j];
            }
    }
    cout << endl << "train finished.";
}


/* Input format: ./train iteration model_init.txt seq_model_01.txt model_01.txt */

int main(int argc, char *argv[])
{
    /* input argument command line */
    int iteration = strtol(argv[1], NULL, 10);
    // cout << iteration;
    char *model_init = argv[2];
    char *seq_model = argv[3];
    char *model_name = argv[4];

    loadHMM(&model, model_init);

    /* read input file */
    ifstream input_file(seq_model);
    string str;
    while(input_file >> str)
    {
        cout << str << endl;
        vector<int> temp;
        for (int i = 0; i < str.length(); i++)
            temp.push_back(str[i] - 'A');
        data.push_back(temp);
    }

    cout << data.size();
    train(iteration);
    FILE *saved_path = open_or_die(model_name, "w"  );
    dumpHMM(saved_path, &model);
    
    return 0;
}
