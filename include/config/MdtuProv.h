#ifndef MDTUPROV_H
#define MDTUPROV_H

#include <vector>
#include <string>

using namespace std;

#define MTDU_PROV_FILE "mdtuprov.json"
#define MAX_SENSOR_COUNT 16

#define DTU_PROV_NAME_ALPHAX1 "alphax1"
#define DTU_PROV_NAME_ALPHAX2 "alphax2"
#define DTU_PROV_NAME_BETA "beta"
#define DTU_PROV_NAME_GAMMA "gamma"
#define DTU_PROV_NAME_GAMMA2 "gamma2"
#define DTU_PROV_NAME_DELTAX "deltax"
#define DTU_PROV_NAME_DELTAY "deltay"
#define DTU_PROV_NAME_DELTAP "deltap"
#define DTU_PROV_NAME_THETA "theta"
#define DTU_PROV_NAME_TAU "tau"
#define DTU_PROV_NAME_QLS "qls"
#define DTU_PROV_NAME_ELD "eld"
#define DTU_PROV_NAME_HLD "hld"
#define DTU_PROV_NAME_GPSVTH "gpsvth"
#define DTU_PROV_NAME_GS "gs"
#define DTU_PROV_NAME_G "g"
#define DTU_PROV_NAME_QT "qt"
#define DTU_PROV_NAME_QL "ql"
#define DTU_PROV_NAME_AYTH "ayth"
#define DTU_PROV_NAME_AZTH "azth"
#define DTU_PROV_NAME_WSTAND "wstand"
#define DTU_PROV_NAME_WTH "wth"
#define DTU_PROV_NAME_WAC "wac"
#define DTU_PROV_NAME_COEFFICIENT "coefficient"
#define DTU_PROV_NAME_ALPHAY1 "alphay1"
#define DTU_PROV_NAME_XIBACK "xiback"
#define DTU_PROV_NAME_XIFRONT "xifront"
#define DTU_PROV_NAME_WDISTYPE "wdistype"
#define DTU_PROV_NAME_FILTERTYPE "filtertype"
#define DTU_PROV_NAME_ROLLD "rolld"
#define DTU_PROV_NAME_ROLLS "rolls"
#define DTU_PROV_NAME_ROLLP "rollp"
#define DTU_PROV_NAME_ROLLQ "rollq"
#define DTU_PROV_NAME_ROLLC "rollc"
#define DTU_PROV_NAME_QWWEIGHT "qwweight"
#define DTU_PROV_NAME_SENSORNUM "sensornum"
#define DTU_PROV_NAME_QAXTMODIF "qaxtmodif"
#define DTU_PROV_NAME_QW "qw"
#define DTU_PROV_NAME_QAX "qax"
#define DTU_PROV_NAME_QTSIZE "qtsize"
#define DTU_PROV_NAME_TAREWEIGHT "tareweight"
#define DTU_PROV_NAME_LOADCAP "loadcap"

enum ValueType
{
    NullType = 0,         //!< null
    FalseType = 1,        //!< false
    TrueType = 2,         //!< true
    ObjectType = 3,       //!< object
    ArrayType = 4,        //!< array
    StringType = 5,       //!< string
    IntType = 6,          //!< int
    DoubleType = 7,       //!< double
    IntArrayType = 8,     //!< int array
    DoubleArrayType = 9,  //!< double array
    Double2ArrayType = 10 //!< double 2 array
};

struct MdtuUnit
{
    string cmd;
    int objectid;
    std::string name;
    int quantity;
    int valueType;

    // value :
    int iValue;
    double dValue;
    std::string sValue;

    // arrayvalue
    int iaValue[32];
    double daValue[32];
    int ddaValueSize;
    double ddaValue[MAX_SENSOR_COUNT][32];
};

class MdtuProv
{
    bool parsed;
    std::vector<MdtuUnit> parameters;

private:
    MdtuProv(/* args */);
    ~MdtuProv();

public:
    static MdtuProv &getInstance()
    {
        static MdtuProv instance;
        return instance;
    }
    void init();
    // 单例模式
    MdtuProv(const MdtuProv &) = delete;
    MdtuProv &operator=(const MdtuProv &) = delete;

    bool isParsed() { return parsed; }
    std::vector<MdtuUnit> getData() { return parameters; }
    MdtuUnit getMdtuUnit(std::string name);
};

#endif