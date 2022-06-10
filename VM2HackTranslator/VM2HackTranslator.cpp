// VM2HackTranslator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
//#include <strstream>
#include <string>
#include <sstream>
#include <algorithm> 
#include <map>
#include <filesystem>
#include <boost/algorithm/string.hpp>
namespace fs = std::filesystem;

enum _commandType {
    C_ARITHMETIC,
    C_PUSH, C_POP,
    C_LABEL, C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
};

std::map<std::string, _commandType> _com =
{
    {"push", C_PUSH}, {"pop", C_POP}, 
    {"add", C_ARITHMETIC},
    {"sub", C_ARITHMETIC},
    {"neg", C_ARITHMETIC},
    {"not", C_ARITHMETIC},
    {"and", C_ARITHMETIC},
    {"or", C_ARITHMETIC},
    {"eq", C_ARITHMETIC},
    {"gt", C_ARITHMETIC},
    {"lt", C_ARITHMETIC},
    {"label", C_LABEL},
    {"goto", C_GOTO},
    {"if-goto", C_IF},
    {"function", C_FUNCTION},
    {"call", C_CALL},
    {"return", C_RETURN}
};

std::ofstream _output;
std::string _outputName, _currentFunction;
int _lineCount = 0, _callCount = 0;

void WriteArithmetic(std::string command)
{
    if (command == "add")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=D+M\n@SP\nM=M+1\n";
        _lineCount += 10;
    }
    else if (command == "sub")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M-D\n@SP\nM=M+1\n";
        _lineCount += 10;
    }
    else if (command == "and")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M&D\n@SP\nM=M+1\n";
        _lineCount += 10;
    }
    else if (command == "or")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nM=M|D\n@SP\nM=M+1\n";
        _lineCount += 10;
    }
    else if (command == "neg")
    {
        _output << "@SP\nM=M-1\nA=M\nM=-M\n@SP\nM=M+1\n";
        _lineCount += 6;

    }
    else if (command == "not")
    {
        _output << "@SP\nM=M-1\nA=M\nM=!M\n@SP\nM=M+1\n";
        _lineCount += 6;

    }
    else if (command == "eq")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n@" << _lineCount + 15 << "\nD;JEQ\n@SP\nA=M\nM=0\n@" << _lineCount + 18 << "\n0;JMP\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n";
        _lineCount += 20;
    }
    else if (command == "lt")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n@" << _lineCount + 15 << "\nD;JLT\n@SP\nA=M\nM=0\n@" << _lineCount + 18 << "\n0;JMP\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n";
        _lineCount += 20;
    }
    else if (command == "gt")
    {
        _output << "@SP\nM=M-1\nA=M\nD=M\n@SP\nM=M-1\nA=M\nD=M-D\n@" << _lineCount + 15 << "\nD;JGT\n@SP\nA=M\nM=0\n@" << _lineCount + 18 << "\n0;JMP\n@SP\nA=M\nM=-1\n@SP\nM=M+1\n";
        _lineCount += 20;
    }
}

void WritePushPop(_commandType command, std::string segment, std::string fileName,  int index)
{
    if (segment == "constant")
    {
        _output << "@"<< index <<"\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        _lineCount += 7;
        return;
    }

    if (command == C_PUSH)
    {
        if (segment == "temp")
        {
            _output << "@" << index + 5 << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            _lineCount += 7;
            return;
        }
        if (segment == "pointer")
        {
            _output << "@" << index + 3 << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            _lineCount += 7;
            return;
        }
        if (segment == "static")
        {
            _output << "@"<<fileName<<"." << index << "\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            _lineCount += 7;
            return;
        }

        if (segment == "local")
            _output << "@LCL\n";
        else if (segment == "argument")
            _output << "@ARG\n";
        else if (segment == "this")
            _output << "@THIS\n";
        else if (segment == "that")
            _output << "@THAT\n";

        _output << "D=M\n@" << index << "\nA=D+A\nD=M\n";
        _output << "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        _lineCount += 10;
    }
    else if (command == C_POP)
    {
        if (segment == "temp")
        {
            _output << "@SP\nM=M-1\nA=M\nD=M\n@" << index + 5 << "\nM=D\n";
            _lineCount += 6;
            return;
        }

        if (segment == "pointer")
        {
            _output << "@SP\nM=M-1\nA=M\nD=M\n@" << index + 3 << "\nM=D\n";
            _lineCount += 6;
            return;
        }

        if (segment == "static")
        {
            _output << "@SP\nM=M-1\nA=M\nD=M\n@" << fileName << "." << index << "\nM=D\n";
            _lineCount += 6;
            return;
        }

        if (segment == "local")
            _output << "@LCL\n";
        else if (segment == "argument")
            _output << "@ARG\n";
        else if (segment == "this")
            _output << "@THIS\n";
        else if (segment == "that")
            _output << "@THAT\n";
        _output << "D=M\n@" << index << "\nD=D+A\n";
        _output << "@SP\nM=M-1\nA=M\nD=D+M\nA=D-M\nM=D-A\n";
        _lineCount += 10;
    }

}

