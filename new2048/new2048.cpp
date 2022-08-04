// new2048.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "new2048.h"
//test
//记录
#define MAX_LOADSTRING 100
#define MAX_CRedo 5//最大的单次可撤销次数
#define MAX_ScorePlayer 10//最多显示前多少名玩家
#define MAX_PlayerName 10//最长输入的玩家名
//动画（未完成）
#define Refresh_ms 20//多少毫秒刷新一次
#define MoveSpeed_ms 100//移动砖块动画速度用时，请勿大于0xffff
#define NewSpeed_ms 100//生成砖块动画速度用时，请勿大于0xffff
//界面
#define MainRCc 500//默认砖块主格子大小（包括xy）
#define ScoreRCcy 60//默认分数栏格子高
#define MenuRCcy 40//默认菜单栏格子高
#define MAX_MenuButton 4//菜单按钮数量
#define NULLSpace 20//每个格子间隙
#define NULLMainSpace 10//每个小方块之间间隙
#define NULLScoreSpace 50//当前分数与最高分之间间隙
#define NULLMenuSpace 10//每个菜单之间间隙
#define AllRCcx (MainRCc + 2 * NULLSpace)//总窗口RECT
#define AllRCcy (MainRCc + ScoreRCcy + MenuRCcy + 4 * NULLSpace)//总
//数字定义
#define Num2 1
#define Num4 2
#define Num8 3
#define Num16 4
#define Num32 5
#define Num64 6
#define Num128 7
#define Num256 8
#define Num512 9
#define Num1024 10
#define Num2048 11
#define Num4096 12
#define Num8192 13
#define Num16384 14
#define Num32768 15
#define Num65536 16
#define Num131072 17
//运动动画状态
#define NewNum 1
#define MoveNum 2
#define AddNum 3
//方向定义
#define FaceNULL 0
#define RIGHT 1
#define LEFT 2
#define DOWN 4
#define UP 8
//计时器
#define EscSecTimer 0

// 全局变量:

struct tagNum
{
    RECT rc;
    int size = 0;
}Num[4][4];//数组

int Score = 0, MaxScore = 0;//分数
int Redo = 0;//撤销和回档次数
int EscSec = 0;//用时
bool Fail = false;
//颜色
const COLORREF Color_AllBackground = RGB(235, 230, 220);//全部背景颜色
const COLORREF Color_Background = RGB(165, 160, 150);//框框背景颜色
const COLORREF Color_PenW = RGB(245, 245, 220);//其他信息颜色1（白）
const COLORREF Color_PenB = RGB(80, 65, 55);//其他信息颜色2（黑）
const COLORREF Color_NumL8Pen = RGB(60, 50, 45);//小于8的数字颜色
const COLORREF Color_NumB8or8Pen = RGB(240, 240, 240);//大于等于8的数字颜色
const COLORREF Color_Num[18]{//数字背景颜色
    RGB(165,160,150),//NULL
    RGB(240,230,220),//2
    RGB(235,225,200),//4
    RGB(240,140,90),//8
    RGB(240,110,70),//16
    RGB(240,100,65),//32
    RGB(250,90,60),//64
    RGB(240,195,130),//128
    RGB(240,210,120),//256
    RGB(250,235,110),//512
    RGB(160,180,255),//1024
    RGB(255,180,170),//2048
    RGB(80,80,70),//4096
    RGB(70,70,60),//8192
    RGB(55,55,48),//16384
    RGB(40,40,34),//32768
    RGB(24,24,20),//65536
    RGB(0,0,0),//131072
};

