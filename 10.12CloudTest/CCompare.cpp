#include "CCompare.h"
#include <iostream>
#include "gdal_priv.h"
Compare::Compare()
{
	string m_strInputRFileName=string();
	string m_strInputGFileName=string();
	string m_strInputBFileName=string();
	m_bInit = false;
	m_nBandCouts = 0;
}
Compare::~Compare()
{
}
//初始化
bool Compare::Initilization(const string & strInputRFileName, const string & strInputGFileName,const string & strInputBFileName)
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
	m_nBandCouts=pDatasetB->GetRasterCount();
	GDALClose(pDatasetB);
	GDALClose(pDatasetR);
	GDALClose(pDatasetG);
	m_bInit=true;
	return true;
}
//执行
bool Compare::Execute(const string & strOutputRFileName,int T)
{
	if (strOutputRFileName.empty())
	{
		std::cout<<"输出文件名为空！";
		system("pause");
		return false;
	}
	m_strOutputRFileName=strOutputRFileName;
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
	GDALDataset *pDstDatsetR = poDriver->Create(m_strOutputRFileName.c_str(), nWidth, nHeight, nBandCount, GDT_Float32, NULL);
	if (pDstDatsetR == 0)
	{
		GDALClose(pSrcDatasetR);
		pSrcDatasetR = 0;
		GDALClose(pSrcDatasetG);
		pSrcDatasetG = 0;
		GDALClose(pSrcDatasetB);
		pSrcDatasetB = 0;
		return false;
	}
	double HRGeoTransForm[6];
	pSrcDatasetR->GetGeoTransform(HRGeoTransForm);
	const char *  pszSRS_WKT =  pSrcDatasetR->GetProjectionRef();	
	pDstDatsetR->SetGeoTransform(HRGeoTransForm);
	pDstDatsetR->SetProjection(pszSRS_WKT);
	pDstDatsetR->SetMetadata(pDstDatsetR->GetMetadata());

	if (false==compare(pSrcDatasetR,pSrcDatasetG,pSrcDatasetB,pDstDatsetR,T))
	{
		GDALClose(pSrcDatasetR);
		pSrcDatasetR = 0;
		GDALClose(pDstDatsetR);
		pDstDatsetR = 0;
		GDALClose(pSrcDatasetG);
		pSrcDatasetG = 0;
		GDALClose(pSrcDatasetB);
		pSrcDatasetB = 0;
		std::cout<<"计算Compare出错！"<<std::endl;
		system("pause");
		return false;
	}

	GDALClose(pSrcDatasetR);
	pSrcDatasetR = 0;
	GDALClose(pDstDatsetR);
	pDstDatsetR = 0;
	GDALClose(pSrcDatasetG);
	pSrcDatasetG = 0;
	GDALClose(pSrcDatasetB);
	pSrcDatasetB = 0;
	return true;
}

//重新排序RGB
bool Compare::compare(GDALDataset * pSrcDatasetR,GDALDataset * pSrcDatasetG,GDALDataset * pSrcDatasetB, GDALDataset * pDrcDatasetR,int T)
{
	int nWidth,nHeight;
	nWidth = pSrcDatasetR->GetRasterXSize();
	nHeight = pSrcDatasetR->GetRasterYSize();
	GDALRasterBand * pSrcBandR = pSrcDatasetR->GetRasterBand(1);
	GDALRasterBand * pDstBandR = pDrcDatasetR->GetRasterBand(1);
	GDALRasterBand * pSrcBandG = pSrcDatasetG->GetRasterBand(1);
	GDALRasterBand * pSrcBandB = pSrcDatasetB->GetRasterBand(1);

	//if (pSrcBandR->GetRasterDataType() != GDT_Float64)
	//{
	//	std::cout<<"格式错误"<<std::endl;
	//	return false;
	//}

	int nXBlockSize,nYBlockSize;
	int iXBlock,iYBlock;
	int iXOffset,iYOffset;
	long int nRealBlockSize = 0;
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
	Float32* pRBuf = new Float32[nBlockSize]; 
	Float32* pGBuf = new Float32[nBlockSize]; 
	Float32* pBBuf = new Float32[nBlockSize];
	Float32 *pDRBuf = new Float32[nBlockSize]; 

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
				return false;
			}
			for (int i=0;i<nBlockSize;i++)
			{
				///////////////////////////////////////////////////////////
				//-------------------------------------------------------->
				//计算值
				//-------------------------------------------------------->
				///////////////////////////////////////////////////////////
			if (pRBuf[i]>T&&pGBuf[i]>350&&pBBuf[i]<120)
				{
					pDRBuf[i]=255;
				}
				else
				{
					pDRBuf[i]=0;
				}
			}

			//写入失败
			if (pDstBandR->RasterIO(GF_Write,iXOffset,iYOffset,w,h,pDRBuf,w,h,GDT_Float64,0,0) != CE_None)
			{
				delete [] pRBuf;
				pRBuf = 0;
				delete [] pGBuf;
				pGBuf = 0;
				delete [] pBBuf;
				pBBuf = 0;
				delete [] pDRBuf;
				pDRBuf = 0;
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
	return true;
}