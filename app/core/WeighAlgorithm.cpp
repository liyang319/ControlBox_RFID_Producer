#include "WeighAlgorithm.h"
#include "Base.h"
#include "AlgorithmUtil.h"
#include "DeviceConfig.h"
#include "Utility.h"
#include "MdtuProv.h"
#include "CycleQueue.h"
#include "WeighData.h"
#include <math.h>
#include <thread>
#include <sstream>
#include <algorithm>
#include "GlobalFlag.h"

using namespace std;

#define LOG_CALCULATE_DATA_ENABLE

bool WeighAlgorithm::bExit = false;

WeighAlgorithm::WeighAlgorithm(/* args */)
{
	COUT << "WeighAlgorithm init!" << endl;

	init();
}

WeighAlgorithm::~WeighAlgorithm()
{
	COUT << "WeighAlgorithm destroy!" << endl;
	delete pDtuObj;
}

//
void WeighAlgorithm::init()
{
	mdtuProvReady = false;
	DeviceConfig &config = DeviceConfig::getInstance();
	int protocol = stoi(config.get_value("protocol", "sensor01"));

	if (protocol == 0)
	{
		dtuType = DTU_TYPE_GYROSCOPE;
		COUT << "Gyroscope..." << endl;
	}
	else // if (protocol == 1)
	{
		dtuType = DTU_TYPE_GC30;
		COUT << "GC31..." << endl;
	}

	//
	// winStep = DEFAULT_WIN_STEP;
	winStep = GlobalFlag::getInstance().winstep;

	// calibration obj
	pDtuObj = new DtuObjCfg();

	pDtuObj->databufsize = _ALGORITHM_DATA_BUF_SIZE_;
	pDtuObj->sensornum = _DEFAULT_SENSOR_NUM_ + 1;
	pDtuObj->qtsize = _DEFAUT_QT_SIZE;
	pDtuObj->markedhandle = 0;
	pDtuObj->state = STATE_DTU_IDLE;

	pDtuObj->oParam.gs = _DEFAULT_GRAVITYACCELERATIONS_;
	pDtuObj->fParam.gamma2 = 0.9;
	pDtuObj->tParam.coefficient = 1;

	// cParam
	ArraySet(pDtuObj->cParam.qWweight, 1, _MAX_SENSOR_NUM_);
	ArraySet(pDtuObj->cParam.qAxsum, 0.0, _MAX_SENSOR_NUM_);

	pDtuObj->cParam.qWweight[0] = 0; // autitude
	pDtuObj->cParam.qT = 40;
	pDtuObj->cParam.qL = 1;
	pDtuObj->cParam.g = _DEFAULT_GRAVITYACCELERATIONS_; // 9.7949

	// rParam
	pDtuObj->rParam.W_fin_cache = 0;

	// tParam
	pDtuObj->tParam.wdistype = 0; // default to 0
	pDtuObj->tParam.tareweight = 0;
	pDtuObj->fParam.filtertype = 0;

	LoadMdtuProvParam();
}

void WeighAlgorithm::StartCalculate()
{
	calculateThread = std::thread(CalculateWeighFunc, this);
	calculateThread.join();
}

void WeighAlgorithm::EndCalculate()
{
	bExit = true;
}

bool WeighAlgorithm::GetAlgorithmResult(unsigned short handleflag, AlgorithmResult &result)
{
	result.handleflag = handleflag;

	if (!LoadMdtuProvParam()) // load parameters from json data
	{
		COUT << "Load mdtuprov param fail!" << endl;
		result.result = false;
		return false;
	}

	if (!GetSensorGroupData(handleflag))
	{
		COUT << "data not ready!" << endl;
		result.result = false;
		return false;
	}

	// 数据预处理(拉平、平移)
	DataTranslation();
	// LogData(STATE_DTU_TRANSLATION);

	// 滤波
	TDomainFilterProc();
	// LogData(STATE_DTU_FILTE);

	// 计算
	GetOutputProc();
	// LogData(STATE_DTU_OUTPUT);

	result.result = true;
	result.wcal = pDtuObj->rParam.wcal;
	result.weight = pDtuObj->rParam.qWfin;

	return true;

	// display
	// SendToCan();
	// DisplayWeight();
}

double WeighAlgorithm::GetFinalWeight(unsigned short flag)
{
	// 使用 find 函数查找元素
	auto it = wFinResultMap.find(flag);
	if (it == wFinResultMap.end())
	{
		COUT << "No Weight value for flag: " << flag << std::endl;
		return 0.0;
	}
	else
	{
		double result = it->second;
		wFinResultMap.erase(flag);
		return result;
	}
}

double WeighAlgorithm::GetFinalCal(unsigned short flag)
{
	// 使用 find 函数查找元素
	auto it = wCalResultMap.find(flag);
	if (it == wCalResultMap.end())
	{
		COUT << "No Cal value for flag: " << flag << std::endl;
		return 0.0;
	}
	else
	{
		double result = it->second;
		wCalResultMap.erase(flag);
		return result;
	}
}

