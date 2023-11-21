#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bitset>
#include <set>
#include <map>
#include <math.h>
using namespace std;
#define below_diagonal (row > col) // for implication table

typedef struct Outcome // for FSM table
{
    string next_state;
    int output;

    Outcome &operator=(const Outcome &outcome)
    {
        next_state = outcome.next_state;
        output = outcome.output;
        return *this;
    }
} Outcome;

struct myCompare // for the key's comparision of map
{
    bool operator()(const std::string &a, const std::string &b) const
    {
        if (a.length() == b.length())
        {
            // use ASCII code to compare when length are the same
            return a < b;
        }
        else
        {
            // shorter string would go to the front
            return a.length() < b.length();
        }
    }
};

typedef struct state_pair // for implication table
{
    string row_next_state;
    string col_next_state;
} state_pair;

typedef struct Output_pairs // for implication table
{
    vector<int> outputs;

    // this function is a must because set would automatically order itself, so it need a rule of comparision
    bool operator<(const Output_pairs &right) const // the "const" is a must because comparision of set's element must be const element
    {
        return (this->outputs < right.outputs);
    }

    bool operator==(const Output_pairs &right) const // the "const" is a must because comparision of set's element must be const element
    {
        return (this->outputs == right.outputs);
    }
} Output_pairs;

typedef struct implications // for implication table
{
    vector<state_pair> pairs;
    bool compatible = true;
} implications;

void read_file(ifstream &in_file, int &in_bits, int &out_bits, int &in_relations, int &in_states,
               string &init_state, vector<string> &states, vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> &FSM_table);

void write_kiss(ofstream &out_file, const int &in_bits, const int &out_bits,
                const string &init_state, const vector<string> &states, const vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> FSM_table);

void write_dot(ofstream &out_file, const string &init_state,
               const vector<string> &states, const vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> FSM_table);

void preprocessing(map<string, map<string, Outcome>, myCompare> FSM_table, vector<vector<implications>> &implication_table, vector<string> &states, vector<string> &input_seq);

void find_compatibility(vector<vector<implications>> &implication_table, vector<string> &states);

void merge(map<string, map<string, Outcome>, myCompare> &FSM_table, vector<vector<implications>> &implication_table, vector<string> &states, vector<string> &input_seq);

void simplify(vector<string> &states, vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> &FSM_table);

int main(int argc, char **argv)
{
    int in_bits;
    int out_bits;
    int in_relations = -1;
    int in_states = -1;
    ifstream in_file(argv[1]);
    ofstream input_dot(argv[2]); // bonus: the not simplified dot file
    ofstream out_kiss(argv[3]);  // simplified kiss
    ofstream out_dot(argv[4]);   // simplified dot
    string init_state;
    vector<string> states;
    vector<string> input_seq;
    map<string, map<string, Outcome>, myCompare> FSM_table;
    read_file(in_file, in_bits, out_bits, in_relations, in_states, init_state, states, input_seq, FSM_table);
    write_dot(input_dot, init_state, states, input_seq, FSM_table); // bonus: the not simplified dot file
    simplify(states, input_seq, FSM_table);
    write_kiss(out_kiss, in_bits, out_bits, init_state, states, input_seq, FSM_table); // simplified kiss
    write_dot(out_dot, init_state, states, input_seq, FSM_table);                      // simplified dot
    return 0;
}

