//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <ObjCUtil.h>
#include <Slice/Util.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

Slice::ObjCGenerator::ModuleMap Slice::ObjCGenerator::_modules;

static string
lookupKwd(const string& name, int baseType, bool mangleCasts = false)
{
    //
    // All lists in this method *must* be kept in case-insensitive
    // alphabetical order.
    //
    static string keywordList[] =
    {
        "auto", "BOOL", "break", "bycopy", "byref", "case", "char", "const", "continue",
        "default", "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "id", "if", "IMP", "in", "inline", "inout", "int", "long", "nil", "NO", "oneway", "out",
        "register", "return", "SEL", "self", "short", "signed", "sizeof", "static", "struct", "super", "switch",
        "typedef", "union", "unsigned", "void", "volatile", "while", "YES"
    };

    static string nsObjectList[] =
    {
        "autorelease", "class", "classForCoder", "copy", "dealloc", "description", "hash", "init", "isa",
        "isProxy", "mutableCopy", "release", "retain", "retainCount", "superclass", "zone"
    };
    static string nsExceptionList[] =
    {
        "callStackReturnAddresses", "name", "raise", "reason", "reserved", "userInfo",
    };

    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(!found)
    {
        switch(baseType)
        {
        case BaseTypeNone:
            break;

        case BaseTypeException:
            found = binary_search(&nsExceptionList[0],
                                  &nsExceptionList[sizeof(nsExceptionList) / sizeof(*nsExceptionList)],
                                  name,
                                  Slice::CICompare());
            if(found)
            {
                break;
            }
            /* FALLTHROUGH */

        case BaseTypeObject:
            found = binary_search(&nsObjectList[0],
                                  &nsObjectList[sizeof(nsObjectList) / sizeof(*nsObjectList)],
                                  name,
                                  Slice::CICompare());
            break;
        }
    }
    if(found || (mangleCasts && (name == "checkedCast" || name == "uncheckedCast")))
    {
        return name + "_";
    }
    return name;
}

static string
lookupParamIdKwd(const string& name)
{
    //
    // All lists in this method *must* be kept in case-insensitive
    // alphabetical order.
    //
    static string keywordList[] =
    {
        "nil", "NO", "YES"
    };
    if(binary_search(&keywordList[0],
                     &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                     name,
                     Slice::CICompare()))
    {
        return name + "_";
    }
    return name;
}

bool
Slice::ObjCGenerator::addModule(const ModulePtr& m, const string& name)
{
    string scoped = m->scoped();
    ModuleMap::const_iterator i = _modules.find(scoped);
    if(i != _modules.end())
    {
        if(i->second.name != name)
        {
            return false;
        }
    }
    else
    {
        ModulePrefix mp;
        mp.m = m;
        mp.name = name;
        _modules[scoped] = mp;
    }
    return true;
}

Slice::ObjCGenerator::ModulePrefix
Slice::ObjCGenerator::modulePrefix(const ModulePtr& m)
{
    return _modules[m->scoped()];
}

string
Slice::ObjCGenerator::moduleName(const ModulePtr& m)
{
    return _modules[m->scoped()].name;
}

ModulePtr
Slice::ObjCGenerator::findModule(const ContainedPtr& cont, int /*baseTypes*/, bool /*mangleCasts*/)
{
    ModulePtr m = ModulePtr::dynamicCast(cont);
    ContainerPtr container = cont->container();
    while(container && !m)
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(container);
        container = contained->container();
        m = ModulePtr::dynamicCast(contained);
    }
    assert(m);
    return m;
}

//
// If the passed name is a scoped name, return the identical scoped
// name, but with all components that are Objective-C keywords
// replaced by their prefixed version; otherwise, if the passed name
// is not scoped, but an Objective-C keyword, return the prefixed
// name; otherwise, check if the name is one of the method names of
// baseTypes; if so, returned the prefixed name; otherwise, return the
// name unchanged.
//
string
Slice::ObjCGenerator::fixId(const string& name, int baseTypes, bool mangleCasts)
{
    if(name.empty())
    {
        return name;
    }
    return lookupKwd(name, baseTypes, mangleCasts);
}