void *WeighAlgorithm::CalculateWeighFunc(WeighAlgorithm *rithm)
{
	COUT << "&&&&&& calculate thread begin!" << endl;

	while (!bExit)
	{
		rithm->Calculate();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	COUT << "&&&&&& heart thread end!" << endl;
	return NULL;
}

void WeighAlgorithm::Calculate()
{
}

bool WeighAlgorithm::LoadMdtuProvParam()
{
	if (mdtuProvReady)
	{
		return true;
	}

	// load json file
	MdtuProv &prov = MdtuProv::getInstance();
	mdtuProvReady = prov.isParsed();
	// std::vector<MdtuUnit> mdtu = prov.getData();

	// load -> dtu
	if (mdtuProvReady)
	{
		std::vector<MdtuUnit> mdtu = prov.getData();
		for (std::size_t i = 0; i < mdtu.size(); ++i)
		{
			MdtuUnit unit = mdtu[i];

			if (unit.name.compare(DTU_PROV_NAME_QAXTMODIF) == 0)
			{
				std::copy_n(std::begin(unit.daValue), unit.quantity, pDtuObj->mParam.qAxtmodif);
			}
			else if (unit.name.compare(DTU_PROV_NAME_QW) == 0)
			{
				std::copy_n(std::begin(unit.daValue), unit.quantity, pDtuObj->cParam.qW);
			}
			else if (unit.name.compare(DTU_PROV_NAME_QAX) == 0)
			{
				for (int i = 0; i < unit.ddaValueSize; i++)
				{
					std::copy_n(std::begin(unit.ddaValue[i]), unit.quantity, pDtuObj->cParam.qAx[i]);
				}
			}
			else if (unit.name.compare(DTU_PROV_NAME_ROLLD) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->eParam.roll_d);
			}
			else if (unit.name.compare(DTU_PROV_NAME_ROLLS) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->eParam.roll_s);
			}
			else if (unit.name.compare(DTU_PROV_NAME_ROLLP) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->eParam.roll_p);
			}
			else if (unit.name.compare(DTU_PROV_NAME_ROLLQ) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->eParam.roll_q);
			}
			else if (unit.name.compare(DTU_PROV_NAME_ROLLC) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->eParam.roll_c);
			}
			else if (unit.name.compare(DTU_PROV_NAME_QWWEIGHT) == 0)
			{
				std::copy_n(std::begin(unit.iaValue), unit.quantity, pDtuObj->cParam.qWweight);
			}
			else if (unit.name.compare(DTU_PROV_NAME_ALPHAX1) == 0)
			{
				pDtuObj->oParam.alphax1 = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_ALPHAX2) == 0)
			{
				pDtuObj->oParam.alphax2 = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_BETA) == 0)
			{
				pDtuObj->oParam.beta = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_GAMMA) == 0)
			{
				pDtuObj->fParam.gamma = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_GAMMA2) == 0)
			{
				pDtuObj->fParam.gamma2 = unit.dValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_DELTAX) == 0)
			{
				pDtuObj->fParam.deltax = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_DELTAY) == 0)
			{
				pDtuObj->fParam.deltay = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_DELTAP) == 0)
			{
				pDtuObj->fParam.deltap = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_THETA) == 0)
			{
				pDtuObj->oParam.theta = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_TAU) == 0)
			{
				pDtuObj->oParam.tau = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_QLS) == 0)
			{
				pDtuObj->oParam.qLs = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_ELD) == 0)
			{
				pDtuObj->tParam.eld = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_HLD) == 0)
			{
				pDtuObj->tParam.hld = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_GPSVTH) == 0)
			{
				pDtuObj->tParam.gpsvth = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_GS) == 0)
			{
				pDtuObj->oParam.gs = unit.dValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_G) == 0)
			{
				pDtuObj->cParam.g = unit.dValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_QT) == 0)
			{
				pDtuObj->cParam.qT = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_QL) == 0)
			{
				pDtuObj->cParam.qL = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_AYTH) == 0)
			{
				pDtuObj->tParam.ayth = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_AZTH) == 0)
			{
				pDtuObj->tParam.azth = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_WSTAND) == 0)
			{
				pDtuObj->tParam.wstand = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_WTH) == 0)
			{
				pDtuObj->tParam.wth = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_WAC) == 0)
			{
				pDtuObj->tParam.wac = unit.dValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_COEFFICIENT) == 0)
			{
				pDtuObj->tParam.coefficient = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_ALPHAY1) == 0)
			{
				pDtuObj->oParam.alphay1 = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_XIBACK) == 0)
			{
				pDtuObj->oParam.xiback = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_XIFRONT) == 0)
			{
				pDtuObj->oParam.xifront = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_WDISTYPE) == 0)
			{
				pDtuObj->tParam.wdistype = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_FILTERTYPE) == 0)
			{
				pDtuObj->fParam.filtertype = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_SENSORNUM) == 0)
			{
				pDtuObj->sensornum = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_QTSIZE) == 0)
			{
				pDtuObj->qtsize = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_TAREWEIGHT) == 0)
			{
				pDtuObj->tParam.tareweight = unit.iValue;
			}
			else if (unit.name.compare(DTU_PROV_NAME_LOADCAP) == 0)
			{
				pDtuObj->tParam.loadcap = unit.iValue;
			}
		}
	}

	//
	if (mdtuProvReady)
	{
		PrintJsonData();
		PrintParameter();
	}

	return mdtuProvReady;
}

bool WeighAlgorithm::GetSensorGroupData(unsigned short handleflag)
{
	CycleQueue *queue = &WeighData::getInstance().weigh_data_record_queue;

	int size = queue->getDataQueueSize();
	if (size < _MAX_DATA_BUFFER_SIZE_)
	{
		// COUT << "data too small!" << endl;
		return false;
	}

	// 从队列取一组（_ALGORITHM_DATA_BUF_SIZE_）数据用于计算
	std::deque<WeighDataUnit> data = queue->getDataFromQueue(_ALGORITHM_DATA_BUF_SIZE_);

	// 最新的flag
	if (data[data.size() - 1].handleflag != handleflag)
	{
		COUT << "data error: the queue header flag[" << data[data.size() - 1].handleflag << "] is not same as required[" << handleflag << "]" << endl;
		return false;
	}

	COUT << "handleflag: " << handleflag << endl;

	for (std::size_t i = 0; i < data.size(); i++)
	{
		WeighDataUnit unit = data[i];
		// COUT << "sensor count:" << unit.sensorDataMap.size() << endl;

		int index = 0;
		for (const auto &pair : unit.sensorDataMap)
		{
			SensorDataUnit sensor = pair.second;

			for (int k = 0; k < SENSOR_MAX_NUM; k++)
			{
				pDtuObj->dParam.qAxtEx[index * SENSOR_MAX_NUM + k + 1][i] = sensor.chVal[k];
			}

			index++;
		}
		pDtuObj->dParam.gpssvnum[i] = unit.locationData.sv_num;
		pDtuObj->dParam.gpsv[i] = unit.locationData.gps_v;
	}

	// handleflag
	pDtuObj->flag = data[data.size() - 1].handleflag;

	// 弹出 winstep 个数据（滑动窗口）
	// queue->removeDataFromQueue(winStep); //由调用方执行

	LogData(STATE_DTU_IMPORTDATA);
	return true;
}

void WeighAlgorithm::DisplayWeight()
{
}
void WeighAlgorithm::SendToCan()
{
}

/**************************
 重量计算过程方法
 平移处理
**************************/
void WeighAlgorithm::DataTranslation()
{
	double x;
	int i;
	int j;

	// for Test
	// TestData();
	// LogData(STATE_DTU_IMPORTDATA);

	// translationAlgorithm
	// 数据预处理
	// 当前角度（所有） Ax_t
	// 当前角度（姿态） Ax_t[0],处理载重传感器
	for (i = 0; i < pDtuObj->sensornum; i++)
	{
		for (j = 0; j < pDtuObj->databufsize; j++)
		{
			// TODO:
			x = pDtuObj->dParam.qAxtEx[i][j] + (pDtuObj->eParam.roll_p[i] - 1) * 16384;

			if (x < 32768)
			{
				// x = x;
			}
			else
			{
				x = (x - 65536);
			}

			if (i == 0)
			{
				pDtuObj->dParam.qAxt[0][j] = pDtuObj->eParam.roll_d[0] * (AlgorithmUtil::nol(x));
				pDtuObj->dParam.qA0xt[j] = pDtuObj->dParam.qAxt[0][j];
			}
			else
			{
				pDtuObj->dParam.qAxt[i][j] = pDtuObj->eParam.roll_d[i] * pDtuObj->eParam.roll_s[i] * pDtuObj->eParam.roll_q[i] * (AlgorithmUtil::nol(x))-pDtuObj->eParam.roll_c[i] * pDtuObj->eParam.roll_s[i] * pDtuObj->eParam.roll_q[i] * pDtuObj->dParam.qA0xt[j];
			}
		}
	}

	// 侧倾角（姿态） A0y_t
	for (j = 0; j < pDtuObj->databufsize; j++)
	{
		// TODO:
		pDtuObj->dParam.qA0yt[j] = AlgorithmUtil::nol(pDtuObj->dParam.qA0ytEx[j]);
	}

	if (dtuType == DTU_TYPE_GYROSCOPE)
	{
		// 加速度（姿态） ay_t
		for (j = 0; j < pDtuObj->databufsize; j++)
		{
			pDtuObj->dParam.a0yt[j] = AlgorithmUtil::nol(pDtuObj->dParam.a0ytEx[j]);
			pDtuObj->dParam.a0zt[j] = AlgorithmUtil::nol(pDtuObj->dParam.a0ztEx[j]);
		}
	}
}

