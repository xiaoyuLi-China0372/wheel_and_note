// MProccess.cpp : Implementation of CMProccess

#include "stdafx.h"
#include "WDMMSDK.h"
#include "MProccess.h"
#include "iphlpapi.h"
#include <afxdlgs.h>
#include<comdef.h>

#include "message_sink_.h"
#include "demux\aes_demux.h"
#include "demux\interface_demux.h"
#include "video_decoder\interface_vd.h"
#include "video_decoder\spu_buffer_t.h"
#include "audio_decoder\interface_audio_decoder.h"
#include "output\media_out.h"
#include "output\audio_buffer_.h"
#include "output\video_buffer_.h"
#include "output\vo_out_ddraw.h"//sdz added
#include "navigate\navigate_interface_t.h"
//include plugin here:
#include "input\interface_input.h"
#include "navigate\navigate_interface_t.h"
#include "global\config_values_c.h"
#include <winuser.h>
#include "./SetDlg/CVideoSetting.h"

#define ID_MSG_FULLSCREEN		WM_USER+129
#define ID_MSG_RESTORE			WM_USER+130
/////////////////////////////////////////////////////////////////////////////
//add by huangyt
static long SetDNParam(CString strIP,CString strProper,CString strUrl)
{

	unsigned long nDataLen;
	unsigned char cmd[2000];
	unsigned long len=0;
	unsigned long sendlen = 0;
	int sockid;
	struct sockaddr_in sockaddr;

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(strIP);
	sockaddr.sin_port = htons((unsigned short)6180);
	
	sockid = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if ( sockid ==INVALID_SOCKET ) 
	{
		return -1;
	}
	
	connect(sockid,(struct sockaddr*)&sockaddr,sizeof(sockaddr));
	
	cmd[0] = 3 >> 8;
	cmd[1] = 3 & 0xFF;
	
	sprintf((char*)(cmd+4),"%s=%s\n",strProper,strUrl);
	nDataLen = strlen((char*)(cmd+4));
	cmd[2] = (unsigned char)(nDataLen >> 8);
	cmd[3] = (unsigned char)(nDataLen & 0xFF);
	len=nDataLen+4;
	sendlen = send(sockid,(const char*)cmd,len,0);
	if ( sendlen == len )
		_close(sockid);

	return 1;
}

// CMProccess
static long GetVariantVar(VARIANT param,long *nParam)
{
	if(param.vt == VT_I4)
		*nParam = param.lVal;
	else if(param.vt == VT_UI4)
		*nParam = param.ulVal;
	else if(param.vt == VT_INT)
		*nParam = param.intVal;
	else if(param.vt == VT_UINT)
		*nParam = param.uintVal;
	else if(param.vt == VT_BOOL)
		*nParam = param.boolVal;
	else if(param.vt == VT_I2)
		*nParam = param.iVal;
	else if(param.vt == VT_UI1)
		*nParam = param.bVal;
	else if(param.vt == VT_BSTR)
	{
//		return 0;
		CString tem = param.bstrVal;
		strcpy((char*)nParam,tem);
	}
	else
		return 0;
	return 1;
}

char *ParseReserver(char *option, char *reserver,char *ret,int f)
{
	char temp[255];
	char *begin = strstr(reserver,option);
	if(begin)
	{
		strcpy(temp,begin);
		char *end = strstr(temp,"+");
		if(end) end[0] = '\0';
		if(f == 0)
		{
			char *end = strstr(temp,":");
			if(end)	strcpy(temp,end+1);
		}
		else if(f == 1)
		{
		}
		strcpy(ret,temp);
	}
	return begin;
}

void CMProccess::init()
{
	if(m_initflag == TRUE)
		return;
	if(m_FrameNum < 1)
		m_FrameNum = 1;
	else if(m_FrameNum > 30)
		m_FrameNum = 30;
	fifo_audio = new audio_buffer_();
	((audio_buffer_*)(fifo_audio))->init(NUM_AUDIO_BUFFERS);
	fifo_video = new video_buffer_();
	((video_buffer_*)(fifo_video))->init(m_FrameNum);
	m_initflag = TRUE;
}

CMProccess::CMProccess()
{
	if (init_flag_avcodec_mutex == 0)
	{
		InitializeCriticalSection (&avcodec_mutex);
		init_flag_avcodec_mutex = 1;
	}

	m_enurl.Empty();
	m_bWindowOnly = TRUE;
	rgbBack = RGB(5,10,5);
	hBrush = CreateSolidBrush(rgbBack);
	CalcExtent(m_sizeExtent);
	m_DFVar.vt = VT_EMPTY;

	config_values_c *config_c = new config_values_c();
	config_c->get_app_path(m_ModuleName);
	config_c->SetIniFileName(m_ModuleName);
	config = config_c;

	message_input = new message_sink_(OnInput,0,int(this));
	message_demux = new message_sink_(OnDemux,0,int(this));
	message_video = new message_sink_(OnVD,0,int(this));
	message_audio = new message_sink_(OnAD,0,int(this));
	message_output = new message_sink_(OnOutput,0,int(this));
	message_navigate = new message_sink_(OnNavigate,0,int(this));

/*
	unsigned int num_of_frame = 15;
	if(num_of_frame < 1)
		num_of_frame = 1;
	else if(num_of_frame > 30)
		num_of_frame = 30;
	fifo_audio = new audio_buffer_();
	((audio_buffer_*)(fifo_audio))->init(NUM_AUDIO_BUFFERS);
	fifo_video = new video_buffer_();
	((video_buffer_*)(fifo_video))->init(num_of_frame);
*/
	fifo_audio = NULL;
	fifo_video = NULL;
	spu_buffer_t *spu_buffer = new spu_buffer_t();
//	spu_buffer->init(35);
	spu_buffer->init(4);
	this->spu_fifo = spu_buffer;

	navigate = new navigate_interface_t();
 
	current_input = new input_interface_t;
	current_aes_input = NULL; // new input_interface_t; //bzb added 2005-07-25

	current_demux = new demux_interface_t();
	current_aes_demux = NULL; //new demux_mpa_t(); //bzb added 2005-07-25

	av_output = new media_out();
	current_video_decoder = new video_decoder_interface();
	current_audio_decoder = new interface_audio_decoder();

	current_input->init(config,message_input);
	current_demux->init(config,message_demux);
	av_output->init(config,message_output);
	current_video_decoder->init(config,message_video);
	current_audio_decoder->init(config,message_audio);
	navigate->init(config,this->message_navigate);

	m_bSendFinish = FALSE;
	m_bOpen = FALSE;
	m_bReadFinish = FALSE;
	hFinish = CreateSemaphore(0,0,1,0);	
	m_hWaitForHandle = CreateSemaphore(0,0,1,0);
//	m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2,2),&wsadata);
	m_cdroom = "";
    char drive[140];
    DWORD drives;
    DWORD i;
    drives=GetLogicalDriveStrings(sizeof(drive),drive);

    for(i=0;i < drives;i+=4)
    {
		char *pd = drive + i;
		if(DRIVE_CDROM==GetDriveType(pd))
		{
			m_cdroom+=pd[0];
		}
    }
	m_str = new char[255];
	m_bUseMenu = 0;
	m_StartDrag = 0;
	m_UseDragDrop = 0;

	m_bRectangle = FALSE;
	m_Rect.SetRectEmpty();

	m_initflag = FALSE;
	m_FrameNum = 0;
	m_isFullScreen = false;
	m_pSettingDlg = NULL;
//	dlg = NULL;
}

CMProccess::~CMProccess()
{
	Disconnect();
//	AfxMessageBox("关闭");
#define DEL(p) {if(p) {delete p; p = NULL;}}
	DEL(current_aes_input); // bzb added 2005-07-25
	DEL(current_input);
//	FreeInputPlugin(long(current_input));
	DEL(navigate);
	DEL(current_aes_demux); // bzb added 2005-07-25
	DEL(current_demux);
	DEL(av_output);
	DEL(current_video_decoder);
	DEL(current_audio_decoder);
	
	DEL(message_input);
	DEL(message_demux);
	DEL(message_video);
	DEL(message_audio);
	DEL(message_output);
	DEL(spu_fifo);
	DEL(fifo_audio);
	DEL(fifo_video);
	DEL(config);			// bzb added 2005-01-21
	DEL(message_navigate);	// bzb added 2005-01-21
	
	DeleteObject(hBrush);
	CloseHandle(hFinish);
	CloseHandle(m_hWaitForHandle);
//	CloseHandle(m_hEvent);
//	CloseHandle(m_hSuccessPlay);
	delete m_str;

//	DEL(dlg);

}



