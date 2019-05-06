//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

extern std::string paramPrefix;

struct ToIfdef
{
    char operator()(char);
};

void printHeader(::IceUtilInternal::Output&);
void printVersionCheck(::IceUtilInternal::Output&);
void printDllExportStuff(::IceUtilInternal::Output&, const std::string&);

const int TypeContextInParam = 1;
const int TypeContextAMIEnd = 2;
const int TypeContextAMIPrivateEnd = 4;
const int TypeContextAMICallPrivateEnd = 8;
const int TypeContextUseWstring = 16;
const int TypeContextLocal = 32;
const int TypeContextCpp11 = 64;

bool isMovable(const TypePtr&);

std::string getUnqualified(const std::string&, const std::string&);
std::string typeToString(const TypePtr&, const std::string& = "", const StringList& = StringList(), int = 0);
std::string typeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string returnTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string inputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string outputTypeToString(const TypePtr&, bool, const std::string& = "", const StringList& = StringList(), int = 0);
std::string operationModeToString(Operation::Mode, bool = false);
std::string opFormatTypeToString(const OperationPtr&, bool);

std::string fixKwd(const std::string&);

void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, bool, int, const std::string&,
                               bool, const StringList& = StringList(), int = 0, const std::string& = "",
                               bool = true, const std::string& = "");

void writeMarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool,
                      int = 0, const std::string& = "", const std::string& = "");
void writeUnmarshalCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool, int = 0,
                        const std::string& = "", const std::string& = "", const std::string& = "");
void writeAllocateCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool, const std::string&,
                       int = 0, const std::string& = "");

std::string getEndArg(const TypePtr&, const StringList&, const std::string&);
void writeEndCode(::IceUtilInternal::Output&, const ParamDeclList&, const OperationPtr&, bool = false);
void writeMarshalUnmarshalDataMemberInHolder(IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, bool);
void writeMarshalUnmarshalAllInHolder(IceUtilInternal::Output&, const std::string&, const DataMemberList&, bool, bool);
void writeStreamHelpers(::IceUtilInternal::Output&, const ContainedPtr&, DataMemberList, bool, bool, bool);
void writeIceTuple(::IceUtilInternal::Output&, const std::string&, DataMemberList, int);

bool findMetaData(const std::string&, const ClassDeclPtr&, std::string&);
bool findMetaData(const std::string&, const StringList&, std::string&);
std::string findMetaData(const StringList&, int = 0);
bool inWstringModule(const SequencePtr&);

std::string getDataMemberRef(const DataMemberPtr&);
}

#endif
