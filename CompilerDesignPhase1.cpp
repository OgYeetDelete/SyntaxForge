#include <iostream>
#include <vector>
#include <set>
#include <cctype>
using namespace std;

bool isOperator(char ch)
{
    return ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='=';
}

int main()
{
    string input;

    cout<<"Enter Expression: ";
    getline(cin,input);

    vector<pair<string,string>> tokens;
    set<string> symbolTable;

    for(int i=0;i<input.length();)
    {
        if(isspace(input[i]))
        {
            i++;
        }

        else if(isalpha(input[i]))
        {
            string id="";

            while(i<input.length() &&
                 (isalnum(input[i]) || input[i]=='_'))
            {
                id+=input[i];
                i++;
            }

            tokens.push_back({id,"IDENTIFIER"});
            symbolTable.insert(id);
        }

        else if(isdigit(input[i]))
        {
            string num="";

            while(i<input.length() && isdigit(input[i]))
            {
                num+=input[i];
                i++;
            }

            tokens.push_back({num,"NUMBER"});
        }

        else if(isOperator(input[i]))
        {
            string op(1,input[i]);

            if(input[i]=='=')
                tokens.push_back({op,"ASSIGNMENT"});
            else
                tokens.push_back({op,"OPERATOR"});

            i++;
        }

        else if(input[i]==';')
        {
            tokens.push_back({";","DELIMITER"});
            i++;
        }

        else if(input[i]=='(' || input[i]==')')
        {
            string br(1,input[i]);
            tokens.push_back({br,"PARENTHESIS"});
            i++;
        }

        else
        {
            cout<<"Invalid Character Found: "<<input[i]<<endl;
            return 0;
        }
    }

    cout<<"\nTOKEN STREAM\n";
    cout<<"-------------------------\n";

    for(auto t : tokens)
    {
        cout<<t.first<<" \t "<<t.second<<endl;
    }

    cout<<"\nSYMBOL TABLE\n";
    cout<<"-------------------------\n";

    for(auto s : symbolTable)
    {
        cout<<s<<endl;
    }

    cout<<"\nSYNTAX VALIDATION\n";
    cout<<"-------------------------\n";

    if(input.back()==';')
        cout<<"VALID STATEMENT\n";
    else
        cout<<"MISSING SEMICOLON\n";

    return 0;
}