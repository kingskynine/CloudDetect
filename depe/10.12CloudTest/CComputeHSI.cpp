#include "CComputeHSI.h"
#include <iostream>
#include "gdal_priv.h"
CComputeHSI::CComputeHSI()
{
	string m_strInputRFileName=string();
	string m_strInputGFileName=string();
	string m_strInputBFileName=string();

	m_bInit = false;
}
CComputeHSI::~CComputeHSI()
{
}
//初始化
bool CComputeHSI::Initilization(const string & strInputRFileName, const string & strInputGFileName,const string & strInputBFileName)
{
	if (strInputRFileName.empty()||strInputGFileName.empty()||strInputBFileName.empty())
	{
		std::cout<<"文件名为空！";
		system("pause");
		return false;
	}
	m_strInputBFileName=strInputBFileName;
	m_strInputGFileName=strInputGFileName;
	m_strInputRFileName=strInputRFileName;
	//注册所有驱动
	GDALAllRegister();
	GDALDataset *pDatasetR=(GDALDataset*)GDALOpen(m_strInputRFileName.c_str(),GA_Update);
	GDALDataset *pDatasetG=(GDALDataset*)GDALOpen(m_strInputGFileName.c_str(),GA_Update);
	GDALDataset *pDatasetB=(GDALDataset*)GDALOpen(m_strInputBFileName.c_str(),GA_Update);
	if (pDatasetB==0||pDatasetG==0||pDatasetR==0)
	{
		m_strInputBFileName.erase(m_strInputBFileName.begin(),m_strInputBFileName.end());
		m_strInputGFileName.erase(m_strInputGFileName.begin(),m_strInputGFileName.end());
		m_strInputRFileName.erase(m_strInputRFileName.begin(),m_strInputRFileName.end());
		std::cout<<"文件打开错误！"<<std::endl;
		system("pause");
		return false;
	}
	GDALClose(pDatasetB);
	GDALClose(pDatasetR);
	GDALClose(pDatasetG);
	m_bInit=true;
	return true;
}





//执行
bool CComputeHSI::Execute(Float32 *pRBuf,const string & strOutputHFileName,
						   Float32 *pGBuf,const string & strOutputSFileName,
						   Float32 *pBBuf,const string & strOutputIFileName)
{
	if (strOutputHFileName.empty()||strOutputSFileName.empty()||strOutputIFileName.empty())
	{
		std::cout<<"输出文件名为空！";
		system("pause");
		return false;
	}
	m_strOutputHFileName=strOutputHFileName;
	m_strOutputSFileName=strOutputSFileName;
	m_strOutputIFileName=strOutputIFileName;

	if (m_bInit==false)
	{
		std::cout<<"未执行初始化！"<<std::endl;
		system("pause");
		return false;
	}
	GDALAllRegister();
	GDALDataset * pSrcDatasetR = (GDALDataset *) GDALOpen(m_strInputRFileName.c_str(),GA_Update);
	GDALDataset * pSrcDatasetG = (GDALDataset *) GDALOpen(m_strInputGFileName.c_str(),GA_Update);
	GDALDataset * pSrcDatasetB = (GDALDataset *) GDALOpen(m_strInputBFileName.c_str(),GA_Update);
	//获取影像宽度
	int nWidth, nHeight;
	nWidth = pSrcDatasetR->GetRasterXSize();
	nHeight = pSrcDatasetR->GetRasterYSize();

	double HRGeoTransForm[6];
	pSrcDatasetR->GetGeoTransform(HRGeoTransForm);

	//创建目标文件
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	if (poDriver == 0)
	{
		GDALClose(pSrcDatasetR);
		pSrcDatasetR = 0;
		GDALClose(pSrcDatasetG);
		pSrcDatasetG = 0;
		GDALClose(pSrcDatasetB);
		pSrcDatasetB= 0;
		return false;
	}	
	int nBandCount = pSrcDatasetR->GetRasterCount();
	GDALDataset *pDrcDatasetH = poDriver->Create(m_strOutputHFileName.c_str(), nWidth, nHeight, nBandCount, GDT_Float32, NULL);
	GDALDataset *pDrcDatasetS = poDriver->Create(m_strOutputSFileName.c_str(), nWidth, nHeight, nBandCount, GDT_Float32, NULL);
	GDALDataset *pDrcDatasetI = poDriver->Create(m_strOutputIFileName.c_str(), nWidth, nHeight, nBandCount, GDT_Float32, NULL);
	if (pDrcDatasetH == 0||pDrcDatasetS == 0||pDrcDatasetI == 0)
	{
		GDALClose(pSrcDatasetR);
		pSrcDatasetR = 0;
		GDALClose(pSrcDatasetG);
		pSrcDatasetG = 0;
		GDALClose(pSrcDatasetB);
		pSrcDatasetB = 0;
		return false;
	}
	const char *  pszSRS_WKT =  pSrcDatasetR->GetProjectionRef();	
	pDrcDatasetH->SetGeoTransform(HRGeoTransForm);
	pDrcDatasetH->SetProjection(pszSRS_WKT);
	pDrcDatasetH->SetMetadata(pSrcDatasetR->GetMetadata());

	pDrcDatasetS->SetGeoTransform(HRGeoTransForm);
	pDrcDatasetS->SetProjection(pszSRS_WKT);
	pDrcDatasetS->SetMetadata(pSrcDatasetR->GetMetadata());

	pDrcDatasetI->SetGeoTransform(HRGeoTransForm);
	pDrcDatasetI->SetProjection(pszSRS_WKT);
	pDrcDatasetI->SetMetadata(pSrcDatasetR->GetMetadata());

	if (false==ComputeHSI(pSrcDatasetR, pSrcDatasetG, pSrcDatasetB,pDrcDatasetH,pDrcDatasetS,pDrcDatasetI,
		pRBuf,pGBuf,pBBuf))
	{
		GDALClose(pSrcDatasetR);
		pSrcDatasetR = 0;
		GDALClose(pDrcDatasetH);
		pDrcDatasetH = 0;
		GDALClose(pSrcDatasetG);
		pSrcDatasetG = 0;
		GDALClose(pDrcDatasetS);
		pDrcDatasetS = 0;
		GDALClose(pSrcDatasetB);
		pSrcDatasetB = 0;
		GDALClose(pDrcDatasetI);
		pDrcDatasetI = 0;
		std::cout<<"计算HSI出错！"<<std::endl;
		system("pause");
		return false;
	}

	GDALClose(pSrcDatasetR);
	pSrcDatasetR = 0;
	GDALClose(pDrcDatasetH);
	pDrcDatasetH = 0;
	GDALClose(pSrcDatasetG);
	pSrcDatasetG = 0;
	GDALClose(pDrcDatasetS);
	pDrcDatasetS = 0;
	GDALClose(pSrcDatasetB);
	pSrcDatasetB = 0;
	GDALClose(pDrcDatasetI);
	pDrcDatasetI = 0;
	return true;
}