void WriteLabel(std::string label)
{
    _output << "(" << label << ")\n";
    
}

void WriteGoto(std::string label)
{
    _output << "@" << label << "\n0;JMP\n";
    _lineCount += 2;
}

void WriteIf(std::string label)
{
    _output << "@SP\nM=M-1\nA=M\nD=M\n@" << label << "\nD;JNE\n";
    _lineCount += 6;
}

void WriteFunction(std::string functionName, int nbLocal)
{
    WriteLabel(functionName);
    for (int i = 0; i < nbLocal; i++)
        WritePushPop(C_PUSH, "constant","", 0);
}

void WriteCall(std::string functionName, int nbArg)
{
    nbArg += 5;
    std::string retAddress = "$ret";
    retAddress.insert(0, functionName);
    retAddress.append(".");
    retAddress.append(std::to_string(_callCount));
    _output << "@" << retAddress << "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; //7
    _output << "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; //7
    _output << "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; //7
    _output << "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; //7
    _output << "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n"; //7
    
    _output << "@SP\nD=M\n@" <<nbArg  << "\nD=D-A\n@ARG\nM=D\n";//6
    _output << "@SP\nD=M\n@LCL\nM=D\n";//4
    _lineCount += 45;
            
    WriteGoto(functionName);
    WriteLabel(retAddress);
    _callCount += 1;
}

void WriteReturn()
{
    _output << "@6\n0;JMP\n";
    _lineCount += 2;
    
}

void WriteAssembly(std::string fileName)
{
    std::string line;
    std::ifstream fi;

    fi.open(fileName);
    fileName = fileName.substr(0, fileName.find(".vm"));
    fileName = fileName.erase(0,fileName.find("\\")+1);
    while (!fi.eof())
    {
        std::stringstream s;
        getline(fi, line);
        line = line.substr(0, line.find("//", 0));
        if (!line.empty())
        {
            boost::trim(line);

            s << line;
            getline(s, line, ' ');
            _commandType command = _com.find(line)->second;

            if (command == C_FUNCTION)
            {
                int nblcl;
                getline(s, line, ' ');
                _currentFunction = line;
                s >> nblcl;
                WriteFunction(_currentFunction, nblcl);
            }

            else if (command == C_CALL)
            {
                int nbarg;
                getline(s, line, ' ');
                s >> nbarg;
                WriteCall(line, nbarg);
            }


            else if (command == C_ARITHMETIC)
            {
                WriteArithmetic(line);
                continue;
            }


            else if (command == C_POP || command == C_PUSH)
            {
                getline(s, line, ' ');
                std::string segment = line;
                int index;
                s >> index;
                WritePushPop(command, segment,fileName, index);
            }

            else if (command == C_LABEL)
            {
                getline(s, line);
                // line = line.substr(0, line.size() - 1);
                line.insert(0, "$");
                line.insert(0, _currentFunction);
                WriteLabel(line);

            }

            else if (command == C_GOTO)
            {
                getline(s, line);
                line.insert(0, "$");
                line.insert(0, _currentFunction);
                WriteGoto(line);

            }

            else if (command == C_IF)
            {
                getline(s, line);
                line.insert(0, "$");
                line.insert(0, _currentFunction);
                WriteIf(line);

            }

            else if (command == C_RETURN)
            {
                WriteReturn();
            }



        }
    }

    fi.close();

}

void WriteBootstrap()
{
    _output << "@261\nD=A\n@SP\nM=D\n";
    
    _output << "@Sys.init\n0; JMP\n";
    _lineCount += 6;
    _output << "@LCL\nD=M\n@FRAME\nM=D\n"; //4
    _output << "@5\nA=D-A\nD=M\n@RET\nM=D\n"; //5
    _output << "@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n";//7
    _output << "@ARG\nD=M\n@SP\nM=D+1\n";//4
    _output << "@FRAME\nD=M\n@1\nA=D-A\nD=M\n@THAT\nM=D\n"; //7
    _output << "@FRAME\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n"; //7
    _output << "@FRAME\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n";  //7
    _output << "@FRAME\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n";  //7
    _output << "@RET\nA=M\n0;JMP\n"; //3

    _lineCount += 51;
    
    
}

int main()
{
    std::ifstream fi;
    std::string path;
    std::string fiName, foName;
    std::size_t found;
    

    std::cin >> path;
    foName = path;
    _outputName = foName;
    foName.append(".asm");
    _output.open(foName);
    WriteBootstrap();

    for (const auto& entry : fs::directory_iterator(path))
    {
        fiName = entry.path().string();
        found = fiName.find(".vm");
        if (found != std::string::npos)
            WriteAssembly(fiName);
    }
    
    
    
    _output.close();

    std::cout << _lineCount << std::endl;

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