STDMETHODIMP CMProccess::Connect(BSTR url, VARIANT reserver,long *ret)
{
	if(m_initflag == FALSE)
	{
		m_FrameNum = NUM_FRAME_BUFFERS;
		init();
	}
	*ret = 0;
	long out[4];
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
//	if(NotifyHandle)
//	{
//		config->register_string("REG","KEY",(char*)NotifyHandle);
//	}
	int i;
#ifdef TIME_LIMIT
	if(config->get_sys_info(SYSINFO_TIMEOUT))
	{
		AfxMessageBox("过期，请注册!");
		return 1;
	}
#endif
	/*
	BYTE bMac[6];
	BYTE bMac1[6];
	LPBYTE pMAC = (LPBYTE)config->get_sys_info(SYSINFO_MAC);
	memcpy(bMac,pMAC,6);
	memcpy(bMac1,pMAC,6);

	static DWORD mask[7] = {0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f};
	for(i = 0; i < 6; i++)
	{
		DWORD byte = bMac[i];
		bMac[i] = BYTE(((byte>>(i+1)) & mask[6-i])| ((byte & mask[i]) << (7-i)));
	}

	char key[255];
	key[0] = '\0';
	CString reg;
	int cal = 0;
	BYTE mod[6] = {0xfe,0xfc,0xf8,0x1f,0x3f,0x7f};
	for(i = 0; i < 3; i++)
		cal += bMac1[i] * mod[i];
	for(i = 3; i< 6;i++)
		cal+= bMac[i]* mod[i];
	reg.Format("%.4x-%.2x%.2x%.2x%.2x%.2x%.2x",cal& 0xFFFF,bMac[5],bMac[4],
		bMac[3],bMac[2],bMac[1],bMac[0]);
	config->get_register_string("REG","KEY",key,255);

	if(reg.CompareNoCase(key) != 0)
	{
		config->write_log(MSG_E_REG,"序列号不正确","");
		CString mac;
		mac.Format("%.2x-%.2x-%.2x-%.2x-%.2x-%.2x",bMac1[5],bMac1[4],bMac1[3],bMac1[2],bMac1[1],bMac1[0]);

		CComVariant reg_info = mac;
		Fire_OnMPMessage(MSG_E_REG,reg_info);
		return S_FALSE;
	}
	//*/
	// TODO: Add your implementation code here
//	if(m_bOpen)
//		return S_FALSE; 
	memset(m_Speed,0,sizeof(m_Speed));
	Disconnect();
	m_bManalClose = 0;
	m_url = url;
//	WaitForSingleObject(hFinish,0);


	static BYTE __default_clut[] = {
	  0x80, 0x80, 0x00,0,
	  0x80, 0x80, 0xbf,0,
	  0x80, 0x80, 0x10,0,
	  0xef, 0x6d, 0x28,0,
	  0x5a, 0xef, 0x51,0,
	  0x80, 0x80, 0xbf,0,
	  0x80, 0x80, 0x36,0,
	  0xef, 0x6d, 0x28,0,
	  0x80, 0x80, 0xbf,0,
	  0x80, 0x80, 0x51,0,
	  0x80, 0x80, 0xbf,0,
	  0x80, 0x80, 0x10,0,
	  0xef, 0x6d, 0x28,0,
	  0x80, 0x80, 0x5c,0,
	  0x80, 0x80, 0xbf,0,
	  0x80, 0x80, 0x1c,0,
	  0xef, 0x6d, 0x28,0
	};
	// TODO: Add your implementation code here
	char *urll = "disk://F:\\info_dvd";
//	url = (long)urll;
	memset(audio_lang,0,sizeof(audio_lang));
	memset(spu_lang,0,sizeof(spu_lang));
	memcpy(this->plt,__default_clut,64);
	for(i = 0; i < 16 ;i ++)
		audio_lang[i].ID = 0xFF;
	for(i = 0; i < 64 ;i ++)
		spu_lang[i].ID = 0xFF;

	int  net_stream = 1;
	char option[255];
	char url_option[255];
	char shot_path[255];
	char url_aes[255];
	char * aes = NULL, *firstURL=NULL;

	m_iVideoInputOpen = 0;
	m_iAudioInputOpen = 0;


	aes = (char *)strstr(m_url, "-audio=MP2@");
	if(aes == NULL) 
		aes = (char *)strstr(m_url, "-AUDIO=MP2@");

	if(aes)
	{
		strcpy(url_aes,aes);
		aes = strstr(url_aes, "@");
		strcpy(url_aes,aes+1);
		if(!current_aes_demux)
		{
			current_aes_demux = new demux_aes_t(); //bzb added 2005-07-25
			current_aes_demux->init(config,message_demux); //bzb added 2005-07-25
		}
		if(!current_aes_input)
		{
			current_aes_input = new input_interface_t; //bzb added 2005-07-25
			current_aes_input->init(config,message_input); //bzb added 2005-07-25
		}
	}

	shot_path[0] = '\0';
	strcpy(url_option,(char*)LPCSTR(m_url));
	firstURL = strstr(url_option, " ");
	if(firstURL && (_strnicmp (m_url, "file://",7))){
		*firstURL = '\0';
	}
	if(_strnicmp (m_url, "file://",7) && _strnicmp (m_url, "disk://",7))
		net_stream = 1;
	else
		net_stream = 0;

	GetVariantVar(reserver,(long*)option);
	if(strlen(option))
	{
		char temp_o[255];
		if(ParseReserver("MUXTYPE",option,temp_o))
		{
			strcat(url_option,"+");
			strcat(url_option,temp_o);
		}
		if(strncmp(url_option,"udp://",6) == 0)
		{
			if(ParseReserver("RECFROM",option,temp_o))
			{
				strcat(url_option,"+");
				strcat(url_option,temp_o);
			}
		}
		if(strncmp("SHOT",option,4) ==0)
		{
			if(!ParseReserver("SHOT",option,shot_path,0))
			{
				shot_path[0] ='\0';
			}
		}
	}
	m_iVideoInputOpen = current_input->open((char*)LPCTSTR(url_option));
	if(!m_iVideoInputOpen) 
	{
		//config->write_log(MSG_E_INOPEN,LPCSTR(m_url),"打开视频源失败","");
		Fire_OnMPMessage(MSG_E_INOPEN,m_DFVar);
	}
	if(aes)
	{
		m_iAudioInputOpen = current_aes_input->open((char*)LPCTSTR(url_aes));
		if(!m_iAudioInputOpen)
		{
			//config->write_log(MSG_E_INOPEN_AES,LPCSTR(m_url),"打开音频源失败","");
			Fire_OnMPMessage(MSG_E_INOPEN_AES,m_DFVar);
		}
	}
	if(!m_iVideoInputOpen && !m_iAudioInputOpen) 
	{	
		*ret = MSG_E_INOPEN;
		return S_FALSE;
	}

	av_output->open(this->m_hWnd,fifo_video,fifo_audio);
	av_output->send_command(MPLAYER_CMD_NetStream, long(net_stream), out);
	current_video_decoder->open(fifo_video,spu_fifo);
	current_audio_decoder->open(fifo_audio);
	if(DEMUX_CANNOT_HANDLE == current_demux->open(current_input,NULL,current_video_decoder,current_audio_decoder)) 
	{
//		config->write_log(MSG_E_DEMUX,"不能识别该媒体格式，可能数据错误","");
		USES_CONVERSION;
		VARIANT var;
		var.vt = VT_BSTR;
		var.bstrVal = A2W("不能识别该媒体格式，可能数据错误");
		*ret = MSG_E_DEMUX;
		Fire_OnMPMessage(MSG_E_DEMUX,var);
		return S_FALSE;
	}
	else
	{
#ifdef NV
		if(navigate->open(current_input,this->current_demux))
		{
			if(navigate->get_info(INPUT_OPTIONAL_DATA_AUDIOLANG,0,(long*)this->audio_lang))
			{
			}
			if(navigate->get_info(INPUT_OPTIONAL_DATA_SPULANG,0,(long*)this->spu_lang))
			{
			}
			if(navigate->get_info(INPUT_OPTIONAL_DATA_CLUT,0,(long*)plt))
			{
			}
		}
#endif 
		long myout;
		current_demux->send_command(MPLAYER_CMD_NAVIPLT,long(plt),&myout);
		current_demux->send_command(MPLAYER_CMD_NAVISPUINFO,long(spu_lang),&myout);
		current_demux->send_command(MPLAYER_CMD_NAVIAINFO,long(audio_lang),&myout);
		Fire_OnMPMessage(MSG_S_DEMUX,m_DFVar);
	}

	// bzb added 2005-07-25
	if(aes)
		current_aes_demux->open(current_aes_input,NULL,current_video_decoder,current_audio_decoder);

	if(shot_path[0] != '\0')
		av_output->send_command(MPLAYER_CMD_SHOT,long(shot_path),out);
	
	if(!aes)
		current_demux->start(current_video_decoder,	current_audio_decoder,0,0);
	else
		current_demux->start(current_video_decoder,NULL,0,0);

	// bzb added 2005-07-25
	if(aes)
		current_aes_demux->start(current_video_decoder,
								current_audio_decoder,
								0,0);
	
	
	m_bOpen = TRUE;
	m_bReadFinish = TRUE;
	//config->write_log(MSG_S_PLAY,"成功播放该视频源",LPCSTR(m_url),"");
	//Fire_OnMPMessage(MSG_S_PLAY,m_DFVar);
	//added by huangyt.
	::PostMessage(this->m_hWnd,10000,MSG_S_PLAY,0);
	current_demux->get_info(MPLAYER_INFO_SPEED,0,m_Speed);

	if((strstr(m_url, "file") != NULL) || (strstr(m_url, "http") != NULL && strstr(m_url, "rec") != NULL))
		m_pbenable = 1;
	else
		m_pbenable = 0;

	return S_OK;
}