string
Slice::ObjCGenerator::fixId(const ContainedPtr& cont, int baseTypes, bool mangleCasts)
{
    return fixId(cont->name(), baseTypes, mangleCasts);
}

string
Slice::ObjCGenerator::fixName(const ContainedPtr& cont, int baseTypes, bool mangleCasts)
{
    return moduleName(findModule(cont, baseTypes, mangleCasts)) + cont->name();
}

string
Slice::ObjCGenerator::getParamId(const ContainedPtr& param)
{
    string n;
    if(ParamDeclPtr::dynamicCast(param) && param->findMetaData("objc:param:", n))
    {
        return lookupParamIdKwd(n.substr(11));
    }
    else
    {
        return lookupParamIdKwd(param->name());
    }
}

string
Slice::ObjCGenerator::getParamName(const ContainedPtr& param, bool internal)
{
    if(internal)
    {
        return "iceP_" + param->name();
    }
    else
    {
        return fixId(param->name());
    }
}

string
Slice::ObjCGenerator::getFactoryMethod(const ContainedPtr& p, bool deprecated)
{
    ClassDefPtr def = ClassDefPtr::dynamicCast(p);
    if(def && def->declaration()->isLocal())
    {
        deprecated = false; // Local classes don't have this issue since they were added after this fix.
    }

    //
    // If deprecated is true, we return uDPConnectionInfo for a class
    // named UDPConnectionInfo, return udpConnectionInfo otherwise.
    //
    string name = fixId(p->name());
    if(name.empty())
    {
        return name;
    }
    else if(deprecated || name.size() < 2 || !isupper(*(name.begin() + 1)))
    {
        *name.begin() = static_cast<char>(tolower(*name.begin()));
    }
    else
    {
        for(string::iterator q = name.begin(); q != name.end() && isalpha(*q); ++q)
        {
            if(q != name.end() - 1 && isalpha(*(q + 1)) && !isupper(*(q + 1)))
            {
                break;
            }
            *q = static_cast<char>(tolower(*q));
        }
    }
    return name;
}

string
Slice::ObjCGenerator::typeToString(const TypePtr& type)
{
    if(!type)
    {
        return "void";
    }

    static const char* builtinTable[] =
    {
        "ICEByte",
        "BOOL",
        "ICEShort",
        "ICEInt",
        "ICELong",
        "ICEFloat",
        "ICEDouble",
        "NSString",
        "ICEObject",
        "id<ICEObjectPrx>",
        "id",            // Dummy--we don't support Slice local Object
        "ICEObject"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinTable[builtin->kind()];
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        string mName = moduleName(findModule(proxy->_class()));
        return "id<" + mName + (proxy->_class()->name()) + "Prx>";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return fixName(seq);
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return fixName(d);
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(cl->isInterface())
        {
            if(cl->definition() && cl->definition()->isDelegate())
            {
                return fixName(cl);
            }
            else if(cl->isLocal())
            {
                return "id<" + fixName(cl) + ">";
            }
            else
            {
                return "ICEObject";
            }
        }
        else if(cl->isLocal())
        {
            string name = fixName(cl);
            return name + "<" + name + ">";
        }
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        return fixName(contained);
    }

    return "???";
}

string
Slice::ObjCGenerator::inTypeToString(const TypePtr& type, bool optional, bool autoreleasing, bool reference)
{
    string s;
    if(optional)
    {
        s = "id";
    }
    else
    {
        s = typeToString(type);
        if(mapsToPointerType(type))
        {
            s += "*";
        }
    }
    if(autoreleasing && !isValueType(type))
    {
        s += " ICE_AUTORELEASING_QUALIFIER";
    }
    if(reference)
    {
        s += "*";
    }
    return s;
}

