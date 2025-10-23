// DataFormater.cpp

#include "DataFormater.h"
#include "Utility.h"
#include "Version.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Base.h"
#include "Utility.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/allocators.h"
#include "WeighData.h"
#include "Utility.h"
#include "MdtuProv.h"
#include "DataDef.h"
#include "GlobalFlag.h"
#include "Utility.h"

using namespace rapidjson;

string DataFormater::FormatLocationData(GPSData &data, uint16_t handleflag)
{
    Document doc;
    doc.SetObject();

    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_HANDLEFLAG, handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_WCAL, "68677.457", doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_WEIGHT, 60000, doc.GetAllocator());     // 计算结果
    doc.AddMember(DTU_REPORT_KEY_DTU, dtu, doc.GetAllocator());

    // // Add sensor object to the document
    Value sensor(kObjectType);
    sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, handleflag, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, DTU_REPORT_KEY_LOCATION, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_CHIPTIME, Value(data.time.c_str(), doc.GetAllocator()), doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_LON, data.longitude, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_LAT, data.latitude, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_GPSHEIGHT, data.gps_height, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_GPSYAW, data.gps_yaw, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_GPSV, data.gps_v, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_SVNUM, data.sv_num, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_PDOP, data.pdop, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_HDOP, data.hdop, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_VDOP, data.vdop, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_KEY_SENSOR, sensor, doc.GetAllocator());

    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorData(SensorDataUnit &data, uint16_t handleflag)
{
    Document doc;
    doc.SetObject();

    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_HANDLEFLAG, handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_WCAL, "68677.457", doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_WEIGHT, 60000, doc.GetAllocator());     // 计算结果
    doc.AddMember(DTU_REPORT_KEY_DTU, dtu, doc.GetAllocator());

    // // Add sensor object to the document
    Value sensor(kObjectType);
    sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, handleflag, doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, Value(data.sensorName.c_str(), doc.GetAllocator()), doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_SENSORSN, Value(data.sensorSN.c_str(), doc.GetAllocator()), doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_CH1VAL, data.chVal[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_CH2VAL, data.chVal[1], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_TEMPVAL, data.tempVal[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_ZEROCALIB, data.settings.zeroCalib[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_GAIN1, data.settings.gain[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_GAIN2, data.settings.gain[1], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_AUTOCALIB, data.settings.autoCalib[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_REVERSEFLAG, data.settings.reverseFlag[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_FILTER, data.settings.filter[0], doc.GetAllocator());
    sensor.AddMember(DTU_REPORT_KEY_VERSIONGC, data.settings.version, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_KEY_SENSOR, sensor, doc.GetAllocator());

    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatModeData(int localMode, bool bOfflineMode)
{
    Document doc;
    doc.SetObject();
    // doc.SetArray();

    // Value object(kObjectType);
    doc.AddMember("cmd", MSG_CMD_MODE_REPORT, doc.GetAllocator());
    doc.AddMember("edge_computing", localMode, doc.GetAllocator());
    doc.AddMember("offline", bOfflineMode ? 1 : 0, doc.GetAllocator());
    // doc.PushBack(object, doc.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatModifParamData()
{
    MdtuProv &prov = MdtuProv::getInstance();
    MdtuUnit mdtuUnit = prov.getMdtuUnit(DTU_PROV_NAME_QAXTMODIF);
    Document doc;
    doc.SetObject();

    // Add cmd, sn
    doc.AddMember("cmd", MSG_CMD_PARAMATERS, doc.GetAllocator());
    doc.AddMember("sn", Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());

    // Add modif array
    Value modif(kArrayType);
    Value obj(kObjectType);

    obj.AddMember("cmd", Value(mdtuUnit.cmd.c_str(), doc.GetAllocator()), doc.GetAllocator());
    obj.AddMember("objectid", mdtuUnit.objectid, doc.GetAllocator());
    obj.AddMember("name", Value(mdtuUnit.name.c_str(), doc.GetAllocator()), doc.GetAllocator());
    obj.AddMember("quantity", mdtuUnit.quantity, doc.GetAllocator());

    // Add valuearray
    Value valuearray(kArrayType);
    for (int i = 0; i < mdtuUnit.ddaValueSize; i++)
    {
        if (mdtuUnit.valueType == DoubleArrayType)
            valuearray.PushBack(mdtuUnit.daValue[i], doc.GetAllocator());
        else
            valuearray.PushBack(mdtuUnit.iaValue[i], doc.GetAllocator());
    }
    obj.AddMember("valuearray", valuearray, doc.GetAllocator());

    modif.PushBack(obj, doc.GetAllocator());
    doc.AddMember("modif", modif, doc.GetAllocator());

    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatCalibrationParamData(int index)
{
    MdtuProv &prov = MdtuProv::getInstance();

    Document doc;
    doc.SetObject();

    // Add cmd, sn, index
    doc.AddMember("cmd", MSG_CMD_PARAMATERS, doc.GetAllocator());
    doc.AddMember("sn", Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("index", index, doc.GetAllocator());

    // Add calibration array
    Value calibration(kArrayType);
    if (index == 1)
    {
        MdtuUnit mdtuUnit_g = prov.getMdtuUnit(DTU_PROV_NAME_G);
        MdtuUnit mdtuUnit_qt = prov.getMdtuUnit(DTU_PROV_NAME_QT);
        MdtuUnit mdtuUnit_ql = prov.getMdtuUnit(DTU_PROV_NAME_QL);
        MdtuUnit mdtuUnit_qwweight = prov.getMdtuUnit(DTU_PROV_NAME_QWWEIGHT);
        MdtuUnit mdtuUnit_qw = prov.getMdtuUnit(DTU_PROV_NAME_QW);
        // Add first object in calibration array
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_g.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_G, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_g.quantity, doc.GetAllocator());
        if (mdtuUnit_g.valueType == StringType)
            obj1.AddMember("value", Value(mdtuUnit_g.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_g.valueType == IntType)
            obj1.AddMember("value", mdtuUnit_g.iValue, doc.GetAllocator());
        else if (mdtuUnit_g.valueType == DoubleType)
            obj1.AddMember("value", mdtuUnit_g.dValue, doc.GetAllocator());
        calibration.PushBack(obj1, doc.GetAllocator());

        Value obj2(kObjectType);
        obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj2.AddMember("objectid", mdtuUnit_qt.objectid, doc.GetAllocator());
        obj2.AddMember("name", DTU_PROV_NAME_QT, doc.GetAllocator());
        obj2.AddMember("quantity", mdtuUnit_qt.quantity, doc.GetAllocator());
        if (mdtuUnit_qt.valueType == StringType)
            obj2.AddMember("value", Value(mdtuUnit_qt.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_qt.valueType == IntType)
            obj2.AddMember("value", mdtuUnit_qt.iValue, doc.GetAllocator());
        else if (mdtuUnit_qt.valueType == DoubleType)
            obj2.AddMember("value", mdtuUnit_qt.dValue, doc.GetAllocator());
        calibration.PushBack(obj2, doc.GetAllocator());

        Value obj3(kObjectType);
        obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj3.AddMember("objectid", mdtuUnit_ql.objectid, doc.GetAllocator());
        obj3.AddMember("name", DTU_PROV_NAME_QL, doc.GetAllocator());
        obj3.AddMember("quantity", mdtuUnit_ql.quantity, doc.GetAllocator());
        if (mdtuUnit_ql.valueType == StringType)
            obj3.AddMember("value", Value(mdtuUnit_ql.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_ql.valueType == IntType)
            obj3.AddMember("value", mdtuUnit_ql.iValue, doc.GetAllocator());
        else if (mdtuUnit_ql.valueType == DoubleType)
            obj3.AddMember("value", mdtuUnit_ql.dValue, doc.GetAllocator());
        calibration.PushBack(obj3, doc.GetAllocator());

        Value obj4(kObjectType);
        obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj4.AddMember("objectid", mdtuUnit_qwweight.objectid, doc.GetAllocator());
        obj4.AddMember("name", DTU_PROV_NAME_QWWEIGHT, doc.GetAllocator());
        obj4.AddMember("quantity", mdtuUnit_qwweight.quantity, doc.GetAllocator());
        Value valuearray1(kArrayType);
        if (mdtuUnit_qwweight.valueType == StringType)
            obj4.AddMember("value", Value(mdtuUnit_qwweight.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_qwweight.valueType == IntType)
            obj4.AddMember("value", mdtuUnit_qwweight.iValue, doc.GetAllocator());
        else if (mdtuUnit_qwweight.valueType == DoubleType)
            obj4.AddMember("value", mdtuUnit_qwweight.dValue, doc.GetAllocator());
        else if (mdtuUnit_qwweight.valueType == IntArrayType)
        {
            for (int i = 0; i < mdtuUnit_qwweight.ddaValueSize; i++)
            {
                valuearray1.PushBack(mdtuUnit_qwweight.iaValue[i], doc.GetAllocator());
            }
            obj4.AddMember("valuearray", valuearray1, doc.GetAllocator());
        }
        else if (mdtuUnit_qwweight.valueType == DoubleArrayType)
        {
            for (int i = 0; i < mdtuUnit_qwweight.ddaValueSize; i++)
            {
                valuearray1.PushBack(mdtuUnit_qwweight.daValue[i], doc.GetAllocator());
            }
            obj4.AddMember("valuearray", valuearray1, doc.GetAllocator());
        }
        calibration.PushBack(obj4, doc.GetAllocator());

        Value obj5(kObjectType);
        obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj5.AddMember("objectid", mdtuUnit_qw.objectid, doc.GetAllocator());
        obj5.AddMember("name", DTU_PROV_NAME_QW, doc.GetAllocator());
        obj5.AddMember("quantity", mdtuUnit_qw.quantity, doc.GetAllocator());
        Value valuearray2(kArrayType);
        if (mdtuUnit_qw.valueType == StringType)
            obj5.AddMember("value", Value(mdtuUnit_qw.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_qw.valueType == IntType)
            obj5.AddMember("value", mdtuUnit_qw.iValue, doc.GetAllocator());
        else if (mdtuUnit_qw.valueType == DoubleType)
            obj5.AddMember("value", mdtuUnit_qw.dValue, doc.GetAllocator());
        else if (mdtuUnit_qw.valueType == IntArrayType)
        {
            cout << "mdtuUnit_qw.ddaValueSize = " << mdtuUnit_qw.ddaValueSize << endl;
            for (int i = 0; i < mdtuUnit_qw.ddaValueSize; i++)
            {
                valuearray2.PushBack(mdtuUnit_qw.iaValue[i], doc.GetAllocator());
            }
            obj5.AddMember("valuearray", valuearray2, doc.GetAllocator());
        }
        else if (mdtuUnit_qw.valueType == DoubleArrayType)
        {
            for (int i = 0; i < mdtuUnit_qw.ddaValueSize; i++)
            {
                valuearray2.PushBack(mdtuUnit_qw.daValue[i], doc.GetAllocator());
            }
            obj5.AddMember("valuearray", valuearray2, doc.GetAllocator());
        }
        calibration.PushBack(obj5, doc.GetAllocator());
    }
    else
    {
        MdtuUnit mdtuUnit_qax = prov.getMdtuUnit(DTU_PROV_NAME_QAX);
        Value valuearray(kArrayType);
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_qax.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_QAX, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_qax.quantity, doc.GetAllocator());
        if (mdtuUnit_qax.valueType == Double2ArrayType)
        {
            for (int i = 0; i < mdtuUnit_qax.ddaValueSize; i++)
            {
                Value innerArray(kArrayType);
                for (int j = 0; j < mdtuUnit_qax.quantity; j++)
                {
                    innerArray.PushBack(mdtuUnit_qax.ddaValue[i][j], doc.GetAllocator());
                }
                valuearray.PushBack(innerArray, doc.GetAllocator());
            }
        }
        obj1.AddMember("valuearray", valuearray, doc.GetAllocator());
        calibration.PushBack(obj1, doc.GetAllocator());
    }
    doc.AddMember("calibration", calibration, doc.GetAllocator());
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorParamData()
{
    MdtuProv &prov = MdtuProv::getInstance();

    Document doc;
    doc.SetObject();

    MdtuUnit mdtuUnit_rolld = prov.getMdtuUnit(DTU_PROV_NAME_ROLLD);
    MdtuUnit mdtuUnit_rolls = prov.getMdtuUnit(DTU_PROV_NAME_ROLLS);
    MdtuUnit mdtuUnit_rollp = prov.getMdtuUnit(DTU_PROV_NAME_ROLLP);
    MdtuUnit mdtuUnit_rollq = prov.getMdtuUnit(DTU_PROV_NAME_ROLLQ);
    MdtuUnit mdtuUnit_rollc = prov.getMdtuUnit(DTU_PROV_NAME_ROLLC);
    // Add cmd, sn, index
    doc.AddMember("cmd", MSG_CMD_PARAMATERS, doc.GetAllocator());
    doc.AddMember("sn", Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());

    // Add calibration array
    Value sensor(kArrayType);
    // Add first object in calibration array
    Value obj1(kObjectType);
    Value valuearray1(kArrayType);
    obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj1.AddMember("objectid", mdtuUnit_rolld.objectid, doc.GetAllocator());
    obj1.AddMember("name", DTU_PROV_NAME_ROLLD, doc.GetAllocator());
    obj1.AddMember("quantity", mdtuUnit_rolld.quantity, doc.GetAllocator());
    if (mdtuUnit_rolld.valueType == IntArrayType)
    {
        for (int i = 0; i < mdtuUnit_rolld.ddaValueSize; i++)
        {
            valuearray1.PushBack(mdtuUnit_rolld.iaValue[i], doc.GetAllocator());
        }
        obj1.AddMember("valuearray", valuearray1, doc.GetAllocator());
    }
    else if (mdtuUnit_rolld.valueType == DoubleArrayType)
    {
        for (int i = 0; i < mdtuUnit_rolld.ddaValueSize; i++)
        {
            valuearray1.PushBack(mdtuUnit_rolld.daValue[i], doc.GetAllocator());
        }
        obj1.AddMember("valuearray", valuearray1, doc.GetAllocator());
    }
    sensor.PushBack(obj1, doc.GetAllocator());

    Value obj2(kObjectType);
    Value valuearray2(kArrayType);
    obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj2.AddMember("objectid", mdtuUnit_rolls.objectid, doc.GetAllocator());
    obj2.AddMember("name", DTU_PROV_NAME_ROLLS, doc.GetAllocator());
    obj2.AddMember("quantity", mdtuUnit_rolls.quantity, doc.GetAllocator());
    if (mdtuUnit_rolls.valueType == IntArrayType)
    {
        for (int i = 0; i < mdtuUnit_rolls.ddaValueSize; i++)
        {
            valuearray2.PushBack(mdtuUnit_rolls.iaValue[i], doc.GetAllocator());
        }
        obj2.AddMember("valuearray", valuearray2, doc.GetAllocator());
    }
    else if (mdtuUnit_rolls.valueType == DoubleArrayType)
    {
        for (int i = 0; i < mdtuUnit_rolls.ddaValueSize; i++)
        {
            valuearray2.PushBack(mdtuUnit_rolls.daValue[i], doc.GetAllocator());
        }
        obj2.AddMember("valuearray", valuearray2, doc.GetAllocator());
    }
    sensor.PushBack(obj2, doc.GetAllocator());

    Value obj3(kObjectType);
    Value valuearray3(kArrayType);
    obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj3.AddMember("objectid", mdtuUnit_rollp.objectid, doc.GetAllocator());
    obj3.AddMember("name", DTU_PROV_NAME_ROLLP, doc.GetAllocator());
    obj3.AddMember("quantity", mdtuUnit_rollp.quantity, doc.GetAllocator());
    if (mdtuUnit_rollp.valueType == IntArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollp.ddaValueSize; i++)
        {
            valuearray3.PushBack(mdtuUnit_rollp.iaValue[i], doc.GetAllocator());
        }
        obj3.AddMember("valuearray", valuearray3, doc.GetAllocator());
    }
    else if (mdtuUnit_rollp.valueType == DoubleArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollp.ddaValueSize; i++)
        {
            valuearray3.PushBack(mdtuUnit_rollp.daValue[i], doc.GetAllocator());
        }
        obj3.AddMember("valuearray", valuearray3, doc.GetAllocator());
    }
    sensor.PushBack(obj3, doc.GetAllocator());

    Value obj4(kObjectType);
    Value valuearray4(kArrayType);
    obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj4.AddMember("objectid", mdtuUnit_rollq.objectid, doc.GetAllocator());
    obj4.AddMember("name", DTU_PROV_NAME_ROLLP, doc.GetAllocator());
    obj4.AddMember("quantity", mdtuUnit_rollq.quantity, doc.GetAllocator());
    if (mdtuUnit_rollq.valueType == IntArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollq.ddaValueSize; i++)
        {
            valuearray4.PushBack(mdtuUnit_rollq.iaValue[i], doc.GetAllocator());
        }
        obj4.AddMember("valuearray", valuearray4, doc.GetAllocator());
    }
    else if (mdtuUnit_rollq.valueType == DoubleArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollq.ddaValueSize; i++)
        {
            valuearray4.PushBack(mdtuUnit_rollq.daValue[i], doc.GetAllocator());
        }
        obj4.AddMember("valuearray", valuearray4, doc.GetAllocator());
    }
    sensor.PushBack(obj4, doc.GetAllocator());

    Value obj5(kObjectType);
    Value valuearray5(kArrayType);
    obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj5.AddMember("objectid", mdtuUnit_rollc.objectid, doc.GetAllocator());
    obj5.AddMember("name", DTU_PROV_NAME_ROLLC, doc.GetAllocator());
    obj5.AddMember("quantity", mdtuUnit_rollc.quantity, doc.GetAllocator());
    if (mdtuUnit_rollc.valueType == IntArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollc.ddaValueSize; i++)
        {
            valuearray5.PushBack(mdtuUnit_rollc.iaValue[i], doc.GetAllocator());
        }
        obj5.AddMember("valuearray", valuearray5, doc.GetAllocator());
    }
    else if (mdtuUnit_rollc.valueType == DoubleArrayType)
    {
        for (int i = 0; i < mdtuUnit_rollc.ddaValueSize; i++)
        {
            valuearray5.PushBack(mdtuUnit_rollc.daValue[i], doc.GetAllocator());
        }
        obj5.AddMember("valuearray", valuearray5, doc.GetAllocator());
    }
    sensor.PushBack(obj5, doc.GetAllocator());

    doc.AddMember("sensor", sensor, doc.GetAllocator());
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatAlgorithmParamData(int index)
{
    MdtuProv &prov = MdtuProv::getInstance();

    Document doc;
    doc.SetObject();

    // Add cmd, sn, index
    doc.AddMember("cmd", MSG_CMD_PARAMATERS, doc.GetAllocator());
    doc.AddMember("sn", Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember("index", index, doc.GetAllocator());

    // Add calibration array
    Value algorithm(kArrayType);
    if (index == 1)
    {
        MdtuUnit mdtuUnit_alphax1 = prov.getMdtuUnit(DTU_PROV_NAME_ALPHAX1);
        MdtuUnit mdtuUnit_alphax2 = prov.getMdtuUnit(DTU_PROV_NAME_ALPHAX2);
        MdtuUnit mdtuUnit_beta = prov.getMdtuUnit(DTU_PROV_NAME_BETA);
        MdtuUnit mdtuUnit_gamma = prov.getMdtuUnit(DTU_PROV_NAME_GAMMA);
        MdtuUnit mdtuUnit_gamma2 = prov.getMdtuUnit(DTU_PROV_NAME_GAMMA2);
        MdtuUnit mdtuUnit_deltax = prov.getMdtuUnit(DTU_PROV_NAME_DELTAX);
        // Add first object in calibration array
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_alphax1.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_ALPHAX1, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_alphax1.quantity, doc.GetAllocator());
        if (mdtuUnit_alphax1.valueType == StringType)
            obj1.AddMember("value", Value(mdtuUnit_alphax1.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_alphax1.valueType == IntType)
            obj1.AddMember("value", mdtuUnit_alphax1.iValue, doc.GetAllocator());
        else if (mdtuUnit_alphax1.valueType == DoubleType)
            obj1.AddMember("value", mdtuUnit_alphax1.dValue, doc.GetAllocator());
        algorithm.PushBack(obj1, doc.GetAllocator());

        Value obj2(kObjectType);
        obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj2.AddMember("objectid", mdtuUnit_alphax2.objectid, doc.GetAllocator());
        obj2.AddMember("name", DTU_PROV_NAME_ALPHAX2, doc.GetAllocator());
        obj2.AddMember("quantity", mdtuUnit_alphax2.quantity, doc.GetAllocator());
        if (mdtuUnit_alphax2.valueType == StringType)
            obj2.AddMember("value", Value(mdtuUnit_alphax2.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_alphax2.valueType == IntType)
            obj2.AddMember("value", mdtuUnit_alphax2.iValue, doc.GetAllocator());
        else if (mdtuUnit_alphax2.valueType == DoubleType)
            obj2.AddMember("value", mdtuUnit_alphax2.dValue, doc.GetAllocator());
        algorithm.PushBack(obj2, doc.GetAllocator());

        Value obj3(kObjectType);
        obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj3.AddMember("objectid", mdtuUnit_beta.objectid, doc.GetAllocator());
        obj3.AddMember("name", DTU_PROV_NAME_BETA, doc.GetAllocator());
        obj3.AddMember("quantity", mdtuUnit_beta.quantity, doc.GetAllocator());
        if (mdtuUnit_beta.valueType == StringType)
            obj3.AddMember("value", Value(mdtuUnit_beta.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_beta.valueType == IntType)
            obj3.AddMember("value", mdtuUnit_beta.iValue, doc.GetAllocator());
        else if (mdtuUnit_beta.valueType == DoubleType)
            obj3.AddMember("value", mdtuUnit_beta.dValue, doc.GetAllocator());
        algorithm.PushBack(obj3, doc.GetAllocator());

        Value obj4(kObjectType);
        obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj4.AddMember("objectid", mdtuUnit_gamma.objectid, doc.GetAllocator());
        obj4.AddMember("name", DTU_PROV_NAME_GAMMA, doc.GetAllocator());
        obj4.AddMember("quantity", mdtuUnit_gamma.quantity, doc.GetAllocator());
        if (mdtuUnit_gamma.valueType == StringType)
            obj4.AddMember("value", Value(mdtuUnit_gamma.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_gamma.valueType == IntType)
            obj4.AddMember("value", mdtuUnit_gamma.iValue, doc.GetAllocator());
        else if (mdtuUnit_gamma.valueType == DoubleType)
            obj4.AddMember("value", mdtuUnit_gamma.dValue, doc.GetAllocator());
        algorithm.PushBack(obj4, doc.GetAllocator());

        Value obj5(kObjectType);
        obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj5.AddMember("objectid", mdtuUnit_gamma2.objectid, doc.GetAllocator());
        obj5.AddMember("name", DTU_PROV_NAME_GAMMA2, doc.GetAllocator());
        obj5.AddMember("quantity", mdtuUnit_gamma2.quantity, doc.GetAllocator());
        Value valuearray2(kArrayType);
        if (mdtuUnit_gamma2.valueType == StringType)
            obj5.AddMember("value", Value(mdtuUnit_gamma2.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_gamma2.valueType == IntType)
            obj5.AddMember("value", mdtuUnit_gamma2.iValue, doc.GetAllocator());
        else if (mdtuUnit_gamma2.valueType == DoubleType)
            obj5.AddMember("value", mdtuUnit_gamma2.dValue, doc.GetAllocator());
        algorithm.PushBack(obj5, doc.GetAllocator());

        Value obj6(kObjectType);
        obj6.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj6.AddMember("objectid", mdtuUnit_deltax.objectid, doc.GetAllocator());
        obj6.AddMember("name", DTU_PROV_NAME_DELTAX, doc.GetAllocator());
        obj6.AddMember("quantity", mdtuUnit_deltax.quantity, doc.GetAllocator());
        if (mdtuUnit_deltax.valueType == StringType)
            obj6.AddMember("value", Value(mdtuUnit_deltax.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_deltax.valueType == IntType)
            obj6.AddMember("value", mdtuUnit_deltax.iValue, doc.GetAllocator());
        else if (mdtuUnit_deltax.valueType == DoubleType)
            obj6.AddMember("value", mdtuUnit_deltax.dValue, doc.GetAllocator());
        algorithm.PushBack(obj6, doc.GetAllocator());
    }
    else if (index == 2)
    {
        MdtuUnit mdtuUnit_tau = prov.getMdtuUnit(DTU_PROV_NAME_TAU);
        MdtuUnit mdtuUnit_qls = prov.getMdtuUnit(DTU_PROV_NAME_QLS);
        MdtuUnit mdtuUnit_eld = prov.getMdtuUnit(DTU_PROV_NAME_ELD);
        MdtuUnit mdtuUnit_hld = prov.getMdtuUnit(DTU_PROV_NAME_HLD);
        MdtuUnit mdtuUnit_gpsvth = prov.getMdtuUnit(DTU_PROV_NAME_GPSVTH);
        MdtuUnit mdtuUnit_ayth = prov.getMdtuUnit(DTU_PROV_NAME_AYTH);
        // Add first object in calibration array
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_tau.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_TAU, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_tau.quantity, doc.GetAllocator());
        if (mdtuUnit_tau.valueType == StringType)
            obj1.AddMember("value", Value(mdtuUnit_tau.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_tau.valueType == IntType)
            obj1.AddMember("value", mdtuUnit_tau.iValue, doc.GetAllocator());
        else if (mdtuUnit_tau.valueType == DoubleType)
            obj1.AddMember("value", mdtuUnit_tau.dValue, doc.GetAllocator());
        algorithm.PushBack(obj1, doc.GetAllocator());

        Value obj2(kObjectType);
        obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj2.AddMember("objectid", mdtuUnit_qls.objectid, doc.GetAllocator());
        obj2.AddMember("name", DTU_PROV_NAME_QLS, doc.GetAllocator());
        obj2.AddMember("quantity", mdtuUnit_qls.quantity, doc.GetAllocator());
        if (mdtuUnit_qls.valueType == StringType)
            obj2.AddMember("value", Value(mdtuUnit_qls.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_qls.valueType == IntType)
            obj2.AddMember("value", mdtuUnit_qls.iValue, doc.GetAllocator());
        else if (mdtuUnit_qls.valueType == DoubleType)
            obj2.AddMember("value", mdtuUnit_qls.dValue, doc.GetAllocator());
        algorithm.PushBack(obj2, doc.GetAllocator());

        Value obj3(kObjectType);
        obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj3.AddMember("objectid", mdtuUnit_eld.objectid, doc.GetAllocator());
        obj3.AddMember("name", DTU_PROV_NAME_ELD, doc.GetAllocator());
        obj3.AddMember("quantity", mdtuUnit_eld.quantity, doc.GetAllocator());
        if (mdtuUnit_eld.valueType == StringType)
            obj3.AddMember("value", Value(mdtuUnit_eld.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_eld.valueType == IntType)
            obj3.AddMember("value", mdtuUnit_eld.iValue, doc.GetAllocator());
        else if (mdtuUnit_eld.valueType == DoubleType)
            obj3.AddMember("value", mdtuUnit_eld.dValue, doc.GetAllocator());
        algorithm.PushBack(obj3, doc.GetAllocator());

        Value obj4(kObjectType);
        obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj4.AddMember("objectid", mdtuUnit_hld.objectid, doc.GetAllocator());
        obj4.AddMember("name", DTU_PROV_NAME_HLD, doc.GetAllocator());
        obj4.AddMember("quantity", mdtuUnit_hld.quantity, doc.GetAllocator());
        if (mdtuUnit_hld.valueType == StringType)
            obj4.AddMember("value", Value(mdtuUnit_hld.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_hld.valueType == IntType)
            obj4.AddMember("value", mdtuUnit_hld.iValue, doc.GetAllocator());
        else if (mdtuUnit_hld.valueType == DoubleType)
            obj4.AddMember("value", mdtuUnit_hld.dValue, doc.GetAllocator());
        algorithm.PushBack(obj4, doc.GetAllocator());

        Value obj5(kObjectType);
        obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj5.AddMember("objectid", mdtuUnit_gpsvth.objectid, doc.GetAllocator());
        obj5.AddMember("name", DTU_PROV_NAME_GPSVTH, doc.GetAllocator());
        obj5.AddMember("quantity", mdtuUnit_gpsvth.quantity, doc.GetAllocator());
        Value valuearray2(kArrayType);
        if (mdtuUnit_gpsvth.valueType == StringType)
            obj5.AddMember("value", Value(mdtuUnit_gpsvth.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_gpsvth.valueType == IntType)
            obj5.AddMember("value", mdtuUnit_gpsvth.iValue, doc.GetAllocator());
        else if (mdtuUnit_gpsvth.valueType == DoubleType)
            obj5.AddMember("value", mdtuUnit_gpsvth.dValue, doc.GetAllocator());
        algorithm.PushBack(obj5, doc.GetAllocator());

        Value obj6(kObjectType);
        obj6.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj6.AddMember("objectid", mdtuUnit_ayth.objectid, doc.GetAllocator());
        obj6.AddMember("name", DTU_PROV_NAME_AYTH, doc.GetAllocator());
        obj6.AddMember("quantity", mdtuUnit_ayth.quantity, doc.GetAllocator());
        if (mdtuUnit_ayth.valueType == StringType)
            obj6.AddMember("value", Value(mdtuUnit_ayth.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_ayth.valueType == IntType)
            obj6.AddMember("value", mdtuUnit_ayth.iValue, doc.GetAllocator());
        else if (mdtuUnit_ayth.valueType == DoubleType)
            obj6.AddMember("value", mdtuUnit_ayth.dValue, doc.GetAllocator());
        algorithm.PushBack(obj6, doc.GetAllocator());
    }
    else if (index == 3)
    {
        MdtuUnit mdtuUnit_wac = prov.getMdtuUnit(DTU_PROV_NAME_WAC);
        MdtuUnit mdtuUnit_coefficient = prov.getMdtuUnit(DTU_PROV_NAME_COEFFICIENT);
        MdtuUnit mdtuUnit_alphay1 = prov.getMdtuUnit(DTU_PROV_NAME_ALPHAY1);
        MdtuUnit mdtuUnit_xiback = prov.getMdtuUnit(DTU_PROV_NAME_XIBACK);
        MdtuUnit mdtuUnit_xifront = prov.getMdtuUnit(DTU_PROV_NAME_XIFRONT);
        MdtuUnit mdtuUnit_wdistype = prov.getMdtuUnit(DTU_PROV_NAME_WDISTYPE);
        MdtuUnit mdtuUnit_filtertype = prov.getMdtuUnit(DTU_PROV_NAME_FILTERTYPE);
        // Add first object in calibration array
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_wac.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_WAC, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_wac.quantity, doc.GetAllocator());
        if (mdtuUnit_wac.valueType == StringType)
            obj1.AddMember("value", Value(mdtuUnit_wac.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_wac.valueType == IntType)
            obj1.AddMember("value", mdtuUnit_wac.iValue, doc.GetAllocator());
        else if (mdtuUnit_wac.valueType == DoubleType)
            obj1.AddMember("value", mdtuUnit_wac.dValue, doc.GetAllocator());
        algorithm.PushBack(obj1, doc.GetAllocator());

        Value obj2(kObjectType);
        obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj2.AddMember("objectid", mdtuUnit_coefficient.objectid, doc.GetAllocator());
        obj2.AddMember("name", DTU_PROV_NAME_COEFFICIENT, doc.GetAllocator());
        obj2.AddMember("quantity", mdtuUnit_coefficient.quantity, doc.GetAllocator());
        if (mdtuUnit_coefficient.valueType == StringType)
            obj2.AddMember("value", Value(mdtuUnit_coefficient.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_coefficient.valueType == IntType)
            obj2.AddMember("value", mdtuUnit_coefficient.iValue, doc.GetAllocator());
        else if (mdtuUnit_coefficient.valueType == DoubleType)
            obj2.AddMember("value", mdtuUnit_coefficient.dValue, doc.GetAllocator());
        algorithm.PushBack(obj2, doc.GetAllocator());

        Value obj3(kObjectType);
        obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj3.AddMember("objectid", mdtuUnit_alphay1.objectid, doc.GetAllocator());
        obj3.AddMember("name", DTU_PROV_NAME_ALPHAY1, doc.GetAllocator());
        obj3.AddMember("quantity", mdtuUnit_alphay1.quantity, doc.GetAllocator());
        if (mdtuUnit_alphay1.valueType == StringType)
            obj3.AddMember("value", Value(mdtuUnit_alphay1.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_alphay1.valueType == IntType)
            obj3.AddMember("value", mdtuUnit_alphay1.iValue, doc.GetAllocator());
        else if (mdtuUnit_alphay1.valueType == DoubleType)
            obj3.AddMember("value", mdtuUnit_alphay1.dValue, doc.GetAllocator());
        algorithm.PushBack(obj3, doc.GetAllocator());

        Value obj4(kObjectType);
        obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj4.AddMember("objectid", mdtuUnit_xiback.objectid, doc.GetAllocator());
        obj4.AddMember("name", DTU_PROV_NAME_XIBACK, doc.GetAllocator());
        obj4.AddMember("quantity", mdtuUnit_xiback.quantity, doc.GetAllocator());
        if (mdtuUnit_xiback.valueType == StringType)
            obj4.AddMember("value", Value(mdtuUnit_xiback.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_xiback.valueType == IntType)
            obj4.AddMember("value", mdtuUnit_xiback.iValue, doc.GetAllocator());
        else if (mdtuUnit_xiback.valueType == DoubleType)
            obj4.AddMember("value", mdtuUnit_xiback.dValue, doc.GetAllocator());
        algorithm.PushBack(obj4, doc.GetAllocator());

        Value obj5(kObjectType);
        obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj5.AddMember("objectid", mdtuUnit_xifront.objectid, doc.GetAllocator());
        obj5.AddMember("name", DTU_PROV_NAME_XIFRONT, doc.GetAllocator());
        obj5.AddMember("quantity", mdtuUnit_xifront.quantity, doc.GetAllocator());
        Value valuearray2(kArrayType);
        if (mdtuUnit_xifront.valueType == StringType)
            obj5.AddMember("value", Value(mdtuUnit_xifront.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_xifront.valueType == IntType)
            obj5.AddMember("value", mdtuUnit_xifront.iValue, doc.GetAllocator());
        else if (mdtuUnit_xifront.valueType == DoubleType)
            obj5.AddMember("value", mdtuUnit_xifront.dValue, doc.GetAllocator());
        algorithm.PushBack(obj5, doc.GetAllocator());

        Value obj6(kObjectType);
        obj6.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj6.AddMember("objectid", mdtuUnit_wdistype.objectid, doc.GetAllocator());
        obj6.AddMember("name", DTU_PROV_NAME_WDISTYPE, doc.GetAllocator());
        obj6.AddMember("quantity", mdtuUnit_wdistype.quantity, doc.GetAllocator());
        if (mdtuUnit_wdistype.valueType == StringType)
            obj6.AddMember("value", Value(mdtuUnit_wdistype.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_wdistype.valueType == IntType)
            obj6.AddMember("value", mdtuUnit_wdistype.iValue, doc.GetAllocator());
        else if (mdtuUnit_wdistype.valueType == DoubleType)
            obj6.AddMember("value", mdtuUnit_wdistype.dValue, doc.GetAllocator());
        algorithm.PushBack(obj6, doc.GetAllocator());

        Value obj7(kObjectType);
        obj7.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj7.AddMember("objectid", mdtuUnit_filtertype.objectid, doc.GetAllocator());
        obj7.AddMember("name", DTU_PROV_NAME_FILTERTYPE, doc.GetAllocator());
        obj7.AddMember("quantity", mdtuUnit_filtertype.quantity, doc.GetAllocator());
        if (mdtuUnit_filtertype.valueType == StringType)
            obj7.AddMember("value", Value(mdtuUnit_filtertype.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_filtertype.valueType == IntType)
            obj7.AddMember("value", mdtuUnit_filtertype.iValue, doc.GetAllocator());
        else if (mdtuUnit_filtertype.valueType == DoubleType)
            obj7.AddMember("value", mdtuUnit_filtertype.dValue, doc.GetAllocator());
        algorithm.PushBack(obj7, doc.GetAllocator());
    }
    else if (index == 4)
    {
        MdtuUnit mdtuUnit_deltay = prov.getMdtuUnit(DTU_PROV_NAME_DELTAY);
        MdtuUnit mdtuUnit_deltap = prov.getMdtuUnit(DTU_PROV_NAME_DELTAP);
        MdtuUnit mdtuUnit_theta = prov.getMdtuUnit(DTU_PROV_NAME_THETA);
        MdtuUnit mdtuUnit_azth = prov.getMdtuUnit(DTU_PROV_NAME_AZTH);
        MdtuUnit mdtuUnit_wstand = prov.getMdtuUnit(DTU_PROV_NAME_WSTAND);
        MdtuUnit mdtuUnit_wth = prov.getMdtuUnit(DTU_PROV_NAME_WTH);
        // Add first object in calibration array
        Value obj1(kObjectType);
        obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj1.AddMember("objectid", mdtuUnit_deltay.objectid, doc.GetAllocator());
        obj1.AddMember("name", DTU_PROV_NAME_DELTAY, doc.GetAllocator());
        obj1.AddMember("quantity", mdtuUnit_deltay.quantity, doc.GetAllocator());
        if (mdtuUnit_deltay.valueType == StringType)
            obj1.AddMember("value", Value(mdtuUnit_deltay.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_deltay.valueType == IntType)
            obj1.AddMember("value", mdtuUnit_deltay.iValue, doc.GetAllocator());
        else if (mdtuUnit_deltay.valueType == DoubleType)
            obj1.AddMember("value", mdtuUnit_deltay.dValue, doc.GetAllocator());
        algorithm.PushBack(obj1, doc.GetAllocator());

        Value obj2(kObjectType);
        obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj2.AddMember("objectid", mdtuUnit_deltap.objectid, doc.GetAllocator());
        obj2.AddMember("name", DTU_PROV_NAME_DELTAP, doc.GetAllocator());
        obj2.AddMember("quantity", mdtuUnit_deltap.quantity, doc.GetAllocator());
        if (mdtuUnit_deltap.valueType == StringType)
            obj2.AddMember("value", Value(mdtuUnit_deltap.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_deltap.valueType == IntType)
            obj2.AddMember("value", mdtuUnit_deltap.iValue, doc.GetAllocator());
        else if (mdtuUnit_deltap.valueType == DoubleType)
            obj2.AddMember("value", mdtuUnit_deltap.dValue, doc.GetAllocator());
        algorithm.PushBack(obj2, doc.GetAllocator());

        Value obj3(kObjectType);
        obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj3.AddMember("objectid", mdtuUnit_theta.objectid, doc.GetAllocator());
        obj3.AddMember("name", DTU_PROV_NAME_THETA, doc.GetAllocator());
        obj3.AddMember("quantity", mdtuUnit_theta.quantity, doc.GetAllocator());
        if (mdtuUnit_theta.valueType == StringType)
            obj3.AddMember("value", Value(mdtuUnit_theta.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_theta.valueType == IntType)
            obj3.AddMember("value", mdtuUnit_theta.iValue, doc.GetAllocator());
        else if (mdtuUnit_theta.valueType == DoubleType)
            obj3.AddMember("value", mdtuUnit_theta.dValue, doc.GetAllocator());
        algorithm.PushBack(obj3, doc.GetAllocator());

        Value obj4(kObjectType);
        obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj4.AddMember("objectid", mdtuUnit_azth.objectid, doc.GetAllocator());
        obj4.AddMember("name", DTU_PROV_NAME_AZTH, doc.GetAllocator());
        obj4.AddMember("quantity", mdtuUnit_azth.quantity, doc.GetAllocator());
        if (mdtuUnit_azth.valueType == StringType)
            obj4.AddMember("value", Value(mdtuUnit_azth.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_azth.valueType == IntType)
            obj4.AddMember("value", mdtuUnit_azth.iValue, doc.GetAllocator());
        else if (mdtuUnit_azth.valueType == DoubleType)
            obj4.AddMember("value", mdtuUnit_azth.dValue, doc.GetAllocator());
        algorithm.PushBack(obj4, doc.GetAllocator());

        Value obj5(kObjectType);
        obj5.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj5.AddMember("objectid", mdtuUnit_wstand.objectid, doc.GetAllocator());
        obj5.AddMember("name", DTU_PROV_NAME_WSTAND, doc.GetAllocator());
        obj5.AddMember("quantity", mdtuUnit_wstand.quantity, doc.GetAllocator());
        Value valuearray2(kArrayType);
        if (mdtuUnit_wstand.valueType == StringType)
            obj5.AddMember("value", Value(mdtuUnit_wstand.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_wstand.valueType == IntType)
            obj5.AddMember("value", mdtuUnit_wstand.iValue, doc.GetAllocator());
        else if (mdtuUnit_wstand.valueType == DoubleType)
            obj5.AddMember("value", mdtuUnit_wstand.dValue, doc.GetAllocator());
        algorithm.PushBack(obj5, doc.GetAllocator());

        Value obj6(kObjectType);
        obj6.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
        obj6.AddMember("objectid", mdtuUnit_wth.objectid, doc.GetAllocator());
        obj6.AddMember("name", DTU_PROV_NAME_WTH, doc.GetAllocator());
        obj6.AddMember("quantity", mdtuUnit_wth.quantity, doc.GetAllocator());
        if (mdtuUnit_wth.valueType == StringType)
            obj6.AddMember("value", Value(mdtuUnit_wth.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
        else if (mdtuUnit_wth.valueType == IntType)
            obj6.AddMember("value", mdtuUnit_wth.iValue, doc.GetAllocator());
        else if (mdtuUnit_wth.valueType == DoubleType)
            obj6.AddMember("value", mdtuUnit_wth.dValue, doc.GetAllocator());
        algorithm.PushBack(obj6, doc.GetAllocator());
    }
    doc.AddMember("algorithm", algorithm, doc.GetAllocator());
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatDtuParamData()
{
    MdtuProv &prov = MdtuProv::getInstance();

    Document doc;
    doc.SetObject();

    MdtuUnit mdtuUnit_sensornum = prov.getMdtuUnit(DTU_PROV_NAME_SENSORNUM);
    MdtuUnit mdtuUnit_qtsize = prov.getMdtuUnit(DTU_PROV_NAME_QTSIZE);
    MdtuUnit mdtuUnit_tareweight = prov.getMdtuUnit(DTU_PROV_NAME_TAREWEIGHT);
    MdtuUnit mdtuUnit_loadcap = prov.getMdtuUnit(DTU_PROV_NAME_LOADCAP);
    // Add cmd, sn, index
    doc.AddMember("cmd", MSG_CMD_PARAMATERS, doc.GetAllocator());
    doc.AddMember("sn", Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());

    // Add calibration array
    Value dtu(kArrayType);
    // Add first object in calibration array
    Value obj1(kObjectType);
    obj1.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj1.AddMember("objectid", mdtuUnit_sensornum.objectid, doc.GetAllocator());
    obj1.AddMember("name", DTU_PROV_NAME_SENSORNUM, doc.GetAllocator());
    obj1.AddMember("quantity", mdtuUnit_sensornum.quantity, doc.GetAllocator());
    if (mdtuUnit_sensornum.valueType == StringType)
        obj1.AddMember("value", Value(mdtuUnit_sensornum.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
    else if (mdtuUnit_sensornum.valueType == IntType)
        obj1.AddMember("value", mdtuUnit_sensornum.iValue, doc.GetAllocator());
    else if (mdtuUnit_sensornum.valueType == DoubleType)
        obj1.AddMember("value", mdtuUnit_sensornum.dValue, doc.GetAllocator());
    dtu.PushBack(obj1, doc.GetAllocator());

    Value obj2(kObjectType);
    obj2.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj2.AddMember("objectid", mdtuUnit_qtsize.objectid, doc.GetAllocator());
    obj2.AddMember("name", DTU_PROV_NAME_QTSIZE, doc.GetAllocator());
    obj2.AddMember("quantity", mdtuUnit_qtsize.quantity, doc.GetAllocator());
    if (mdtuUnit_qtsize.valueType == StringType)
        obj2.AddMember("value", Value(mdtuUnit_qtsize.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
    else if (mdtuUnit_qtsize.valueType == IntType)
        obj2.AddMember("value", mdtuUnit_qtsize.iValue, doc.GetAllocator());
    else if (mdtuUnit_qtsize.valueType == DoubleType)
        obj2.AddMember("value", mdtuUnit_qtsize.dValue, doc.GetAllocator());
    dtu.PushBack(obj2, doc.GetAllocator());

    Value obj3(kObjectType);
    obj3.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj3.AddMember("objectid", mdtuUnit_tareweight.objectid, doc.GetAllocator());
    obj3.AddMember("name", DTU_PROV_NAME_TAREWEIGHT, doc.GetAllocator());
    obj3.AddMember("quantity", mdtuUnit_tareweight.quantity, doc.GetAllocator());
    if (mdtuUnit_tareweight.valueType == StringType)
        obj3.AddMember("value", Value(mdtuUnit_tareweight.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
    else if (mdtuUnit_tareweight.valueType == IntType)
        obj3.AddMember("value", mdtuUnit_tareweight.iValue, doc.GetAllocator());
    else if (mdtuUnit_tareweight.valueType == DoubleType)
        obj3.AddMember("value", mdtuUnit_tareweight.dValue, doc.GetAllocator());
    dtu.PushBack(obj3, doc.GetAllocator());

    Value obj4(kObjectType);
    obj4.AddMember("cmd", MSG_CMD_REG_SET, doc.GetAllocator());
    obj4.AddMember("objectid", mdtuUnit_loadcap.objectid, doc.GetAllocator());
    obj4.AddMember("name", DTU_PROV_NAME_LOADCAP, doc.GetAllocator());
    obj4.AddMember("quantity", mdtuUnit_loadcap.quantity, doc.GetAllocator());
    if (mdtuUnit_loadcap.valueType == StringType)
        obj4.AddMember("value", Value(mdtuUnit_loadcap.sValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
    else if (mdtuUnit_loadcap.valueType == IntType)
        obj4.AddMember("value", mdtuUnit_loadcap.iValue, doc.GetAllocator());
    else if (mdtuUnit_loadcap.valueType == DoubleType)
        obj4.AddMember("value", mdtuUnit_loadcap.dValue, doc.GetAllocator());
    dtu.PushBack(obj4, doc.GetAllocator());

    doc.AddMember("dtu", dtu, doc.GetAllocator());
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorLocationGroupData_FullData_Compress(WeighDataUnit &data, AlgorithmResult caculateResult)
{
    Document doc;
    doc.SetObject();

    int localMode = COMPUTE_MODE_CLOUD;
    if (GlobalFlag::getInstance().bEdgeComputeMode && GlobalFlag::getInstance().bReportFullData)
    {
        localMode = COMPUTE_MODE_EDGE_ALL;
    }
    else if (GlobalFlag::getInstance().bEdgeComputeMode)
    {
        localMode = COMPUTE_MODE_EDGE_SIMPLE;
    }

    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_COM_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_WCAL, caculateResult.wcal, doc.GetAllocator());     // 计算结果
    dtu.AddMember(DTU_REPORT_COM_KEY_WEIGHT, caculateResult.weight, doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_COM_KEY_EDGE, localMode, doc.GetAllocator());               // 云端计算
    dtu.AddMember(DTU_REPORT_COM_KEY_FULLDATA, 1, doc.GetAllocator());                   // 组合数据
#ifdef EXCEPTION_REPORT
    if (GlobalFlag::getInstance().exceptionCode != 0)
    {
        dtu.AddMember(DTU_REPORT_COM_KEY_EXCETION, GlobalFlag::getInstance().exceptionCode, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_COM_KEY_EXSENSORS, GlobalFlag::getInstance().expSensors, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_COM_KEY_EXGC31S, GlobalFlag::getInstance().expGC31s, doc.GetAllocator());
    }
#endif
    doc.AddMember(DTU_REPORT_COM_KEY_DTU, dtu, doc.GetAllocator());

    // Location Data
    Value location(kObjectType);
    // location.AddMember(DTU_REPORT_COM_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_SENSORNAME, DTU_REPORT_KEY_LOCATION, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_CHIPTIME, Value(data.locationData.time.c_str(), doc.GetAllocator()), doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_LON, data.locationData.longitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_LAT, data.locationData.latitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_GPSHEIGHT, data.locationData.gps_height, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_GPSYAW, data.locationData.gps_yaw, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_GPSV, data.locationData.gps_v, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_SVNUM, data.locationData.sv_num, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_PDOP, data.locationData.pdop, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_HDOP, data.locationData.hdop, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_VDOP, data.locationData.vdop, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_COM_KEY_LOCATION, location, doc.GetAllocator());

    // Sensor Data
    for (auto it = data.sensorDataMap.begin(); it != data.sensorDataMap.end(); ++it)
    {
        if (it->second.bActive)
        {
            Value sensor(kObjectType);
            if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
            {
                // sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, Value(it->second.sensorName.c_str(), doc.GetAllocator()), doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_COM_KEY_SENSORSN, Value(it->second.sensorSN.c_str(), doc.GetAllocator()), doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_COM_KEY_CH1VAL, it->second.chVal[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_COM_KEY_CH2VAL, it->second.chVal[1], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_COM_KEY_TEMPVAL, it->second.tempVal[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_ZEROCALIB, it->second.settings.zeroCalib[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_GAIN1, it->second.settings.gain[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_GAIN2, it->second.settings.gain[1], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_AUTOCALIB, it->second.settings.autoCalib[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_REVERSEFLAG, it->second.settings.reverseFlag[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_FILTER, it->second.settings.filter[0], doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_KEY_VERSIONGC, it->second.settings.version, doc.GetAllocator());
            }
            else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
            {
                sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, Value(it->second.sensorName.c_str(), doc.GetAllocator()), doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_FIRMVER, 380, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_STATUS, 2048, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_WTMEASRT, 0, doc.GetAllocator());
                uint32_t chlVal = (static_cast<uint32_t>(it->second.chVal[0]) << 16) | static_cast<uint32_t>(it->second.chVal[1]);
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_NETWEIGHT, chlVal, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_TAREWEIGHT, 0, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_FULLSCALE, 0, doc.GetAllocator());
                // sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_SCALEDIV, 0, doc.GetAllocator());
            }
            doc.AddMember(Value(it->second.sensorName.c_str(), doc.GetAllocator()), sensor, doc.GetAllocator()); // Value(it->second.sensorName, doc.GetAllocator())
        }
    }
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorLocationGroupData_LocalMode_Compress(WeighDataUnit &data, AlgorithmResult caculateResult)
{
    Document doc;
    doc.SetObject();

    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_COM_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_COM_KEY_WCAL, caculateResult.wcal, doc.GetAllocator());     // 计算结果
    dtu.AddMember(DTU_REPORT_COM_KEY_WEIGHT, caculateResult.weight, doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_COM_KEY_EDGE, 1, doc.GetAllocator());                       // 本地计算
#ifdef EXCEPTION_REPORT
    if (GlobalFlag::getInstance().exceptionCode != 0)
    {
        dtu.AddMember(DTU_REPORT_COM_KEY_EXCETION, GlobalFlag::getInstance().exceptionCode, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_COM_KEY_EXSENSORS, GlobalFlag::getInstance().expSensors, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_COM_KEY_EXGC31S, GlobalFlag::getInstance().expGC31s, doc.GetAllocator());
    }
#endif
    doc.AddMember(DTU_REPORT_COM_KEY_DTU, dtu, doc.GetAllocator());

    // Location Data
    Value location(kObjectType);
    location.AddMember(DTU_REPORT_COM_KEY_CHIPTIME, Value(data.locationData.time.c_str(), doc.GetAllocator()), doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_LON, data.locationData.longitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_LAT, data.locationData.latitude, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_COM_KEY_GPSHEIGHT, data.locationData.gps_height, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_GPSYAW, data.locationData.gps_yaw, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_GPSV, data.locationData.gps_v, doc.GetAllocator());
    location.AddMember(DTU_REPORT_COM_KEY_SVNUM, data.locationData.sv_num, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_PDOP, data.locationData.pdop, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_HDOP, data.locationData.hdop, doc.GetAllocator());
    // location.AddMember(DTU_REPORT_KEY_VDOP, data.locationData.vdop, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_KEY_LOCATION, location, doc.GetAllocator());

    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorLocationGroupData_FullData(WeighDataUnit &data, AlgorithmResult caculateResult)
{
    Document doc;
    doc.SetObject();
    int localMode = COMPUTE_MODE_CLOUD;
    if (GlobalFlag::getInstance().bEdgeComputeMode && GlobalFlag::getInstance().bReportFullData)
    {
        localMode = COMPUTE_MODE_EDGE_ALL;
    }
    else if (GlobalFlag::getInstance().bEdgeComputeMode)
    {
        localMode = COMPUTE_MODE_EDGE_SIMPLE;
    }

    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_WCAL, caculateResult.wcal, doc.GetAllocator());     // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_WEIGHT, caculateResult.weight, doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_EDGE, localMode, doc.GetAllocator());               // 模式
    dtu.AddMember(DTU_REPORT_KEY_FULLDATA, 1, doc.GetAllocator());                   // 组合数据
#ifdef EXCEPTION_REPORT
    if (GlobalFlag::getInstance().exceptionCode != 0)
    {
        dtu.AddMember(DTU_REPORT_KEY_EXCETION, GlobalFlag::getInstance().exceptionCode, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_KEY_EXSENSORS, GlobalFlag::getInstance().expSensors, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_KEY_EXGC31S, GlobalFlag::getInstance().expGC31s, doc.GetAllocator());
    }
#endif
    doc.AddMember(DTU_REPORT_KEY_DTU, dtu, doc.GetAllocator());

    // Location Data
    Value location(kObjectType);
    location.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_SENSORNAME, DTU_REPORT_KEY_LOCATION, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_CHIPTIME, Value(data.locationData.time.c_str(), doc.GetAllocator()), doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_LON, data.locationData.longitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_LAT, data.locationData.latitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSHEIGHT, data.locationData.gps_height, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSYAW, data.locationData.gps_yaw, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSV, data.locationData.gps_v, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_SVNUM, data.locationData.sv_num, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_PDOP, data.locationData.pdop, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_HDOP, data.locationData.hdop, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_VDOP, data.locationData.vdop, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_KEY_LOCATION, location, doc.GetAllocator());

    // Sensor Data
    for (auto it = data.sensorDataMap.begin(); it != data.sensorDataMap.end(); ++it)
    {
        if (it->second.bActive)
        {
            Value sensor(kObjectType);
            if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31 || GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
            {
                sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, Value(it->second.sensorName.c_str(), doc.GetAllocator()), doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_SENSORSN, Value(it->second.sensorSN.c_str(), doc.GetAllocator()), doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_CH1VAL, it->second.chVal[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_CH2VAL, it->second.chVal[1], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_TEMPVAL, it->second.tempVal[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_ZEROCALIB, it->second.settings.zeroCalib[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_GAIN1, it->second.settings.gain[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_GAIN2, it->second.settings.gain[1], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_AUTOCALIB, it->second.settings.autoCalib[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_REVERSEFLAG, it->second.settings.reverseFlag[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_FILTER, it->second.settings.filter[0], doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_VERSIONGC, it->second.settings.version, doc.GetAllocator());
            }
            else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
            {
                sensor.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_KEY_SENSORNAME, Value(it->second.sensorName.c_str(), doc.GetAllocator()), doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_FIRMVER, 380, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_STATUS, 2048, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_WTMEASRT, 0, doc.GetAllocator());
                // uint32_t chlVal = (static_cast<uint32_t>(it->second.chVal[0]) << 16) | static_cast<uint32_t>(it->second.chVal[1]);
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_NETWEIGHT, caculateResult.wcal, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_TAREWEIGHT, 0, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_FULLSCALE, 0, doc.GetAllocator());
                sensor.AddMember(DTU_REPORT_TDA04D_KEY_CH1_SCALEDIV, 0, doc.GetAllocator());
            }
            doc.AddMember(Value(it->second.sensorName.c_str(), doc.GetAllocator()), sensor, doc.GetAllocator()); // Value(it->second.sensorName, doc.GetAllocator())
        }
    }
    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatSensorLocationGroupData_LocalMode(WeighDataUnit &data, AlgorithmResult caculateResult)
{
    COUT << "------FormatSensorLocationGroupData_LocalMode------" << endl;
    Document doc;
    doc.SetObject();
    // // Add dtu object to the document
    Value dtu(kObjectType);
    dtu.AddMember(DTU_REPORT_KEY_DTUSN, Value(Utility::getDeviceSN().c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_VERSION, Value(VERSION, doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_DTUTYPE, Value(GlobalFlag::getInstance().sensorType.c_str(), doc.GetAllocator()), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_SENSORSUM, WeighData::getInstance().sensor_num, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_HANDLEFLAG, data.handleflag, doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_REPORTTIME, Utility::getTimestamp(), doc.GetAllocator());
    dtu.AddMember(DTU_REPORT_KEY_WCAL, caculateResult.wcal, doc.GetAllocator());     // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_WEIGHT, caculateResult.weight, doc.GetAllocator()); // 计算结果
    dtu.AddMember(DTU_REPORT_KEY_EDGE, 1, doc.GetAllocator());                       // 本地计算
#ifdef EXCEPTION_REPORT
    if (GlobalFlag::getInstance().exceptionCode != 0)
    {
        dtu.AddMember(DTU_REPORT_KEY_EXCETION, GlobalFlag::getInstance().exceptionCode, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_KEY_EXSENSORS, GlobalFlag::getInstance().expSensors, doc.GetAllocator());
        dtu.AddMember(DTU_REPORT_KEY_EXGC31S, GlobalFlag::getInstance().expGC31s, doc.GetAllocator());
    }
#endif
    doc.AddMember(DTU_REPORT_KEY_DTU, dtu, doc.GetAllocator());

    // Location Data
    Value location(kObjectType);
    location.AddMember(DTU_REPORT_KEY_CHIPTIME, Value(data.locationData.time.c_str(), doc.GetAllocator()), doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_LON, data.locationData.longitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_LAT, data.locationData.latitude, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSHEIGHT, data.locationData.gps_height, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSYAW, data.locationData.gps_yaw, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_GPSV, data.locationData.gps_v, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_SVNUM, data.locationData.sv_num, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_PDOP, data.locationData.pdop, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_HDOP, data.locationData.hdop, doc.GetAllocator());
    location.AddMember(DTU_REPORT_KEY_VDOP, data.locationData.vdop, doc.GetAllocator());
    doc.AddMember(DTU_REPORT_KEY_LOCATION, location, doc.GetAllocator());

    // // Convert the document to a JSON string
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string retStr = buffer.GetString();
    // std::cout << retStr << std::endl;
    return retStr;
}

string DataFormater::FormatIniData()
{
    string data = "";
    string iniContent = "";
    Utility::readFileContent(DEFAULT_CONFIG_FILE_PATH, iniContent);
    if (!iniContent.empty())
    {
        Document doc;
        doc.SetObject();
        doc.AddMember("content", Value(iniContent.c_str(), doc.GetAllocator()), doc.GetAllocator());

        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);

        // std::cout << buffer.GetString() << std::endl;
        data = buffer.GetString();
    }
    return data;
}

string DataFormater::FormatJsonData()
{
    string jsonContent = "";
    Utility::readFileContent(DEFAULT_PARAMS_FILE_PATH, jsonContent);
    return jsonContent;
}

string DataFormater::FormatExceptionUIContent(ExceptionDBDataUnit exception)
{
    string retStr = "";
    string errStr = "";
    // retStr = Utility::getTimeFromTimestamp(exception.timestamp) + "\r\n";
    // std::ostringstream oss;
    // oss << "exceptionCode=" << static_cast<int>(exception.exceptionCode)
    //     << "   expSensors=" << static_cast<int>(exception.expSensors)
    //     << "   GC31s=" << static_cast<int>(exception.expGC31s);
    // retStr += oss.str();
    string strJsonErr = "";
    string strComErr = "";
    string strDataErr = "";
    string strServerErr = "";
    if ((exception.exceptionCode & EXCEPTION_JSON_PARSE) != 0)
    {
        cout << "json error" << endl;
        strJsonErr = "JSON异常";
        errStr += strJsonErr;
    }
    if ((exception.exceptionCode & EXCEPTION_SERVER_COM) != 0)
    {
        cout << "com error" << endl;
        strServerErr = "服务器异常";
        if (!errStr.empty())
            errStr += "  ";
        errStr += strServerErr;
    }
    if (exception.expSensors != 0)
    {
        strDataErr = "数据异常:" + GetExceptionSensor(exception.expSensors);
        if (!errStr.empty())
            errStr += "  ";
        errStr += strDataErr;
    }
    if (exception.expGC31s != 0)
    {
        strComErr = "通信异常:" + GetExceptionSensor(exception.expGC31s);
        if (!errStr.empty())
            errStr += "  ";
        errStr += strComErr;
    }
    retStr = Utility::getTimeFromTimestamp(exception.timestamp) + "\r\n" + errStr;
    cout << "---retStr = " << errStr << endl;
    return retStr;
}

string DataFormater::GetExceptionSensor(uint8_t expSensors)
{
    std::string result;

    // 检查每一位，最低位对应字符 '1'
    for (int i = 0; i < 8; ++i)
    {
        if (expSensors & (1 << i))
        {                                    // 检查第 i 位是否为 1
            result += std::to_string(i + 1); // 将 i + 1 转换为字符串并添加到结果中
        }
    }

    return result;
}