int CMProccess::input_message(long id,long info)
{
	switch(id)
	{
	case MSG_S_RECEIVEDATA:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;
	case MSG_N_RECEIVEDATA:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;	
	case MSG_S_RECORD:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;
	case MSG_E_RECORD:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;
	case MSG_S_ENDRECORD:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;
	case MSG_E_ENDRECORD:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;
	case MSG_S_FINISHINPUT:
		m_bReadFinish = FALSE;
		if(m_bManalClose == 0)
		{
			::PostMessage(this->m_hWnd,10000,id,0);
		}
		break;
	default:
		break;
	}

	return 1;
}
int CMProccess::demux_message(long id,long info)
{

	switch(id)
	{
	case MSG_S_FINISH:
		av_output->close();
		if(this->m_hWnd)
		{
			//modified by huangyt.
			//SendMessage(this->m_hWnd,10000,id,info);
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_I_STREAMNAME:
		{
			long output[2];
			this->av_output->send_command(MPLAYER_CMD_STREAMNAME, info, output);

		}
		break;
	case MSG_I_USERDATA:
		{
			long output[2];
			this->av_output->send_command(MPLAYER_CMD_USERDATA, info, output);

		}
	default:
		break;
	}
	return 1;
}
int CMProccess::vd_message(long id,long info)
{
	switch(id)
	{
	case MSG_S_VDECODE:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_E_VDECODE:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_S_REGETDATA:
		::PostMessage(this->m_hWnd,10000,id,0);
		break;	
	case MSG_A_SPUCH:
		{
			LanguageInfo *pLI = (LanguageInfo*)info;
			memcpy(spu_lang+pLI->ID,pLI,sizeof(LanguageInfo));
//			this->spu_lang[pLI->ID].ID = pLI->ID;
//			this->spu_lang[pLI->ID].Canhandle = pLI->Canhandle;
//			strcpy(this->spu_lang[pLI->ID].Language,pLI->Language);
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	}
	return 1;
}

int CMProccess::ad_message(long id,long info)
{
	switch(id)
	{
	case MSG_S_ADECODE:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_E_ADECODE:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_A_AUDIOCH:
		{
			LanguageInfo *pLI = (LanguageInfo*)info;
			memcpy(audio_lang+pLI->ID,pLI,sizeof(LanguageInfo));
//			this->spu_lang[pLI->ID].ID = pLI->ID;
//			this->spu_lang[pLI->ID].Canhandle = pLI->Canhandle;
//			strcpy(this->spu_lang[pLI->ID].Language,pLI->Language);
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	}
	return 1;
}



int CMProccess::output_message(long id,long info)
{
	switch(id)
	{
	case MSG_S_PLAYVIDEO:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_N_PLAYVIDEO:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_S_FINISH:
		fifo_video->clear_mem();
		fifo_video->flush(0);
		fifo_audio->flush(0);
		spu_fifo->flush();
		//config->write_log(id,"关闭视频源","");
		if(m_bManalClose == 0)
			::PostMessage(this->m_hWnd,10000,id,info);
		ReleaseSemaphore(hFinish,1,0);		//zjx.视频显示线程结束，通知Disconnect结束
//		SetEvent(m_hEvent);
		break;
	case MSG_N_FRAME_AVSYNC:
		ReleaseSemaphore(this->m_hWaitForHandle,1,0);
		::PostMessage(this->m_hWnd,10000,id,info);
		break;
	case MSG_S_FRAME:
//		ReleaseSemaphore(m_hSuccessPlay,1,0);
		Sleep(1);
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_S_SAMPLE:
		{
			::PostMessage(this->m_hWnd,10000,id,info);
		}
		break;
	case MSG_S_SHOT:
		ReleaseSemaphore(this->m_hWaitForHandle,1,0);
		::PostMessage(this->m_hWnd,10000,id,info);
		break;
	case MSG_N_MOVE:
		::PostMessage(this->m_hWnd,10000,id,info);
		break;
	default:
		break;
	}
	return 1;
}

int CMProccess::navigate_message(long id,long info)
{
//	this->OnMsg->message_recv(id,info);
	return 1;
}
STDMETHODIMP CMProccess::Disconnect()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	m_bManalClose = 1;
	if(m_bOpen)
	{
		if (m_pSettingDlg != NULL)
		{
			m_pSettingDlg->DestroyWindow();
			delete m_pSettingDlg;
			m_pSettingDlg = NULL;
		}
		this->av_output->send_command(MPLAYER_CMD_RESUME,0,0);
		m_bOpen = 0;
		m_bReadFinish = FALSE;
		long out = 0;
		this->current_video_decoder->send_command(MPLAYER_CMD_SHOW,1,0);
		this->current_audio_decoder->send_command(MPLAYER_CMD_SHOW,1,0);
		this->current_demux->close();
		if(current_aes_demux)	
		{
			this->current_aes_demux->close();
//			delete current_aes_demux;
//			current_aes_demux = NULL;
		}
/*		if(current_aes_input)
		{
			this->current_aes_input->close();
			delete current_aes_input;
			current_aes_input = NULL;
		}
*/	//	this->current_demux->send_command(MPLAYER_CMD_STOP,0,&out);

		WaitForSingleObject(hFinish,INFINITE);

		VARIANT var;
		var.vt = VT_EMPTY;
		this->Fire_OnMPMessage(MSG_S_FINISHINPUT,var);

	}
	m_bManalClose = 0;
	return S_OK;
}

STDMETHODIMP CMProccess::GetInfo(long InfoID, VARIANT param, VARIANT *info)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here
	long buf[512];
	memset(buf,0,sizeof(buf));
	long ret = 0;
	vo_out_ddraw * pddraw=(vo_out_ddraw *)this->av_output->get_ddraw();//sdz added
	if(!pddraw)return S_FALSE;
	switch(InfoID)
	{
		case MPLAYER_VIDEO_RECT:	
			{
				GetVideoParam(param.lVal,info);
				return S_OK;
			}
	}
	switch(InfoID & MPLAYER_INFO_MASK)
	{
	case MPLAYER_INFO_SYSBASE:
		switch(InfoID)
		{
		case MPLAYER_INFO_TOTALTIME:
			info->vt = VT_I4;
			
			//info->lVal = this->current_demux->get_stream_length();
			this->current_demux->get_info(MPLAYER_INFO_TOTALTIME, 0, &ret);
			info->lVal = ret;
			break;
		case MPLAYER_INFO_TOTALFRAME:
			info->vt = VT_I4;
			this->current_demux->get_info(MPLAYER_INFO_TOTALFRAME, 0, &ret);
			info->lVal = ret;
			break;	
		case MPLAYER_INFO_DEMIMETYPE:
			{
				CComBSTR mime =current_demux->get_mimetypes();
				info->vt = VT_BSTR;
				info->bstrVal = mime.Copy();
			}
			break;
		case MPLAYER_INFO_INPROTOCAL:
			{
				this->current_input->get_info(InfoID,long(&param),(long*)buf);
				CComBSTR bstrinfo =(char*)buf;
				info->vt = VT_BSTR;
				info->bstrVal = bstrinfo.Copy();
			}
			break;
		case MPLAYER_INFO_DEPROTOCAL:
			{
				this->current_demux->get_info(InfoID,long(&param),(long*)buf);
				CComBSTR bstrinfo =(char*)buf;
				info->vt = VT_BSTR;
				info->bstrVal = bstrinfo.Copy();
			}
			break;
		case MPLAYER_INFO_VDPROTOCAL:
			{
				this->current_video_decoder->get_info(InfoID,long(&param),(long*)buf);
				CComBSTR bstrinfo =(char*)buf;
				info->vt = VT_BSTR;
				info->bstrVal = bstrinfo.Copy();
			}
			break;
		case MPLAYER_INFO_ADPROTOCAL:
			{
				this->current_audio_decoder->get_info(InfoID,long(&param),(long*)buf);
				CComBSTR bstrinfo =(char*)buf;
				info->vt = VT_BSTR;
				info->bstrVal = bstrinfo.Copy();
			}
			break;
		case MPLAYER_INFO_ALANG:
//			this->current_audio_decoder->get_info(InfoID,0,(long*)buf);
			{
				SAFEARRAYBOUND bound[1];
				SAFEARRAY *m_PSA;

				bound[0].cElements = 8 ;
				bound[0].lLbound = 0;
				m_PSA = SafeArrayCreate(VT_BSTR,1,bound);
				BSTR *pData;
				SafeArrayAccessData(m_PSA,(void HUGEP**)&pData);
				for(int i = 0; i < 8; i++)
				{
					USES_CONVERSION;
					pData[i] = A2W(audio_lang[i].Language);
				}
				info->vt = VT_ARRAY | VT_BSTR;
				info->parray = m_PSA;
			}
			break;
		case MPLAYER_INFO_SPULANG:
		//	this->current_video_decoder->get_info(InfoID,0,(long*)buf);
			{
				SAFEARRAYBOUND bound[1];
				SAFEARRAY *m_PSA;

				bound[0].cElements = 32 ;
				bound[0].lLbound = 0;
				m_PSA = SafeArrayCreate(VT_BSTR,1,bound);
				BSTR *pData;
				SafeArrayAccessData(m_PSA,(void HUGEP**)&pData);
				for(int i = 0; i < 32; i++)
				{
					USES_CONVERSION;
					pData[i] = A2W(spu_lang[i].Language);
				}
				info->vt = VT_ARRAY | VT_BSTR;
				info->parray = m_PSA;
			}
			break;
		case MPLAYER_INFO_SEEKABLE:
			{
				info->vt = VT_BOOL;
				info->boolVal =FALSE;
				if((this->current_input->get_capabilities () & INPUT_CAP_SEEKABLE) == 0)
				{
					buf[0] = 0;
					break;
				}
				current_demux->get_info(InfoID,0,(long*)buf);
				info->boolVal =BOOL( buf[0]);
			}

			break;
		default:
			break;
		}
		break;
	case MPLAYER_INFO_INBASE:
		this->current_input->get_info(InfoID,long(&param),(long*)buf);
		switch(InfoID)
		{
		case MPLAYER_INFO_PEEKADDR://bstr return Here:
			{
				CComBSTR bstrinfo =(char*)buf;
				info->vt = VT_BSTR;
				info->bstrVal = bstrinfo.Copy();
			}
			break;
		case MPLAYER_INFO_INID:
			{
				CComBSTR ident = current_input->get_identifier();
				info->vt = VT_BSTR;
				info->bstrVal = ident.Copy();
			}
			break;
		case MPLAYER_INFO_BITRATE:
			info->vt = VT_I4;
			info->lVal = buf[0];
			break;
		case MPLAYER_INFO_FILEPOS:
			{
				info->vt = VT_I4;
				info->lVal = *(long*)buf;
				break;
			}
		default:
			break;
		}
		break;
	case MPLAYER_INFO_DEBASE:
		this->current_demux->get_info(InfoID,long(&param),(long*)buf);
		switch(InfoID)
		{
		case MPLAYER_INFO_DVDFLAG://BOOL return Here:
		case MPLAYER_INFO_PAUSEFLAG:
			info->vt = VT_BOOL;
			info->boolVal = BOOL(buf[0]);
			break;
		case MPLAYER_INFO_SPEED:
			{
				int count = 0;
				while(buf[count++]);
				SAFEARRAYBOUND bound[1];
				SAFEARRAY *m_PSA;

				bound[0].cElements = count ;
				bound[0].lLbound = 0;
				m_PSA = SafeArrayCreate(VT_I4,1,bound);
				long *pData;
				SafeArrayAccessData(m_PSA,(void HUGEP**)&pData);
				memcpy(pData,buf,sizeof(long) * count);
				info->vt = VT_ARRAY | VT_I4;
				info->parray = m_PSA;
			}
			break;
		case MPLAYER_INFO_CURSPEED://long
		case MPLAYER_INFO_CUR_PROCESS://long
			info->vt = VT_I4;
			info->lVal = buf[0];
		case MPLAYER_INFO_DEID:
			{
				CComBSTR ident = current_demux->get_identifier();
				info->vt = VT_BSTR;
				info->bstrVal = ident.Copy();
			}
			break;
		default:
			break;
		}
		break;
	case MPLAYER_INFO_VIDEOBASE:
		if (InfoID == MPLAYER_INFO_PLAYING)
		{			
			info->vt = VT_I4;
			info->lVal = 0;
			this->current_input->get_info(MPLAYER_INFO_ISRECEIVE,long(&param),(long*)buf);
			info->lVal = info->lVal | (buf[0]);
			this->av_output->get_info(MPLAYER_INFO_ISPLAY,long(&param),(long*)buf);
			info->lVal = info->lVal | (buf[0]<<1);
			this->current_demux->get_info(MPLAYER_INFO_PAUSEFLAG,long(&param),(long*)buf);
			info->lVal = info->lVal | (buf[0]<<2);
			this->current_input->get_info(MPLAYER_INFO_ISRECORD,long(&param),(long*)buf);
			info->lVal = info->lVal | (buf[0]<<3);						
			this->current_demux->get_info(MPLAYER_INFO_ISMUTE,long(&param),(long*)buf);
			info->lVal = info->lVal | (buf[0]<<4);						
			return S_OK;
		}
		if(InfoID == MPLAYER_INFO_DISPLAYMODE)
		{
			info->vt = VT_I4;
			info->lVal = 0;
			this->av_output->get_info(MPLAYER_INFO_DISPLAYMODE,long(&param),(long*)buf);
			info->lVal = buf[0];			
			return S_OK;
		}
		this->current_video_decoder->get_info(InfoID,long(&param),(long*)buf);
		switch(InfoID)
		{
		case MPLAYER_INFO_CURSUB://long
			info->vt = VT_I4;
			info->lVal = buf[0];
			break;
		case MPLAYER_INFO_VDID:
			{
				CComBSTR ident = current_video_decoder->get_identifier();
				info->vt = VT_BSTR;
				info->bstrVal = ident.Copy();
			}
			break;
		default:
			break;
		}
		break;
	case MPLAYER_INFO_AUDIOBASE:
		this->current_audio_decoder->get_info(InfoID,long(&param),(long*)buf);
		switch(InfoID)
		{
		case MPLAYER_INFO_AUDIO://long
			info->vt = VT_I4;
			info->lVal = buf[0];
			break;
		case MPLAYER_INFO_ADID:
			{
				CComBSTR ident = current_audio_decoder->get_identifier();
				info->vt = VT_BSTR;
				info->bstrVal = ident.Copy();
			}
			break;
		default:
			break;
		}
		break;
	case MPLAYER_INFO_AOUTBASE:
	case MPLAYER_INFO_VOUTBASE:
	case MPLAYER_INFO_OUTBASE:
		this->av_output->get_info(InfoID,long(&param),(long*)buf);
		{
			switch(InfoID)
			{
			case MPLAYER_INFO_CURTIME:	//vt_i4 here:
			case MPLAYER_INFO_PROBEFLAG:
			case MPLAYER_INFO_ACHNUM:
			case MPLAYER_INFO_VIDEOMODE:
			case MPLAYER_INFO_INTERLACE:
			case MPLAYER_INFO_CURFRAME:
				info->vt = VT_I4;
				info->lVal = *(long*)buf;
				break;
			case MPLAYER_INFO_AVOLUME:
				info->vt = VT_UI4;
				info->ulVal = buf[0];
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	return S_OK;
}

int CMProccess::MessageEvent(long id,WPARAM wParam,LPARAM lParam)
{
#ifdef TIME_LIMIT
	if(config->get_sys_info(SYSINFO_TIMEOUT))
	{
		AfxMessageBox("过期，请注册!");
		return 1;
	}
#endif
	int send = 0;
	CComVariant var;
//	VT_EMPTY(&var);
	switch(wParam)
	{
	case MSG_S_FINISH:
		//modified by huangyt.
//		if ( !m_bSendFinish )
//		{
//			send = 1;
//		}
		send = 1;
		m_bSendFinish = FALSE;
		break;
	case MSG_E_INOPEN:
		{
			var = "打开视频源失败"; 
			send = 1;
		}
		break;
	case MSG_N_FRAME_AVSYNC:
		send = 1;
		break;
	case MSG_S_SHOT:
		send = 1;
		var = LPCTSTR(lParam);
		break;
	case MSG_S_DEMUX:
		send = 1;
		break;
	case MSG_E_DEMUX:
		send = 1;
		break;
	case MSG_S_PLAY:
		send = 1;
//		m_bSendFinish = TRUE;
		break;
	case MSG_S_VDECODE:
		send = 1;
		var = long(lParam);
		break;
	case MSG_E_VDECODE:
		send = 1;
		var = long(lParam);
		break;
	case MSG_A_SPUCH:
		{
			LanguageInfo *pLI = (LanguageInfo*)lParam;
			var = pLI->Language;
		}
		break;
	case MSG_S_FRAME:
		send = 1;
		var = long(lParam);
		break;
	case MSG_S_SAMPLE:
		send = 1;
		var = long(lParam);
		break;
	case MSG_S_ADECODE:
		send = 1;
		var = long(lParam);
		break;
	case MSG_E_ADECODE:
		send = 1;
		break;
	case MSG_A_AUDIOCH:
		{
			LanguageInfo *pLI = (LanguageInfo*)lParam;
			var = pLI->Language;
		}
		break;
	case MSG_S_RECORD:
		send = 1;
		break;
	case MSG_E_RECORD:
		send =1;
		break;
	case MSG_S_ENDRECORD:
		send = 1;
		break;
	case MSG_E_ENDRECORD:
		send = 1;
		break;
	case MSG_S_FINISHINPUT:
		send = 1;
		break;
	case MSG_N_MOVE:
		send = 1;
		var = long(lParam);
	case MSG_S_REGETDATA:
		send = 1;		
		break;	
	case MSG_S_RECEIVEDATA:
		send = 1;		
		break;	
	case MSG_N_RECEIVEDATA:
		send = 1;		
		break;	
	case MSG_S_PLAYVIDEO:
		send = 1;		
		break;	
	case MSG_N_PLAYVIDEO:
		send = 1;		
		break;	
	default:
		break;
	}
	if(send)
	{
		this->Fire_OnMPMessage(wParam,var);
	}
	return 0;
} 


STDMETHODIMP CMProccess::FireEvent(long ID, VARIANT info)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here
	Fire_OnMPMessage(ID,info);
	return S_OK;
}

int CMProccess::GetAesUrl(char *switch_url, long *nParam, int index)
{
	char * aes = NULL, *firstURL=NULL;

	aes = strstr(switch_url, "-audio=MP2@");
	if(aes == NULL) 
		aes = strstr(switch_url, "-AUDIO=MP2@");

	if(aes)
	{
		if(index == 1)
		{
			aes = strstr(switch_url, "@");
			*nParam = (long)(aes+1);
		}else if(index == 0){
			*nParam = (long)switch_url;
			firstURL = strstr(switch_url, " ");
			if(firstURL){
				*firstURL = '\0';
			}
		}
		return 1;
	}
	return 0;
}

STDMETHODIMP CMProccess::SetCallbackOwner(long id,  VARIANT *ret)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	if ( current_demux != NULL )
	{
		current_demux->Set_Owner(id);
		this->av_output->send_command(MPLAYER_CMD_SETOWNER,id,0);
	}
	else
		return S_FALSE;
	return S_OK;
}

//该函数默认的返回值位FALSE
//当这个ID能被处理或者正确处理后，则各个处理模块返回TRUE
STDMETHODIMP CMProccess::SendCommand(long nCmdID, VARIANT param,VARIANT *ret)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// TODO: Add your implementation code here
//	if(m_SettingDlg.DoModal() == IDCANCEL)
//		return S_FALSE; 
//	CAboutDlg abdlg;
//	abdlg.DoModal();
	vo_out_ddraw * pddraw=(vo_out_ddraw *)this->av_output->get_ddraw();
	if(pddraw==NULL)
		return S_FALSE;
	int is_Wait = 0;
	ret->vt = VT_BOOL;
	ret->boolVal = TRUE;
#if 0
	if(m_bOpen == 0)
	{
		switch(nCmdID)
		{
		case MPLAYER_CMD_REG:
		case MPLAYER_CMD_ENABLEMEMU:
		case MPLAYER_CMD_SETSHMODE:
		case MPLAYER_CMD_VOLUME:
			break;
		default:
			ret->boolVal = FALSE;
			return S_OK;
		}
	}
#endif
	long nParam = 0;
	long  output[64];
	memset(output,0,sizeof(output));
	output[0] = 0;
	if(param.vt == VT_I4)
		nParam = param.lVal;
	else if(param.vt == VT_UI4)
		nParam = param.ulVal;
	else if(param.vt == VT_INT)
		nParam = param.intVal;
	else if(param.vt == VT_UINT)
		nParam = param.uintVal;
	else if(param.vt == VT_BOOL)
		nParam = param.boolVal;
	else if(param.vt == VT_I2)
		nParam = param.iVal;
	else if(param.vt == VT_UI1)
		nParam = param.bVal;
	else if(param.vt == VT_BSTR)
	{
		CString tem = param.bstrVal;
		strcpy(bstr,tem);
		nParam = (long)bstr;
//		nParam = long(LPCTSTR(tem));
	}
	if(nCmdID == MPLAYER_VIDEO_RECT)
	{
		ret->vt = VT_BOOL;
		ret->boolVal = SetVideoParam(param);
		return S_OK;
	}
	if(this->m_bOpen == 0)
	{
		if(nCmdID == MPLAYER_CMD_SWITCHSRC)
		{
			CComBSTR url = bstr;
			long IsSuccess; 
			Connect(url,*ret,&IsSuccess);
			ret->vt = VT_I4;
			ret->lVal = IsSuccess;
			return S_OK;
		}
	}
	if(nCmdID == MPLAYER_CMD_DISPLAYMODE)
	{
		if (this->m_bOpen == true)
		{			
			ret->vt = VT_BOOL;
			ret->boolVal = false;		
		}
		else
		{
			this->av_output->send_command(nCmdID,nParam,output);
			ret->vt = VT_BOOL;
			ret->boolVal = !!output[0];
		}		
		return S_OK;
	}
	if(nCmdID == MPLAYER_CMD_FULLSCREEN)
	{
		if (nParam != 0)
		{			
			if (m_pSettingDlg == NULL)
			{
				AfxSetResourceHandle(gInstance);		
				
				m_pSettingDlg = new CCVideoSetting;
				m_pSettingDlg->SetAv_output(av_output);
				m_pSettingDlg->Create(MAKEINTRESOURCE(IDD_SETTING));
				int width = ::GetSystemMetrics(SM_CXSCREEN);
				int height = ::GetSystemMetrics(SM_CYSCREEN);
				m_pSettingDlg->ModifyStyle(WS_CAPTION,0,0);
				::SetWindowPos(m_pSettingDlg->m_hWnd,HWND_NOTOPMOST,0, 0, width, height, SWP_FRAMECHANGED);
			}
			m_pSettingDlg->ShowWindow(SW_SHOW);  
			m_isFullScreen = true;		
		}
		else
		{
			if (m_pSettingDlg != NULL)
			{
				m_pSettingDlg->ShowWindow(SW_HIDE);
			}
			m_isFullScreen = false;
		}
		ret->vt = VT_BOOL;
		ret->boolVal = true;
		return S_OK;
	}
	if(nCmdID == MPLAYER_CMD_HTTP)
	{
		long ret[64];
		ret[0] = 0;
#define CMD_HTTP_PAUSE	1
#define CMD_HTTP_RESUME 2
		switch (nParam)
		{
		case CMD_HTTP_PAUSE:
			
			is_Wait = this->av_output->send_command(MPLAYER_CMD_PAUSE,nParam,ret);
			if(ret[0] > 0)
			{
				this->current_demux->send_command(MPLAYER_CMD_PAUSE2, ret[0], output);
			}		
			break;
		case CMD_HTTP_RESUME:
			is_Wait = this->av_output->send_command(MPLAYER_CMD_RESUME,nParam,ret);
			if(ret[0] > 0)
			{
				this->current_demux->send_command(MPLAYER_CMD_RESUME2, ret[0], output);
			}			
			break;
		default :
			break;
		}
		if(is_Wait)
		{
			WaitForSingleObject(m_hWaitForHandle,INFINITE);
		}		
		return S_OK;
	}
	switch(nCmdID & MPLAYER_CMD_MASK)
	{
	case MPLAYER_CMD_SYSBASE:
		switch(nCmdID)
		{
		case MPLAYER_CMD_REG:
			output[0] = config->register_string("REG","KEY",bstr);
			break;
		case MPLAYER_CMD_ENABLEMENU:
			m_bUseMenu = nParam;
			break;
		case MPLAYER_CMD_INITFRAME:
			m_FrameNum = nParam;
			init();
			break;
		case MPLAYER_CMD_BKGCOLOR:
			rgbBack = nParam;
			DeleteObject(hBrush);
			hBrush = CreateSolidBrush(rgbBack);
			Invalidate();  
			UpdateWindow();
			break;
		default:
			break;
		}
		break;
	case MPLAYER_CMD_INBASE:	//input here
		switch(nCmdID)
		{
		case MPLAYER_CMD_RECORD:
			if(m_iVideoInputOpen)
			{
				is_Wait = this->current_input->send_command(nCmdID,nParam,output);
			}
			else if(m_iAudioInputOpen)
			{
				is_Wait = this->current_aes_input->send_command(nCmdID,nParam,output);
			}
			
			break;
		default:
			is_Wait = this->current_input->send_command(nCmdID,nParam,output);
			break;
		}
		break;
	case MPLAYER_CMD_DEBASE:	//demux here
		if(param.vt == VT_BSTR && nCmdID == MPLAYER_CMD_SWITCHSRC)
		{
			strcpy(tmp_bstr,bstr);
			if(GetAesUrl(tmp_bstr, &nParam, 0))
			{
				is_Wait = this->current_demux->send_command(nCmdID,nParam,output);
				if(GetAesUrl(bstr, &nParam, 1))
				{
					if(current_aes_demux && ((demux_aes_t*)current_aes_demux)->m_bFinishThread==0)
					{
						is_Wait = this->current_aes_demux->send_command(nCmdID,nParam,output);
					}
					else
					{
						CComBSTR url = bstr;
						long IsSuccess; 
						Connect(url,*ret,&IsSuccess);
						ret->vt = VT_I4;
						ret->lVal = IsSuccess;
						return S_OK;
					}
				}
				break;
			}
		}
		if(nCmdID==MPLAYER_CMD_JUMP_PROCESS || nCmdID==MPLAYER_CMD_JUMP || nCmdID == MPLAYER_CMD_JUMPFRAME || nCmdID == MPLAYER_CMD_JUMPTIME)
			is_Wait = this->av_output->send_command(MPLAYER_CMD_RESUME,nParam,output);

		if(current_aes_demux)
			is_Wait = this->current_aes_demux->send_command(nCmdID,nParam,output);
		if(	m_bOpen && (nCmdID == MPLAYER_CMD_SPEED))
			is_Wait = this->av_output->send_command(nCmdID,nParam,output);

		if(m_bOpen && (nCmdID == MPLAYER_CMD_PAUSE))
		{
			if(m_pbenable == 1)
			{	
				long ret[64];
				ret[0] = 0;
				is_Wait = this->av_output->send_command(nCmdID,nParam,ret);
				if(ret[0] > 0)
					this->current_demux->send_command(nCmdID, ret[0], output);
			}
			break;
		}
		if(m_bOpen && (nCmdID == MPLAYER_CMD_RESUME))
		{
			if(m_pbenable == 1)
			{		
				long ret[64];
				ret[0] = 0;
				is_Wait = this->av_output->send_command(nCmdID,nParam,ret);			
				if(ret[0] > 0)
					this->current_demux->send_command(nCmdID, ret[0], output);
			}
			break;
		}
		if(m_bOpen && (nCmdID == MPLAYER_CMD_NEXTFRAME))
		{
			if(m_pbenable == 1)
			{			
				long ret[64];
				ret[0] = 0;
				is_Wait = this->av_output->send_command(nCmdID,nParam,ret);		
				if(ret[0] > 0)
					this->current_demux->send_command(nCmdID, ret[0], output);
			}
			break;
		}
		if(m_bOpen && (nCmdID == MPLAYER_CMD_PREFRAME))
		{
			if(m_pbenable == 1)
			{			
				long ret[64];
				ret[0] = 0;
				is_Wait = this->av_output->send_command(nCmdID,nParam,ret);
				if(ret[0] > 0)
				{
					//FILE *flog = fopen("C:\\cmd.log", "a");
					//fprintf(flog, "CURPTS:%08u\n", ret[0]);
					//fclose(flog);
					this->current_demux->send_command(nCmdID, ret[0], output);
				}
			}
			break;
		}
		if(m_bOpen && (nCmdID == MPLAYER_CMD_JUMPFRAME || nCmdID == MPLAYER_CMD_JUMPTIME))
		{
			this->av_output->send_command(nCmdID, nParam, output);
		}

		is_Wait = this->current_demux->send_command(nCmdID,nParam,output);
		break;
	case MPLAYER_CMD_VDBASE:	//video decoder here
		is_Wait = this->current_video_decoder->send_command(nCmdID,nParam,output);
		break;
	case MPLAYER_CMD_ADBASE:	//audio decoder here
		is_Wait = this->current_audio_decoder->send_command(nCmdID,nParam,output);
		break;
	case MPLAYER_CMD_VPBASE:	//video play here
		//add by huangyt.
		if ( nCmdID == MPLAYER_CMD_SHOW )
		{	
			is_Wait = this->current_video_decoder->send_command(nCmdID,nParam,output);
			is_Wait = this->current_audio_decoder->send_command(nCmdID,nParam,output);
		}
		if ( nCmdID == MPLAYER_CMD_SHOT )
		{	
			if (m_bOpen == false)
			{
				break;
			}
		}
		is_Wait = this->av_output->send_command(nCmdID,nParam,output);
		
		break;
	case MPLAYER_CMD_APBASE:	//audio play here
		is_Wait = this->av_output->send_command(nCmdID,nParam,output);
		break;
	default:
		break;
	}
	ret->boolVal = !!output[0];
	if(is_Wait)
		WaitForSingleObject(m_hWaitForHandle,INFINITE);
	return S_OK;
}

static int GetMenuIndex(HMENU hMenu,char *str,int nc)
{
	int nItem = GetMenuItemCount(hMenu);
                                                                     
	for(int i = 0 ; i < nItem ; i++)
	{
		char name[256];
		GetMenuString(hMenu,i,name,255,MF_BYCOMMAND | MF_BYPOSITION);
		name[255] = '\0';
		if(nc == -1)
		{
			if(_stricmp(str,name) == 0)
				return i;
		}
		else
		{
			if(_strnicmp(str,name,nc-1) == 0)
				return i;
		}
	}
	return -1;
}
void CMProccess::OnPopupMenu(int x,int y)
{
	HMENU m_menu = LoadMenu(gInstance,MAKEINTRESOURCE(IDR_MAINMENU));
	HMENU popupmenu = GetSubMenu(m_menu,0);
	HMENU submenu ;
//	InsertMenu(popupmenu,0,MF_BYPOSITION | MF_POPUP,0,"dd");
	int i;
	char str[255];
	long cur_ch = 0;
	VARIANT info;
	if((i = GetMenuIndex(popupmenu,"播放",sizeof("播放"))) >= 0)
	{
		submenu = GetSubMenu(popupmenu,i);
		for(i = 0; i < m_cdroom.GetLength(); i++)
		{
			sprintf(str,"播放光盘 %c",m_cdroom[i]);
			InsertMenu(submenu,0,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING,ID_PLAY_DISK +i,str);
		}
	}

	if(this->m_bReadFinish && m_bOpen)
	{
		//加载子图象
		GetInfo(MPLAYER_INFO_DVDFLAG,info,&info);
		GetVariantVar(info,&cur_ch);
		if(cur_ch) // if is DVD 
		{
			if((i = GetMenuIndex(popupmenu,"选择字幕",sizeof("选择字幕"))) >= 0)
			{
				submenu = GetSubMenu(popupmenu,i);
//				DeleteMenu(submenu,0,MF_BYPOSITION);
			
				this->current_video_decoder->get_info(MPLAYER_INFO_CURSUB,0,&cur_ch);
				if(cur_ch <= -2)
				{
					GetMenuString(submenu,ID_STOPSPU,str,255,MF_BYCOMMAND);
					ModifyMenu(submenu,ID_STOPSPU,MF_BYCOMMAND | MF_CHECKED,ID_STOPSPU,str);
				}
				for(i = 0; i < 64 ; i++)
				{
					if(this->spu_lang[i].ID != 0xFF)
					{
						if(strcmp(spu_lang[i].Language,"") == 0)
							sprintf(spu_lang[i].Language,"字幕%d",spu_lang[i].ID+1);
						InsertMenu(submenu,-1,MF_BYCOMMAND | MF_BYPOSITION | MF_STRING,ID_SET_SPUID + spu_lang[i].ID,spu_lang[i].Language);
						if(long(spu_lang[i].ID) == cur_ch)
						{
							CheckMenuItem(submenu,ID_SET_SPUID + spu_lang[i].ID,MF_CHECKED);
						}
					}
				}
			}

			if((i = GetMenuIndex(popupmenu,"选择声音",sizeof("选择声音"))) >= 0)
			{
				submenu = GetSubMenu(popupmenu,i);
//				DeleteMenu(submenu,0,MF_BYPOSITION);
			
				this->current_audio_decoder->get_info(MPLAYER_INFO_CURAD,0,&cur_ch);
				if(cur_ch == -3)
				{
					cur_ch = -1;
					GetMenuString(submenu,ID_AUDIO_MUTE,str,255,MF_BYCOMMAND);
					ModifyMenu(submenu,ID_AUDIO_MUTE,MF_BYCOMMAND | MF_CHECKED,ID_AUDIO_MUTE,str);
				}
				for(i = 0; i < 16 ; i++)
				{
					if(this->audio_lang[i].ID != 0xFF)
					{
						InsertMenu(submenu,-1,MF_BYCOMMAND | MF_BYPOSITION | MF_STRING,ID_SET_AUDIOID + audio_lang[i].ID,audio_lang[i].Language);
						if(long(audio_lang[i].ID) == cur_ch)
						{
							CheckMenuItem(submenu,ID_SET_AUDIOID + audio_lang[i].ID,MF_CHECKED);
						}
					}
				}
			}
		}
		else
		{
			/*
			if((i = GetMenuIndex(popupmenu,"选择字幕",sizeof("选择字幕"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
			*/
			
			if((i = GetMenuIndex(popupmenu,"选择声音",sizeof("选择声音"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
			
//			if((i = GetMenuIndex(popupmenu,"视频处理",sizeof("视频处理"))) >= 0)
//			{
//				DeleteMenu(popupmenu,i,MF_BYPOSITION);
//			}	
			
		}

		
		//初始化视频模式：
		if((i = GetMenuIndex(popupmenu,"视频处理",sizeof("视频处理"))) >= 0)
		{
/*
			GetInfo(MPLAYER_INFO_INTERLACE,info,&info);
			GetVariantVar(info,&cur_ch);
			if(cur_ch <= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}

			else */
			{
				GetInfo(MPLAYER_INFO_VIDEOMODE,info,&info);
				GetVariantVar(info,&cur_ch);
				submenu = GetSubMenu(popupmenu,i);
				int menuid = 0;
				switch(cur_ch)
				{
				case 0:
					menuid = ID_VIDEO_MODE0;
					break;
				case 1:
					menuid = ID_VIDEO_MODE1;
					break;
				case 2:
					menuid = ID_VIDEO_MODE2;
					break;
				case 3:
					menuid = ID_VIDEO_MODE3;
					break;
				case 4:
					menuid = ID_VIDEO_MODE4;
					break;
				case 5:
					menuid = ID_VIDEO_MODE5;
				default:
					break;
				}
				if(menuid)
				{
					GetMenuString(submenu,menuid,str,255,MF_BYCOMMAND);					
					ModifyMenu(submenu,menuid,MF_BYCOMMAND | MF_CHECKED,menuid,str);
				}
				GetInfo(MPLAYER_INFO_PROBEFLAG,info,&info);
				GetVariantVar(info,&cur_ch);
				if(cur_ch==1)
				{
					GetMenuString(submenu,ID_ENABLE_PROBE,str,255,MF_BYCOMMAND);					
					ModifyMenu(submenu,ID_ENABLE_PROBE,MF_BYCOMMAND | MF_CHECKED,ID_DISABLE_PROBE,str);
				}
			}				
		}			
		
		
		//播放章节
		mrl_t **lplpmrl = current_input->get_dir(NULL,NULL);
		char *cur_url = current_input->get_mrl();
		if(cur_url && lplpmrl)
		{
			mrl_t *mrl = *lplpmrl;
			if((i = GetMenuIndex(popupmenu,"跳到",sizeof("跳到"))) >= 0)
			{
				mrl_t *pmrl = mrl;
				submenu = GetSubMenu(popupmenu,i);
				DeleteMenu(submenu,0,MF_BYPOSITION);
				if(pmrl->type == 0xffff)
				{
					VoidDict *pDirDict = (VoidDict*)pmrl->pVoid;
					for(i = 0; i < pDirDict->GetSize(); i++)
					{
						RecordDir *pDir = (RecordDir*)pDirDict->GetAt(i);
						char m_dir[150];
						for(int j = 0 ; j < pDir->m_FileDict.GetSize(); j++)
						{
							RecordName *pRecName = (RecordName *)pDir->m_FileDict[j];
							strcpy(m_dir,pDir->m_path);
							char *end = strstr(m_dir,"\n");
							if(end) end[0] = '/';
							strcat(m_dir,pRecName->name);
							if(strcmp(cur_url,m_dir) == 0)
								InsertMenu(submenu,-1,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING | MF_CHECKED,ID_GOTO_CHN +(i<<8 ) + j,m_dir);
							else
								InsertMenu(submenu,-1,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING ,ID_GOTO_CHN +(i<<8 ) + j,m_dir);
						}
					}
				}
				else
				{
					i = 0;
					while(pmrl->id != 0xFF)
					{
						if(strcmp(cur_url,pmrl->mrl) == 0)
							InsertMenu(submenu,-1,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING | MF_CHECKED,ID_GOTO_INDEX +i,pmrl->mrl);
						else
							InsertMenu(submenu,-1,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING,ID_GOTO_INDEX +i,pmrl->mrl);
						pmrl = &mrl[++i];
					}
				}
			}
		}
		else
		{
			if((i = GetMenuIndex(popupmenu,"跳到",sizeof("跳到"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
		}

		long isPause;
		current_demux->get_info(MPLAYER_INFO_PAUSEFLAG,0,&isPause);
		if(isPause)
		{
			GetMenuString(popupmenu,ID_PLAY_PAUSE,str,255,MF_BYCOMMAND);
			memcpy(str,"继续",sizeof("继续")-1);
			ModifyMenu(popupmenu,ID_PLAY_PAUSE,MF_BYCOMMAND,ID_PLAY_RESUME,str);
		}
		//初始化播放速度
		GetInfo(MPLAYER_INFO_SEEKABLE,m_DFVar,&info);
		if(info.boolVal == FALSE)
		{
			if((i = GetMenuIndex(popupmenu,"向前播放",sizeof("向前播放"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
			if((i = GetMenuIndex(popupmenu,"向后播放",sizeof("向后播放"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
			if((i = GetMenuIndex(popupmenu,"向后5秒",sizeof("向后5秒"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
			if((i = GetMenuIndex(popupmenu,"向前5秒",sizeof("向前5秒"))) >= 0)
			{
				DeleteMenu(popupmenu,i,MF_BYPOSITION);
			}
		}
		else
		{
			current_demux->get_info(MPLAYER_INFO_CURSPEED,0,&cur_ch);
			if((i = GetMenuIndex(popupmenu,"向前播放",sizeof("向前播放"))) >= 0)
			{
				submenu = GetSubMenu(popupmenu,i);
				DeleteMenu(submenu,0,MF_BYPOSITION);
				for(i = 0; i < 64; i++)
				{
					if(m_Speed[i] == 0)
						break;
					else if(m_Speed[i] < 0)
						continue;
					else if(m_Speed[i] < 64)
						sprintf(str,"1/%dX",64/m_Speed[i]);
					else
						sprintf(str,"%dX",m_Speed[i]/64);
					InsertMenu(submenu,0,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING,ID_SET_SPEEDX +i,str);
					if(m_Speed[i] == cur_ch)
						CheckMenuItem(submenu,ID_SET_SPEEDX +i,MF_CHECKED);
				}
			}
			if((i = GetMenuIndex(popupmenu,"向后播放",sizeof("向后播放"))) >= 0)
			{
				submenu = GetSubMenu(popupmenu,i);
				DeleteMenu(submenu,0,MF_BYPOSITION);
				for(i = 0; i < 64; i++)
				{
					if(m_Speed[i] >= 0)
						break;
					else if(abs(m_Speed[i]) < 64)
						sprintf(str,"1/%dX",64/abs(m_Speed[i]));
					else
						sprintf(str,"%dX",abs(m_Speed[i])/64);
					InsertMenu(submenu,0,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING,ID_SET_SPEEDX +i,str);
					if(m_Speed[i] == cur_ch)
						CheckMenuItem(submenu,ID_SET_SPEEDX +i,MF_CHECKED);
				}
			}
		}
		//Audio info
		current_audio_decoder->get_info(MPLAYER_INFO_CURAD,0,&cur_ch);
		if(cur_ch == -3)
		{
			GetMenuString(popupmenu,ID_AUDIO_MUTE,str,255,MF_BYCOMMAND);
			ModifyMenu(popupmenu,ID_AUDIO_MUTE,MF_BYCOMMAND | MF_CHECKED,ID_AUDIO_UNMUTE,str);
		}
		else
		{
			av_output->get_info(MPLAYER_INFO_ACHNUM,0,&cur_ch);
			if(cur_ch == 1)
			{
				GetMenuString(popupmenu,ID_AUDIO_RCH,str,255,MF_BYCOMMAND);
				ModifyMenu(popupmenu,ID_AUDIO_RCH,MF_BYCOMMAND | MF_GRAYED,ID_AUDIO_RCH,str);
				GetMenuString(popupmenu,ID_AUDIO_LCH,str,255,MF_BYCOMMAND);
				ModifyMenu(popupmenu,ID_AUDIO_LCH,MF_BYCOMMAND | MF_GRAYED,ID_AUDIO_LCH,str);
				GetMenuString(popupmenu,ID_AUDIO_STEREO,str,255,MF_BYCOMMAND);
				ModifyMenu(popupmenu,ID_AUDIO_STEREO,MF_BYCOMMAND | MF_GRAYED,ID_AUDIO_STEREO,str);
			}
			else
			{
				av_output->get_info(MPLAYER_INFO_AVOLUME,0,&cur_ch);
				if(HIWORD(cur_ch) == 0)
				{
					GetMenuString(popupmenu,ID_AUDIO_LCH,str,255,MF_BYCOMMAND);
					ModifyMenu(popupmenu,ID_AUDIO_LCH,MF_BYCOMMAND | MF_CHECKED,ID_AUDIO_LCH,str);
				}
				else if(LOWORD(cur_ch) == 0)
				{
					GetMenuString(popupmenu,ID_AUDIO_RCH,str,255,MF_BYCOMMAND);
					ModifyMenu(popupmenu,ID_AUDIO_RCH,MF_BYCOMMAND | MF_CHECKED,ID_AUDIO_RCH,str);
				}
				else
				{
					GetMenuString(popupmenu,ID_AUDIO_STEREO,str,255,MF_BYCOMMAND);
					ModifyMenu(popupmenu,ID_AUDIO_STEREO,MF_BYCOMMAND | MF_CHECKED,ID_AUDIO_STEREO,str);
				}
			}
		}
	}
	else
	{
		GetMenuString(popupmenu,ID_PLAY_PAUSE,str,255,MF_BYCOMMAND);
		ModifyMenu(popupmenu,ID_PLAY_PAUSE,MF_BYCOMMAND | MF_GRAYED,ID_PLAY_PAUSE,str);
		GetMenuString(popupmenu,ID_PLAY_STOP,str,255,MF_BYCOMMAND);
		ModifyMenu(popupmenu,ID_PLAY_STOP,MF_BYCOMMAND | MF_GRAYED,ID_PLAY_STOP,str);
		GetMenuString(popupmenu,ID_PRE_JUMP,str,255,MF_BYCOMMAND);
		ModifyMenu(popupmenu,ID_PRE_JUMP,MF_BYCOMMAND | MF_GRAYED,ID_PRE_JUMP,str);
		GetMenuString(popupmenu,ID_BACK_JUMP,str,255,MF_BYCOMMAND);
		ModifyMenu(popupmenu,ID_BACK_JUMP,MF_BYCOMMAND | MF_GRAYED,ID_BACK_JUMP,str);
		if((i = GetMenuIndex(popupmenu,"向前播放",sizeof("向前播放"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}
		if((i = GetMenuIndex(popupmenu,"向后播放",sizeof("向后播放"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}
		if((i = GetMenuIndex(popupmenu,"跳到",sizeof("跳到"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}
		if((i = GetMenuIndex(popupmenu,"选择字幕",sizeof("选择字幕"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}
		if((i = GetMenuIndex(popupmenu,"选择声音",sizeof("选择声音"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}
		if((i = GetMenuIndex(popupmenu,"视频处理",sizeof("视频处理"))) >= 0)
		{
			DeleteMenu(popupmenu,i,MF_BYPOSITION);
		}			
	}


	if((i = GetMenuIndex(popupmenu,"投放视频",sizeof("投放视频"))) >= 0)
	{
		submenu = GetSubMenu(popupmenu,i);
		DeleteMenu(submenu,0,MF_BYPOSITION);
		char Buffer[2048] = {0};
		CString strtmp;
		for ( int j = 0;j < 16;j++ )
		{
			strtmp.Format( "%d",j+1 );
			GetPrivateProfileString("PAGE1", strtmp, "", Buffer, sizeof(Buffer), "Skin\\Settings\\dlg1.ini");
			if ( strcmp( Buffer,"0" ) > 0 )
				InsertMenu(submenu,j+1,MF_BYPOSITION | MF_BYCOMMAND | MF_STRING,ID_SWITCH_MONITOR+j,Buffer);

		}

	}

	TrackPopupMenu(popupmenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		x,y,0,this->m_hWnd,NULL);
	
}

LRESULT CMProccess::OnMenu(WPARAM id, LPARAM lParam, BOOL& bHandled)
{
	CString strValue;
	//CString strSection;

	char szBuf[2048] = {0};
	int subid = 0;
	CComVariant param;
	VARIANT ret;
	ret.vt = VT_EMPTY;
	long cmdid = 0;
	long handle = 0;
	if((id & 0xf000) == 0xd000)
	{
		subid = id & 0xfff;
		cmdid = MPLAYER_CMD_CHAPTER;
		do
		{
			param = subid;
			SendCommand(cmdid,param,&ret);
//			subid++;
		}while(FALSE);
		if(ret.boolVal)
		{
//			param = subid--;
			Fire_OnMenu(MSG_N_MENUBASE | ID_GOTO_INDEX,param);
		}
	}
	else if((id & 0xf000) == 0xe000)
	{
		subid = id & 0xff;
		switch(id & 0xff00)
		{
		case ID_SET_SPEEDX:
			handle = 1;
			cmdid = MPLAYER_CMD_SPEED;
			param = m_Speed[subid];
			break;
		case ID_SET_SPUID:
			handle = 1;
			cmdid = MPLAYER_CMD_SPUCH;
			param = this->spu_lang[subid].ID;
			break;
		case ID_SET_AUDIOID:
			handle = 1;
			cmdid = MPLAYER_CMD_AUDIOCH;
			param = this->audio_lang[subid].ID;
			break;
		//add by huangyt.
		case ID_SWITCH_MONITOR:
			{
				CString str;
				BSTR changeurl;
				str.Format("%d",subid+1);
				GetPrivateProfileString("PAGE1", str, "", szBuf, sizeof(szBuf), "Skin\\ContainerRight\\panels\\Panel1\\dlg1.ini");
				//AfxMessageBox(szBuf);
				strValue = szBuf;
				strValue = strValue.Right( strValue.GetLength() - strValue.Find(":")-1 );
				//IP.
				m_dnurl = strValue.Left( strValue.Find(":") );
				//channel.
				strValue = strValue.Right( strValue.GetLength() - strValue.Find(":")-1 );
				//set DN.
				m_enurl = m_url;
				SetDNParam(m_dnurl,"dec0.stream_url",m_url);
				//disconnect
				Disconnect();
				//connect
				long IsSuccess;
				str.Empty();
				str = "http://";
				str += m_dnurl;
				str += ":6100/stream/5";
				changeurl = str.AllocSysString();
				//AfxMessageBox(str);
				Connect(changeurl,ret,&IsSuccess);
				::SysFreeString( changeurl );
				
			}
			
			break;
		case ID_PLAY_DISK:
			{
				wchar_t ss[255];
				swprintf(ss,L"disk://%c:",m_cdroom[subid]);
				long IsSuccess;
				Connect(ss,ret,&IsSuccess);
				param = ss;
				if(!IsSuccess)
					Fire_OnMenu(MSG_N_MENUBASE | ID_PLAY_DISK,param);
			}
			break;
		case ID_GOTO_INDEX:
			cmdid = MPLAYER_CMD_CHAPTER;
			do
			{
				param = subid;
				SendCommand(cmdid,param,&ret);
				subid++;
			}while(ret.boolVal == FALSE);
			if(ret.boolVal)
			{
				param = subid--;
				Fire_OnMenu(MSG_N_MENUBASE | ID_GOTO_INDEX,param);
			}
			break;
		default:
			break;
		}
	}
	else
	{
		switch(id)
		{
//		case ID_RESTORE_SIZE:
//			av_output->send_command(MPLAYER_CMD_SHOW_PARTIAL,0,0);
//			break;
		case ID_VIDEO_ZOOM:
			{
				AFX_MANAGE_STATE(AfxGetStaticModuleState());
				AfxSetResourceHandle(gInstance);
				
				long  output[64];
				memset(output,0,sizeof(output));
				output[0] = 0;

//				CCVideoSetting *dlg = new CCVideoSetting;
//				dlg->SetAv_output(av_output);
//				dlg->Create(MAKEINTRESOURCE(IDD_SETTING));
//				dlg->ShowWindow(SW_SHOW);
				CCVideoSetting dlg;
				dlg.SetAv_output(av_output);
				dlg.DoModal();
			}
			
			break;
		//talkback. add by huangyt.
		case ID_AUDIO_TALKBACK:
			{
				CString strUrl = "udp:";
				CString strIP;
				strValue.Empty();
				if ( m_enurl.GetLength() > 0 )
				{
					strValue = m_enurl;
					strValue = strValue.Right( strValue.GetLength() - strValue.Find(":")-1 );
					strIP = strValue.Left( strValue.Find(":") );
					
					strUrl += strIP;
					strUrl += ":7777";
					//set 
					SetDNParam(m_dnurl,"enc0.stream_url",strUrl);
					strUrl.Empty();
					strUrl = "udp://";
					strUrl += m_dnurl;
					strUrl += ":7777";
					strIP = strIP.Right(strIP.GetLength()-strIP.ReverseFind(47)-1);
					//AfxMessageBox(strIP);
					SetDNParam(strIP,"dec0.stream_url",strUrl);
				}
				else
				{
					//strUrl = "udp://0.0.0.0:4000";
					//SetDNParam(m_dnurl,"enc0.stream_url",strUrl);
					param=0;
					Fire_OnMenu(MSG_N_MENUBASE | ID_AUDIO_TALKBACK,param);
					Fire_OnMPMessage(MSG_N_MENUBASE | ID_AUDIO_TALKBACK,param);
				}


			}
			break;
		case ID_PLAY_URL:
			param = 0;
			Fire_OnMenu(MSG_N_MENUBASE | ID_PLAY_URL,param);
			break;
		case ID_PLAY_BROWSE:
			param = 0;
			Fire_OnMenu(MSG_N_MENUBASE | ID_PLAY_BROWSE,param);
			break;
		case ID_PLAY_PAUSE:
			handle = 1;
			cmdid = MPLAYER_CMD_PAUSE;
			param = 0;
			break;
		case ID_PLAY_RESUME:
			handle = 1;
			cmdid = MPLAYER_CMD_RESUME;
			param = 0;
			break;
		case ID_NORMALSCREEN:
			av_output->send_command(MPLAYER_CMD_SHOW_PARTIAL,0,0);
			::PostMessage( GetParent(), ID_MSG_RESTORE,0,0);
			param = 0;
			Fire_OnMenu(MSG_N_MENUBASE | ID_NORMALSCREEN,param);
			break;
		case ID_FULLSCREEN:
			//SendMessage(0,0,0);
			::PostMessage( GetParent(), ID_MSG_FULLSCREEN,0,0);
			param = 0;
			Fire_OnMenu(MSG_N_MENUBASE | ID_FULLSCREEN,param);
			break;
		case ID_PLAY_STOP:
			Disconnect();
			param = 0;
			Fire_OnMenu(MSG_N_MENUBASE | ID_PLAY_STOP,param);
			break;
		case ID_PRE_JUMP:
			GetInfo(MPLAYER_INFO_CURTIME,ret,&ret);
			GetVariantVar(ret,&handle);
			GetInfo(MPLAYER_INFO_TOTALTIME,ret,&ret);
			GetVariantVar(ret,&cmdid);
			param = min(handle + 5,cmdid);
			cmdid = MPLAYER_CMD_JUMP;
			handle = 1;
			break;
		case ID_BACK_JUMP:
			GetInfo(MPLAYER_INFO_CURTIME,ret,&ret);
			GetVariantVar(ret,&handle);
			param = max(handle - 5,0);
			cmdid = MPLAYER_CMD_JUMP;
			handle = 1;
			break;
		case ID_AUDIO_MUTE:
			cmdid = MPLAYER_CMD_MUTE;
			param = 1;
			handle = 1;
			break;
		case ID_AUDIO_UNMUTE:
			cmdid = MPLAYER_CMD_MUTE;
			param = 0;
			handle = 1;
			break;
		case ID_AUDIO_STEREO:
			this->current_audio_decoder->get_info(MPLAYER_INFO_CURAD,0,&cmdid);
			if(cmdid == -3)
			{
				param = 0;
				SendCommand(MPLAYER_CMD_MUTE,param,&ret);
			}
			av_output->get_info(MPLAYER_INFO_AVOLUME,0,&handle);
			handle = max(handle >>16,handle & 0xFFFF);
			param = (handle <<16) | handle;
			cmdid = MPLAYER_CMD_VOLUME;
			handle = 1;
			break;
		case ID_AUDIO_RCH:
			this->current_audio_decoder->get_info(MPLAYER_INFO_CURAD,0,&cmdid);
			if(cmdid == -3)
			{
				param = 0;
				SendCommand(MPLAYER_CMD_MUTE,param,&ret);
			}
			av_output->get_info(MPLAYER_INFO_AVOLUME,0,&handle);
			param = max(handle >>16,handle & 0xFFFF);
			cmdid = MPLAYER_CMD_VOLUME;
			handle = 1;
			break;
		case ID_AUDIO_LCH:
			this->current_audio_decoder->get_info(MPLAYER_INFO_CURAD,0,&cmdid);
			if(cmdid == -3)
			{
				param = 0;
				SendCommand(MPLAYER_CMD_MUTE,param,&ret);
			}
			av_output->get_info(MPLAYER_INFO_AVOLUME,0,&handle);
			handle = max(handle >>16,handle & 0xFFFF);
			param = handle <<16;
			cmdid = MPLAYER_CMD_VOLUME;
			handle = 1;
			break;
		case ID_STOPSPU:
			handle = 1;
			cmdid = MPLAYER_CMD_OSD;
			param = 0;
			::InvalidateRect(this->m_hWnd,0,1);
			break;
		case ID_STARTSPU:
			handle = 1;
			cmdid = MPLAYER_CMD_OSD;
			param = 2;
			::InvalidateRect(this->m_hWnd,0,1);
			break;
		case ID_VIDEO_MODE0:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 0;
			break;
		case ID_VIDEO_MODE1:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 1;
			break;
		case ID_VIDEO_MODE2:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 2;
			break;
		case ID_VIDEO_MODE3:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 3;
			break;
		case ID_VIDEO_MODE4:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 4;
			break;
		case ID_VIDEO_MODE5:
			handle = 1;
			cmdid = MPLAYER_CMD_SETSHMODE;
			param = 5;
			break;
		case ID_DISABLE_PROBE:
			handle = 1;
			cmdid = MPLAYER_CMD_SETPROBLE;
			param = 0;
			break;
		case ID_ENABLE_PROBE:
			handle = 1;
			cmdid = MPLAYER_CMD_SETPROBLE;
			param = 1;
			break;
		default:
			break;
		}
	}
	if(handle)
	{
		SendCommand(cmdid,param,&ret);
		if(ret.boolVal)
		{
			if((id & 0xf000) == 0xe000)
			{
				id &= 0xff00;
			}
			Fire_OnMenu(MSG_N_MENUBASE | id,param);
		}
	}
	return 0;
}


STDMETHODIMP CMProccess::get_UseDragDrop(long *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	*pVal = m_UseDragDrop;

	return S_OK;
}

STDMETHODIMP CMProccess::put_UseDragDrop(long newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	m_UseDragDrop = newVal;

	return S_OK;
}
bool CMProccess::SetVideoParam(VARIANT param)
{
	vo_out_ddraw * pddraw = (vo_out_ddraw *)this->av_output->get_ddraw();
	if (pddraw == NULL)
	{
		return false;
	}
	if(param.vt == (VT_ARRAY|VT_I4))
	{
		long * pValue=NULL;
		SAFEARRAY* pArray=param.parray;
		SafeArrayAccessData(pArray,(void**)&pValue);
		long low(0),high(0);
		SafeArrayGetLBound(pArray,1,&low);
		SafeArrayGetUBound(pArray,1,&high);
		if(high-low+1 < 4)
		{
			return false;
		}
		
		if(pValue[low]<0||pValue[low]>=10000)				
		{
			return false;
		}	
		if(pValue[low+1]<0||pValue[low+1]>=10000)
		{
			return false;
		}
		if(pValue[low+2]<=0||pValue[low+2]>10000)				
		{
			return false;
		}
		if(pValue[low+3]<=0||pValue[low+3]>10000)				
		{
			return false;
		}
		if(pValue[low]+pValue[low+2] > 10000)
			return false;			
		if(pValue[low+1]+pValue[low+3] > 10000)
			return false;
		pddraw->m_MiniInfo.m_magRect.left=pValue[low];	
		pddraw->m_MiniInfo.m_magRect.top=pValue[low+1];
		pddraw->m_MiniInfo.m_magRect.right=pddraw->m_MiniInfo.m_magRect.left+pValue[low+2];
		pddraw->m_MiniInfo.m_magRect.bottom=pddraw->m_MiniInfo.m_magRect.top+pValue[low+3];
	}
	else if(param.vt == (VT_BOOL))
	{
		if (param.boolVal == true)
		{
			pddraw->m_MiniInfo.isMagRectUse = true;
		} 
		else
		{
			pddraw->m_MiniInfo.isMagRectUse = false;
		}
	}	
	return true;
}
bool CMProccess::GetVideoParam(long param, VARIANT * info)
{
	info->vt = VT_BOOL;
	info->boolVal = false;

	if (info == NULL)
	{
		return false;
	}
	
	vo_out_ddraw * pddraw=(vo_out_ddraw *)this->av_output->get_ddraw();
	if(pddraw == NULL)
	{
		return false;
	}
	
	SAFEARRAYBOUND bound[1];
	SAFEARRAY *m_PSA;

	bound[0].cElements = 5;
	bound[0].lLbound = 0;
	m_PSA = SafeArrayCreate(VT_I4,1,bound);
	long *pData;
	SafeArrayAccessData(m_PSA,(void HUGEP**)&pData);
	for(int i = 0; i < 5; i++)
	{
		switch (i)
		{
		case 0:			
			pData[i] = pddraw->m_MiniInfo.isMagRectUse;
			break;
		case 1:
			pData[i] = pddraw->m_MiniInfo.m_magRect.left;			
			break;
		case 2:
			pData[i] = pddraw->m_MiniInfo.m_magRect.top;
			break;
		case 3:
			pData[i] = pddraw->m_MiniInfo.m_magRect.Width();
			break;
		case 4:
			pData[i] = pddraw->m_MiniInfo.m_magRect.Height();
			break;
		default:
			break;
		}
	}
	info->vt = VT_ARRAY | VT_I4;
	info->parray = m_PSA;
	return true;
}
DLGPROC CMProccess::GetDialogProc()
{
	// TODO: 在此添加专用代码和/或调用基类
	dropTarget.Create(this->m_hWnd, MK_LBUTTON);	// bzb added 2005-05-23 
	return __super::GetDialogProc();
}
