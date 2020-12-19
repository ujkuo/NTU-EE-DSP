#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include "Ngram.h"
#include "File.h"
#include "Vocab.h"
using namespace std;

/* define struct node */
typedef struct node 
{
    double probability;
    string current_word;
    string previous_all_words;
    struct point *previous_node;
}Node;

/* define global variables */
const int NGRAM_ORDER = 2;
Vocab voc;
Ngram model(voc, NGRAM_ORDER);

int cnt = 0;
string s = "<s>", t = "</s>";
vector<string> result;
map<string, vector<string> > Map; // new

/* calculate the bigram probability by using functions in provide header files */
inline double bigram_probability(Vocab& voc, Ngram& model, const char* word1, const char* word2)
{
    VocabIndex w1 = voc.getIndex(word1);
    VocabIndex w2 = voc.getIndex(word2);
    if(w1 == Vocab_None)
        w1 = voc.getIndex(Vocab_Unknown);
    if(w2 == Vocab_None)
        w2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = {w1, Vocab_None};
    return model.wordProb(w2, context);
}

inline void lm_open(char* model_name)
{
    File lmFile(model_name, "r");
    model.read(lmFile);
    lmFile.close();
}

void outfile(char* output, vector<string> result)
{
    ofstream ans(output);
    for(vector<string>::iterator i = result.begin(); i != result.end(); i++)
        ans << (*i) << endl;
    ans.close();
    cout << "Outfile successfully." << endl;
    // for(auto i = ans.begin(); i != ans.end(); i++)
}

/* build a map */
inline void mappify(char* mapping)
{
    ifstream Map_file(mapping);
    if(Map_file.is_open())
    {
        string eachline, zhuyin, chinese;
        while(getline(Map_file, eachline))
        {
            zhuyin = eachline.substr(0, eachline.find(" "));
            chinese = eachline.substr(eachline.find(" ")+1, eachline.size());
            vector<string> tmp_vector;
            //cnt = chinese.find(" ");
            while((cnt = chinese.find(" ")) != string::npos)
            {
                string str = chinese.substr(0, cnt);
                if(str != "") tmp_vector.push_back(str);
                chinese.erase(0, cnt+1);
                
            }
            Map.insert(pair<string, vector<string> >(zhuyin, tmp_vector));
        }
        vector<string> first, last;
        first.push_back(s), last.push_back(t);
        Map.insert(pair<string, vector<string> >(s, first)), Map.insert(pair<string, vector<string> >(t, last));
        Map_file.close();
    }
    else
        cout << "read file break ==";
}

void viterbi(vector<string> sentence)
{
    int len = sentence.size();
    vector< vector<Node> > Tree;
    string all;
    double p = 0.0;
    Node finalNode;
    int times = 0;
    for(vector<string>::iterator i = sentence.begin() + 1; i != sentence.end(); i++)
    {
        vector<Node> subTree;
        for(vector<string>::iterator j = Map[*i].begin(); j != Map[*i].end(); j++)
        {
            Node node;
            Node* preNode;
            double maxProb = -100000.0, finalProb = -100000.0;
            if(times == 0)
            {
                p = bigram_probability(voc, model, s.c_str(), (*j).c_str());
                node.probability = p;
                node.current_word = (*j);
                node.previous_all_words = s + " " + (*j);
                node.previous_node = NULL;
            }
            else
            {
                for(int k = 0; k < Tree[times-1].size(); k++)
                {
                    Node tmpTree = Tree[times-1][k];
                    p = bigram_probability(voc, model, tmpTree.current_word.c_str(), (*j).c_str());
                    double tmp_prob = p + tmpTree.probability;
                    if(maxProb < tmp_prob)
                    {
                        maxProb = tmp_prob;
                        preNode = &Tree[times-1][k];
                        all = tmpTree.previous_all_words;
                    }
                }
                node.probability = p;
                node.current_word = (*j);
                node.previous_all_words = all + " " + (*j);
                node.previous_node = preNode;

                if(times == len - 2)
                    if(finalProb < maxProb)
                    {
                        finalNode = node;
                        finalProb = maxProb;
                    }
                //subTree.push_back(node);
            }
            subTree.push_back(node);
        }
        Tree.push_back(subTree);
        times ++;
    }
    result.push_back(finalNode.previous_all_words);
}

void segment(char* segmented_file)
{
    ifstream data(segmented_file);
    string line;
    if(data.is_open())
    {
        cout << "Open segment file successfully." << endl;
        while(getline(data, line))
        {
            // int times = 0;
            vector<string> sentence;
            sentence.push_back(s);
            // count = line.find(" ")
            while((cnt = line.find(" ")) != string::npos)
            {
                string str = line.substr(0, line.find(" "));
                if(str != "") sentence.push_back(str);
                line.erase(0, cnt+1);
            }
            if(cnt == line.find("\n")) sentence.push_back(t);

            viterbi(sentence);
            // times++;

        }
        data.close();
    }
}


/* input format: ./mydisambig <segmented file> <mapping> <model> <output> */
int main(int argc, char *argv[])
{
    // cout << "argv[2] is " << argv[2] << endl;
    // cout << "Test successful" << endl;
    char* segmented_file = argv[1];
    char* mapping = argv[2];
    char* model_name = argv[3];
    char* output = argv[4];
    cout << segmented_file << " " << mapping << " " << model_name << " " << output << endl;
    
    lm_open(model_name);
    mappify(mapping);
    segment(segmented_file);
    outfile(output, result);

    return 0;
}