/**************************
 重量计算过程方法
 标定值分配与排序
**************************/
void WeighAlgorithm::GetWdif(void)
{
	int k;
	int i;
	int j;
	double sum_wa;
	double qAxdsum;
	double qAxdsum1;
	int S;
	int N;

	S = pDtuObj->qtsize;
	N = pDtuObj->sensornum;

	// get wdif
	if (pDtuObj->tParam.wdistype != 1)
	{
		for (j = 0; j < S; j++)
		{
			sum_wa = 0.0;
			for (i = 0; i < N; i++)
			{
				sum_wa += (pDtuObj->cParam.qWweight[i] * pDtuObj->cParam.qAx[i][j]);
			}

			if (0 < sum_wa)
			{
				pDtuObj->cParam.qAxsum[j] = sum_wa;
			}
			else
			{
				pDtuObj->cParam.qAxsum[j] = 1.0;
			}
		}

		for (i = 0; i < N; i++)
		{
			for (j = 0; j < S; j++)
			{
				if (0 < pDtuObj->cParam.qAx[i][j])
				{
					pDtuObj->rParam.qWdis[i][j] = (pDtuObj->cParam.qWweight[i] * pDtuObj->cParam.qAx[i][j] / pDtuObj->cParam.qAxsum[j] * pDtuObj->cParam.qW[j]);
				}
				else
				{
					pDtuObj->rParam.qWdis[i][j] = 0.0;
				}
			}
		}
	}
	else
	{
		qAxdsum = 0.0;
		for (i = 0; i < N; i++)
		{
			qAxdsum += (pDtuObj->cParam.qWweight[i] * pDtuObj->cParam.qAx[i][0]);
		}

		if (0 < qAxdsum)
		{
			// qAxdsum = qAxdsum;
		}
		else
		{
			qAxdsum = 1.0;
		}

		for (i = 0; i < N; i++)
		{
			pDtuObj->rParam.qWdis[i][0] = (pDtuObj->cParam.qWweight[i] * pDtuObj->cParam.qAx[i][0] / qAxdsum * pDtuObj->cParam.qW[0]);
		}

		for (j = 1; j < S; j++)
		{
			for (i = 0; i < N; i++)
			{
				qAxdsum1 = 0.0;
				for (k = 0; k < N; k++)
				{
					qAxdsum1 += (pDtuObj->cParam.qWweight[k] * (pDtuObj->cParam.qAx[k][j] - pDtuObj->cParam.qAx[k][j - 1]));
				}

				if (0 < qAxdsum1)
				{
					// qAxdsum1 = qAxdsum1;
				}
				else
				{
					qAxdsum1 = 1.0;
				}

				if (0 < pDtuObj->cParam.qAx[i][j])
				{
					pDtuObj->rParam.qWdis[i][j] = pDtuObj->rParam.qWdis[i][j - 1] + pDtuObj->cParam.qWweight[i] * (pDtuObj->cParam.qAx[i][j] - pDtuObj->cParam.qAx[i][j - 1]) / qAxdsum1 * (pDtuObj->cParam.qW[j] - pDtuObj->cParam.qW[j - 1]);
				}
				else
				{
					pDtuObj->rParam.qWdis[i][j] = pDtuObj->rParam.qWdis[i][j - 1];
				}
			}
		}
	}
}

/**************************
 重量计算过程方法
 时域滤波
**************************/
void WeighAlgorithm::TDomainFilterProc()
{
	int i;
	int M;
	int N;

	double gamma;

	M = pDtuObj->databufsize;
	N = pDtuObj->sensornum;
	gamma = pDtuObj->fParam.gamma;

	// 应力传感器从1开始
	for (i = 1; i < N; i++)
	{
		// Et_Ax_t
		pDtuObj->eParam.qEtAxt[i] = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.qAxt[i]);

		// Dt_Ax_t
		pDtuObj->eParam.qDtAxt[i] = AlgorithmUtil::Dvart(M, gamma, pDtuObj->dParam.qAxt[i], pDtuObj->eParam.qEtAxt[i]);

		// St_Ax_t
		pDtuObj->eParam.qStAxt[i] = pDtuObj->eParam.qEtAxt[i] + pDtuObj->fParam.deltap * sqrt(pDtuObj->eParam.qDtAxt[i]);
	}

	// attitude below
	double EtAxt = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.qA0xt);
	double DtAxt = AlgorithmUtil::Dvart(M, gamma, pDtuObj->dParam.qA0xt, EtAxt);
	double StAxt = EtAxt + pDtuObj->fParam.deltax * sqrt(DtAxt); // Et_A0x_t

	// 姿态 roll
	pDtuObj->eParam.qEtA0xt = EtAxt;
	pDtuObj->eParam.qDtA0xt = DtAxt;
	pDtuObj->eParam.qStA0xt = StAxt;

	// 姿态 pitch
	pDtuObj->eParam.qEtA0yt = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.qA0yt);						  // Et_A0y_t
	pDtuObj->eParam.qDtA0yt = AlgorithmUtil::Dvart(M, gamma, pDtuObj->dParam.qA0yt, pDtuObj->eParam.qEtA0yt); // Dt_A0y_t
	pDtuObj->eParam.qStA0yt = pDtuObj->eParam.qEtA0yt + pDtuObj->fParam.deltay * sqrt(pDtuObj->eParam.qDtA0yt);

	if (dtuType == DTU_TYPE_GYROSCOPE)
	{
		// 姿态 ay
		pDtuObj->eParam.qEtayt = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.a0yt); // Et_ay_t

		// 姿态 az
		pDtuObj->eParam.qEtazt = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.a0zt); // Et_az_t
		pDtuObj->eParam.qDtazt = AlgorithmUtil::Dvart(M, gamma, pDtuObj->dParam.a0zt, pDtuObj->eParam.qEtazt);
	}

	// 温度 temp
	pDtuObj->eParam.qEtTt = AlgorithmUtil::Emeant(M, gamma, pDtuObj->dParam.qTt); // Et_T_t
}

