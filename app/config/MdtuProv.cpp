#include "MdtuProv.h"
#include "Base.h"
#include "Utility.h"
#include "document.h"
#include "GlobalFlag.h"
#include "DataDef.h"
#include "DBExceptionData.h"

using namespace std;
using namespace rapidjson;

MdtuProv::MdtuProv(/* args */)
{
    COUT << "MdtuProv init!" << endl;
    init();
}

MdtuProv::~MdtuProv()
{
    COUT << "MdtuProv destroy" << endl;
}

void MdtuProv::init()
{
    parsed = false;
    COUT << "------------MdtuProv::init()----------" << endl;
    if (!Utility::fileExists(MTDU_PROV_FILE))
    {
        COUT << "mdtuprov.json file not exist!" << endl;
        GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_JSON_PARSE;
        DBExceptionData::getInstance().inserExceptionData();
        return;
    }

    Document document;
    string content = Utility::getFileContent(MTDU_PROV_FILE);

    // json文件是否完整？
    if (document.Parse(content.c_str()).HasParseError())
    {
        rapidjson::ParseErrorCode code = document.GetParseError();
        COUT << "非法 MTDU JSON! error: " << rapidjson::GetParseErrorFunc(code) << std::endl;
        GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_JSON_PARSE;
        DBExceptionData::getInstance().inserExceptionData();
        return;
    }

    // 必须是数组型的json
    if (!document.IsArray())
    {
        std::cout << "Error: JSON is not an array" << std::endl;
        GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_JSON_PARSE;
        DBExceptionData::getInstance().inserExceptionData();
        return;
    }

    // 遍历
    for (SizeType i = 0; i < document.Size(); i++)
    {
        // 获取当前元素
        const Value &obj = document[i];

        // make a MdtuUnit obj
        MdtuUnit mdtu;

        // 每个元素是一个object
        if (obj.GetType() != kObjectType)
        {
            COUT << "Error: element[" << i << "] is not an object!" << endl;
            continue;
        }

        // cmd
        if (obj.HasMember("cmd"))
        {
            const Value &cmd = obj["cmd"];
            if (cmd.IsString())
            {
                mdtu.cmd = cmd.GetString();
            }
        }

        // objectid
        if (obj.HasMember("objectid"))
        {
            const Value &objectid = obj["objectid"];
            if (objectid.IsInt())
            {
                mdtu.objectid = objectid.GetInt();
            }
        }

        // name
        if (obj.HasMember("name"))
        {
            const Value &name = obj["name"];
            if (name.IsString())
            {
                mdtu.name = name.GetString();
            }
        }

        // quantity
        if (obj.HasMember("quantity"))
        {
            const Value &quantity = obj["quantity"];
            if (quantity.IsInt())
            {
                mdtu.quantity = quantity.GetInt();
            }
        }

        // default type
        mdtu.valueType = IntType;

        // value
        if (obj.HasMember("value"))
        {
            const Value &value = obj["value"];
            if (value.IsString())
            {
                mdtu.sValue = value.GetString();
                mdtu.valueType = StringType;
                // COUT << mdtu.name << ":" << mdtu.sValue << endl;
            }
            else if (value.IsInt())
            {
                mdtu.iValue = value.GetInt();
                mdtu.dValue = value.GetInt();
                mdtu.valueType = IntType;
                // COUT << mdtu.name << ":" << mdtu.iValue << endl;
            }
            else if (value.IsDouble())
            {
                mdtu.dValue = value.GetDouble();
                mdtu.iValue = value.GetDouble();
                mdtu.valueType = DoubleType;
                // COUT << mdtu.name << ":" << mdtu.dValue << endl;
            }
        }

        // arrayValue
        if (obj.HasMember("valuearray"))
        {
            const Value &valuearray = obj["valuearray"];

            if (valuearray.IsArray())
            {
                mdtu.ddaValueSize = valuearray.Size();

                for (SizeType k = 0; k < valuearray.Size(); k++)
                {
                    const Value &item = valuearray[k];
                    if (item.IsDouble())
                    {
                        mdtu.daValue[k] = item.GetDouble();
                        mdtu.iaValue[k] = item.GetDouble();
                        mdtu.valueType = DoubleArrayType;
                    }
                    else if (item.IsInt())
                    {
                        mdtu.iaValue[k] = item.GetInt();
                        mdtu.daValue[k] = item.GetInt();
                        mdtu.valueType = IntArrayType;
                    }
                    else if (item.IsArray())
                    {
                        mdtu.valueType = Double2ArrayType;
                        for (SizeType j = 0; j < item.Size(); j++)
                        {
                            const Value &vv = item[j];
                            if (vv.IsDouble())
                            {
                                mdtu.ddaValue[k][j] = vv.GetDouble();
                            }
                            else if (vv.IsInt())
                            {
                                mdtu.ddaValue[k][j] = vv.GetInt();
                            }
                        }
                    }
                }
            }
        }

        parameters.push_back(mdtu);
    }

    COUT << "Parse " << MTDU_PROV_FILE << " succeed!" << endl;

    parsed = true;
}

MdtuUnit MdtuProv::getMdtuUnit(string name)
{
    MdtuUnit unit;

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        unit = parameters[i];
        if (unit.name.compare(name) == 0)
        {
            break;
        }
    }

    return unit;
}