void read_file(ifstream &in_file, int &in_bits, int &out_bits, int &in_relations, int &in_states,
               string &init_state, vector<string> &states, vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> &FSM_table)
{
    string input;
    stringstream line;
    vector<string> data_line;

    while (getline(in_file, input))
    {
        line.clear(); // To clear string stream
        line.str(""); // To clear string stream
        if (input == ".end_kiss" || input == ".end_kiss\r")
            break;

        string command;
        line << input;
        line >> command;
        if (command == ".start_kiss")
            continue;
        else if (command == ".i")
            line >> in_bits;
        else if (command == ".o")
            line >> out_bits;
        else if (command == ".p")
            line >> in_relations;
        else if (command == ".s")
            line >> in_states;
        else if (command == ".r")
            line >> init_state;
        else // command is input now
        {
            if (in_relations == -1 || in_states == -1) // no .p or .s in file
                data_line.push_back(input);            // store data line, extract data later
            else                                       // there are .p and .s
            {
                string cur_state;
                Outcome outcome;
                line >> cur_state >> outcome.next_state >> outcome.output;
                FSM_table[cur_state][command] = outcome; // add an element in table
            }
        }
    }

    if (data_line.size() != 0) // the procedure of condition of no .p or .s in file
    {
        in_relations = data_line.size();
        for (int i = 0; i < in_relations; i++)
        {
            line.clear(); // To clear string stream
            line.str(""); // To clear string stream
            line << data_line[i];
            string in;
            string cur_state;
            Outcome outcome;
            line >> in >> cur_state >> outcome.next_state >> outcome.output;
            FSM_table[cur_state][in] = outcome; // add an element in table
        }
    }

    for (const auto &OuterPair : FSM_table)
        states.push_back(OuterPair.first); // Get all the states

    for (int i = 0; i < (int)pow(2, in_bits); i++) // Get all possible input
    {
        bitset<64> tmp = (i);                                                     // decimal to binary
        string in = (tmp.to_string()).substr(tmp.to_string().length() - in_bits); // extract the last "in_bits" bit from string
        input_seq.push_back(in);
    }
}

void write_kiss(ofstream &out_file, const int &in_bits, const int &out_bits,
                const string &init_state, const vector<string> &states, const vector<string> &input_seq,
                map<string, map<string, Outcome>, myCompare> FSM_table)
{
    out_file << ".start_kiss" << endl;
    out_file << ".i " << in_bits << endl;
    out_file << ".o " << out_bits << endl;
    out_file << ".p " << (int)(states.size() * pow(2, in_bits)) << endl;
    out_file << ".s " << states.size() << endl;
    out_file << ".r " << init_state << endl;
    for (const string cur_state : states)
    {
        for (const string input : input_seq)
        {
            Outcome outcome = FSM_table[cur_state][input];
            out_file << input << " " << cur_state << " " << outcome.next_state << " " << outcome.output << endl;
        }
    }
    out_file << ".end_kiss" << endl;
}

void write_dot(ofstream &out_file, const string &init_state,
               const vector<string> &states, const vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> FSM_table)
{
    out_file << "digraph STG {" << endl;
    out_file << "\trankdir=LR;" << endl;
    out_file << endl;
    out_file << "\tINIT [shape=point];" << endl;
    for (const string state : states)
        out_file << "\t" << state << " [label=\"" << state << "\"];" << endl;
    out_file << endl;
    out_file << "\tINIT -> " << init_state << endl;
    for (const string cur_state : states)
    {
        for (const string input : input_seq)
        {
            Outcome outcome = FSM_table[cur_state][input];
            out_file << "\t" << cur_state << " -> " << outcome.next_state << " [label=\"" << input << "/" << outcome.output << "\"];" << endl;
        }
    }
    out_file << "}" << endl;
}

// init the implication table:
// (1) find states which cannot be merged at foremost
// (2) build up the implication table
void preprocessing(map<string, map<string, Outcome>, myCompare> FSM_table,
                   vector<vector<implications>> &implication_table, vector<string> &states, vector<string> &input_seq)
{
    // (1) find states which cannot be merged at foremost
    set<Output_pairs> possible_out_pairs;
    // find possible output pairs
    for (const string &state : states)
    {
        Output_pairs output_pairs;
        for (const string &input : input_seq)
            output_pairs.outputs.push_back(FSM_table[state][input].output);
        possible_out_pairs.insert(output_pairs);
    }

    // categorize indices by output pair
    vector<vector<int>> same_out_pair(possible_out_pairs.size());
    int group_num = 0;
    for (const Output_pairs &group : possible_out_pairs)
    {
        for (int i = 0; i < states.size(); i++)
        {
            Output_pairs output_pairs;
            for (int j = 0; j < input_seq.size(); j++)
                output_pairs.outputs.push_back(FSM_table[states[i]][input_seq[j]].output);
            if (output_pairs == group)
                same_out_pair[group_num].push_back(i);
        }
        group_num++;
    }

    // if output pair of [row and col] are not the same, then they must can not be merged
    for (int row = 0; row < states.size(); row++)
    {
        for (int col = 0; col < states.size(); col++)
        {
            if (below_diagonal)
            {
                int row_group, col_group;
                for (int i = 0; i < group_num; i++)
                {
                    for (const int &idx : same_out_pair[i])
                    {
                        if (row == idx)
                            row_group = i;
                        if (col == idx)
                            col_group = i;
                    }
                }
                if (row_group != col_group)
                    implication_table[row][col].compatible = false;
            }
        }
    }

    // (2) build up the implication table
    for (int row = 0; row < states.size(); row++)
    {
        for (int col = 0; col < states.size(); col++)
        {
            if (below_diagonal)
            {
                vector<state_pair> tmp;
                for (int idx = 0; idx < input_seq.size(); idx++)
                {
                    state_pair pair;
                    pair.row_next_state = FSM_table[states[row]][input_seq[idx]].next_state;
                    pair.col_next_state = FSM_table[states[col]][input_seq[idx]].next_state;
                    tmp.push_back(pair);
                }
                implication_table[row][col].pairs.assign(tmp.begin(), tmp.end());
            }
            else
                implication_table[row][col].compatible = false;
        }
    }
}

