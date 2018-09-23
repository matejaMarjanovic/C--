%option noyywrap
%option nounput

%{
    #include <iostream>
    #include <string>
    #include <cstdlib>
    #include <vector>
    #include <utility>
    
    using namespace std;
    
    #include "expression.hpp"
    #include "statements.hpp"
    #include "function.hpp"
    #include "parser.tab.hpp"

    
    void yyerror(const string &msg) {
        cerr << msg << endl;    
        exit(EXIT_FAILURE);
    }
%}

%%

return                      return returnToken;
main {
    yylval.s = new string(yytext);
    return mainToken;
}
double                      return doubleTypeToken;
int                         return intTypeToken;
if                          return ifToken;
else                        return elseToken;
">="                        return gteqToken;
"<="                        return lteqToken;
"=="                        return eqToken;
"!="                        return neqToken;
[-+*/><=;,{}()^&|~!\"']     return *yytext;
-?[0-9]+ {
    yylval.i = atoi(yytext);
    return intToken;
}
-?[0-9]+\.[0-9]* {
    yylval.d = atof(yytext);
    return doubleToken;
}
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.s = new string(yytext);
    return idToken;
}
[ \t\n] { }

%%