string
Slice::ObjCGenerator::outTypeToString(const TypePtr& type, bool optional, bool autoreleasing, bool reference)
{
    if(!type)
    {
        return "void";
    }

    string s;
    if(optional)
    {
        s = "id";
    }
    else
    {
        SequencePtr seq = SequencePtr::dynamicCast(type);
        DictionaryPtr d = DictionaryPtr::dynamicCast(type);
        if(isString(type))
        {
            s = "NSMutableString";
        }
        else if(seq)
        {
            string prefix = moduleName(findModule(seq));
            s = prefix + "Mutable" + seq->name();
        }
        else if(d)
        {
            string prefix = moduleName(findModule(d));
            s = prefix + "Mutable" + d->name();
        }
        else
        {
            s = typeToString(type);
        }
        if(mapsToPointerType(type))
        {
            s += "*";
        }
    }
    if(autoreleasing && (!isValueType(type) || optional))
    {
        s += " ICE_AUTORELEASING_QUALIFIER";
    }
    if(reference)
    {
        s += "*";
    }
    return s;
}

string
Slice::ObjCGenerator::typeToObjCTypeString(const TypePtr& type)
{
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        return moduleName(findModule(proxy->_class())) + (proxy->_class()->name()) + "Prx";
    }
    else
    {
        return typeToString(type);
    }
}

bool
Slice::ObjCGenerator::isValueType(const TypePtr& type)
{
    if(!type)
    {
        return true;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindString:
            case Builtin::KindObject:
            case Builtin::KindValue:
            case Builtin::KindObjectProxy:
            case Builtin::KindLocalObject:
            {
                return false;
                break;
            }
            default:
            {
                return true;
                break;
            }
        }
    }
    if(EnumPtr::dynamicCast(type))
    {
        return true;
    }
    return false;
}

bool
Slice::ObjCGenerator::isString(const TypePtr& type)
{
    if(!type)
    {
        return false;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    return builtin && builtin->kind() == Builtin::KindString;
}
bool
Slice::ObjCGenerator::isClass(const TypePtr& type)
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue;
    }
    return ClassDeclPtr::dynamicCast(type);
}

bool
Slice::ObjCGenerator::mapsToPointerType(const TypePtr& type)
{
    if(isValueType(type))
    {
        return false;
    }
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
       return builtin->kind() != Builtin::KindObjectProxy && builtin->kind() != Builtin::KindLocalObject;
    }
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl && cl->isInterface())
    {
        if(cl->isLocal() || (cl->definition() && cl->definition()->isDelegate()))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    return !ProxyPtr::dynamicCast(type);
}

string
Slice::ObjCGenerator::getBuiltinName(const BuiltinPtr& builtin)
{
    switch(builtin->kind())
    {
        case Builtin::KindByte:
        {
            return "Byte";
        }
        case Builtin::KindBool:
        {
            return "Bool";
        }
        case Builtin::KindShort:
        {
            return "Short";
        }
        case Builtin::KindInt:
        {
            return "Int";
        }
        case Builtin::KindLong:
        {
            return "Long";
        }
        case Builtin::KindFloat:
        {
            return "Float";
        }
        case Builtin::KindDouble:
        {
            return "Double";
        }
        case Builtin::KindString:
        {
            return "String";
        }
        case Builtin::KindObject:
        case Builtin::KindValue:
        {
            return "Object";
        }
        case Builtin::KindObjectProxy:
        {
            return "Proxy";
        }
        default:
        {
            assert(false);
        }
    }
    return "NO__SUCH__TYPE";
}

