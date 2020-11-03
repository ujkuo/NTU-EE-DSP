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
#include <array>
using namespace std;

HMM model;
vector< vector<int> > data; // 10000*50
const int STATE_NUM = 6;
const int OBSERVATION_NUM = 6;
const int SEQ_NUM = 50;

void train(int iteration)
{
    for(int i = 0; i < iteration; i++)
    {
        // cout << "iteration: " << i << endl;
        // cout << data.size();
        double PI[STATE_NUM] = {0.0};
        double GAMMA[STATE_NUM][OBSERVATION_NUM] = {0.0};
        double GAMMA_T[STATE_NUM] = {0.0};
        double EPSILON[STATE_NUM][STATE_NUM] = {0.0};

        for(int j = 0; j < data.size(); j++)
        {
            // cout << j << " ";
            double alpha[STATE_NUM][SEQ_NUM] = {0.0};
            double betta[STATE_NUM][SEQ_NUM] = {0.0};
            double gamma[STATE_NUM][SEQ_NUM] = {0.0};
            double epsilon[SEQ_NUM-1][STATE_NUM][STATE_NUM] = {0.0};
            
            /* array<array<double, SEQ_NUM>, STATE_NUM>  alpha;
            array<array<double, SEQ_NUM>, STATE_NUM>  betta;
            array<array<double, SEQ_NUM>, STATE_NUM>  gamma;
            array<array<array<double, STATE_NUM>, STATE_NUM>, SEQ_NUM - 1>   epsilon; */

            /* calculate alpha */
            for(int k = 0; k < STATE_NUM; k++)
                alpha[k][0] = model.initial[k] * model.observation[data[j][0]][k];

            for(int k = 1; k < SEQ_NUM; k++)
            {
                for(int m = 0; m < STATE_NUM; m++)
                {
                    double sum = 0.0;
                    for(int n = 0; n < STATE_NUM; n++)
                        sum += alpha[n][k-1] * model.transition[n][m];
                    alpha[m][k] = sum * model.observation[data[j][k]][m];
                }
            }

            /* calculate betta */
            for(int k = 0; k < STATE_NUM; k++)
                betta[k][SEQ_NUM-1] = 1.0;
            for(int k = SEQ_NUM - 2; k >= 0; k--)
            {
                for(int m = 0; m < STATE_NUM; m++)
                {
                    double sum = 0.0;
                    for(int n = 0; n < STATE_NUM; n++)
                        sum += model.transition[m][n] * model.observation[data[j][k+1]][n] * betta[n][k+1];
                    betta[m][k] = sum;
                }
            }

            /* calculate gamma: NEW METHOD*/
            for(int k = 0; k < SEQ_NUM; k++)
            {
                double sum = 0.0;
                for(int m = 0; m < STATE_NUM; m++)
                {
                    gamma[m][k] = alpha[m][k] * betta[m][k];
                    sum += gamma[m][k];
                }
                for(int m = 0; m < STATE_NUM; m++)
                    gamma[m][k]= gamma[m][k] / sum;
            }

            /* calculate epsilon */
            for(int k = 0; k < SEQ_NUM-1; k++)
            {
                double sum = 0.0;
                for(int m = 0; m < STATE_NUM; m++)
                    for(int n = 0; n < STATE_NUM; n++)
                    {
                        epsilon[k][m][n] = alpha[m][k] * model.transition[m][n] * model.observation[data[j][k+1]][n] * betta[n][k+1];
                        sum += epsilon[k][m][n];
                    }

                /* NEW METHOD */
                for(int m = 0; m < STATE_NUM; m++)
                    for(int n = 0; n < STATE_NUM; n++)
                        epsilon[k][m][n] = epsilon[k][m][n] / sum;
            }

           /*===================================*/
            /* Below is the new method to update */
            for(int m = 0; m < STATE_NUM; m++)
                PI[m] += gamma[m][0];

            for(int m = 0; m < STATE_NUM; m++)
            {
                for(int n = 0; n < SEQ_NUM; n++)
                    GAMMA[m][data[j][n]] += gamma[m][n];
                GAMMA_T[m] += gamma[m][SEQ_NUM - 1];
            }

            for(int m = 0; m < STATE_NUM; m++)
            {
                for(int n = 0; n < STATE_NUM; n++)
                {
                    double sum = 0.0;
                    for(int k = 0; k < SEQ_NUM - 1; k++)
                        sum += epsilon[k][m][n];
                    EPSILON[m][n] += sum;
                }
            }
            /*===================================*/
       }


        /*====================================================*/
        /* Below is the new method to update model parameters */

        for(int m = 0; m < STATE_NUM; m++)
            model.initial[m] = PI[m] / data.size();

        for(int m = 0; m < STATE_NUM; m++)
        {
            double sum = 0.0;
            for(int n = 0; n < OBSERVATION_NUM; n++)
                sum += GAMMA[m][n];
            sum -= GAMMA_T[m];
            for(int n = 0; n < STATE_NUM; n++)
                model.transition[m][n] = EPSILON[m][n] / sum;
        }

        for(int m = 0; m < STATE_NUM; m++)
        {
            double sum = 0.0;
            for(int n = 0; n < OBSERVATION_NUM; n++)
                sum += GAMMA[m][n];
            for(int n = 0; n < OBSERVATION_NUM; n++)
                model.observation[n][m] = GAMMA[m][n] / sum;
        }

        /*====================================================*/

    }
    // cout << endl << "train finished.";
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
        // cout << str << endl;
        vector<int> temp;
        for (int i = 0; i < str.length(); i++)
            temp.push_back(str.at(i) - 'A');
        data.push_back(temp);
    }

    // cout << data.size();
    train(iteration);
    FILE *saved_path = open_or_die(model_name, "w"  );
    dumpHMM(saved_path, &model);
    
    return 0;
}
