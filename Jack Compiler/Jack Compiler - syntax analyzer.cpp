// Jack Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm> 
#include <map>
#include <filesystem>
#include <boost/algorithm/string.hpp>

namespace fs = std::filesystem;
int _tabCount = 0;
std::ofstream _tokenOut, _parseOut;
std::ifstream _tokenIn;
std::string _tokenOutName, _parseOutName;

enum _tokenType
{
    KEYWORD, SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST
};
std::vector<char> _symbols
{
    '{', '}' , '(' , ')' , '[' , ']' , '.' , ',' , ';' , '+' , '-' , '*' , '/' , '&' , '|' , '<' , '>' , '=' , '~'
};

std::vector<char> _op
{
    '+' , '-' , '*' , '/','&' , '|' , '<' , '>' , '='
};

std::vector<std::string> _keywords
{
    "class" , "constructor" , "function" , "method" , "field" , "static" ,
"var", "int" , "char" , "boolean" , "void" , "true" , "false" , "null" , "this" ,
"let" , "do" , "if" , "else" , "while" , "return"
};

std::vector<std::string> _statements
{
    "let" , "do" , "if" ,  "while" , "return"
};
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}





class parser {
private:

    void debugTab()
    {
        for (int j = 0; j < _tabCount; j++)
            _parseOut << "  ";
    }

    void putToken()
    {
        std::string line;
        debugTab();
        getline(_tokenIn, line);
        _parseOut << line << std::endl;
    }

    std::string LookAhead()
    {
        std::string line;
        std::streampos cur;
        cur = _tokenIn.tellg();
        getline(_tokenIn, line, ' ');
        getline(_tokenIn, line, ' ');
        _tokenIn.seekg(cur);

        return line;
    }
    std::string LookTwiceAhead()
    {
        std::string line;
        std::streampos cur;
        cur = _tokenIn.tellg();
        getline(_tokenIn, line);
        getline(_tokenIn, line, ' ');
        getline(_tokenIn, line, ' ');
        _tokenIn.seekg(cur);

        return line;
    }
    void CompileClass()
    {
        std::string line;
        _parseOut << "<class>" << std::endl;
        _tabCount += 1;

        for (int i = 0; i < 3; i++)
        {
            putToken();
        }

        line = LookAhead();
        
        while (line == "static" || line == "field")
        {
            CompileClassVarDec();
            line = LookAhead();
            
        }
            
        while (line == "constructor" || line == "function" || line == "method")
        {
            CompileSubroutineDec();
            line = LookAhead();

        }

        putToken();

        _tabCount -= 1;
        _parseOut << "</class>" << std::endl;
    }

    void CompileClassVarDec()
    {
        std::string line;
        debugTab();
        _parseOut << "<classVarDec>" << std::endl;
        _tabCount += 1;

        for (int i = 0; i < 3; i++)
        {
            putToken();
        }
        line = LookAhead();

        while (line !=";")
        {
            putToken();
            putToken();
            line = LookAhead();

        }
        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</classVarDec>" << std::endl;

    }

    void CompileSubroutineDec()
    {
        std::string line;

        debugTab();
        _parseOut << "<subroutineDec>" << std::endl;
        _tabCount += 1;

        for (int i = 0; i < 4; i++)
        {
            putToken();
        }

        CompileParameterList();

        putToken();

        CompileSubroutineBody();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</subroutineDec>" << std::endl;

    }

    void CompileParameterList()
    {    
        std::string line;
        

        debugTab();
        _parseOut << "<parameterList>" << std::endl;
        _tabCount += 1;

        
        line = LookAhead();

        if (line != ")")
        {
            putToken();
            putToken();


            line = LookAhead();
            while (line != ")")
            {
                putToken();
                putToken();
                putToken();

                line = LookAhead();
            }
        }
        
        _tabCount -= 1;
        debugTab();
        _parseOut << "</parameterList>" << std::endl;
    }    
    
    void CompileSubroutineBody()
    {
        std::string line;

        debugTab();
        _parseOut << "<subroutineBody>" << std::endl;
        _tabCount += 1;

        putToken();

        line = LookAhead();
        while (line == "var")
        {
            CompileVarDec();
            line = LookAhead();
        }
        
        CompileStatements();

        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</subroutineBody>" << std::endl;
    }

    void CompileVarDec()
    {    
        std::string line;
        debugTab();
        _parseOut << "<varDec>" << std::endl;
        _tabCount += 1;
        for (int i = 0; i < 3; i++)
        {
            putToken();
        }
        
        line = LookAhead();

        while (line != ";")
        {
            putToken();
            putToken();

            line = LookAhead();

        }
        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</varDec>" << std::endl;
    }    
         
    void CompileStatements()
    {   
        std::string line;
        std::vector<std::string>::iterator itstate;

        debugTab();
        _parseOut << "<statements>" << std::endl;
        _tabCount += 1;
        line = LookAhead();
        itstate = std::find(_statements.begin(), _statements.end(), line);
        while (itstate != _statements.end())
        {
            if (line == "if")
                CompileIf();
            else if (line == "do")
                CompileDo();
            else if (line == "let")
                CompileLet();
            else if (line == "return")
                CompileReturn();
            else if (line == "while")
                CompileWhile();

            line = LookAhead();
            itstate = std::find(_statements.begin(), _statements.end(), line);
            
        }

        _tabCount -= 1;
        debugTab();
        _parseOut << "</statements>" << std::endl;

    }   
        