FILE* pf_Score, * pf_Redo;
RECT RC, MainRC, ScoreRC, MenuRC;//全部的大小与放砖块的主格子的大小等
RECT  StandardSizeRC; double RelSize = 1;//缩放窗口后的大小相对于标准大小的值，用于改变窗口后仍能适应大小
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
bool JudgeNumMove(short MoveFace, tagNum* lpFirstNum = &Num[0][0], int* lpScore = &Score);//将数字块朝指定方向移动，移动成功返回true
bool isFail(tagNum* lpFirstNum = &Num[0][0]);//是否无法移动（失败）
void MakeNewGame(tagNum* lpFirstNum = &Num[0][0]);//开始新的一局
void MakeRandNum(tagNum* lpFirstNum = &Num[0][0]);//制造一个新的数字（2或4）
void UpdateRC();//更新所有RC

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // TODO: 在此处放置代码。
    srand(unsigned(GetTickCount64()));
    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NEW2048, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NEW2048));
    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NEW2048));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)CreateSolidBrush(Color_AllBackground);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NEW2048);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//以下为消息处理
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        break;
    case WM_CREATE:
        {
        //获得系统参数
        int cxScreen = GetSystemMetrics(SM_CXSCREEN);
        int cyScreen = GetSystemMetrics(SM_CYSCREEN);
        int cyCaption = GetSystemMetrics(SM_CYCAPTION);
        int cxSizeFrame = GetSystemMetrics(SM_CXSIZEFRAME);
        int cySizeFrame = GetSystemMetrics(SM_CYSIZEFRAME);
        RECT WinRC;
        WinRC.left = (cxScreen - AllRCcx) / 2;
        WinRC.top = (cyScreen - AllRCcy) / 2;
        WinRC.right = AllRCcx + 2 * cxSizeFrame + 9;//因为未知原因导致实际窗口大小小了7像素，所以加了这两个七
        WinRC.bottom = AllRCcy + cyCaption + 2 * cySizeFrame + 9;
        SetWindowPos(hWnd, NULL, WinRC.left, WinRC.top, WinRC.right, WinRC.bottom, SWP_NOOWNERZORDER);
        SetRect(&StandardSizeRC, 0, 0, AllRCcx, AllRCcy);
        UpdateRC();
        InvalidateRect(hWnd, &RC, FALSE);
        MakeNewGame();
        SetTimer(hWnd, EscSecTimer, 1000, NULL);
        /*
        Num[0][0].size = 17;
        Num[0][1].size = 16;
        Num[0][2].size = 15;
        Num[0][3].size = 14;
        Num[1][0].size = 13;
        Num[1][1].size = 12;
        Num[1][2].size = 11;
        Num[1][3].size = 10;
        Num[2][0].size = 9;
        Num[2][1].size = 8;
        Num[2][2].size = 7;
        Num[2][3].size = 6;
        Num[3][0].size = 5;
        Num[3][1].size = 4;
        Num[3][2].size = 3;
        Num[3][3].size = 2;
        */
        }
        break;
    case WM_SIZE:
    {
        //GetClientRect(hWnd, &RC);
        RC.right = LOWORD(lParam);
        RC.bottom = HIWORD(lParam);

        if (StandardSizeRC.right > 0 && StandardSizeRC.bottom > 0) {//防止除数小于0
            double StandardRight = StandardSizeRC.right, StandardButtom = StandardSizeRC.bottom;
            RelSize = min(RC.right / StandardRight, RC.bottom / StandardButtom);
        }
        UpdateRC();
    }
        //InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_KEYFIRST:
        switch (wParam)
        {
        case 'A':
        case VK_LEFT:
        case 'W':
        case VK_UP:
        case 'D':
        case VK_RIGHT:
        case 'S':
        case VK_DOWN:
        if (JudgeNumMove(wParam))
            MakeRandNum();
        if (isFail())
            Fail = true;
            break;
        default:
            break;
        }
        InvalidateRect(hWnd, &RC, TRUE);
        break;
    case WM_PAINT:
    {
#define My_RoundRect(ahdc,aRC,width,height) RoundRect(ahdc, aRC.left, aRC.top, aRC.right, aRC.bottom, width * RelSize, height * RelSize)//更方便地绘制，RelSize在更改窗口大小后才会变
        HDC hdc;
        PAINTSTRUCT ps;
        HBRUSH hBrush;
        HFONT hFont;
        HPEN hPen;
        TCHAR XName[100] = {};
        TCHAR XNum[13] = {};
        hdc = BeginPaint(hWnd, &ps);
        hPen = CreatePen(PS_NULL, 0, 0);
        SelectObject(hdc, hPen);
        //背景
        SetBkMode(hdc, TRANSPARENT);

        hBrush = CreateSolidBrush(Color_Background);
        SelectObject(hdc, hBrush);
        My_RoundRect(hdc, ScoreRC, 50, 50);
        My_RoundRect(hdc, MainRC, 50, 50);
        My_RoundRect(hdc, MenuRC, 50, 50);
        //分数
        if (Score > MaxScore)
            MaxScore = Score;
        swprintf(XName, _T(" %ds  分数：%d  最高分：%d"), EscSec, Score, MaxScore);
        hFont = CreateFontA(RelSize * ScoreRCcy, RelSize * 370 / wcslen(XName), 0, 0, RelSize * 700, 0, 0, 0, 0, 0, 0, 0, 0, (LPCSTR)XNum);
        SelectObject(hdc, hFont);
        SetTextColor(hdc, Color_PenB);
        DrawText(hdc, XName, -1, &ScoreRC, NULL);
        //砖块
        int NumRCc = (MainRCc - 5 * NULLMainSpace) / 4;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (Num[y][x].size != NULL) {
                    swprintf(XNum, _T("%d\0"), 1 << Num[y][x].size);
                    int Long = wcslen(XNum) + 1;
                    Num[y][x].rc.left = ((x + 1) * NULLMainSpace + x * NumRCc + NULLSpace) * RelSize;
                    Num[y][x].rc.top = ((y + 1) * NULLMainSpace + y * NumRCc + ScoreRCcy + NULLSpace * 2) * RelSize;
                    Num[y][x].rc.right = ((x + 1) * (NULLMainSpace + NumRCc) + NULLSpace) * RelSize;
                    Num[y][x].rc.bottom = ((y + 1) * (NULLMainSpace + NumRCc) + ScoreRCcy + NULLSpace * 2) * RelSize;
                    //背景
                    hBrush = CreateSolidBrush(Color_Num[Num[y][x].size]);
                    SelectObject(hdc, hBrush);
                    RoundRect(hdc, Num[y][x].rc.left, Num[y][x].rc.top, Num[y][x].rc.right, Num[y][x].rc.bottom, 20 * RelSize, 20 * RelSize);
                    //字体
                    hFont = CreateFontA((NumRCc / Long + NULLMainSpace * 3) * RelSize, NumRCc / Long * RelSize, 0, 0, RelSize * 700, 0, 0, 0, 0, 0, 0, 0, 0, (LPCSTR)XNum);
                    SelectObject(hdc, hFont);
                    if (Num[y][x].size < 3)
                        SetTextColor(hdc, Color_NumL8Pen);
                    else
                        SetTextColor(hdc, Color_NumB8or8Pen);
                    DrawText(hdc, (LPCWSTR)XNum, -1, &Num[y][x].rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                }
            }
        }
        DeleteObject(hFont);
        DeleteObject(hBrush);
        DeleteObject(hPen);
        EndPaint(hWnd, &ps);
    }
    if (Fail) {
        KillTimer(hWnd, EscSecTimer);
        MessageBox(hWnd, _T("是否重新开始？"), _T("你死了"), MB_OK);
        MakeNewGame();
        Score = 0;
        EscSec = 0;
        SetTimer(hWnd, EscSecTimer, 1000, NULL);
        if (!isFail())
            Fail = false;
        InvalidateRect(hWnd, &RC, TRUE);
        }
        break;
    case WM_TIMER:
        switch (wParam)
        {
        case EscSecTimer:
            EscSec++;
            InvalidateRect(hWnd, &RC, TRUE);
            break;
        default:
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//以下为函数
bool JudgeNumMove(short MoveFace, tagNum* lpFirstNum, int* lpScore) {
    tagNum* pNumCur = lpFirstNum, * pNum2 = pNumCur;
    bool isMove = false;
#define NAN 999
    BOOL NumCurDoNotIn_x = NAN, NumCurDoNotIn_y = NAN, pNum2add = NAN, ListStart = RIGHT, xyAdd = 1;
    switch (MoveFace)
    {
    case RIGHT:
    case VK_RIGHT:
    case 'D':
        pNum2add = 1;//在pNumCur右边
        NumCurDoNotIn_x = 3;
        ListStart = LEFT;
        break;
    case LEFT:
    case VK_LEFT:
    case 'A':
        pNum2add = -1;//在pNumCur左边
        NumCurDoNotIn_x = 0;
        ListStart = RIGHT;
        break;
    case DOWN:
    case VK_DOWN:
    case 'S':
        pNum2add = 4;//在pNumCur下边
        NumCurDoNotIn_y = 3;
        ListStart = LEFT;
        break;
    case UP:
    case VK_UP:
    case 'W':
    default:
        pNum2add = -4;//在pNumCur上边
        NumCurDoNotIn_y = 0;
        ListStart = RIGHT;
        break;
    }
    int x = 0; int y = 0;
    for (short times = 1; times <= 6; times++) {
        if (ListStart == LEFT) {
            pNumCur = lpFirstNum + 15; pNum2 = pNumCur + pNum2add;
        }
        else {
            pNumCur = lpFirstNum; pNum2 = pNumCur + pNum2add;
        }
        for (int ft = 0; ft < 16; ft++) {
            if (ListStart == LEFT) {
                x = 3 - ft % 4;//3210
                y = 3 - (ft - ft % 4) / 4;//3210
            }
            else {
                x = ft % 4;//0123
                y = (ft - ft % 4) / 4;//0123
            }

            if (x != NumCurDoNotIn_x && y != NumCurDoNotIn_y) {//保证不越界，比如Num2在NumCur左边，则当NumCur的x==0时，不与Num2比较，也就是不与x==-1的比较
                switch (times)
                {
                case 4:
                    if (pNumCur->size != NULL && pNumCur->size == pNum2->size) {//可相加
                        pNum2->size = pNumCur->size + 1;
                        (*lpScore) += 1 << pNum2->size;
                        pNumCur->size = NULL;
                        isMove = true;
                    }
                    break;
                default:
                    if (pNumCur->size != NULL && pNum2->size == NULL) {//可移动
                        pNum2->size = pNumCur->size;
                        pNumCur->size = NULL;
                        isMove = true;
                    }
                    break;
                }
            }
            if (ListStart == LEFT) {
                pNumCur--; pNum2--;
            }
            else {
                pNumCur++; pNum2++;
            }
        }
    }
#undef NAN
    return isMove;
}

bool isFail(tagNum* lpFirstNum) {
    tagNum* pNumCur = lpFirstNum, * pNum2 = pNumCur;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (pNumCur->size == NULL)//是否有为空的情况
                return false;
            //以下为是否有可合成情况，仅与Cur右边和下边比较即可遍历
            if (x != 3) {
                pNum2 = pNumCur + 1;//在Cur右边
                if (pNumCur->size == pNum2->size)
                    return false;
            }
            if (y != 3) {
                pNum2 = pNumCur + 4;//在Cur下边
                if (pNumCur->size == pNum2->size)
                    return false;
            }
            pNumCur++;
        }
    }
    return true;
}

