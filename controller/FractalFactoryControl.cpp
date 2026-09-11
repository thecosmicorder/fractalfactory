#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include "FractalFactoryShared.h"

#pragma comment(lib,"Comctl32.lib")

static FractalFactorySharedState* gState=nullptr;
static HANDLE gMap=nullptr;
static std::array<HWND,FRACTAL_FACTORY_PARAM_COUNT> gSliders{};
static std::array<HWND,FRACTAL_FACTORY_PARAM_COUNT> gValues{};
static HWND gStatus=nullptr;

static const wchar_t* kNames[FRACTAL_FACTORY_PARAM_COUNT]={
 L"Fractal Type",L"Zoom",L"Pan X",L"Pan Y",L"Rotation",L"Iterations",L"Power",L"Julia X",L"Julia Y",
 L"Chaos",L"Chaos Rate",L"Symmetry",L"Warp",L"Hue",L"Saturation",L"Brightness",L"Contrast",L"Seed",L"Preset"
};

static float rnd(std::uint32_t& s){ s=s*1664525u+1013904223u; return float((s>>8)&0x00FFFFFFu)/float(0x01000000u); }

static void ApplyPreset(int preset){
 if(!gState) return; preset=std::clamp(preset,0,255); std::uint32_t s=0x9E3779B9u^(std::uint32_t(preset+1)*2654435761u); int family=preset/16;
 auto& p=gState->values;
 p[0]=float(family)/15.0f; p[1]=0.25f+0.48f*rnd(s); p[2]=0.35f+0.30f*rnd(s); p[3]=0.35f+0.30f*rnd(s); p[4]=rnd(s);
 p[5]=0.24f+0.70f*rnd(s); p[6]=0.10f+0.80f*rnd(s); p[7]=rnd(s); p[8]=rnd(s); p[9]=(family>=10?0.25f:0.05f)+0.62f*rnd(s);
 p[10]=0.08f+0.72f*rnd(s); p[11]=(family>=7&&family<=9)?0.25f+0.55f*rnd(s):0.35f*rnd(s); p[12]=0.08f+0.62f*rnd(s);
 p[13]=rnd(s); p[14]=0.55f+0.45f*rnd(s); p[15]=0.45f+0.45f*rnd(s); p[16]=0.35f+0.45f*rnd(s); p[17]=rnd(s); p[18]=float(preset)/255.0f;
 ++gState->sequence;
}

static void Defaults(){
 if(!gState) return; float d[FRACTAL_FACTORY_PARAM_COUNT]={0,0.35f,0.5f,0.5f,0.5f,0.55f,0.5f,0.5f,0.25f,0.35f,0.5f,0.2f,0.6f,0.6f,0.8f,0.5f,0.5f,0.1234f,0};
 for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++) gState->values[i]=d[i]; ++gState->sequence;
}

static void RefreshUI(){
 if(!gState) return;
 for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++){
  int maxv=(i==18)?255:1000; int pos=(i==18)?int(gState->values[i]*255.0f+0.5f):int(gState->values[i]*1000.0f+0.5f);
  SendMessageW(gSliders[i],TBM_SETPOS,TRUE,pos);
  wchar_t b[64]; if(i==0) swprintf_s(b,L"%d / 15",int(gState->values[i]*15.0f+0.5f)); else if(i==18) swprintf_s(b,L"%d / 255",pos); else swprintf_s(b,L"%.3f",gState->values[i]);
  SetWindowTextW(gValues[i],b); (void)maxv;
 }
}