string
Slice::ObjCGenerator::getOptionalHelperGetter(const TypePtr& type)
{
    if(isValueType(type))
    {
        BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
        if(builtin)
        {
            return "get" + getBuiltinName(builtin);
        }
        if(EnumPtr::dynamicCast(type))
        {
            return "getInt";
        }
    }
    return "get";
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
StringList
Slice::ObjCGenerator::splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
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
Slice::ObjCGenerator::getOptionalFormat(const TypePtr& type)
{
    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return "ICEOptionalFormatF1";
        }
        case Builtin::KindShort:
        {
            return "ICEOptionalFormatF2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return "ICEOptionalFormatF4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return "ICEOptionalFormatF8";
        }
        case Builtin::KindString:
        {
            return "ICEOptionalFormatVSize";
        }
        case Builtin::KindObject:
        case Builtin::KindValue:
        {
            return "ICEOptionalFormatClass";
        }
        case Builtin::KindObjectProxy:
        {
            return "ICEOptionalFormatFSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return "ICEOptionalFormatSize";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? "ICEOptionalFormatFSize" : "ICEOptionalFormatVSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            "ICEOptionalFormatFSize" : "ICEOptionalFormatVSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? "ICEOptionalFormatFSize" : "ICEOptionalFormatVSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return "ICEOptionalFormatFSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return "ICEOptionalFormatClass";
}

void
Slice::ObjCGenerator::writeMarshalUnmarshalCode(Output &out, const TypePtr& type, const string& param,
                                                bool marshal, bool autoreleased) const
{
    string stream = marshal ? "ostr" : "istr";
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);

    if(builtin)
    {
        string name;
        if(builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)
        {
            if(marshal)
            {
                out << nl << "[" << stream << " writeValue:" << param << "];";
            }
            else
            {
                if(autoreleased)
                {
                    out << nl << "[" << stream << " readValue:&" << param << "];";
                }
                else
                {
                    out << nl << "[" << stream << " newValue:&" << param << "];";
                }
            }
        }
        else if(builtin->kind() == Builtin::KindObjectProxy)
        {
            if(marshal)
            {
                out << nl << "[" << stream << " writeProxy:" << param << "];";
            }
            else
            {
                if(autoreleased)
                {
                    out << nl << param << " = [" << stream << " readProxy:[ICEObjectPrx class]];";
                }
                else
                {
                    out << nl << param << " = [" << stream << " newProxy:[ICEObjectPrx class]];";
                }
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "[" << stream << " write" << getBuiltinName(builtin) << ":" << param << "];";
            }
            else
            {
                if(autoreleased || isValueType(builtin))
                {
                    out << nl << param << " = [" << stream << " read" << getBuiltinName(builtin) << "];";
                }
                else
                {
                    out << nl << param << " = [" << stream << " new" << getBuiltinName(builtin) << "];";
                }
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        if(marshal)
        {
            out << nl << "[" << stream << " writeProxy:(id<ICEObjectPrx>)" << param << "];";
        }
        else
        {
            string name = moduleName(findModule(prx->_class())) + prx->_class()->name() + "Prx";
            out << nl << param << " = (id<" << name << ">)[" << stream;
            if(autoreleased)
            {
                out << " readProxy:";
            }
            else
            {
                out << " newProxy:";
            }
            //
            // We use objc_getClass to get the proxy class instead of [name class]. This is to avoid
            // a warning if the proxy is forward declared.
            //
            if(prx->_class()->definition())
            {
                out << "[" << name << " class]];";
            }
            else
            {
                out << "objc_getClass(\"" << name << "\")];";
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            // Cast avoids warning for forward-declared classes.
            out << nl << "[" << stream << " writeValue:(ICEObject*)" << param << "];";
        }
        else
        {
            if(autoreleased)
            {
                out << nl << "[" << stream << " " << "readValue:(ICEObject**)&" << param;
            }
            else
            {
                out << nl << "[" << stream << " " << "newValue:(ICEObject**)&" << param;
            }

            if(cl->isInterface())
            {
                out << "];";
            }
            else
            {
                string name = moduleName(findModule(cl)) + cl->name();
                if(cl->definition())
                {
                    out << " expectedType:[" << name << " class]];";
                }
                else
                {
                    out << " expectedType:objc_getClass(\"" << name << "\")];";
                }
            }
        }
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        if(marshal)
        {
            out << nl << "[" << stream << " writeEnumerator:" << param << " min:" << en->minValue()
                << " max:" << en->maxValue() << "];";
        }
        else
        {
            out << nl << param << " = " << "[" << stream << " readEnumerator:" << en->minValue()
                << " max:" << en->maxValue() << "];";
        }
        return;
    }

    ContainedPtr c = ContainedPtr::dynamicCast(type);
    assert(c);
    string name = moduleName(findModule(c)) + c->name() + "Helper";
    if(marshal)
    {
        out << nl << "[" + name << " write:" << param << " stream:" << stream << "];";
    }
    else
    {
        if(StructPtr::dynamicCast(type))
        {
            if(autoreleased)
            {
                out << nl << "[" << name << " read:" << stream << " value:&" << param << "];";
            }
            else
            {
                out << nl << "[" << name << " readRetained:" << stream << " value:&" << param << "];";
            }
        }
        else
        {
            if(autoreleased)
            {
                out << nl << param << " = [" << name << " read:" << stream << "];";
            }
            else
            {
                out << nl << param << " = [" << name << " readRetained:" << stream << "];";
            }
        }
    }
}

void
Slice::ObjCGenerator::writeOptMemberMarshalUnmarshalCode(Output &out, const TypePtr& type, const string& param,
                                                         bool marshal) const
{
    string stream = marshal ? "ostr" : "istr";
    string optionalHelper;
    string helper;

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindObjectProxy)
        {
            optionalHelper = "ICEVarLengthOptionalHelper";
            helper = "[ICEProxyHelper class]";
        }
        else
        {
            writeMarshalUnmarshalCode(out, type, param, marshal, false);
            return;
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        writeMarshalUnmarshalCode(out, type, param, marshal, false);
        return;
    }

    EnumPtr en = EnumPtr::dynamicCast(type);
    if(en)
    {
        writeMarshalUnmarshalCode(out, type, param, marshal, false);
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        optionalHelper = "ICEVarLengthOptionalHelper";
        helper = "objc_getClass(\"" + moduleName(findModule(prx->_class())) + prx->_class()->name() + "PrxHelper\")";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        if(st->isVariableLength())
        {
            optionalHelper = "ICEVarLengthOptionalHelper";
        }
        else
        {
            optionalHelper = "ICEFixedLengthOptionalHelper";
        }
        helper = "[" + typeToString(st) + "Helper class]";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        TypePtr element = seq->type();
        if(element->isVariableLength())
        {
            optionalHelper = "ICEVarLengthOptionalHelper";
        }
        else if(element->minWireSize() == 1)
        {
            writeMarshalUnmarshalCode(out, type, param, marshal, false);
            return;
        }
        else
        {
            optionalHelper = "ICEFixedSequenceOptionalHelper";
        }
        helper = "[" + moduleName(findModule(seq)) + seq->name() + "Helper class]";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        if(d->keyType()->isVariableLength() || d->valueType()->isVariableLength())
        {
            optionalHelper = "ICEVarLengthOptionalHelper";
        }
        else
        {
            optionalHelper = "ICEFixedDictionaryOptionalHelper";
        }
        helper = "[" + moduleName(findModule(d)) + d->name() + "Helper class]";
    }

    out << nl;
    if(marshal)
    {
        out << "[" << optionalHelper  << " write:" << param << " stream:" << stream << " helper:" << helper << "];";
    }
    else
    {
        out << param << " = [" << optionalHelper << " readRetained:" << stream << " helper:" << helper << "];";
    }

}

void
Slice::ObjCGenerator::writeOptParamMarshalUnmarshalCode(Output &out, const TypePtr& type, const string& param,
                                                        int tag, bool marshal) const
{
    string helper;
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(builtin)
    {
        helper = "ICE" + getBuiltinName(builtin) + "Helper";
    }
    else if(proxy)
    {
        helper = moduleName(findModule(proxy->_class())) + (proxy->_class()->name()) + "PrxHelper";
    }
    else
    {
        helper = typeToString(type) + "Helper";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        out << nl;
        if(marshal)
        {
            out << "[" << helper << " writeOptional:" << param << " stream:ostr tag:" << tag << "];";
        }
        else
        {
            out << "[" << helper << " readOptional:&" << param << " stream:istr tag:" << tag << "];";
        }
        return;
    }

    out << nl;
    if(marshal)
    {
        out << "[" << helper << " writeOptional:" << param << " stream:ostr tag:" << tag << "];";
    }
    else
    {
        out << param << " = [" << helper << " readOptional:istr tag:" << tag << "];";
    }
}

void
Slice::ObjCGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, true);
}

