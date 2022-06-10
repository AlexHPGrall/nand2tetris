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

enum _symKind
{
    s_STATIC, s_FIELD, s_ARG, s_VAR, s_NONE
};

enum _segment
{
    CONST,
    ARG, LOCAL,
    STATIC, THIS,
    THAT, POINTER,
    TEMP
};

enum _command
{
    ADD, SUB,
    NEG, EQ, GT, LT,
    AND, OR, NOT, MUL, DIV
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
_segment symKindToSeg(_symKind sym)
{
    switch (sym)
    {
    case s_STATIC: return STATIC;
        break;
    case s_FIELD: return THIS;
        break;
    case s_ARG: return ARG;
        break; 
    case s_VAR: return LOCAL;
        break;
    case s_NONE:
        break;
    default:
        break;
    }
}
_command getComFromOp(char op)
{
    
    
    if(op ==  '+') return ADD;
    if(op ==  '-') return SUB;
    if(op ==  '=') return EQ;
    if(op ==  '>') return GT;
    if(op ==  '<') return LT;
    if(op ==  '&') return AND;
    if(op ==  '|') return OR;
    if(op ==  '*') return MUL;
    if(op ==  '/') return DIV;
    
    
    
}

class symbolTable {

    int fIndex = 0;
    int sIndex = 0;
    int aIndex = 0;
    int vIndex = 0;

    

    struct Fields
    {
        std::string name, type;
        _symKind kind;
        int index;
    };
    std::vector<Fields> classSymTab, subRSymTab;
    
public:
    void startSubroutine()
    {
        aIndex = 0;
        vIndex = 0;
        subRSymTab.clear();
    }

    
    void define(std::string name, std::string type, _symKind kind)
    {
        Fields field = { name, type , kind , getIndex(kind) };
        if (kind == s_FIELD || kind == s_STATIC)
        {
            classSymTab.push_back(field);
        }
        

        else
        {
            subRSymTab.push_back(field);
        }
    }

    int numberOf_inclass(_symKind skind)
    {
        int nKind = 0;

        for (auto& i : classSymTab)
        {
            if (i.kind == skind)
                nKind++;
        }

        return nKind;

    }

    _symKind kindOf(std::string name)
    {
        std::vector<Fields>::iterator it;
        it = std::find_if(subRSymTab.begin(), subRSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != subRSymTab.end())
            return it->kind;
        it = std::find_if(classSymTab.begin(), classSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != classSymTab.end())
            return it->kind;
        return s_NONE;
    }
    std::string typeOf(std::string name)
    {
        std::vector<Fields>::iterator it;
        it = std::find_if(subRSymTab.begin(), subRSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != subRSymTab.end())
            return it->type;
        it = std::find_if(classSymTab.begin(), classSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != classSymTab.end())
            return it->type;
        return "not found";
    }

    int indexOf(std::string name)
    {
        std::vector<Fields>::iterator it;
        it = std::find_if(subRSymTab.begin(), subRSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != subRSymTab.end())
            return it->index;
        it = std::find_if(classSymTab.begin(), classSymTab.end(),
            [&n = name](const Fields& f) -> bool {return f.name == n; });
        if (it != classSymTab.end())
            return it->index;
        return -1;
    }


    void cleartable()
    {
        /*for (auto& i : classSymTab)
        {
            std::cout << i.name << " ";
            std::cout << i.kind << " ";
            std::cout << i.index << " ";
            std::cout << std::endl;
        }*/
        fIndex = 0;
        sIndex = 0;
        aIndex = 0;
        vIndex = 0;

        classSymTab.clear();
        subRSymTab.clear();
    }

 
    int getIndex(_symKind kind)
    {
        switch (kind)
        {
        case s_STATIC: return sIndex++;
        case s_FIELD: return fIndex++;
        case s_ARG: return aIndex++;
        case s_VAR: return vIndex++;
        }
    }
  
};

class VMwriter
{
public:
    void Constructor(std::string fileName)
    {        
        fileName = fileName.substr(0, fileName.find(".jack"));
        fileName = fileName.erase(0, fileName.find("\\") + 1);

        fileName.append(".vm");

        _parseOut.open(fileName);
                  
    }

