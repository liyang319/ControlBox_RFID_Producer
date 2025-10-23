#ifndef WEIGH_ALGORITHM_H
#define WEIGH_ALGORITHM_H

#include <thread>
#include <map>

//
#define _MAX_SENSOR_NUM_ 14
#define _DEFAULT_SENSOR_NUM_ 8
#define _DEFAULT_GRAVITYACCELERATIONS_ 9.7949

#define _QT_MAX_SIZE_ 32
#define _DEFAUT_QT_SIZE _QT_MAX_SIZE_

#define _ALGORITHM_DATA_BUF_SIZE_ 10
#define _MAX_DATA_BUFFER_SIZE_ _ALGORITHM_DATA_BUF_SIZE_
#define _ALGORITHM_IGNORE_SIZE_ (_ALGORITHM_DATA_BUF_SIZE_ + 5)

/******************************************************************/
/*       DTU OBJECT CONFIG STRUCTURE                            */
/******************************************************************/
typedef struct _InputDataParam
{
	// input
	unsigned short received; // rceived
	unsigned short index;	 // data merge index
	unsigned short handle;
	unsigned short flag;	// 0:free 1:merging 2:used
	unsigned short errcode; // error code after issue happened
	unsigned short bufsize; //_DATA_BUFFER_SIZE_

	double gpssvnum[_MAX_DATA_BUFFER_SIZE_]; // star number
	double gpsv[_MAX_DATA_BUFFER_SIZE_];	 // gps speed

	// below 6 data get from sensor
	double qTt[_MAX_DATA_BUFFER_SIZE_];						 // Temperature
	double qAxtEx[_MAX_SENSOR_NUM_][_MAX_DATA_BUFFER_SIZE_]; // imported sensor data
	double qA0xtEx[_MAX_DATA_BUFFER_SIZE_];					 // sensor0 roll
	double qA0ytEx[_MAX_DATA_BUFFER_SIZE_];					 // sensor0 Pitch

	double a0ytEx[_MAX_DATA_BUFFER_SIZE_]; // Sensor0 AccelerationY 加速度Y
	double a0ztEx[_MAX_DATA_BUFFER_SIZE_]; // Sensor0 AccelerationZ 加速度Z

	// data from translation
	double qAxt[_MAX_SENSOR_NUM_][_ALGORITHM_DATA_BUF_SIZE_]; // after DataTranslation
	double qA0xt[_ALGORITHM_DATA_BUF_SIZE_];				  // sensor0 roll
	double qA0yt[_ALGORITHM_DATA_BUF_SIZE_];				  // sensor0 Pitch

	double a0yt[_ALGORITHM_DATA_BUF_SIZE_]; // Sensor0 加速度Y
	double a0zt[_ALGORITHM_DATA_BUF_SIZE_]; // Sensor0 加速度Z
} InputDataParam;

/******************************************************************/
/*       DTU OBJECT CONFIG STRUCTURE                            */
/******************************************************************/
typedef struct _ExDataParam
{
	// data
	double qEtAxt[_MAX_SENSOR_NUM_];
	double qEtA0xt;
	double qEtA0yt;
	double qEtayt;
	double qEtazt;
	double qEtTt;

	double qDtAxt[_MAX_SENSOR_NUM_];
	double qDtA0xt;
	double qDtA0yt;
	double qDtazt;

	double qStAxt[_MAX_SENSOR_NUM_];
	double qStA0xt;
	double qStA0yt;

	// 预处理参数
	int roll_d[_MAX_SENSOR_NUM_ + 1];
	int roll_s[_MAX_SENSOR_NUM_ + 1];
	int roll_p[_MAX_SENSOR_NUM_ + 1];
	int roll_q[_MAX_SENSOR_NUM_ + 1];
	int roll_c[_MAX_SENSOR_NUM_ + 1];
} ExDataParam;

typedef struct _OutputDataParam
{
	// output
	double qWdis[_MAX_SENSOR_NUM_][_QT_MAX_SIZE_];

	double qWsingle[_MAX_SENSOR_NUM_];
	double qWpre;
	double qWfin;
	double wcal;
	double wcalfft;

	double W_fin_cache; // 上一轮计算结果
	double gpsv_cache;	// 上一轮速度
} OutputDataParam;

typedef struct _FilterParam
{
	// 滤波参数 Filter parameters
	double gamma;
	double gamma2; // low-pass first order filter

	double deltax;
	double deltay;
	double deltap;

	int filtertype;
	double newgamma2; // low-pass first order filter
} FilterParam;

typedef struct _OFFSETPARAM
{
	// 补偿公式参数
	double alphax1;
	double alphax2;
	double beta;
	double theta;
	double tau;
	double qLs;

	double alphay1;
	double xiback;
	double xifront;

	double gs; // fixed to _DEFAULT_GRAVITYACCELERATIONS_
} OffsetParam;

typedef struct _CalibrationParam
{
	int qT;	  // 标定温度（输入）
	int qL;	  // 燃油体积（输入）
	double g; // fixed to _DEFAULT_GRAVITYACCELERATIONS_

	double qW[_QT_MAX_SIZE_];
	int qWweight[_MAX_SENSOR_NUM_]; // To Get qWdis
	double qAxsum[_QT_MAX_SIZE_];

	// From Server
	double qAx[_MAX_SENSOR_NUM_][_QT_MAX_SIZE_]; // max size is 32
} CalibrationParam;