const string Slice::ObjCGenerator::MetaDataVisitor::_objcPrefix = "objc:";
const string Slice::ObjCGenerator::MetaDataVisitor::_msg = "ignoring invalid metadata";

bool
Slice::ObjCGenerator::MetaDataVisitor::visitUnitStart(const UnitPtr& p)
{
    //
    // Validate global metadata in the top-level file and all included files.
    //
    StringList files = p->allFiles();

    for(StringList::iterator q = files.begin(); q != files.end(); ++q)
    {
        string file = *q;
        DefinitionContextPtr dc = p->findDefinitionContext(file);
        assert(dc);
        StringList globalMetaData = dc->getMetaData();
        int headerDir = 0;
        int dllExport = 0;
        for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end();)
        {
            string s = *r++;

            if(s.find(_objcPrefix) == 0)
            {
                static const string objcHeaderDirPrefix = "objc:header-dir:";
                static const string objcDllExportPrefix = "objc:dll-export:";
                if(s.find(objcHeaderDirPrefix) == 0 && s.size() > objcHeaderDirPrefix.size())
                {
                    headerDir++;
                    if(headerDir > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid global metadata `" << s
                             << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }
                else if(s.find(objcDllExportPrefix) == 0 && s.size() > objcDllExportPrefix.size())
                {
                    dllExport++;
                    if(dllExport > 1)
                    {
                        ostringstream ostr;
                        ostr << "ignoring invalid global metadata `" << s
                             << "': directive can appear only once per file";
                        dc->warning(InvalidMetaData, file, -1, ostr.str());
                        globalMetaData.remove(s);
                    }
                    continue;
                }

                ostringstream ostr;
                ostr << "ignoring invalid global metadata `" << s << "'";
                dc->warning(InvalidMetaData, file, -1, ostr.str());

                globalMetaData.remove(s);
            }
        }
        dc->setMetaData(globalMetaData);
    }

    return true;
}