void MakeNewGame(tagNum* lpFirstNum) {
    tagNum* pNum = lpFirstNum;
    for (int i = 0; i < 16; i++) {
        pNum->size = NULL;
        pNum++;
    }
    MakeRandNum(lpFirstNum);
    MakeRandNum(lpFirstNum);
}

void MakeRandNum(tagNum* lpFirstNum) {
    tagNum* pNum = lpFirstNum;
    int NULLNum[16] = {}, j = -1;
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (pNum->size == NULL) {
                NULLNum[j + 1] = x + 4 * y;
                j++;
            }
            pNum++;
        }
    }
    if (j != -1) {
        pNum = lpFirstNum + NULLNum[j ? rand() % j : 0];
        pNum->size = (rand() % 10 >= 9 ? 2 : 1);//10%概率生成4
    }
}

void UpdateRC() {
#define My_SetRect(aRC,left,top,right,buttom) (SetRect(&aRC,(left)*RelSize,(top)*RelSize,(right)*RelSize,(buttom)*RelSize))
        My_SetRect(RC,      0,          0,                                  MainRCc + 2 * NULLSpace,    AllRCcy);
        My_SetRect(ScoreRC, NULLSpace,  NULLSpace,                          MainRCc + NULLSpace,        ScoreRCcy + NULLSpace);
        My_SetRect(MainRC,  NULLSpace,  ScoreRCcy + 2 * NULLSpace,          MainRCc + NULLSpace,        MainRCc + ScoreRCcy + 2 * NULLSpace);
        My_SetRect(MenuRC,  NULLSpace,  AllRCcy - MenuRCcy - NULLSpace,     MainRCc + NULLSpace,        AllRCcy - NULLSpace);
}