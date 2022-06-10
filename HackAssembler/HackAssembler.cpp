// HackAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include <fstream>
#include <strstream>
#include <string>
#include <sstream>
#include <bitset>
#include <map> 
#include <algorithm> 

std::map<std::string, std::string> dest =
{
    {"M" , "001"},
    {"D" , "010"},
    {"A" , "100"},
    {"MD" , "011"},
    {"DM" , "011"},
    {"AM" , "101"},
    {"MA" , "101"},
    {"DA" , "110"},
    {"AD" , "110"},
    {"AMD" , "111"},
    {"MDA" , "111"},
    {"DAM" , "111"},
    {"MAD" , "111"},
    {"ADM" , "111"},
    {"DMA" , "111"}
};

std::map<std::string, std::string> comp =
{
    {"0", "0101010"},  {"M",   "1110000"},
    {"1", "0111111"},  {"!M",  "1110001"},
    {"-1","0111010"},  {"-M",  "1110011"},
    {"D", "0001100"},  {"M+1", "1110111"}, {"1+M", "1110111"},
    {"A", "0110000"},  {"M-1", "1110010"},
    {"!D","0001101"},  {"D+M", "1000010"}, {"M+D", "1000010"},
    {"!A","0110001"},  {"D-M", "1010011"},
    {"-D","0001111"},  {"M-D", "1000111"},
    {"-A","0110011"},  {"D&M", "1000000"}, {"M&D", "1000000"},
    {"D+1","0011111"}, {"1+D","0011111"}, {"D|M", "1010101"}, {"M|D", "1010101"},
    {"A+1","0110111"}, {"1+A","0110111"},
    {"D-1","0001110"},
    {"A-1","0110010"},
    {"D+A","0000010"}, {"A+D","0000010"},
    {"D-A","0010011"},
    {"A-D","0000111"},
    {"D&A","0000000"}, {"A&D","0000000"},
    {"D|A","0010101"}, {"A|D","0010101"}
};

std::map<std::string, std::string> jump =
{

    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"},

};

std::map<std::string, int> symbol =
{
    {"SP", 0}, {"LCL", 1}, {"ARG", 2}, {"THIS", 3}, {"THAT", 4},
    {"SCREEN", 16384}, {"KBD", 24576},
    {"R0", 0}, {"R1", 1},{"R2", 2},{"R3", 3},{"R4", 4},
    {"R5", 5}, {"R6", 6},{"R7", 7},{"R8", 8},{"R9", 9},
    {"R10", 10}, {"R11",11 },{"R12",12 },{"R13",13 },{"R14", 14},{"R15", 15},
};

std::string toBinary(int n)
{
    std::string r;
    while (n != 0) { r = (n % 2 == 0 ? "0" : "1") + r; n /= 2; }
    return r;
}

void reset(std::stringstream& stream)
{
    const static std::stringstream initial;

    stream.str(std::string());
    stream.clear();
    stream.copyfmt(initial);
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void FirstPass(std::string fiName)
{
    std::ifstream fi;
    std::string line, line2, strung;
    std::size_t found;
    char check;
    int lineCount = 0;
    

    fi.open(fiName);
    
    while (!fi.eof())
    {
        
        std::stringstream s;

        getline(fi, line);
        if (!line.empty())
        {
            line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
            if (line[0] == '/' && line[1] == '/')
                continue;
            line2 = line;
            s << line << std::ends;
            s >> check;
            if (check == '(')
            { 
                getline(s, line, ')');
                symbol.insert(std::pair<std::string, int>(line, lineCount));
                continue;
                               
            }

            lineCount++;

        }
    }

    fi.close();
}

void SecondPass(std::string fiName, std::string foName)
{
    std::ifstream fi;
    std::ofstream fo;
    std::string line, line2, strung;
    std::size_t found;
    char check;
    char instruction[16];
    int varCount = 16;

    fi.open(fiName);
    fo.open(foName);
    while (!fi.eof())
    {
        for (int i = 0; i < 16; i++)
            instruction[i] = '0';
        std::stringstream s;

        getline(fi, line);
        if (!line.empty())
        {
            line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
            if (line[0] == '(')
                continue;
            if (line[0] == '/' && line[1] == '/')
                continue;
            line2 = line;
            s << line << std::ends;
            s >> check;
            if (check == '@')
            {
                fo << '0';
                found = line2.find("//");
                if (found != std::string::npos)
                {
                    getline(s, line, '/');
                }
                else
                {
                    getline(s, line);
                    line = line.substr(0, line.size() - 1);
                }

                if (is_number(line))
                {
                    fo << std::bitset<15>(std::stoi(line)).to_string() << std::endl;
                }
                else if (symbol.find(line) != symbol.end())
                {
                    fo << std::bitset<15>(symbol.find(line)->second).to_string() << std::endl;
                }
                else
                {
                    fo << std::bitset<15>(varCount).to_string() << std::endl;
                    symbol.insert(std::pair<std::string, int>(line,varCount));
                    varCount++;
                }
            }



            else
            {
                instruction[0] = '1';
                instruction[1] = '1';
                instruction[2] = '1';

                reset(s);
                s << line << std::ends;
                found = line2.find('=');
                if (found != std::string::npos)
                {
                    getline(s, line, '=');
                    strung = dest.find(line)->second;
                    instruction[10] = strung[0];
                    instruction[11] = strung[1];
                    instruction[12] = strung[2];

                }

                found = line2.find(';');
                if (found != std::string::npos)
                {
                    getline(s, line, ';');
                    strung = comp.find(line)->second;
                    instruction[3] = strung[0];
                    instruction[4] = strung[1];
                    instruction[5] = strung[2];
                    instruction[6] = strung[3];
                    instruction[7] = strung[4];
                    instruction[8] = strung[5];
                    instruction[9] = strung[6];
                    getline(s, line);
                    strung = jump.find(line.substr(0, 3))->second;
                    instruction[13] = strung[0];
                    instruction[14] = strung[1];
                    instruction[15] = strung[2];

                }
                else {

                    found = line2.find("//");
                    if (found != std::string::npos)
                    {
                        getline(s, line, '/');
                        if (!line.empty())
                        {
                            strung = comp.find(line)->second;
                        }
                        else
                            strung = instruction;

                    }

                    else
                    {
                        getline(s, line);
                        strung = comp.find(line.substr(0, line.size() - 1))->second;
                    }

                    instruction[3] = strung[0];
                    instruction[4] = strung[1];
                    instruction[5] = strung[2];
                    instruction[6] = strung[3];
                    instruction[7] = strung[4];
                    instruction[8] = strung[5];
                    instruction[9] = strung[6];
                }

                for (int i = 0; i < 16; i++)
                    fo << instruction[i];

                fo << std::endl;
            }

        }
    }

    fi.close();
    fo.close();
}



int main()
{
    
    std::string fiName, foName;

    std::size_t found;

    char check;
    char instruction[16];
    
     std::cin >> fiName;
     foName = fiName;
     fiName.append(".asm");
     foName.append(".hack");

     FirstPass(fiName);
     SecondPass(fiName, foName);
    
    return 0;


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