    void WritePush(_segment seg, int index)
    {
        _parseOut << "push ";
        switch (seg)
        {
        case CONST: _parseOut << "constant ";
            break;
        case ARG: _parseOut << "argument ";
            break;
        case LOCAL: _parseOut << "local ";
            break;
        case STATIC: _parseOut << "static ";
            break;
        case THIS: _parseOut << "this ";
            break;
        case THAT: _parseOut << "that ";
            break;
        case POINTER: _parseOut << "pointer ";
            break;
        case TEMP: _parseOut << "temp ";
            break;
        default:
            break;
        }

        if (index == -1)
            _parseOut << 1 << std::endl << "neg";
        else
            _parseOut << index;

        _parseOut << std::endl;
    }

    void WritePop(_segment seg, int index)
    {
        _parseOut << "pop ";
        switch (seg)
        {
        case CONST: _parseOut << "constant ";
            break;
        case ARG: _parseOut << "argument ";
            break;
        case LOCAL: _parseOut << "local ";
            break;
        case STATIC: _parseOut << "static ";
            break;
        case THIS: _parseOut << "this ";
            break;
        case THAT: _parseOut << "that ";
            break;
        case POINTER: _parseOut << "pointer ";
            break;
        case TEMP: _parseOut << "temp ";
            break;
        default:
            break;
        }

        _parseOut << index;

        _parseOut << std::endl;
    }

    void WriteArithmetic(_command com)
    {
        switch (com)
        {
        case ADD: _parseOut << "add";
            break;
        case SUB: _parseOut << "sub";
            break;
        case NEG: _parseOut << "neg";
            break;
        case EQ :  _parseOut << "eq";
            break;
        case GT: _parseOut << "gt";
            break;
        case LT: _parseOut << "lt";
            break;
        case AND: _parseOut << "and";
            break;
        case OR: _parseOut << "or";
            break;
        case NOT: _parseOut << "not";
            break;
        case MUL: _parseOut << "call Math.multiply 2";
            break;
        case DIV: _parseOut << "call Math.divide 2";
            break;
        default:
            break;
        }

        _parseOut << std::endl;
    }

    void WriteLabel(std::string label)
    {
        _parseOut << "label " << label << std::endl;
    }

    void WriteGoto(std::string label)
    {
        _parseOut << "goto " << label << std::endl;
    }

    void WriteIf(std::string label)
    {
        _parseOut << "if-goto " << label << std::endl;
    }

    void WriteCall(std::string name, int nargs)
    {
        _parseOut << "call " << name << " "<< nargs<< std::endl;
    }
    void WriteFunction(std::string name, int local)
    {
        _parseOut << "function " << name << " " << local << std::endl;
    }

    void WriteReturn()
    {
        _parseOut << "return" << std::endl;
    }