/**************************
 重量计算过程方法
 插值公式方法
**************************/
double WeighAlgorithm::Interpol_x(double *qAs, double *qWs, double x)
{
	int j;
	double qAss[_QT_MAX_SIZE_];

	double res;
	double maxqws;
	double maxqas;

	double xs;
	double x2;
	double x3;
	double x4;
	double x5;
	double x6;

	double xy;
	double x2y;
	double x3y;

	double lsdel;
	double lsa = 0;
	double lsb = 0;
	double lsc = 0;

	xs = 0.01 * x;

	x2 = 0.0;
	x3 = 0.0;
	x4 = 0.0;
	x5 = 0.0;
	x6 = 0.0;

	xy = 0.0;
	x2y = 0.0;
	x3y = 0.0;

	for (j = 0; j < pDtuObj->qtsize; j++)
	{
		qAss[j] = 0.01 * qAs[j];
		x2 += pow(qAss[j], 2);
		x3 += pow(qAss[j], 3);
		x4 += pow(qAss[j], 4);
		x5 += pow(qAss[j], 5);
		x6 += pow(qAss[j], 6);

		xy += (qAss[j] * qWs[j]);
		x2y += (pow(qAss[j], 2) * qWs[j]);
		x3y += (pow(qAss[j], 3) * qWs[j]);
	}

	lsdel = x2 * x4 * x6 + x3 * x5 * x4 + x4 * x3 * x5 - x2 * x5 * x5 - x3 * x3 * x6 - x4 * x4 * x4;
	if (lsdel == 0)
	{
		res = 0.0;
	}
	else
	{
		lsa = (xy * x4 * x6 + x2y * x5 * x4 + x3y * x3 * x5 - xy * x5 * x5 - x2y * x3 * x6 - x3y * x4 * x4) / lsdel;
		lsb = (x2 * x2y * x6 + x3 * x3y * x4 + x4 * xy * x5 - x2 * x3y * x5 - x3 * xy * x6 - x4 * x2y * x4) / lsdel;
		lsc = (x2 * x4 * x3y + x3 * x5 * xy + x4 * x3 * x2y - x2 * x5 * x2y - x3 * x3 * x3y - x4 * x4 * xy) / lsdel;
	}

	// 在数值越界的时候改为线性拟合，应该能避免数值越界太多时爆破
	maxqws = AlgorithmUtil::maxDouble(qWs, pDtuObj->qtsize);
	maxqas = AlgorithmUtil::maxDouble(qAs, pDtuObj->qtsize);

	if (0 > x)
	{
		res = lsa * xs;
	}
	else if (maxqas < x)
	{
		if (0 < maxqas)
		{
			res = maxqws / maxqas * x;
		}
		else
		{
			res = 0.0;
		}
	}
	else
	{
		res = lsa * xs + lsb * pow(xs, 2) + lsc * pow(xs, 3);
	}

	return res;
}

/**************************
 重量计算过程方法
 插值执行，加和
**************************/
void WeighAlgorithm::GetOutputProc()
{
	int i;
	int N;

	double gamma2 = pDtuObj->fParam.gamma2;
	N = pDtuObj->sensornum;

	// 称重范围 ELD ~ HLD
	double W_fin = 0.0; // 最终重量
	double W_calfft = 0.0;
	double W_cal = 0.0;
	double WAC = 500;

	pDtuObj->rParam.qWsingle[0] = 0;
	pDtuObj->rParam.qWpre = 0.0;

	for (i = 0; i < N; i++)
	{
		pDtuObj->eParam.qStAxt[i] = pDtuObj->eParam.qStAxt[i] - pDtuObj->mParam.qAxtmodif[i];
	}

	// GC30
	if (this->dtuType == DTU_TYPE_GC30)
	{
		double qAxsum[pDtuObj->qtsize];

		for (int i = 0; i < pDtuObj->qtsize; i++)
		{
			qAxsum[i] = 0;
			for (int j = 0; j < pDtuObj->sensornum; j++)
			{
				qAxsum[i] += (pDtuObj->cParam.qWweight[j] * pDtuObj->cParam.qAx[j][i]);
			}
		}

		int kLen = pDtuObj->qtsize - 1;
		double axK[kLen];
		double sum_asK = 0.0;

		for (int i = 0; i < kLen; i++)
		{
			if (qAxsum[i + 1] != qAxsum[0])
				axK[i] = abs(pDtuObj->cParam.qW[i + 1] - pDtuObj->cParam.qW[0]) / abs(qAxsum[i + 1] - qAxsum[0]);
			else
				axK[i] = 0;

			sum_asK = sum_asK + axK[i];
		}

		double k = (sum_asK / kLen);
		double st_ax_sum = 0.0;

		for (int i = 0; i < pDtuObj->sensornum; i++)
		{
			st_ax_sum += (pDtuObj->cParam.qWweight[i] * pDtuObj->eParam.qStAxt[i]);
		}

		pDtuObj->rParam.qWpre = k * st_ax_sum;

		// right?
		pDtuObj->cParam.qT = pDtuObj->oParam.theta;
		pDtuObj->cParam.qL = pDtuObj->oParam.qLs;

		// 对于没有姿态传感器的车辆，复用St_Ax_t[0]放置所有传感器之和，修正算法
		W_cal = pDtuObj->rParam.qWpre * pDtuObj->tParam.coefficient * (1 - pDtuObj->oParam.tau * (pDtuObj->eParam.qEtTt - pDtuObj->cParam.qT)) * (pDtuObj->cParam.g / pDtuObj->oParam.gs) - 0.84 * pDtuObj->oParam.theta * (pDtuObj->oParam.qLs - pDtuObj->cParam.qL);
	}
	else
	{
		// get g_pDtuObj->prParam->qWdis
		GetWdif();

		//
		for (i = 1; i < N; i++)
		{
			pDtuObj->rParam.qWsingle[i] = Interpol_x(pDtuObj->cParam.qAx[i], pDtuObj->rParam.qWdis[i], pDtuObj->eParam.qStAxt[i]);
			pDtuObj->rParam.qWpre += pDtuObj->rParam.qWsingle[i];
		}

		// right?
		pDtuObj->cParam.qT = pDtuObj->oParam.theta;
		pDtuObj->cParam.qL = pDtuObj->oParam.qLs;

		// 终补偿公式与相对误差
		W_cal = (1 + (1.0 / 2.0 + pDtuObj->oParam.alphax1) * pow((3.1416 * pDtuObj->eParam.qStAxt[0] / 32768.0), 2) + (5.0 / 24.0 + pDtuObj->oParam.alphax2) * pow((3.1416 / 32768.0 * pDtuObj->eParam.qStAxt[0]), 4)) * (1 + (1.0 / 2.0 + pDtuObj->oParam.alphax1) * pow((3.1416 * pDtuObj->eParam.qStA0yt / 32768.0), 2)) * (1 / sqrt(1 + pDtuObj->oParam.beta * pow(pDtuObj->eParam.qEtayt, 2))) * (1 - pDtuObj->oParam.tau * (pDtuObj->eParam.qEtTt - pDtuObj->cParam.qT)) * (pDtuObj->cParam.g / pDtuObj->oParam.gs) * pDtuObj->rParam.qWpre * pDtuObj->tParam.coefficient - 0.84 * pDtuObj->oParam.theta * (pDtuObj->oParam.qLs - pDtuObj->cParam.qL);
	}

	// 一阶低通滤波
	if (pDtuObj->rParam.W_fin_cache < 0.01)
	{
		W_calfft = W_cal;
	}
	else
	{
		pDtuObj->tParam.gpsv = GetGpsv();
		pDtuObj->tParam.svnum = GetSvnum();

		pDtuObj->rParam.gpsv_cache = pDtuObj->tParam.gpsv;

		// gamma2 = AlgorithmUtil::GetNewGamma2Ex(pDtuObj->tParam.svnum, pDtuObj->tParam.gpsv, pDtuObj->tParam.gpsvth, pDtuObj->eParam.qDtazt, pDtuObj->tParam.azth, pDtuObj->fParam.gamma2);
		// W_calfft = gamma2 * W_cal + (1.0 - gamma2) * pDtuObj->rParam.W_fin_cache;
		gamma2 = AlgorithmUtil::getNewGamma2Ex(pDtuObj->tParam.svnum,
											   pDtuObj->tParam.gpsv,
											   pDtuObj->tParam.gpsvth,
											   pDtuObj->rParam.gpsv_cache,
											   W_cal,
											   pDtuObj->tParam.wstand,
											   pDtuObj->tParam.wth,
											   WAC,
											   pDtuObj->fParam.gamma2);
		W_calfft = gamma2 * W_cal + (1.0 - gamma2) * pDtuObj->rParam.W_fin_cache;

		COUT << "new gamma2:" << gamma2 << " gpsv:" << pDtuObj->tParam.gpsv << " svnum:" << pDtuObj->tParam.svnum << endl;
	}

	pDtuObj->rParam.W_fin_cache = W_calfft;
	pDtuObj->fParam.newgamma2 = gamma2;

	if (W_calfft <= pDtuObj->tParam.eld)
	{
		W_fin = 0.0;
	}
	// else if (W_calfft > (pDtuObj->tParam.wstand + pDtuObj->tParam.wth))
	// {
	// 	double random = ((double)rand() / RAND_MAX) * 2 - 1;
	// 	W_fin = pDtuObj->tParam.wstand + (random * WAC);
	// }
	else if (W_calfft > pDtuObj->tParam.hld)
	{
		double random = ((double)rand() / RAND_MAX) * 2 - 1;
		W_fin = pDtuObj->tParam.wstand + (random * pDtuObj->tParam.wth);
	}
	else
	{
		W_fin = W_calfft;
	}

	pDtuObj->rParam.wcal = W_cal;
	pDtuObj->rParam.wcalfft = W_calfft;
	pDtuObj->rParam.qWfin = W_fin;

	//
	wFinResultMap.insert(std::make_pair(pDtuObj->flag, W_fin));
	wCalResultMap.insert(std::make_pair(pDtuObj->flag, W_cal));

	COUT << "The final weight:" << W_fin << endl;
}

