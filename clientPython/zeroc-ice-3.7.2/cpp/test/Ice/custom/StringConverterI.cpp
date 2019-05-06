//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <StringConverterI.h>

using namespace std;
using namespace Ice;

Byte*
Test::StringConverterI::toUTF8(const char* sourceStart, const char* sourceEnd, UTF8Buffer& buffer) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    Byte* targetStart = buffer.getMoreBytes(size, 0);
    Byte* targetEnd = targetStart + size;

    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = static_cast<Byte>(tolower(sourceStart[i]));
    }

    return targetEnd;
}

void
Test::StringConverterI::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                                 string& target) const
{
    size_t size = static_cast<size_t>(sourceEnd - sourceStart);
    target.resize(size);
    for(size_t i = 0; i < size; ++i)
    {
        target[i] = static_cast<Byte>(toupper(sourceStart[i]));
    }
}

Byte*
Test::WstringConverterI::toUTF8(const wchar_t* sourceStart, const wchar_t* sourceEnd, UTF8Buffer& buffer) const
{
    wstring ws(sourceStart, sourceEnd);
    string s = wstringToString(ws);

    size_t size = s.size();
    Byte* targetStart = buffer.getMoreBytes(size, 0);
    Byte* targetEnd = targetStart + size;

    for(size_t i = 0; i < size; ++i)
    {
        targetStart[i] = static_cast<Byte>(tolower(s[i]));
    }
    return targetEnd;
}

void
Test::WstringConverterI::fromUTF8(const Byte* sourceStart, const Byte* sourceEnd,
                                  wstring& target) const
{
    string s(sourceStart, sourceEnd);
    for(size_t i = 0; i < s.size(); ++i)
    {
        s[i] = static_cast<char>(toupper(s[i]));
    }
    target = stringToWstring(s);
}