    void Close()
    {
        _parseOut.close();
    }

};

class parser {
private:
    VMwriter vm;
    symbolTable table;
    std::string className;
    int labIndex;
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
    std::string extractToken()
    {
        std::string token, empt;
        getline(_tokenIn, token, ' ');
        getline(_tokenIn, token, '<');
        getline(_tokenIn, empt);
        token.pop_back();
        return token;
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

    std::string getTokenType()
    {
        std::string line;
        std::streampos cur;
        cur = _tokenIn.tellg();
        
        getline(_tokenIn, line, ' ');
        _tokenIn.seekg(cur);

        return line;
    }

    void CompileClass()
    {
        std::string line;
        for (int i = 0; i < 3; i++)
        {
            getline(_tokenIn, line);
        }

        line = LookAhead();
        
        while (line == "static" || line == "field")
        {
            CompileClassVarDec();
            line = LookAhead();
            
        }
            
        while (line == "constructor" || line == "function" || line == "method")
        {
            CompileSubroutineDec(line);
            line = LookAhead();

        }

        
    }

    void CompileClassVarDec()
    {
        
        std::string line, token, name, type, kind;
        _symKind sk = s_STATIC;
        kind = extractToken();
        if (kind == "field")
            sk = s_FIELD;
        type = extractToken();
        name = extractToken();
        table.define(name, type, sk);
        line = LookAhead();

        while (line != ";")
        {
            getline(_tokenIn, line);
            name = extractToken();
            table.define(name, type, sk);
            line = LookAhead();
        }
        getline(_tokenIn, line);
        

        

    }

    void CompileSubroutineDec(std::string subRTypr)
    {
        std::string line, name, type;
        
        table.startSubroutine();
        type = extractToken();//method function or constructor
        getline(_tokenIn, line);//return type
        name = className;
        name = (name.append(".")).append(extractToken());
        getline(_tokenIn, line);

        CompileParameterList(type);
        getline(_tokenIn, line);
        //TODO: code for constructor(call malloc) or method(add a hidden argument this of type className )
        
        CompileSubroutineBody(name, type);

        

        
       

    }

    void CompileParameterList(std::string funcType)
    {    
        std::string line, name, type;
        
        
        if (funcType == "method")
            table.define("this", "Point", s_ARG);
        
        line = LookAhead();

        if (line != ")")
        {
            type = extractToken();
            name = extractToken();
            table.define(name, type, s_ARG);
            
            line = LookAhead();
            while (line != ")")
            {
                getline(_tokenIn, line);
                type = extractToken();
                name = extractToken();
                table.define(name, type, s_ARG);
                

                line = LookAhead();
            }
        }
        
        
    }    
    
    void CompileSubroutineBody(std::string funcName, std:: string type)
    {
        std::string line;
        int nloc = 0;
                
        getline(_tokenIn, line);

        line = LookAhead();
        while (line == "var")
        {
            nloc += CompileVarDec();
            line = LookAhead();
        }
        vm.WriteFunction(funcName, nloc);
        if (type == "constructor")
        {
            vm.WritePush(CONST, table.numberOf_inclass(s_FIELD));
            vm.WriteCall("Memory.alloc", 1);
            vm.WritePop(POINTER, 0);
        }

        else if (type == "method")
        {
            vm.WritePush(ARG, 0);
            vm.WritePop(POINTER, 0);
        }
        
        CompileStatements();

        

        getline(_tokenIn, line);
    }

    int CompileVarDec()
    {    
        int nloc = 0;
        std::string line, token, name, type;
        getline(_tokenIn, line);
        type = extractToken();
        name = extractToken();
        table.define(name, type, s_VAR);
        nloc++;
        line = LookAhead();

        while (line != ";")
        {
            getline(_tokenIn, line);
            name = extractToken();
            table.define(name, type, s_VAR);
            nloc++;
            line = LookAhead();
        }
        getline(_tokenIn, line);      
        return nloc;
    }    
         
   

    void CompileStatements()
    {   
        std::string line;
        std::vector<std::string>::iterator itstate;

        
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

        

    }   
        
    void CompileDo()
    {    
        std::string line, name, cName;
        cName = className;

        getline(_tokenIn, line);

        int narg = 0;
        name = extractToken();
        line = LookAhead();
        if (line == ".")
        {
            if (table.kindOf(name) != s_NONE)//method call 
            {
                vm.WritePush(symKindToSeg(table.kindOf(name)), table.indexOf(name)); // implicit object pointer argument push
                narg++;
                name = (table.typeOf(name).append(extractToken())).append(extractToken());

            }
            else //function call
            {
                name = (name.append(extractToken())).append(extractToken());
            }

        }
        else { //method call from within class
            vm.WritePush(POINTER, 0); // implicit object pointer argument push
            narg++;
            name = (cName.append(".")).append(name);
                }

        getline(_tokenIn, line);//(

        narg += CompileExpressionList();//arguments :need to know the number
        vm.WriteCall(name, narg);
        vm.WritePop(TEMP, 0);

        getline(_tokenIn, line);//)
        getline(_tokenIn, line);//;
        
    }    
         
    void CompileLet()
    {    
        std::string line, name;
        

        getline(_tokenIn, line);
        name = extractToken();

        line = LookAhead();
        if (line == "[")//TODO: Array
        {
            vm.WritePush(symKindToSeg(table.kindOf(name)), table.indexOf(name));

            getline(_tokenIn, line);
            
            CompileExpression();

            getline(_tokenIn, line);

            vm.WriteArithmetic(ADD);

            getline(_tokenIn, line);

            CompileExpression();

            vm.WritePop(TEMP, 0);
            vm.WritePop(POINTER, 1);
            vm.WritePush(TEMP, 0);
            vm.WritePop(THAT, 0);

            getline(_tokenIn, line);
        }
        else {
            getline(_tokenIn, line);

            CompileExpression();

            getline(_tokenIn, line);
            vm.WritePop(symKindToSeg(table.kindOf(name)), table.indexOf(name));
        }
        
        //std::cout << name << " " << table.kindOf(name) << " " << table.indexOf(name) << std::endl;
    }    
         
    void CompileWhile()
    {    
        std::string line;
        std::string Label1 = "Label";
        std::string Label2 = "Label";
        Label1 = Label1.append(std::to_string(labIndex));
        Label2 = Label2.append(std::to_string(labIndex + 1));
        labIndex+=2;

        getline(_tokenIn, line);
        getline(_tokenIn, line);
        vm.WriteLabel(Label1);
        CompileExpression();
        vm.WriteArithmetic(NOT);
        vm.WriteIf(Label2);
        getline(_tokenIn, line);
        getline(_tokenIn, line);

        CompileStatements();
        vm.WriteGoto(Label1);
        vm.WriteLabel(Label2);
        getline(_tokenIn, line);

        
    }    
         
    void CompileReturn()
    {    
        std::string line;
        

        getline(_tokenIn, line);

        line = LookAhead();
        if (line == ";")
            vm.WritePush(CONST, 0);

        else
            CompileExpression();
        
        getline(_tokenIn, line);
        vm.WriteReturn();
        
    }    
         
    void CompileIf()
    {
        std::string line;
        
        std::string Label1 = "Label";
        std::string Label2 = "Label";
        Label1 = Label1.append(std::to_string(labIndex));
        
        labIndex++;
        getline(_tokenIn, line);
        getline(_tokenIn, line);

        CompileExpression();
        vm.WriteArithmetic(NOT);
        vm.WriteIf(Label1);
        getline(_tokenIn, line);
        getline(_tokenIn, line);

        CompileStatements();

        getline(_tokenIn, line);
        
        line = LookAhead();
        if (line == "else")
        {
            Label2 = Label2.append(std::to_string(labIndex ));
            vm.WriteGoto(Label2);
            vm.WriteLabel(Label1);
            labIndex++;
            getline(_tokenIn, line);
            getline(_tokenIn, line);

            CompileStatements();

            getline(_tokenIn, line);
            vm.WriteLabel(Label2);
        }
        else
        {
            vm.WriteLabel(Label1);
        }

        
    }

    void CompileExpression()
    {
        std::string line;
        std::vector<char>::iterator itops;
        char op;
                        
        
        CompileTerm();
        
        line = LookAhead();
        if (line == "&lt;")
            line = "<";
        if (line == "&gt;")
            line = ">";
        if (line == "&amp;")
            line = "&";

        
        op = line[0];
        itops = std::find(_op.begin(), _op.end(), op);
        while (itops != _op.end())
        {
            getline(_tokenIn, line);
            CompileTerm();
            vm.WriteArithmetic(getComFromOp(op));
            line = LookAhead();
            itops = std::find(_op.begin(), _op.end(), line[0]);
        }


        
    }

    void CompileTerm()
    {
        std::string line, name, cName, str;
        

        line = LookAhead();
        
        if(line == "~" || line == "-")
        {

            line = extractToken();
            
            CompileTerm();
            
            if (line == "~")
                vm.WriteArithmetic(NOT);
            if (line == "-")
                vm.WriteArithmetic(NEG);
        }
        else if (line == "(")
        {
            getline(_tokenIn, line);
            CompileExpression();
            getline(_tokenIn, line);
        }
        else if (is_number(line))
        {
            
            line = extractToken();
            
            vm.WritePush(CONST, std::stoi(line));
            
        }
        else if (line == "false" || line == "null")
        {
            getline(_tokenIn, line);
            vm.WritePush(CONST, 0);
        }
        else if (line == "true")
        {
            getline(_tokenIn, line);
            vm.WritePush(CONST, -1);
        }
        else if (line == "this")
        {
            getline(_tokenIn, line);
            vm.WritePush(POINTER, 0);
        }

        
        else if (getTokenType() == "<stringConstant>")
        {
            
            line = extractToken();
            vm.WritePush(CONST, line.size());
            vm.WriteCall("String.new", 1);
            for (int i = 0; i < line.size(); i++)
            {
                vm.WritePush(CONST, (int)line[i]);
                vm.WriteCall("String.appendChar", 2);
            }

        }
        //TODO: case term is subroutineCall , arrays, term is string constant -> do objects before string cause they are objects

        else 
        {
            line = LookTwiceAhead();
            if (line == "[") //array access
            {
                name = extractToken();
                vm.WritePush(symKindToSeg(table.kindOf(name)), table.indexOf(name));
                getline(_tokenIn, line);
                CompileExpression();
                vm.WriteArithmetic(ADD);
                vm.WritePop(POINTER, 1);
                vm.WritePush(THAT, 0);
                getline(_tokenIn, line);
            }

            else if (line == "." || line == "(")//subrroutine call
            {
                
                int narg = 0;
                name = extractToken();
                line = LookAhead();
                if (line == ".")
                {
                    if (table.kindOf(name) != s_NONE)//method call 
                    {
                        vm.WritePush(symKindToSeg(table.kindOf(name)), table.indexOf(name)); // implicit object pointer argument push
                        narg++;
                        name = (table.typeOf(name).append(extractToken())).append(extractToken());
                        
                    }
                    else
                    {
                        name = (name.append(extractToken())).append(extractToken());
                    }
                    
                }

                else { //method call from within class
                    cName = className;
                    vm.WritePush(POINTER, 0); // implicit object pointer argument push
                    narg++;
                    name = (cName.append(".")).append(name);
                }
                getline(_tokenIn, line);//(

                narg += CompileExpressionList();//arguments :need to know the number
                vm.WriteCall(name, narg);

                getline(_tokenIn, line);//)
            }
            
            


            else //a simple variable identifier
            {
                line = extractToken();
                vm.WritePush(symKindToSeg(table.kindOf(line)) , table.indexOf(line));
            }
                
            
        }

            

        
    }

    int CompileExpressionList()
    {
        std::string line;
        int narg = 0;
        

        line = LookAhead();
        if (line != ")")
        {
            CompileExpression();
            narg++;
            line = LookAhead();
            while (line == ",")
            {
                getline(_tokenIn, line);
                CompileExpression();
                narg++;
                line = LookAhead();
            }

        }

        return narg;
    }

public:
    void Constructor(std::string fileName)
    {
        std::string line, lineBuff;
        //table.cleartable();
        
        vm.Constructor(fileName);
        fileName = fileName.substr(0, fileName.find(".jack"));
        fileName = fileName.erase(0, fileName.find("\\") + 1);
        className = fileName;
        className[0] = toupper(className[0]);
        _tokenIn.open(_tokenOutName);

        getline(_tokenIn, line);


        CompileClass();
        table.cleartable();
        vm.Close();
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
        std::cout << line << std::endl;


        for (int i = 0; i < line.length(); i++)
        {
            itsym = std::find(_symbols.begin(), _symbols.end(), line[i]);
            //itkeys = std::find(_keywords.begin(), _keywords.end(), line.substr(0,i+1));
            if (itsym != _symbols.end())
                return i;
            /*if (itkeys != _keywords.end() && line[i+1] == ' ')
                return i+1;*/
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