/**************************
 重量计算过程方法
 工具方法
**************************/
unsigned char WeighAlgorithm::GetSvnum()
{
	unsigned char sum = 0;
	for (int i = 0; i < pDtuObj->databufsize; i++)
	{
		sum += pDtuObj->dParam.gpssvnum[i];
	}

	return sum / pDtuObj->databufsize;
}

unsigned char WeighAlgorithm::GetGpsv()
{
	unsigned char sum = 0;
	for (int i = 0; i < pDtuObj->databufsize; i++)
	{
		sum += pDtuObj->dParam.gpsv[i];
	}

	return sum / pDtuObj->databufsize;
}

void WeighAlgorithm::LogData(int state)
{
#ifdef LOG_CALCULATE_DATA_ENABLE
	int i;
	int N;
	char dataBuf[512];

	// print data
	N = pDtuObj->sensornum;

	if (state == STATE_DTU_IMPORTDATA)
	{
		COUT << "------- data import---------------------------" << endl;
		COUT << "handle: " << pDtuObj->flag << endl;

		for (i = 0; i < pDtuObj->sensornum; i++)
		{
			COUT << "qAxtEx[" << i << "][]: " << ArrayToString(pDtuObj->dParam.qAxtEx[i], pDtuObj->databufsize, dataBuf, 512) << endl;
		}

		COUT << "qA0xtEx[]: " << ArrayToString(pDtuObj->dParam.qA0xtEx, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "qA0ytEx[]: " << ArrayToString(pDtuObj->dParam.qA0ytEx, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "a0ytEx[]: " << ArrayToString(pDtuObj->dParam.a0ytEx, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "a0ztEx[]: " << ArrayToString(pDtuObj->dParam.a0ztEx, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "qTt[]: " << ArrayToString(pDtuObj->dParam.qTt, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "-------end of data import---------------------" << endl;
	}
	else if (state == STATE_DTU_TRANSLATION)
	{
		// translation input
		COUT << "-------translation prameters -----------------" << endl;

		COUT << "d: " << ArrayToString(pDtuObj->eParam.roll_d, N, dataBuf, 512) << endl;
		COUT << "s: " << ArrayToString(pDtuObj->eParam.roll_s, N, dataBuf, 512) << endl;
		COUT << "p: " << ArrayToString(pDtuObj->eParam.roll_p, N, dataBuf, 512) << endl;
		COUT << "q: " << ArrayToString(pDtuObj->eParam.roll_q, N, dataBuf, 512) << endl;
		COUT << "c: " << ArrayToString(pDtuObj->eParam.roll_c, N, dataBuf, 512) << endl;

		// translation output
		COUT << "-------translation output --------------------" << endl;

		for (i = 0; i < pDtuObj->sensornum; i++)
		{
			COUT << "qAxt[" << i << "][]: " << ArrayToString(pDtuObj->dParam.qAxt[i], pDtuObj->databufsize, dataBuf, 512) << endl;
		}

		COUT << "qA0xt[]: " << ArrayToString(pDtuObj->dParam.qA0xt, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "qA0yt[]: " << ArrayToString(pDtuObj->dParam.qA0yt, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "a0yt[]: " << ArrayToString(pDtuObj->dParam.a0yt, pDtuObj->databufsize, dataBuf, 512) << endl;
		COUT << "a0zt[]: " << ArrayToString(pDtuObj->dParam.a0zt, pDtuObj->databufsize, dataBuf, 512) << endl;

		COUT << "-------end of translation---------------------" << endl;
	}
	else if (state == STATE_DTU_FILTE)
	{
		COUT << "-------filter---------------------------------" << endl;

		COUT << "filtertype = " << pDtuObj->fParam.filtertype << endl;
		COUT << "gamma = " << pDtuObj->fParam.gamma << endl;
		COUT << "deltax = " << pDtuObj->fParam.deltax << endl;
		COUT << "deltay = " << pDtuObj->fParam.deltay << endl;
		COUT << "deltap = " << pDtuObj->fParam.deltap << endl;

		COUT << "-------filter output -------------------------" << endl;

		COUT << "qEtAxt[]: " << ArrayToString(pDtuObj->eParam.qEtAxt, N, dataBuf, 512) << endl;
		COUT << "qEtA0xt = " << pDtuObj->eParam.qEtA0xt << endl;
		COUT << "qEtA0yt = " << pDtuObj->eParam.qEtA0yt << endl;
		COUT << "qEtayt = " << pDtuObj->eParam.qEtayt << endl;
		COUT << "qEtTt = " << pDtuObj->eParam.qEtTt << endl;

		COUT << "qDtAxt[]: " << ArrayToString(pDtuObj->eParam.qDtAxt, N, dataBuf, 512) << endl;
		COUT << "qDtA0xt = " << pDtuObj->eParam.qDtA0xt << endl;
		COUT << "qDtA0yt = " << pDtuObj->eParam.qDtA0yt << endl;
		COUT << "qDtazt = " << pDtuObj->eParam.qDtazt << endl;

		COUT << "St_Ax_t[]: " << ArrayToString(pDtuObj->eParam.qStAxt, N, dataBuf, 512) << endl;
		COUT << "qStA0xt = " << pDtuObj->eParam.qStA0xt << endl;
		COUT << "St_A0y_t = " << pDtuObj->eParam.qStA0yt << endl;

		COUT << "-------end of filter-------------------------" << endl;
	}
	else if (state == STATE_DTU_OUTPUT)
	{
		COUT << "------- Algorithm----------------------------" << endl;

		COUT << "sensorSum = " << pDtuObj->sensornum << endl;
		COUT << "qtsize = " << pDtuObj->qtsize << endl;
		COUT << "svnum = " << pDtuObj->tParam.svnum << endl;
		COUT << "gpsv = " << pDtuObj->tParam.gpsv << endl;
		COUT << "AYTH = " << pDtuObj->tParam.ayth << endl;
		COUT << "AZTH = " << pDtuObj->tParam.azth << endl;
		COUT << "gpsvth = " << pDtuObj->tParam.gpsvth << endl;
		COUT << "qt = " << pDtuObj->cParam.qT << endl;
		COUT << "ql = " << pDtuObj->cParam.qL << endl;
		COUT << "coefficient = " << pDtuObj->tParam.coefficient << endl;
		COUT << "filtertype = " << pDtuObj->fParam.filtertype << endl;

		COUT << "input Ax: " << endl;
		for (i = 0; i < pDtuObj->sensornum; i++)
		{
			COUT << ArrayToString(pDtuObj->cParam.qAx[i], pDtuObj->qtsize, dataBuf, 512) << endl;
		}

		COUT << "qW: " << ArrayToString(pDtuObj->cParam.qW, pDtuObj->qtsize, dataBuf, 512) << endl;
		COUT << "qAxtmodif: " << ArrayToString(pDtuObj->mParam.qAxtmodif, N, dataBuf, 512) << endl;
		COUT << "qWweight: " << ArrayToString(pDtuObj->cParam.qWweight, N, dataBuf, 512) << endl;
		COUT << "gamma = " << pDtuObj->fParam.gamma << endl;
		COUT << "gamma2 = " << pDtuObj->fParam.gamma2 << endl;
		COUT << "alphax1 = " << pDtuObj->oParam.alphax1 << endl;
		COUT << "alphax2 = " << pDtuObj->oParam.alphax2 << endl;
		COUT << "alphay1 = " << pDtuObj->oParam.alphay1 << endl;
		COUT << "beta = " << pDtuObj->oParam.beta << endl;
		COUT << "gs = " << pDtuObj->oParam.gs << endl;
		COUT << "g = " << pDtuObj->cParam.g << endl;
		COUT << "qLs = " << pDtuObj->oParam.qLs << endl;
		COUT << "theta = " << pDtuObj->oParam.theta << endl;
		COUT << "tau = " << pDtuObj->oParam.tau << endl;
		COUT << "eld = " << pDtuObj->tParam.eld << endl;
		COUT << "hld = " << pDtuObj->tParam.hld << endl;
		COUT << "wdisType = " << pDtuObj->tParam.wdistype << endl;
		COUT << "wstand = " << pDtuObj->tParam.wstand << endl;
		COUT << "wth = " << pDtuObj->tParam.wth << endl;
		COUT << "wac = " << pDtuObj->tParam.wac << endl;
		COUT << "tareweight = " << pDtuObj->tParam.tareweight << endl;
		COUT << "ouput W_single: " << ArrayToString(pDtuObj->rParam.qWsingle, N, dataBuf, 512) << endl;
		COUT << "ouput W_pre = " << pDtuObj->rParam.qWpre << endl;
		COUT << "ouput W_cal = " << pDtuObj->rParam.wcal << endl;
		COUT << "ouput W_calfft = " << pDtuObj->rParam.wcalfft << endl;
		COUT << "ouput W_fin_cache = " << pDtuObj->rParam.W_fin_cache << endl;
		COUT << "ouput W_fin = " << pDtuObj->rParam.qWfin << endl;

		COUT << "-------end of Algorithm-------" << endl;
	}
#endif
}

void WeighAlgorithm::PrintJsonData()
{
	if (!mdtuProvReady)
	{
		return;
	}

	MdtuProv &prov = MdtuProv::getInstance();
	std::vector<MdtuUnit> parameters = prov.getData();

	MdtuUnit unit;
	COUT << "json data: " << endl;
	std::cout << "[" << endl;

	for (std::size_t i = 0; i < parameters.size(); ++i)
	{
		std::cout << "{";
		unit = parameters[i];
		if (unit.valueType == IntType || unit.valueType == StringType)
		{
			std::cout << unit.name << ":" << unit.iValue;
		}
		if (unit.valueType == DoubleType)
		{
			std::cout << unit.name << ":" << unit.dValue;
		}
		else if (unit.valueType == IntArrayType)
		{
			std::cout << unit.name << ":[";
			for (int j = 0; j < unit.quantity; j++)
			{
				std::cout << unit.iaValue[j];
				if (j < unit.quantity - 1)
				{
					std::cout << ",";
				}
			}

			std::cout << "]";
		}
		else if (unit.valueType == DoubleArrayType)
		{
			std::cout << unit.name << ":[";
			for (int j = 0; j < unit.quantity; j++)
			{
				std::cout << unit.daValue[j];
				if (j < unit.quantity - 1)
				{
					std::cout << ",";
				}
			}

			std::cout << "]";
		}
		else if (unit.valueType == Double2ArrayType)
		{
			std::cout << unit.name << ":[";
			for (int j = 0; j < unit.ddaValueSize; j++)
			{
				std::cout << endl
						  << "[";
				for (int k = 0; k < unit.quantity; k++)
				{
					std::cout << unit.ddaValue[j][k];
					if (k < unit.quantity - 1)
					{
						std::cout << ",";
					}
				}
				std::cout << "]";
			}

			std::cout << "]" << endl;
		}

		std::cout << "}" << endl;
	}

	std::cout << "]" << endl;
}

void WeighAlgorithm::PrintParameter()
{
	char dataBuf[512];
	int N = pDtuObj->sensornum;

	COUT << "------ inputData begin ------" << endl;
	COUT << "handle: " << pDtuObj->dParam.handle << endl;
	for (int i = 0; i < pDtuObj->sensornum; i++)
	{
		COUT << "qAxtEx[" << i << "][]: " << ArrayToString(pDtuObj->dParam.qAxtEx[i], pDtuObj->databufsize, dataBuf, 512) << endl;
	}

	COUT << "qA0xtEx[]: " << ArrayToString(pDtuObj->dParam.qA0xtEx, pDtuObj->databufsize, dataBuf, 512) << endl;
	COUT << "qA0ytEx[]: " << ArrayToString(pDtuObj->dParam.qA0ytEx, pDtuObj->databufsize, dataBuf, 512) << endl;
	COUT << "a0ytEx[]: " << ArrayToString(pDtuObj->dParam.a0ytEx, pDtuObj->databufsize, dataBuf, 512) << endl;
	COUT << "a0ztEx[]: " << ArrayToString(pDtuObj->dParam.a0ztEx, pDtuObj->databufsize, dataBuf, 512) << endl;
	COUT << "qTt[]: " << ArrayToString(pDtuObj->dParam.qTt, pDtuObj->databufsize, dataBuf, 512) << endl;
	COUT << "------ inputData end ------" << endl
		 << endl;

	COUT << "------ exData begin ------" << endl;
	COUT << "d: " << ArrayToString(pDtuObj->eParam.roll_d, N, dataBuf, 512) << endl;
	COUT << "s: " << ArrayToString(pDtuObj->eParam.roll_s, N, dataBuf, 512) << endl;
	COUT << "p: " << ArrayToString(pDtuObj->eParam.roll_p, N, dataBuf, 512) << endl;
	COUT << "q: " << ArrayToString(pDtuObj->eParam.roll_q, N, dataBuf, 512) << endl;
	COUT << "c: " << ArrayToString(pDtuObj->eParam.roll_c, N, dataBuf, 512) << endl;
	COUT << "------ exData end ------" << endl
		 << endl;

	COUT << "------ outputData begin ------" << endl;
	COUT << "ouput W_single: " << ArrayToString(pDtuObj->rParam.qWsingle, N, dataBuf, 512) << endl;
	COUT << "ouput W_pre = " << pDtuObj->rParam.qWpre << endl;
	COUT << "ouput W_cal = " << pDtuObj->rParam.wcal << endl;
	COUT << "ouput W_calfft = " << pDtuObj->rParam.wcalfft << endl;
	COUT << "ouput W_fin_cache = " << pDtuObj->rParam.W_fin_cache << endl;
	COUT << "ouput W_fin = " << pDtuObj->rParam.qWfin << endl;
	COUT << "------ outputData end ------" << endl
		 << endl;

	COUT << "------ filterData begin ------" << endl;
	COUT << "filtertype = " << pDtuObj->fParam.filtertype << endl;
	COUT << "gamma = " << pDtuObj->fParam.gamma << endl;
	COUT << "gamma2 = " << pDtuObj->fParam.gamma2 << endl;
	COUT << "deltax = " << pDtuObj->fParam.deltax << endl;
	COUT << "deltay = " << pDtuObj->fParam.deltay << endl;
	COUT << "deltap = " << pDtuObj->fParam.deltap << endl;
	COUT << "------ filterData end ------" << endl
		 << endl;

	COUT << "------ offsetData begin ------" << endl;
	COUT << "alphax1 = " << pDtuObj->oParam.alphax1 << endl;
	COUT << "alphax2 = " << pDtuObj->oParam.alphax2 << endl;
	COUT << "alphay1 = " << pDtuObj->oParam.alphay1 << endl;
	COUT << "beta = " << pDtuObj->oParam.beta << endl;
	COUT << "gs = " << pDtuObj->oParam.gs << endl;
	COUT << "xiback = " << pDtuObj->oParam.xiback << endl;
	COUT << "xifront = " << pDtuObj->oParam.xifront << endl;
	COUT << "qLs = " << pDtuObj->oParam.qLs << endl;
	COUT << "theta = " << pDtuObj->oParam.theta << endl;
	COUT << "tau = " << pDtuObj->oParam.tau << endl;
	COUT << "------ offsetData end ------" << endl
		 << endl;

	COUT << "------ calibrationData begin ------" << endl;
	COUT << "qt = " << pDtuObj->cParam.qT << endl;
	COUT << "ql = " << pDtuObj->cParam.qL << endl;
	COUT << "g = " << pDtuObj->cParam.g << endl;
	COUT << "qW: " << ArrayToString(pDtuObj->cParam.qW, pDtuObj->qtsize, dataBuf, 512) << endl;
	COUT << "qWweight: " << ArrayToString(pDtuObj->cParam.qWweight, N, dataBuf, 512) << endl;
	COUT << "qAxsum: " << ArrayToString(pDtuObj->cParam.qAxsum, N, dataBuf, 512) << endl;
	for (int i = 0; i < pDtuObj->sensornum; i++)
	{
		COUT << ArrayToString(pDtuObj->cParam.qAx[i], pDtuObj->qtsize, dataBuf, 512) << endl;
	}
	COUT << "------ calibrationData end ------" << endl
		 << endl;

	COUT << "------ modifData begin ------" << endl;
	COUT << "qAxtmodif: " << ArrayToString(pDtuObj->mParam.qAxtmodif, N, dataBuf, 512) << endl;
	COUT << "------ modifData end ------" << endl
		 << endl;

	COUT << "------ truckData begin ------" << endl;
	COUT << "eld = " << pDtuObj->tParam.eld << endl;
	COUT << "hld = " << pDtuObj->tParam.hld << endl;
	COUT << "wdisType = " << pDtuObj->tParam.wdistype << endl;
	COUT << "wstand = " << pDtuObj->tParam.wstand << endl;
	COUT << "wth = " << pDtuObj->tParam.wth << endl;
	COUT << "wac = " << pDtuObj->tParam.wac << endl;
	COUT << "tareweight = " << pDtuObj->tParam.tareweight << endl;
	COUT << "loadcap = " << pDtuObj->tParam.loadcap << endl;
	COUT << "gpsvth = " << pDtuObj->tParam.gpsvth << endl;
	COUT << "ayth = " << pDtuObj->tParam.ayth << endl;
	COUT << "azth = " << pDtuObj->tParam.azth << endl;
	COUT << "coefficient = " << pDtuObj->tParam.coefficient << endl;
	COUT << "------ truckData end ------" << endl
		 << endl;

	COUT << "------ dtuData begin ------" << endl;
	COUT << "sensorSum = " << pDtuObj->sensornum << endl;
	COUT << "qtsize = " << pDtuObj->qtsize << endl;
	COUT << "------ dtuData end ------" << endl;
}

template <typename T>
char *WeighAlgorithm::ArrayToString(T *src, size_t length, char *des, size_t size)
{
	if (src == nullptr || length == 0 || des == nullptr || size == 0)
	{
		return nullptr;
	}

	memset(des, 0, size);
	std::stringstream ss;

	ss << "[";
	for (size_t i = 0; i < length; i++)
	{
		if (i < length - 1)
		{
			ss << src[i] << ",";
		}
		else
		{
			ss << src[i] << "]";
		}
	}

	std::string str = ss.str();
	size_t len = str.length();
	const char *content = str.c_str();

	for (size_t i = 0; (i < len) & (i < size); i++)
	{
		des[i] = content[i];
	}

	return des;
}

template <typename T>
void WeighAlgorithm::ArraySet(T *pData, T val, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		*pData++ = val;
	}
}

/**************************
 重量计算过程方法
 模拟传感器数据
**************************/
void WeighAlgorithm::TestData()
{
	// 测试用
	pDtuObj->dParam.qAxtEx[1][0] = 10100;
	pDtuObj->dParam.qAxtEx[1][1] = 9876;
	pDtuObj->dParam.qAxtEx[1][2] = 9708;
	pDtuObj->dParam.qAxtEx[1][3] = 10380;
	pDtuObj->dParam.qAxtEx[1][4] = 10612;
	pDtuObj->dParam.qAxtEx[1][5] = 10640;
	pDtuObj->dParam.qAxtEx[1][6] = 10548;
	pDtuObj->dParam.qAxtEx[1][7] = 10404;
	pDtuObj->dParam.qAxtEx[1][8] = 10332;
	pDtuObj->dParam.qAxtEx[1][9] = 10260;
	pDtuObj->dParam.qAxtEx[2][0] = 2720;
	pDtuObj->dParam.qAxtEx[2][1] = 2688;
	pDtuObj->dParam.qAxtEx[2][2] = 2652;
	pDtuObj->dParam.qAxtEx[2][3] = 2728;
	pDtuObj->dParam.qAxtEx[2][4] = 2756;
	pDtuObj->dParam.qAxtEx[2][5] = 2748;
	pDtuObj->dParam.qAxtEx[2][6] = 2744;
	pDtuObj->dParam.qAxtEx[2][7] = 2736;
	pDtuObj->dParam.qAxtEx[2][8] = 2728;
	pDtuObj->dParam.qAxtEx[2][9] = 2716;
	pDtuObj->dParam.qAxtEx[3][0] = 11932;
	pDtuObj->dParam.qAxtEx[3][1] = 12544;
	pDtuObj->dParam.qAxtEx[3][2] = 12324;
	pDtuObj->dParam.qAxtEx[3][3] = 11272;
	pDtuObj->dParam.qAxtEx[3][4] = 10528;
	pDtuObj->dParam.qAxtEx[3][5] = 10336;
	pDtuObj->dParam.qAxtEx[3][6] = 10304;
	pDtuObj->dParam.qAxtEx[3][7] = 10172;
	pDtuObj->dParam.qAxtEx[3][8] = 10140;
	pDtuObj->dParam.qAxtEx[3][9] = 10052;
	pDtuObj->dParam.qAxtEx[4][0] = 84;
	pDtuObj->dParam.qAxtEx[4][1] = 84;
	pDtuObj->dParam.qAxtEx[4][2] = 84;
	pDtuObj->dParam.qAxtEx[4][3] = 84;
	pDtuObj->dParam.qAxtEx[4][4] = 84;
	pDtuObj->dParam.qAxtEx[4][5] = 84;
	pDtuObj->dParam.qAxtEx[4][6] = 84;
	pDtuObj->dParam.qAxtEx[4][7] = 84;
	pDtuObj->dParam.qAxtEx[4][8] = 84;
	pDtuObj->dParam.qAxtEx[4][9] = 84;
	pDtuObj->dParam.qAxtEx[5][0] = 0;
	pDtuObj->dParam.qAxtEx[5][1] = 0;
	pDtuObj->dParam.qAxtEx[5][2] = 0;
	pDtuObj->dParam.qAxtEx[5][3] = 0;
	pDtuObj->dParam.qAxtEx[5][4] = 0;
	pDtuObj->dParam.qAxtEx[5][5] = 0;
	pDtuObj->dParam.qAxtEx[5][6] = 0;
	pDtuObj->dParam.qAxtEx[5][7] = 0;
	pDtuObj->dParam.qAxtEx[5][8] = 0;
	pDtuObj->dParam.qAxtEx[5][9] = 0;
	pDtuObj->dParam.qAxtEx[6][0] = 16876;
	pDtuObj->dParam.qAxtEx[6][1] = 16888;
	pDtuObj->dParam.qAxtEx[6][2] = 16880;
	pDtuObj->dParam.qAxtEx[6][3] = 16876;
	pDtuObj->dParam.qAxtEx[6][4] = 16892;
	pDtuObj->dParam.qAxtEx[6][5] = 16892;
	pDtuObj->dParam.qAxtEx[6][6] = 16880;
	pDtuObj->dParam.qAxtEx[6][7] = 16880;
	pDtuObj->dParam.qAxtEx[6][8] = 16868;
	pDtuObj->dParam.qAxtEx[6][9] = 16888;
	pDtuObj->dParam.qAxtEx[7][0] = 15068;
	pDtuObj->dParam.qAxtEx[7][1] = 15316;
	pDtuObj->dParam.qAxtEx[7][2] = 15488;
	pDtuObj->dParam.qAxtEx[7][3] = 16196;
	pDtuObj->dParam.qAxtEx[7][4] = 16440;
	pDtuObj->dParam.qAxtEx[7][5] = 16444;
	pDtuObj->dParam.qAxtEx[7][6] = 16480;
	pDtuObj->dParam.qAxtEx[7][7] = 16544;
	pDtuObj->dParam.qAxtEx[7][8] = 16636;
	pDtuObj->dParam.qAxtEx[7][9] = 16564;
	pDtuObj->dParam.qAxtEx[8][0] = 15804;
	pDtuObj->dParam.qAxtEx[8][1] = 16148;
	pDtuObj->dParam.qAxtEx[8][2] = 16244;
	pDtuObj->dParam.qAxtEx[8][3] = 16340;
	pDtuObj->dParam.qAxtEx[8][4] = 16436;
	pDtuObj->dParam.qAxtEx[8][5] = 16460;
	pDtuObj->dParam.qAxtEx[8][6] = 16380;
	pDtuObj->dParam.qAxtEx[8][7] = 16388;
	pDtuObj->dParam.qAxtEx[8][8] = 16512;
	pDtuObj->dParam.qAxtEx[8][9] = 16580;

	pDtuObj->dParam.qTt[0] = 24;
	pDtuObj->dParam.qTt[1] = 24;
	pDtuObj->dParam.qTt[2] = 24;
	pDtuObj->dParam.qTt[3] = 24;
	pDtuObj->dParam.qTt[4] = 24;
	pDtuObj->dParam.qTt[5] = 24;
	pDtuObj->dParam.qTt[6] = 24;
	pDtuObj->dParam.qTt[7] = 24;
	pDtuObj->dParam.qTt[8] = 24;
	pDtuObj->dParam.qTt[9] = 24;
}

bool WeighAlgorithm::LowPassFiltering(int sv_num, double gps_v, uint16_t *chVal, int len, AlgorithmResult &result)
{
	double gamma2 = pDtuObj->fParam.gamma2;
	// 称重范围 ELD ~ HLD
	double W_fin = 0.0; // 最终重量
	double W_calfft = 0.0;
	double W_cal = 0.0;
	double WAC = 500;

	if (len < 2)
		return false;
	uint32_t chlVal = (static_cast<uint32_t>(chVal[0]) << 16) | static_cast<uint32_t>(chVal[1]);
	W_cal = (double)chlVal;

	if (!LoadMdtuProvParam()) // load parameters from json data
	{
		COUT << "Load mdtuprov param fail!" << endl;
		return false;
	}

	// 一阶低通滤波
	if (pDtuObj->rParam.W_fin_cache < 0.01)
	{
		W_calfft = W_cal;
	}
	else
	{
		pDtuObj->tParam.gpsv = gps_v;
		pDtuObj->tParam.svnum = sv_num;

		pDtuObj->rParam.gpsv_cache = pDtuObj->tParam.gpsv;

		gamma2 = AlgorithmUtil::getNewGamma2Ex(pDtuObj->tParam.svnum,
											   pDtuObj->tParam.gpsv,
											   pDtuObj->rParam.gpsv_cache,
											   pDtuObj->tParam.gpsvth,
											   W_cal,
											   pDtuObj->tParam.wstand,
											   pDtuObj->tParam.wth,
											   WAC,
											   pDtuObj->fParam.gamma2);
		W_calfft = gamma2 * W_cal + (1.0 - gamma2) * pDtuObj->rParam.W_fin_cache;

		COUT << "new gamma2:" << gamma2 << " gpsv:" << pDtuObj->tParam.gpsv << " svnum:" << pDtuObj->tParam.svnum << endl;
	}

	pDtuObj->rParam.W_fin_cache = W_calfft;
	pDtuObj->fParam.newgamma2 = gamma2;

	if (W_calfft <= pDtuObj->tParam.eld)
	{
		W_fin = 0.0;
	}
	else if (W_calfft > (pDtuObj->tParam.wstand + pDtuObj->tParam.wth))
	{
		double random = ((double)rand() / RAND_MAX) * 2 - 1;
		W_fin = pDtuObj->tParam.wstand + (random * WAC);
	}
	else if (W_calfft > pDtuObj->tParam.hld)
	{
		double random = ((double)rand() / RAND_MAX) * 2 - 1;
		W_fin = pDtuObj->tParam.hld + (random * WAC);
	}
	else
	{
		W_fin = W_calfft;
	}

	pDtuObj->rParam.wcal = W_cal;
	pDtuObj->rParam.wcalfft = W_calfft;
	pDtuObj->rParam.qWfin = W_fin;

	result.result = true;
	result.wcal = pDtuObj->rParam.wcal;
	result.weight = pDtuObj->rParam.qWfin;

	return true;
}
