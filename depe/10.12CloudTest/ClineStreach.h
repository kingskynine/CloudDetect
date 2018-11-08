#ifdef LINESTREATCH_H_INCLUDE
#endif LINESTREATCH_H_INCLUDE
#include <string>
using std::string;
typedef unsigned short UInt16;
typedef double Float32;
class GDALDataset;
class CLineStreach
{
public:
	CLineStreach();
	~CLineStreach();
	bool Initilization(const string & strInputRFileName,
		const string & strInputGFileName,const string & strInputBFileName);
	bool Execute(Float32 *pRBuf,const string & strOutputHFileName,
		Float32 *pGBuf,const string & strOutputSFileName,
		Float32 *pBBuf,const string & strOutputIFileName,
		int AimsMin,int AimsMax);	
	bool LineStrecth(GDALDataset * pSrcDatasetR,GDALDataset * pSrcDatasetG,GDALDataset * pSrcDatasetB,
		GDALDataset * pDrcDatasetR,GDALDataset * pDrcDatasetG,GDALDataset * pDrcDatasetB,
		Float32 *pRBuf,Float32 *pGBuf,Float32 *pBBuf,int AimsMin,int AimsMax);
protected:
	string m_strInputRFileName;//����R�ļ���
	string m_strInputGFileName;//����G�ļ���
	string m_strInputBFileName;//����B�ļ���
	string m_strOutputHFileName;//���H�ļ���
	string m_strOutputSFileName;//���S�ļ���
	string m_strOutputIFileName;//���I�ļ���
	Float32 adfRmaxmin[2];
	Float32 adfGmaxmin[2];
	Float32 adfBmaxmin[2];
	bool   m_bInit;
};