void find_compatibility(vector<vector<implications>> &implication_table, vector<string> &states)
{
    while (1)
    {
        bool have_change = false;
        for (int row = 0; row < states.size(); row++)
        {
            for (int col = 0; col < states.size(); col++)
            {
                if (below_diagonal && implication_table[row][col].compatible)
                {
                    for (int idx = 0; idx < implication_table[row][col].pairs.size(); idx++)
                    {
                        if (implication_table[row][col].pairs[idx].row_next_state != implication_table[row][col].pairs[idx].col_next_state)
                        {

                            int next_row, next_col;
                            for (int i = 0; i < states.size(); i++)
                            {
                                if (states[i] == implication_table[row][col].pairs[idx].row_next_state)
                                    next_row = i;
                                if (states[i] == implication_table[row][col].pairs[idx].col_next_state)
                                    next_col = i;
                            }

                            if (!(implication_table[next_row][next_col].compatible || implication_table[next_col][next_row].compatible))
                            {
                                implication_table[row][col].compatible = false;
                                have_change = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!have_change)
            break;
    }
}

void merge(map<string, map<string, Outcome>, myCompare> &FSM_table, vector<vector<implications>> &implication_table,
           vector<string> &states, vector<string> &input_seq)
{
    vector<string> to_delete_states;
    while (1)
    {
        bool have_change = false;
        for (int row = 0; row < states.size(); row++)
        {
            for (int col = 0; col < states.size(); col++)
            {
                if (below_diagonal && implication_table[row][col].compatible)
                {
                    have_change = true;
                    to_delete_states.push_back(states[row]);
                    // update FSM table: replace [row state] with [col state] in all the Outcome field because [row state] will be deleted
                    for (const string &state : states)
                    {
                        for (const string &input : input_seq)
                        {
                            if (FSM_table[state][input].next_state == states[row])
                                FSM_table[state][input].next_state = states[col];
                        }
                    }

                    // update implication table: remark those [row and col] as "uncompatible" because [row state] will be deleted
                    for (int i = 0; i < states.size(); i++)
                    {
                        for (int j = 0; j < states.size(); j++)
                        {
                            if ((i == row || j == row) && below_diagonal)
                                implication_table[i][j].compatible = false;
                        }
                    }
                }
            }
        }

        if (!have_change)
            break;
    }

    // update FSM table: delete redundant states
    for (const string &state : to_delete_states)
        FSM_table.erase(state);

    // update states: delete redundant states
    for (int i = 0; i < to_delete_states.size(); i++)
    {
        for (int j = 0; j < states.size(); j++)
        {
            if (states[j] == to_delete_states[i])
            {
                states.erase(states.begin() + j);
                to_delete_states.erase(to_delete_states.begin() + i);
                i = -1;
                break;
            }
        }
    }
}

void simplify(vector<string> &states, vector<string> &input_seq, map<string, map<string, Outcome>, myCompare> &FSM_table)
{
    vector<vector<implications>> implication_table(states.size(), vector<implications>(states.size()));
    preprocessing(FSM_table, implication_table, states, input_seq);
    find_compatibility(implication_table, states);
    merge(FSM_table, implication_table, states, input_seq);
}