    void CompileDo()
    {    
        std::string line;
        debugTab();
        _parseOut << "<doStatement>" << std::endl;
        _tabCount += 1;

        putToken();

        putToken();
        line = LookAhead();
        if (line == ".")
        {
            putToken();
            putToken();
        }

        putToken();

        CompileExpressionList();

        putToken();
        putToken();
        _tabCount -= 1;
        debugTab();
        _parseOut << "</doStatement>" << std::endl;
    }    
         
    void CompileLet()
    {    
        std::string line;
        debugTab();
        _parseOut << "<letStatement>" << std::endl;
        _tabCount += 1;

        putToken();
        putToken();

        line = LookAhead();
        if (line == "[")
        {
            putToken();
            
            CompileExpression();

            putToken();
        }

        putToken();

        CompileExpression();

        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</letStatement>" << std::endl;
    }    
         
    void CompileWhile()
    {    
        std::string line;
        debugTab();
        _parseOut << "<whileStatement>" << std::endl;
        _tabCount += 1;

        putToken();
        putToken();

        CompileExpression();

        putToken();
        putToken();

        CompileStatements();

        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</whileStatement>" << std::endl;
    }    
         
    void CompileReturn()
    {    
        std::string line;
        debugTab();
        _parseOut << "<returnStatement>" << std::endl;
        _tabCount += 1;

        putToken();

        line = LookAhead();
        if (line != ";")
            CompileExpression();
        
        putToken();

        _tabCount -= 1;
        debugTab();
        _parseOut << "</returnStatement>" << std::endl;
    }    
         
    void CompileIf()
    {
        std::string line;
        debugTab();
        _parseOut << "<ifStatement>" << std::endl;
        _tabCount += 1;

        putToken();
        putToken();

        CompileExpression();

        putToken();
        putToken();

        CompileStatements();

        putToken();

        line = LookAhead();
        if (line == "else")
        {
            putToken();
            putToken();

            CompileStatements();

            putToken();
        }

        _tabCount -= 1;
        debugTab();
        _parseOut << "</ifStatement>" << std::endl;
    }

    void CompileExpression()
    {
        std::string line;
        std::vector<char>::iterator itops;

        debugTab();
        _parseOut << "<expression>" << std::endl;
        _tabCount += 1;

        CompileTerm();

        line = LookAhead();
        itops = std::find(_op.begin(), _op.end(), line[0]);
        while (itops != _op.end())
        {
            putToken();
            CompileTerm();

            line = LookAhead();
            itops = std::find(_op.begin(), _op.end(), line[0]);
        }


        _tabCount -= 1;
        debugTab();
        _parseOut << "</expression>" << std::endl;
    }

    void CompileTerm()
    {
        std::string line;
        debugTab();
        _parseOut << "<term>" << std::endl;
        _tabCount += 1;

        line = LookAhead();
        if(line == "~" || line == "-")
        {
            putToken();
            CompileTerm();
        }
        else if (line == "(")
        {
            putToken();
            CompileExpression();
            putToken();
        }
        else
        {
            line = LookTwiceAhead();
            if (line == "[")
            {
                putToken();
                putToken();
                CompileExpression();
                putToken();
            }

            else if (line == "." || line == "(")
            {
                putToken();
                line = LookAhead();
                if (line == ".")
                {
                    putToken();
                    putToken();
                }

                putToken();

                CompileExpressionList();

                putToken();
            }

            else
                putToken();
        }


        _tabCount -= 1;
        debugTab();
        _parseOut << "</term>" << std::endl;
    }

    void CompileExpressionList()
    {
        std::string line;
        debugTab();
        _parseOut << "<expressionList>" << std::endl;
        _tabCount += 1;

        line = LookAhead();
        if (line != ")")
        {
            CompileExpression();
            line = LookAhead();
            while (line == ",")
            {
                putToken();
                CompileExpression();

                line = LookAhead();
            }

        }

        _tabCount -= 1;
        debugTab();
        _parseOut << "</expressionList>" << std::endl;
    }

public:
    void Constructor(std::string fileName)
    {
        std::string line, lineBuff;
        

        fileName = fileName.substr(0, fileName.find(".jack"));
        fileName = fileName.erase(0, fileName.find("\\") + 1);
        
        fileName.append(".xml");

        _parseOut.open(fileName);
        _tokenIn.open(_tokenOutName);

        getline(_tokenIn, line);


        CompileClass();
        _parseOut.close();
        _tokenIn.close();
    }
};