bool
Slice::ObjCGenerator::MetaDataVisitor::visitModuleStart(const ModulePtr& p)
{
    validate(p);
    return true;
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitModuleEnd(const ModulePtr&)
{
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitClassDecl(const ClassDeclPtr& p)
{
    validate(p);
}

bool
Slice::ObjCGenerator::MetaDataVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    validate(p);
    return true;
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitClassDefEnd(const ClassDefPtr&)
{
}

bool
Slice::ObjCGenerator::MetaDataVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    validate(p);
    return true;
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitExceptionEnd(const ExceptionPtr&)
{
}

bool
Slice::ObjCGenerator::MetaDataVisitor::visitStructStart(const StructPtr& p)
{
    validate(p);
    return true;
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitStructEnd(const StructPtr&)
{
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitOperation(const OperationPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitParamDecl(const ParamDeclPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitDataMember(const DataMemberPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitSequence(const SequencePtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitDictionary(const DictionaryPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitEnum(const EnumPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::visitConst(const ConstPtr& p)
{
    validate(p);
}

void
Slice::ObjCGenerator::MetaDataVisitor::validate(const ContainedPtr& cont)
{
    ModulePtr m = ModulePtr::dynamicCast(cont);
    if(m)
    {
        bool error = false;
        bool foundPrefix = false;

        StringList meta = getMetaData(m);
        for(StringList::iterator p = meta.begin(); p != meta.end();)
        {
            string s = *p++;
            const string prefix = "objc:prefix:";
            string name;
            if(s.find(prefix) == 0)
            {
                foundPrefix = true;
                name = trim(s.substr(prefix.size()));
                if(name.empty())
                {
                    m->definitionContext()->warning(InvalidMetaData, m->definitionContext()->filename(),
                                                    m->line(), _msg + " `" + s + "'");
                    meta.remove(s);
                    error = true;
                }
                else
                {
                    if(!addModule(m, name))
                    {
                        modulePrefixError(m, s);
                    }
                }
            }
            else
            {
                m->definitionContext()->warning(InvalidMetaData, m->definitionContext()->filename(),
                                                m->line(), _msg + " `" + s + "'");
                meta.remove(s);
                error = true;
            }
        }
        setMetaData(m, meta);

        if(!error && !foundPrefix)
        {
            StringList names = splitScopedName(m->scoped());
            string name;
            for(StringList::const_iterator i = names.begin(); i != names.end(); ++i)
            {
                name += *i;
            }
            if(!addModule(m, name))
            {
                modulePrefixError(m, "");
            }
        }
    }

    EnumPtr en = EnumPtr::dynamicCast(cont);
    if(en)
    {
        StringList meta = getMetaData(en);
        for(StringList::iterator p = meta.begin(); p != meta.end();)
        {
            string s = *p;
            if(s != "objc:scoped")
            {
                en->definitionContext()->warning(InvalidMetaData, en->definitionContext()->filename(),
                                                en->line(), _msg + " `" + s + "'");
                meta.erase(p++);
            }
            else
            {
                ++p;
            }
        }
        setMetaData(en, meta);
    }
}

StringList
Slice::ObjCGenerator::MetaDataVisitor::getMetaData(const ContainedPtr& cont)
{
    StringList localMetaData = cont->getMetaData();
    for(StringList::const_iterator p = localMetaData.begin(); p != localMetaData.end();)
    {
        string s = *p++;
        if(s.find(_objcPrefix) != 0)
        {
            localMetaData.remove(s);
        }
    }
    return localMetaData;
}

void
Slice::ObjCGenerator::MetaDataVisitor::setMetaData(const ContainedPtr& cont, const StringList& metadata)
{
    StringList localMetaData = cont->getMetaData();
    for(StringList::const_iterator p = localMetaData.begin(); p != localMetaData.end();)
    {
        string s = *p++;
        if(s.find(_objcPrefix) == 0)
        {
            localMetaData.remove(s);
        }
    }
    localMetaData.insert(localMetaData.end(), metadata.begin(), metadata.end());
    cont->setMetaData(localMetaData);
}

void
Slice::ObjCGenerator::MetaDataVisitor::modulePrefixError(const ModulePtr& m, const string& metadata)
{
    string file = m->definitionContext()->filename();
    string line = m->line();
    ModulePrefix mp = modulePrefix(m);
    string old_file = mp.m->definitionContext()->filename();
    string old_line = mp.m->line();
    ostringstream os;
    if(!metadata.empty())
    {
        os << _msg << " `" << metadata << "': ";
    }
    os << "inconsistent module prefix previously defined ";
    if(old_file != file)
    {
         os << "in " << old_file << ":";
    }
    else
    {
        os << "at line ";
    }
    os << line;
    os << " as `" << mp.name << "'" << endl;
    m->definitionContext()->warning(All, file, line, os.str());
}