//线性拉伸
bool CComputeHSI::ComputeHSI(GDALDataset * pSrcDatasetR,GDALDataset * pSrcDatasetG,GDALDataset * pSrcDatasetB,
							   GDALDataset * pDrcDatasetR,GDALDataset * pDrcDatasetG,GDALDataset * pDrcDatasetB,
							   Float32 *pRBuf,Float32 *pGBuf,Float32 *pBBuf)
{
	int nWidth,nHeight;
	nWidth = pSrcDatasetR->GetRasterXSize();
	nHeight = pSrcDatasetR->GetRasterYSize();
	GDALRasterBand * pSrcBandR = pSrcDatasetR->GetRasterBand(1);
	GDALRasterBand * pDstBandR = pDrcDatasetR->GetRasterBand(1);
	GDALRasterBand * pSrcBandG = pSrcDatasetG->GetRasterBand(1);
	GDALRasterBand * pDstBandG = pDrcDatasetG->GetRasterBand(1);
	GDALRasterBand * pSrcBandB = pSrcDatasetB->GetRasterBand(1);
	GDALRasterBand * pDstBandB = pDrcDatasetB->GetRasterBand(1);
	if (pSrcBandR->GetRasterDataType() != GDT_Float32)
	{
		std::cout<<"格式不是GDT_Float32"<<std::endl;
		system("pause");
		return false;
	}

	int nXBlockSize,nYBlockSize;
	int iXBlock,iYBlock;
	int iXOffset,iYOffset;
	int w = 0;
	int h =	0;

	pSrcBandR->GetBlockSize(&nXBlockSize,&nYBlockSize);	

	if (nXBlockSize<64||nXBlockSize>512)		
	{
		nXBlockSize = 512;
	}
	if (nYBlockSize<64||nYBlockSize>512)
	{
		nYBlockSize = 512;
	}
	int nXBlocks = (nWidth + nXBlockSize - 1)/nXBlockSize;
	int nYBlocks = (nHeight + nYBlockSize - 1)/nYBlockSize;
	long int nBlockSize = nXBlockSize * nYBlockSize;
	pRBuf = new Float32[nBlockSize]; 
	pGBuf = new Float32[nBlockSize]; 
	pBBuf = new Float32[nBlockSize];
	Float32 *pDRBuf = new Float32[nBlockSize]; 
	Float32 *pDGBuf = new Float32[nBlockSize]; 
	Float32 *pDBBuf = new Float32[nBlockSize];
	max=new Float32[nBlockSize];
	min=new Float32[nBlockSize];
	for (iYBlock=0; iYBlock<nYBlocks; iYBlock++)
	{
		iYOffset = iYBlock*nYBlockSize;
		for (iXBlock=0; iXBlock<nXBlocks; iXBlock++)
		{
			iXOffset = iXBlock*nXBlockSize;
			w = nXBlockSize;
			h = nYBlockSize;
			//影像边缘不足一个块的宽和长时的操作
			if (iXBlock == nXBlocks - 1)
			{
				w = nWidth - iXBlock*nXBlockSize;				
			}
			if (iYBlock == nYBlocks - 1)
			{
				h = nHeight - iYBlock*nYBlockSize;				
			}
			nBlockSize = w*h;	
			/////////////////////////////////////////////////////
			//-------------------------------------------------->
			//无效值如何处理
			//-------------------------------------------------->
			/////////////////////////////////////////////////////

			//double a=pSrcBandR->GetNoDataValue(pbSuccess=NULL);
			//读取失败
			if (pSrcBandR->RasterIO(GF_Read,iXOffset,iYOffset,w,h,pRBuf,w,h,GDT_Float64,0,0) != CE_None||
				pSrcBandG->RasterIO(GF_Read,iXOffset,iYOffset,w,h,pGBuf,w,h,GDT_Float64,0,0) != CE_None||
				pSrcBandB->RasterIO(GF_Read,iXOffset,iYOffset,w,h,pBBuf,w,h,GDT_Float64,0,0) != CE_None)
			{
				delete [] pRBuf;
				pRBuf = 0;
				delete [] pGBuf;
				pGBuf = 0;
				delete [] pBBuf;
				pBBuf = 0;
				std::cout<<"读取文件出错！"<<std::endl;
				return false;
			}
			for (int i=0;i<nBlockSize;i++)
			{
				///////////////////////////////////////////////////////////
				//-------------------------------------------------------->
				//计算HSI
				//-------------------------------------------------------->
				///////////////////////////////////////////////////////////
				max[i]=Max(Max(pRBuf[i],pGBuf[i]),pBBuf[i]);
				min[i]=Min(Min(pRBuf[i],pGBuf[i]),pBBuf[i]);
				if (max[i]==pRBuf[i])
				{
					pDRBuf[i]=(M_PI/3)*(pGBuf[i]-pBBuf[i])/(max[i]-min[i]);
				}
				if (max[i]==pGBuf[i])
				{
					pDRBuf[i]=(M_PI/3)*(pBBuf[i]-pRBuf[i])/(max[i]-min[i])+(2*M_PI)/3;
				}
				if (max[i]==pBBuf[i])
				{
					pDRBuf[i]=(M_PI/3)*(pRBuf[i]-pGBuf[i])/(max[i]-min[i])+(4*M_PI)/3;
				}
				if (pDRBuf[i]<0)
				{
					pDRBuf[i]+=2*M_PI;
				}
				pDGBuf[i]=max[i]-min[i];
				pDBBuf[i]=(max[i]+min[i])/2;
			}

			//写入失败
			if (pDstBandR->RasterIO(GF_Write,iXOffset,iYOffset,w,h,pDRBuf,w,h,GDT_Float64,0,0) != CE_None||
				pDstBandG->RasterIO(GF_Write,iXOffset,iYOffset,w,h,pDGBuf,w,h,GDT_Float64,0,0) != CE_None||
				pDstBandB->RasterIO(GF_Write,iXOffset,iYOffset,w,h,pDBBuf,w,h,GDT_Float64,0,0) != CE_None)
			{
				delete [] pRBuf;
				pRBuf = 0;
				delete [] pGBuf;
				pGBuf = 0;
				delete [] pBBuf;
				pBBuf = 0;
				delete [] pDRBuf;
				pDRBuf = 0;
				delete [] pDGBuf;
				pDGBuf = 0;
				delete [] pDBBuf;
				pDBBuf = 0;
				std::cout<<"写入文件出错！"<<std::endl;
				return false;
			}
		}
	}
	delete [] pRBuf;
	pRBuf = 0;
	delete [] pGBuf;
	pGBuf = 0;
	delete [] pBBuf;
	pBBuf = 0;
	delete [] pDRBuf;
	pDRBuf = 0;
	delete [] pDGBuf;
	pDGBuf = 0;
	delete [] pDBBuf;
	pDBBuf = 0;
	return true;
}
Float32 CComputeHSI::Max(Float32 a,Float32 b)
{
	if (a>=b)
	{
		return a;
	}else
	{
		return b;
	}
}
Float32 CComputeHSI::Min(Float32 a,Float32 b)
{
	if (a>=b)
	{
		return b;
	}else
	{
		return a;
	}
}