typedef struct _ModifParam
{
	double qA0xtmodif;
	double qAxtmodif[_MAX_SENSOR_NUM_];
} ModifParam;

typedef struct _TRUCKPARAM
{
	int svnum;	  // 卫星数量
	int wdistype; // wdistype 标定计算类型 0 无序策略 1 累计策略
	double gpsv;  // 车辆速度

	double gpsvth; // GPSVTH 判断车辆停止的速度阈值
	double ayth;   // AYTH 判断车辆停止的加速度阈值
	double azth;   // AZTH 判断颠簸的加速度az阈值

	// 参考阈值：稳定停车熄火：小于50，
	// 车辆点火瞬间过载（约5秒）：2000~4000，
	// 停车发动：100~1000，
	// 车辆持续行驶：大于1000，
	// 车辆熄火装货：小于500
	double eld; // ELD = 0.0; // empty payload deviaton 0 kg
	double hld; // HLD = 100000.0; // high payload deviaton 100000 kg 100t

	double coefficient; // DTU中的系数
	double wstand;
	double wth;
	double wac;

	int tareweight; // teare weight of the truck
	int loadcap;	// teare weight of the truck
} TruckParam;

typedef struct _DtuObjCfg
{
	// State Machine
	unsigned short flag; // flag to runing a state machine instance
	unsigned short state;
	unsigned short statebak;
	unsigned short errcode;

	unsigned short sensornum;	 // sensor number, +1 for attitude
	unsigned short qtsize;		 // calibration data buffer size
	unsigned short databufsize;	 // the algorthm data window
	unsigned short markedhandle; // mraked group handle for algorithm

	unsigned short cyclenum; // cycle number
	unsigned short maxcycle; // cycle number

	// tuck
	TruckParam tParam; // misc parameters about truck

	// input data
	InputDataParam dParam;

	// temp data
	ExDataParam eParam;

	// modif
	ModifParam mParam;

	// filter paramters
	FilterParam fParam;

	// offsets
	OffsetParam oParam;

	// calibration
	CalibrationParam cParam;

	// output data
	OutputDataParam rParam;
} DtuObjCfg;

enum DtuType
{
	DTU_TYPE_NONE = 0,	// none
	DTU_TYPE_GYROSCOPE, // gyroscope
	DTU_TYPE_GC30,		// gc30
	DTU_TYPE_MAX
};

enum DtuCalculateState
{
	STATE_DTU_IDLE = 0,	   // IDLE, TO CHECK FOR THE NEXT COMMAND TO SEND
	STATE_DTU_INITIAL,	   // INTIAL, TO CHECK FOR THE NEXT COMMAND TO SEND
	STATE_DTU_IMPORTDATA,  // IMPORT DATA
	STATE_DTU_TRANSLATION, // TRANSLATION
	STATE_DTU_GETWDIF,	   // GET WDIF
	STATE_DTU_FILTE,	   // FILTE
	STATE_DTU_OUTPUT,	   // OUTPUT
	STATE_DTU_STATEMAX
};

typedef struct _AlgorithmResult
{
	bool result;			   // ture: succeed;
	unsigned short handleflag; // input parameter
	double weight;			   // the algorithm result
	double wcal;			   // the algorithm result
} AlgorithmResult;

class WeighAlgorithm
{
private:
	DtuObjCfg *pDtuObj;
	DtuType dtuType;
	bool mdtuProvReady;
	int winStep;
	unsigned short handleflag;

	static bool bExit;
	std::thread calculateThread; // 计算线程

	// 结果
	std::map<unsigned short, double> wFinResultMap;
	std::map<unsigned short, double> wCalResultMap;

private:
	WeighAlgorithm(/* args */);
	~WeighAlgorithm();

	void TestData();
	void DataTranslation();
	void GetWdif();
	void TDomainFilterProc();
	void GetOutputProc();
	double Interpol_x(double *qAs, double *qWs, double x);
	unsigned char GetSvnum();
	unsigned char GetGpsv();
	bool LoadMdtuProvParam();
	bool GetSensorGroupData(unsigned short handleflag);

	static void *CalculateWeighFunc(WeighAlgorithm *rithm); // thread function
	void Calculate();
	void DisplayWeight();
	void SendToCan();
	void LogData(int state);
	void PrintJsonData();
	void PrintParameter();

	template <typename T>
	char *ArrayToString(T *src, size_t length, char *des, size_t size);
	template <typename T>
	void ArraySet(T *pData, T val, int len);

	double GetFinalWeight(unsigned short flag);
	double GetFinalCal(unsigned short flag);

	void StartCalculate();
	void EndCalculate();

public:
	static WeighAlgorithm &getInstance()
	{
		static WeighAlgorithm instance;
		return instance;
	}
	void init();
	// 单例模式
	WeighAlgorithm(const WeighAlgorithm &) = delete;
	WeighAlgorithm &operator=(const WeighAlgorithm &) = delete;

	bool GetAlgorithmResult(unsigned short handleflag, AlgorithmResult &result);
	bool LowPassFiltering(int sv_num, double gps_v, uint16_t *chVal, int len, AlgorithmResult &result);
};

#endif