class tokenizer
{
private:
    _tokenType TokenType(std::string tok)
    {
        std::vector<char>::iterator itsym; 
        std::vector<std::string>::iterator itkeys;
        if(tok.length() == 1)
        {
            itsym = std::find(_symbols.begin(), _symbols.end(), tok[0]);
            if (itsym != _symbols.end())
                return SYMBOL;
        }
            

        itkeys = std::find(_keywords.begin(), _keywords.end(), tok);
        if (itkeys != _keywords.end())
            return KEYWORD;

        if (tok[0] == '\"')
            return STRING_CONST;

        if (is_number(tok))
            return INT_CONST;
        else if(tok != "" && tok != " ")
            return IDENTIFIER;
    }

    //given a string, return the index where a symbol or a complete keyword is found, if no symbol is found, then the whole string is assumed to be a single token
    int GetToken(std::string line)
    {   
        std::vector<char>::iterator itsym;
        std::vector<std::string>::iterator itkeys;

        for (int i = 0; i < line.length(); i++)
        {
            itsym = std::find(_symbols.begin(), _symbols.end(), line[i]);
            itkeys = std::find(_keywords.begin(), _keywords.end(), line.substr(0,i+1));
            if (itsym != _symbols.end())
                return i;
            if (itkeys != _keywords.end())
                return i+1;
        }
           

        return (line.length() );
    }

    void WriteToken(std::string token, _tokenType type)
    {
        if (type == KEYWORD)
        {
            _tokenOut << "<keyword> " << token << " </keyword>" << std::endl;
        }
        else if (type == SYMBOL)
        {
            if (token == "<")
                token = "&lt;";
            if (token == ">")
                token = "&gt;";
            if (token == "&")
                token = "&amp;";
            _tokenOut << "<symbol> " << token << " </symbol>" << std::endl;
            
        }
        else if (type == STRING_CONST)
        {
            token = token.substr(1, token.length() - 1);
            _tokenOut << "<stringConstant> " << token << " </stringConstant>" << std::endl;
        }
        else if (type == INT_CONST)
        {
            _tokenOut << "<integerConstant> " << token << " </integerConstant>" << std::endl;
        }
        else if (type == IDENTIFIER)
        {
            _tokenOut << "<identifier> " << token << " </identifier>" << std::endl;
        }
    }

public:
    void Tokenize(std::string fileName)
    {
        std::string line, lineBuff;
        std::ifstream fi;
        std::size_t found;        

        int tokIndex;

        fi.open(fileName);
        fileName = fileName.substr(0, fileName.find(".jack"));
        fileName = fileName.erase(0, fileName.find("\\") + 1);
        _tokenOutName = fileName;
        _tokenOutName.append("T.xml");
        _tokenOut.open(_tokenOutName);
        _tokenOut << "<tokens>\n";
        while (!fi.eof())
        {
            std::stringstream s;
            std::string token;
            _tokenType type;

            getline(fi, line);
            //remove comments
            line = line.substr(0, line.find("//", 0));
            //line = line.substr(0, line.find("/**", 0));
            //doesn't properly remove inline documentation comment, remove any insstruction on the same line
            found = line.find("/**");
            if (found != std::string::npos)
            {
                found = line.find("*/");
                while (found == std::string::npos)
                {
                    getline(fi, line);
                    found = line.find("*/");
                }
                //line = line.substr(found, line.size()-found);
                getline(fi, line);
                    
            }
            boost::trim(line);            
            //line.erase(remove_if(line.begin(), line.end(), isspace), line.end());

            //_tokenOut << line << " " << line.length() << std::endl;
            lineBuff = line;
            found = 1;
            while (found != std::string::npos)
            {
                found = lineBuff.find(" ");
                line = lineBuff.substr(0, lineBuff.find(" "));
                lineBuff = lineBuff.substr( lineBuff.find(" ") + 1, lineBuff.length() - lineBuff.find(" "));
                
                
                while (!line.empty())
                {
                    if (line.length() == 1)
                    {
                        tokIndex = -1;
                        token = line;
                        line.clear();
                    }
                    else
                    {
                        tokIndex = GetToken(line);

                        token = line.substr(0, std::max(tokIndex, 1));
                        line = line.substr(std::max(tokIndex, 1), line.length() - std::max(tokIndex, 1));
                    }
                    type = TokenType(token);

                    if (type == STRING_CONST)
                        if (token[token.length() - 1] != '\"')
                        {
                            //std::cout << lineBuff << std::endl;
                            token.append(" ");
                            token.append(lineBuff.substr(0, lineBuff.find("\"")));
                            lineBuff = lineBuff.substr(lineBuff.find("\"") + 1, lineBuff.length() - lineBuff.find("\""));
                        }
                       
                    

                    WriteToken(token, type);
                }
            }

        }


        _tokenOut << "</tokens>";
        fi.close();
        _tokenOut.close();
    }
};

int main()
{
    std::ifstream fi;
    std::string path;
    std::string fiName, foName;
    std::size_t found;
    tokenizer tok;
    parser comp;
    std::cin >> path;
          
    
    for (const auto& entry : fs::directory_iterator(path))
    {
        fiName = entry.path().string();
        found = fiName.find(".jack");
        if (found != std::string::npos)
        {
            tok.Tokenize(fiName);
            comp.Constructor(fiName);
        }
            
            
    }



    

    
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