static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
 switch(m){
  case WM_CREATE:{
   HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
   for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++){
    int col=i/10,row=i%10,x=20+col*460,y=22+row*58;
    HWND lab=CreateWindowW(L"STATIC",kNames[i],WS_CHILD|WS_VISIBLE,x,y,120,20,h,nullptr,nullptr,nullptr);
    SendMessage(lab,WM_SETFONT,(WPARAM)font,TRUE);
    gSliders[i]=CreateWindowExW(0,TRACKBAR_CLASSW,L"",WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_NOTICKS,x+125,y-4,250,30,h,(HMENU)(100+i),nullptr,nullptr);
    SendMessage(gSliders[i],TBM_SETRANGE,TRUE,MAKELONG(0,i==18?255:1000));
    gValues[i]=CreateWindowW(L"STATIC",L"",WS_CHILD|WS_VISIBLE|SS_RIGHT,x+380,y,65,20,h,nullptr,nullptr,nullptr);
    SendMessage(gValues[i],WM_SETFONT,(WPARAM)font,TRUE);
   }
   HWND b1=CreateWindowW(L"BUTTON",L"Reset",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,20,615,120,32,h,(HMENU)500,nullptr,nullptr);
   HWND b2=CreateWindowW(L"BUTTON",L"Random Preset",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,155,615,150,32,h,(HMENU)501,nullptr,nullptr);
   HWND title=CreateWindowW(L"STATIC",L"DETACHABLE CONTROL PANEL  |  changes are sent live to Fractal Factory in Resolume",WS_CHILD|WS_VISIBLE,330,621,560,22,h,nullptr,nullptr,nullptr);
   gStatus=title; SendMessage(b1,WM_SETFONT,(WPARAM)font,TRUE); SendMessage(b2,WM_SETFONT,(WPARAM)font,TRUE); SendMessage(title,WM_SETFONT,(WPARAM)font,TRUE);
   RefreshUI(); return 0;
  }
  case WM_HSCROLL:{
   HWND src=(HWND)l; for(int i=0;i<FRACTAL_FACTORY_PARAM_COUNT;i++) if(src==gSliders[i]){
    int pos=(int)SendMessage(src,TBM_GETPOS,0,0); gState->values[i]=(i==18)?float(pos)/255.0f:float(pos)/1000.0f;
    if(i==18) ApplyPreset(pos); else ++gState->sequence; RefreshUI(); break;
   } return 0;
  }
  case WM_COMMAND:{
   if(LOWORD(w)==500){ Defaults(); RefreshUI(); return 0; }
   if(LOWORD(w)==501){ static std::uint32_t r=0xC0FFEEu; r=r*1664525u+1013904223u; ApplyPreset(int((r>>16)&255)); RefreshUI(); return 0; }
   break;
  }
  case WM_DESTROY: PostQuitMessage(0); return 0;
 }
 return DefWindowProcW(h,m,w,l);
}

int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,LPWSTR,int show){
 INITCOMMONCONTROLSEX ic{sizeof(ic),ICC_BAR_CLASSES}; InitCommonControlsEx(&ic);
 gMap=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(FractalFactorySharedState),FRACTAL_FACTORY_MAPPING);
 if(!gMap){ MessageBoxW(nullptr,L"Could not create Fractal Factory shared control state.",L"Fractal Factory",MB_ICONERROR); return 1; }
 gState=(FractalFactorySharedState*)MapViewOfFile(gMap,FILE_MAP_ALL_ACCESS,0,0,sizeof(FractalFactorySharedState));
 if(!gState){ CloseHandle(gMap); return 1; }
 if(gState->magic!=FRACTAL_FACTORY_MAGIC){ gState->magic=FRACTAL_FACTORY_MAGIC; gState->version=1; gState->sequence=0; Defaults(); }
 WNDCLASSEXW wc{sizeof(wc)}; wc.lpfnWndProc=WndProc; wc.hInstance=hi; wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1); wc.lpszClassName=L"FractalFactoryControlWindow"; wc.hIcon=LoadIcon(nullptr,IDI_APPLICATION); wc.hIconSm=wc.hIcon;
 RegisterClassExW(&wc);
 HWND h=CreateWindowExW(WS_EX_APPWINDOW,wc.lpszClassName,L"Fractal Factory Control",WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,950,710,nullptr,nullptr,hi,nullptr);
 if(!h) return 1; ShowWindow(h,show); UpdateWindow(h);
 MSG msg{}; while(GetMessageW(&msg,nullptr,0,0)>0){ TranslateMessage(&msg); DispatchMessageW(&msg); }
 UnmapViewOfFile(gState); CloseHandle(gMap); return (int)msg.wParam;
}
