

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
#include "output\vo_out_ddraw.h"
#include "navigate\navigate_interface_t.h"

#include "input\interface_input.h"
#include "navigate\navigate_interface_t.h"
#include "global\config_values_c.h"
#include <winuser.h>
#include "./SetDlg/CVideoSetting.h"

#define ID_MSG_FULLSCREEN		WM_USER+129
#define ID_MSG_RESTORE			WM_USER+130


static long SetDNParam(CString strIP,CString strProper,CString strUrl)
{}


static long GetVariantVar(VARIANT param,long *nParam)
{}

char *ParseReserver(char *option, char *reserver,char *ret,int f)
{}

void CMProccess::init()
{}

CMProccess::CMProccess()
{}

CMProccess::~CMProccess()
{}



STDMETHODIMP CMProccess::Connect(BSTR url, VARIANT reserver,long *ret)
{}


int CMProccess::input_message(long id,long info)
{}
int CMProccess::demux_message(long id,long info)
{}
int CMProccess::vd_message(long id,long info)
{}

int CMProccess::ad_message(long id,long info)
{}



int CMProccess::output_message(long id,long info)
{}

int CMProccess::navigate_message(long id,long info)
{}
STDMETHODIMP CMProccess::Disconnect()
{}

STDMETHODIMP CMProccess::GetInfo(long InfoID, VARIANT param, VARIANT *info)
{}

int CMProccess::MessageEvent(long id,WPARAM wParam,LPARAM lParam)
{} 


STDMETHODIMP CMProccess::FireEvent(long ID, VARIANT info)
{}

int CMProccess::GetAesUrl(char *switch_url, long *nParam, int index)
{}

STDMETHODIMP CMProccess::SetCallbackOwner(long id,  VARIANT *ret)
{}



STDMETHODIMP CMProccess::SendCommand(long nCmdID, VARIANT param,VARIANT *ret)
{}

static int GetMenuIndex(HMENU hMenu,char *str,int nc)
{}
void CMProccess::OnPopupMenu(int x,int y)
{}

LRESULT CMProccess::OnMenu(WPARAM id, LPARAM lParam, BOOL& bHandled)
{}


STDMETHODIMP CMProccess::get_UseDragDrop(long *pVal)
{}

STDMETHODIMP CMProccess::put_UseDragDrop(long newVal)
{}
bool CMProccess::SetVideoParam(VARIANT param)
{}
bool CMProccess::GetVideoParam(long param, VARIANT * info)
{}
DLGPROC CMProccess::GetDialogProc()
{}
