#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <iterator>
#include <algorithm>
#include "Ngram.h"
#include "File.h"
#include "Vocab.h"
using namespace std;

const int NGRAM_ORDER = 2;
Vocab voc;
Ngram model(voc, NGRAM_ORDER);

typedef struct n{
    int index; // used to store the index of pre-node
    double probability;
    string gram;
}Node;

map<string, vector<Node> > MAP;
map<string, vector<string> > mapu; // new
vector<string> answer;

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

inline void mm(char* mapping)
{
    ifstream map_file(mapping);
    string tmp;
    while(getline(map_file, tmp))
    {
        string word = tmp.substr(0,2);
        int length = tmp.length();
        vector<Node> line;
        for(int i = 0; i < length; i++)
        {
            if(tmp[i] != ' ')
            {
                Node ZhuYin;
                ZhuYin.index = 0;
                ZhuYin.probability = 0.0;
                ZhuYin.gram = tmp.substr(i,2);
                line.push_back(ZhuYin);
                i++;
            }
            else
                continue;
        }
        MAP[word] = line;
        // cout << MAP[word];
    }
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

vector<string> process(string line)
{
    vector<string> return_vector;
    int length = line.length();
    for(int i = 0; i < length; i++)
    {
        if(line[i] != ' ')
        {
            string tmp = line.substr(i,2);
            return_vector.push_back(tmp);
            i++;
        }
        else
            continue;
    }
    return return_vector;
}

void backtrack(vector<string> answer, vector< vector<Node> >graph)
{
    int current_index = 0;
    for(vector< vector<Node> >::iterator i = graph.end()-1; i != graph.begin()-1; i--)
    {
        answer.push_back((*i)[current_index].gram);
        current_index = (*i)[current_index].index;
        //cout << *i;
    }
}

void vv(char* segment_file, vector<string>& answer, Vocab& voc, Ngram& model, map<string, vector<Node> > MAP, char* output)
{
    ifstream segment(segment_file);
    //ofstream decode(output);
    string line;
    vector< vector<Node> > graph;
    while(getline(segment, line))
    {
        graph.clear();
        vector<string> text = process(line);
        vector<Node> last;
        last.push_back((Node){0, 0.0, "</s>"});
        graph.push_back(last);

        // find first probability
        for(vector<Node>::iterator i = graph.begin()->begin(); i != graph.begin()->end(); i++)
        {
            // char* tmp = i->gram.c_str();
            i->probability = bigram_probability(voc, model, "", i->gram.c_str());
        }

        for(vector< vector<Node> >::iterator i = graph.begin()+1; i != graph.end(); i++)
        {
            for(vector<Node>::iterator j = i->begin(); j != i->end(); j++)
            {
                double prob = -100000;
                int target = 0, current_index = 0;
                for(vector<Node>::iterator k = (i-1)->begin(); k != (i-1)->end(); k++)
                {
                    // char* w1 = k->gram.c_str(), w2 = j->gram.c_str();
                    double temp = bigram_probability(voc, model, k->gram.c_str(), j->gram.c_str()) + k->probability;
                    if(temp >= prob)
                    {
                        target = current_index;
                        prob = temp;
                    }
                }
                // update node and probability
                j->index = target;
                j->probability = prob;
            }
        }
        backtrack(answer, graph);
        
        /*for(vector<string>::iterator i = answer.begin(); i != answer.end(); i++)
        {
            cout << *i << " ";
        }*/
    }
}


typedef struct point // new
{
    double prob;
    string current_word;
    string previous_all_words;
    struct point *previous_node;
}newNode;

int cnt = 0;
string s = "<s>", t = "</s>";
vector<string> result;

inline void mappify(char* mapping)
{
    ifstream mapu_file(mapping);
    bool flag = mapu_file.is_open();
    if(flag == true)
    {
        string eachline, zhuyin, chinese;
        while(getline(mapu_file, eachline))
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
            mapu.insert(pair<string, vector<string> >(zhuyin, tmp_vector));
        }
        vector<string> first, last;
        first.push_back(s), last.push_back(t);
        mapu.insert(pair<string, vector<string> >(s, first)), mapu.insert(pair<string, vector<string> >(t, last));
        mapu_file.close();
    }
    else
        cout << "read file break ==";
}

