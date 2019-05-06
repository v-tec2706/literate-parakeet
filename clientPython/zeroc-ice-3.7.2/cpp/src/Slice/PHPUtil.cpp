//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/PHPUtil.h>
#include <functional>
#include <algorithm>
#include <vector>

using namespace std;
using namespace Slice;

static string
lowerCase(const string& s)
{
    string result(s);
#ifdef ICE_CPP11_COMPILER
    transform(result.begin(), result.end(), result.begin(),
              [](char c){ return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
#else
    transform(result.begin(), result.end(), result.begin(), ::tolower);
#endif
    return result;
}

static string
lookupKwd(const string& name)
{
    string lower = lowerCase(name); // PHP is case insensitive.

    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "_halt_compiler", "abstract", "and", "array", "as", "break", "callable", "case", "catch", "class", "clone",
        "const", "continue", "declare", "default", "die", "do", "echo", "else", "elseif", "empty", "enddeclare",
        "endfor", "endforeach", "endif", "endswitch", "endwhile", "eval", "exit", "extends", "final", "finally",
        "for", "foreach", "function", "global", "goto", "if", "implements", "include", "include_once", "instanceof",
        "insteadof", "interface", "isset", "list", "namespace", "new", "or", "print", "private", "protected", "public",
        "require", "require_once", "return", "static", "switch", "this", "throw", "trait", "try", "unset", "use", "var",
        "while", "xor", "yield"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                lower);
    return found ? "_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

string
Slice::PHP::scopedToName(const string& scoped, bool ns)
{
    string result;
    if(ns)
    {
        result = fixIdent(scoped);
        if(result.find("::") == 0)
        {
            result.replace(0, 2, "\\");
        }

        string::size_type pos;
        while((pos = result.find("::")) != string::npos)
        {
            result.replace(pos, 2, "\\");
        }
    }
    else
    {
        string str = scoped;
        if(str.find("::") == 0)
        {
            str.erase(0, 2);
        }

        string::size_type pos;
        while((pos = str.find("::")) != string::npos)
        {
            str.replace(pos, 2, "_");
        }

        result = fixIdent(str);
    }

    return result;
}

string
Slice::PHP::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
Slice::PHP::escapeName(const string& name)
{
    string result = name;

    string::size_type pos = 0;
    while((pos = result.find("\\", pos)) != string::npos)
    {
        result.insert(pos, "\\");
        pos += 2;
    }

    return result;
}