void viterbi(vector<string> sentence)
{
    int len = sentence.size();
    vector< vector<newNode> > Tree;
    string all;
    double p = 0.0;
    newNode finalNode;
    int times = 0;
    for(vector<string>::iterator i = sentence.begin() + 1; i != sentence.end(); i++)
    {
        vector<newNode> subTree;
        for(vector<string>::iterator j = mapu[*i].begin(); j != mapu[*i].end(); j++)
        {
            newNode node;
            newNode* preNode;
            double max_prob = -100000.0, final_prob = -100000.0;
            if(times == 0)
            {
                p = bigram_probability(voc, model, s.c_str(), (*j).c_str());
                node.prob = p;
                node.current_word = (*j);
                node.previous_all_words = s + " " + (*j);
                node.previous_node = NULL;
            }
            else
            {
                for(int k = 0; k < Tree[times-1].size(); k++)
                {
                    newNode tmpTree = Tree[times-1][k];
                    p = bigram_probability(voc, model, tmpTree.current_word.c_str(), (*j).c_str());
                    double tmp_prob = p + tmpTree.prob;
                    if(max_prob < tmp_prob)
                    {
                        max_prob = tmp_prob;
                        preNode = &Tree[times-1][k];
                        all = tmpTree.previous_all_words;
                    }
                }
                node.prob = p;
                node.current_word = (*j);
                node.previous_all_words = all + " " + (*j);
                node.previous_node = preNode;

                if(times == len - 2)
                    if(final_prob < max_prob)
                    {
                        finalNode = node;
                        final_prob = max_prob;
                    }
                //subTree.p
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
    /*File lmFile(model_name, "r");
    model.read(lmFile);
    lmFile.close();*/


    // begin of new attempt
    //vector<string> result;
    // int count = 0;
    mappify(mapping);
    /*
    ifstream mapu_file(mapping);
    if(mapu_file.is_open())
    {
        string each_line, zhuyin, chinese;
        while(getline(mapu_file, each_line))
        {
            zhuyin = each_line.substr(0, each_line.find(" "));
            chinese = each_line.substr(each_line.find(" ")+1, each_line.size());
            vector<string> tmp_vector;
            // count = chinese.find(" ");
            while((count = chinese.find(" ")) != string::npos)
            {
                string str = chinese.substr(0, count);
                if(str != "") tmp_vector.push_back(str);
                chinese.erase(0, count+1);
                
            }
            mapu.insert(pair<string, vector<string> >(zhuyin, tmp_vector));
        }
        // add <s> and </s>
        vector<string> first, last;
        first.push_back("<s>"), last.push_back("</s>");
        mapu.insert(pair<string, vector<string> >("<s>", first)), mapu.insert(pair<string, vector<string> >("</s>", last));
        mapu_file.close();
    }*/
    segment(segmented_file);

    /*ifstream data(segmented_file);
    string line;
    if(data.is_open())
    {
        while(getline(data, line))
        {
            vector<string> each_vector;
            each_vector.push_back(s);
            // int nume, times = 0;
            while((cnt = line.find(" ")) != string::npos)
            {
                string str = line.substr(0, line.find(" "));
                if(str != "") each_vector.push_back(str);
                line.erase(0, cnt+1);

            }
            if(cnt == line.find("\n"))
                each_vector.push_back(t);

            vector< vector<newNode> > Tree;
            string ALL;
            double p = 0.0;
            int len = each_vector.size();
            newNode finalNode;
            int times = 0;
            for(vector<string>::iterator i = each_vector.begin()+1; i != each_vector.end(); i++)
            {
                vector<newNode> subTree;
                for(vector<string>::iterator j = mapu[*i].begin(); j != mapu[*i].end(); j++)
                {
                    string first_word = "<s>";
                    newNode eachNode;
                    newNode *preNode;
                    double max_prob = -100000, b = -100000;
                    if(times == 0)
                    {
                        p = bigram_probability(voc, model, first_word.c_str(), (*j).c_str());
                        eachNode.current_word = (*j);
                        eachNode.prob = p;
                        eachNode.previous_all_words = "<s> " + (*j);
                        eachNode.previous_node = NULL;

                    }
                    else
                    {
                        for(int k = 0; k < Tree[times-1].size(); k++)
                        {
                            newNode tmpTree = Tree[times-1][k];
                            p = bigram_probability(voc, model, tmpTree.current_word.c_str(), (*j).c_str());
                            double tmpP = p + tmpTree.prob;
                            if(max_prob < tmpP)
                            {
                                max_prob = tmpP;
                                preNode = &Tree[times-1][k];
                                ALL = tmpTree.previous_all_words;
                            }
                        }
                        eachNode.prob = p;
                        eachNode.current_word = (*j);
                        eachNode.previous_node = preNode;
                        eachNode.previous_all_words = ALL + " " + (*j);

                        if(times == len - 2)
                        {
                            if(b<max_prob)
                            {
                                finalNode = eachNode;
                                b = max_prob;

                            }
                        }
                        
                    }
                    subTree.push_back(eachNode);
                }
                Tree.push_back(subTree);
                times ++;
            }
            result.push_back(finalNode.previous_all_words);

        }
        data.close();
    }
    else
        cout << "2 Break";*/
    outfile(output, result);
    // end of new attempt
    //
    /* input file */
    //ifstream map_file(mapping);
    //mappify(mapping);

    /*File lmfile(model_name, "r");
    model.read(lmfile);
    lmfile.close();*/
    //lm_open(model_name);
    //viterbi(segmented_file, answer, voc, model, MAP, output);

    //char buffer[512];
    //FILE *input_file = fopen(segmented_file, "r");
    /*while(fgets(buffer, sizeof(buffer), input_file) != NULL)
    {
        vector<string> output = viterbi
    }*/
    //outfile(output);

    return 0;
}
