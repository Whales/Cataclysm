#if (defined _WIN32 || defined WINDOWS)  // This is a Windows-only file!

#include "catacurse.h"
#include <cstdlib>

//***********************************
//Globals                           *
//***********************************

const WCHAR *szWindowClass = (L"CataCurseWindow");    //Class name :D
HINSTANCE WindowINST;   //the instance of the window
HWND WindowHandle;      //the handle of the window
HDC WindowDC;           //Device Context of the window, used for backbuffer
int WindowX;            //X pos of the actual window, not the curses window
int WindowY;            //Y pos of the actual window, not the curses window
int WindowWidth;        //Width of the actual window, not the curses window
int WindowHeight;       //Height of the actual window, not the curses window
int lastchar;          //the last character that was pressed, resets in getch
int inputdelay;         //How long getch will wait for a character to be typed
WINDOW *_windows;  //Probably need to change this to dynamic at some point
int WindowCount;        //The number of curses windows currently in use
HDC backbuffer;         //an off-screen DC to prevent flickering, lower cpu
HBITMAP backbit;        //the bitmap that is used in conjunction wth the above
int fontwidth;          //the width of the font, background is always this size
int fontheight;         //the height of the font, background is always this size
int halfwidth;          //half of the font width, used for centering lines
int halfheight;          //half of the font height, used for centering lines
HFONT font;             //Handle to the font created by CreateFont
RGBQUAD *windowsPalette;  //The coor palette, 16 colors emulates a terminal
pairs *colorpairs;   //storage for pair'ed colored, should be dynamic, meh
unsigned char *dcbits;  //the bits of the screen image, for direct access
char szDirectory[MAX_PATH] = "";

//***********************************
//Non-curses, Window functions      *
//***********************************

//Registers, creates, and shows the Window!!
bool WinCreate(bool initgl)
{
    int success;
    WNDCLASSEXW WindowClassType;
    int WinBorderHeight;
    int WinBorderWidth;
    int WinTitleSize;
    unsigned int WindowStyle;
    const WCHAR *szTitle=  (L"Cataclysm");
    WinTitleSize = GetSystemMetrics(SM_CYCAPTION);      //These lines ensure
    WinBorderWidth = GetSystemMetrics(SM_CXDLGFRAME) * 2;  //that our window will
    WinBorderHeight = GetSystemMetrics(SM_CYDLGFRAME) * 2; // be a perfect size
    WindowClassType.cbSize = sizeof(WNDCLASSEXW);
    WindowClassType.style = 0;//No point in having a custom style, no mouse, etc
    WindowClassType.lpfnWndProc = ProcessMessages;//the procedure that gets msgs
    WindowClassType.cbClsExtra = 0;
    WindowClassType.cbWndExtra = 0;
    WindowClassType.hInstance = WindowINST;// hInstance
    WindowClassType.hIcon = LoadIcon(WindowINST, IDI_APPLICATION);//Default Icon
    WindowClassType.hIconSm = LoadIcon(WindowINST, IDI_APPLICATION);//Default Icon
    WindowClassType.hCursor = LoadCursor(NULL, IDC_ARROW);//Default Pointer
    WindowClassType.lpszMenuName = NULL;
    WindowClassType.hbrBackground = 0;//Thanks jday! Remove background brush
    WindowClassType.lpszClassName = szWindowClass;
    success = RegisterClassExW(&WindowClassType);
    if ( success== 0){
        return false;
    }
    WindowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME) & ~(WS_MAXIMIZEBOX);
    WindowHandle = CreateWindowExW(WS_EX_APPWINDOW || WS_EX_TOPMOST * 0,
                                   szWindowClass , szTitle,WindowStyle, WindowX,
                                   WindowY, WindowWidth + (WinBorderWidth) * 1,
                                   WindowHeight + (WinBorderHeight +
                                   WinTitleSize) * 1, 0, 0, WindowINST, NULL);
    if (WindowHandle == 0){
        return false;
    }
    ShowWindow(WindowHandle,5);
    return true;
};

//Unregisters, releases the DC if needed, and destroys the window.
void WinDestroy()
{
    if ((WindowDC > 0) && (ReleaseDC(WindowHandle, WindowDC) == 0)){
        WindowDC = 0;
    }
    if ((!WindowHandle == 0) && (!(DestroyWindow(WindowHandle)))){
        WindowHandle = 0;
    }
    if (!(UnregisterClassW(szWindowClass, WindowINST))){
        WindowINST = 0;
    }
};

//This function processes any Windows messages we get. Keyboard, OnClose, etc
LRESULT CALLBACK ProcessMessages(HWND__ *hWnd,unsigned int Msg,
                                 WPARAM wParam, LPARAM lParam)
{
    switch (Msg){
        case WM_CHAR:               //This handles most key presses
            lastchar=(int)wParam;
            switch (lastchar){
                case 13:            //Reroute ENTER key for compatilbity purposes
                    lastchar=10;
                    break;
                case 8:             //Reroute BACKSPACE key for compatilbity purposes
                    lastchar=127;
                    break;
            };
            break;
        case WM_KEYDOWN:                //Here we handle non-character input
            switch (wParam){
                case VK_LEFT:
                    lastchar = 52;
                    break;
                case VK_RIGHT:
                    lastchar = 54;
                    break;
                case VK_UP:
                    lastchar = 56;
                    break;
                case VK_DOWN:
                    lastchar = 50;
                    break;
                default:
                    break;
            };
        case WM_ERASEBKGND:
            return 1;               //We don't want to erase our background
        case WM_PAINT:              //Pull from our backbuffer, onto the screen
            BitBlt(WindowDC, 0, 0, WindowWidth, WindowHeight, backbuffer, 0, 0,SRCCOPY);
            ValidateRect(WindowHandle,NULL);
            break;
        case WM_DESTROY:
            exit(0);//A messy exit, but easy way to escape game loop
        default://If we didnt process a message, return the default value for it
            return DefWindowProcW(hWnd, Msg, wParam, lParam);
    };
    return 0;
};

//The following 3 methods use mem functions for fast drawing
inline void VertLineDIB(int x, int y, int y2,int thickness, unsigned char color)
{
    int j;
    for (j=y; j<y2; j++)
        memset(&dcbits[x+j*WindowWidth],color,thickness);
};
inline void HorzLineDIB(int x, int y, int x2,int thickness, unsigned char color)
{
    int j;
    for (j=y; j<y+thickness; j++)
        memset(&dcbits[x+j*WindowWidth],color,x2-x);
};
inline void FillRectDIB(int x, int y, int width, int height, unsigned char color)
{
    int j;
    for (j=y; j<y+height; j++)
        memset(&dcbits[x+j*WindowWidth],color,width);
};

void DrawWindow(WINDOW *win)
{
    int i,j,drawx,drawy;
    char tmp;
    for (j=0; j<win->height; j++){
        if (win->line[j].touched)
            for (i=0; i<win->width; i++){
                win->line[j].touched=false;
                drawx=((win->x+i)*fontwidth);
                drawy=((win->y+j)*fontheight);//-j;
                if (((drawx+fontwidth)<=WindowWidth) && ((drawy+fontheight)<=WindowHeight)){
                tmp = win->line[j].chars[i];
                int FG = win->line[j].FG[i];
                int BG = win->line[j].BG[i];
                FillRectDIB(drawx,drawy,fontwidth,fontheight,BG);

                if ( tmp > 0){
                if (tmp==95){
                        HorzLineDIB(drawx,drawy+fontheight-2,drawx+fontwidth,1,FG);
                    } else {
                    int color = RGB(windowsPalette[FG].rgbRed,windowsPalette[FG].rgbGreen,windowsPalette[FG].rgbBlue);
                    SetTextColor(backbuffer,color);
                    ExtTextOut(backbuffer,drawx,drawy,0,NULL,&tmp,1,NULL);
                    }
                } else if (  tmp < 0 ) {
                    switch (tmp) {
                    case -60://box bottom/top side (horizontal line)
                        HorzLineDIB(drawx,drawy+halfheight,drawx+fontwidth,1,FG);
                        break;
                    case -77://box left/right side (vertical line)
                        VertLineDIB(drawx+halfwidth,drawy,drawy+fontheight,2,FG);
                        break;
                    case -38://box top left
                        HorzLineDIB(drawx+halfwidth,drawy+halfheight,drawx+fontwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy+halfheight,drawy+fontheight,2,FG);
                        break;
                    case -65://box top right
                        HorzLineDIB(drawx,drawy+halfheight,drawx+halfwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy+halfheight,drawy+fontheight,2,FG);
                        break;
                    case -39://box bottom right
                        HorzLineDIB(drawx,drawy+halfheight,drawx+halfwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy,drawy+halfheight+1,2,FG);
                        break;
                    case -64://box bottom left
                        HorzLineDIB(drawx+halfwidth,drawy+halfheight,drawx+fontwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy,drawy+halfheight+1,2,FG);
                        break;
                    case -63://box bottom north T (left, right, up)
                        HorzLineDIB(drawx,drawy+halfheight,drawx+fontwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy,drawy+halfheight,2,FG);
                        break;
                    case -61://box bottom east T (up, right, down)
                        VertLineDIB(drawx+halfwidth,drawy,drawy+fontheight,2,FG);
                        HorzLineDIB(drawx+halfwidth,drawy+halfheight,drawx+fontwidth,1,FG);
                        break;
                    case -62://box bottom south T (left, right, down)
                        HorzLineDIB(drawx,drawy+halfheight,drawx+fontwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy+halfheight,drawy+fontheight,2,FG);
                        break;
                    case -59://box X (left down up right)
                        HorzLineDIB(drawx,drawy+halfheight,drawx+fontwidth,1,FG);
                        VertLineDIB(drawx+halfwidth,drawy,drawy+fontheight,2,FG);
                        break;
                    case -76://box bottom east T (left, down, up)
                        VertLineDIB(drawx+halfwidth,drawy,drawy+fontheight,2,FG);
                        HorzLineDIB(drawx,drawy+halfheight,drawx+halfwidth,1,FG);
                        break;
                    default:
                        // SetTextColor(DC,_windows[w].line[j].chars[i].color.FG);
                        // TextOut(DC,drawx,drawy,&tmp,1);
                        break;
                    }
                    };//switch (tmp)
                }//(tmp < 0)
            };//for (i=0;i<_windows[w].width;i++)
    };// for (j=0;j<_windows[w].height;j++)
    win->draw=false;                //We drew the window, mark it as so
};

//Check for any window messages (keypress, paint, mousemove, etc)
void CheckMessages()
{
    MSG msg;
    while (PeekMessage(&msg, 0 , 0, 0, PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
};

//***********************************
//Psuedo-Curses Functions           *
//***********************************

//Basic Init, create the font, backbuffer, etc
WINDOW *initscr(void)
{

    GetCurrentDirectory(sizeof(szDirectory) - 1, szDirectory);
    _windows = new WINDOW[20];         //initialize all of our variables
    BITMAPINFO bmi;
    lastchar=-1;
    inputdelay=-1;
    fontheight=16;
    fontwidth=8;
    halfwidth=fontwidth / 2;
    halfheight=fontheight / 2;
    WindowWidth=80*fontwidth;
    WindowHeight=25*fontheight;
    WindowX=(GetSystemMetrics(SM_CXSCREEN) / 2)-WindowWidth;    //center this
    WindowY=(GetSystemMetrics(SM_CYSCREEN) / 2)-WindowHeight;   //sucker
    WinCreate(TRUE);    //Create the actual window, register it, etc
    CheckMessages();    //Let the message queue handle setting up the window
    WindowDC = GetDC(WindowHandle);
    backbuffer = CreateCompatibleDC(WindowDC);
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WindowWidth;
    bmi.bmiHeader.biHeight = -WindowHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount=8;
    bmi.bmiHeader.biCompression = BI_RGB;   //store it in uncompressed bytes
    bmi.bmiHeader.biSizeImage = WindowWidth * WindowHeight * 1;
    bmi.bmiHeader.biClrUsed=16;         //the number of colors in our palette
    bmi.bmiHeader.biClrImportant=16;    //the number of colors in our palette
    backbit = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&dcbits, NULL, 0);
    DeleteObject(SelectObject(backbuffer, backbit));//load the buffer into DC

   // char *filedir;
    //filedir = new char[strlen(szDirectory)+strlen("\\data\\termfont.ttf")];
    //sprintf(filedir,"%s%s",szDirectory,"\data\\termfont.ttf");
    int nResults = AddFontResourceExA("data\\termfont.fon",FR_PRIVATE,NULL);
    //if (nResults==0)    //Try to load a .ttf instead!
    //    int nResults = AddFontResourceExA("data\\termfont.ttf",FR_PRIVATE,NULL);

   // delete filedir;
   if (nResults>0){
    font = CreateFont(fontheight, fontwidth, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                      PROOF_QUALITY, FF_MODERN, "Terminus");   //Create our font
   } else{
       font = CreateFont(fontheight, fontwidth, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                      PROOF_QUALITY, FF_MODERN, "FixedSys");   //Create our font
   }
                     // PROOF_QUALITY, FF_MODERN, "FixedSys");   //Create our font
    //FixedSys will be user-changable at some point in time??
    SetBkMode(backbuffer, TRANSPARENT);//Transparent font backgrounds
    SelectObject(backbuffer, font);//Load our font into the DC
    WindowCount=0;
    return newwin(25,80,0,0);   //create the 'stdscr' window and return its ref
};

WINDOW *newwin(int nlines, int ncols, int begin_y, int begin_x)
{
    int i,j;
    WINDOW *newwindow;
    newwindow=&_windows[WindowCount];
    _windows[WindowCount].x=begin_x;
    _windows[WindowCount].y=begin_y;
    _windows[WindowCount].width=ncols;
    _windows[WindowCount].height=nlines;
    _windows[WindowCount].inuse=true;
    _windows[WindowCount].draw=false;
    _windows[WindowCount].BG=0;
    _windows[WindowCount].FG=8;
    _windows[WindowCount].cursorx=0;
    _windows[WindowCount].cursory=0;
     _windows[WindowCount].line = new curseline[nlines];

    for (j=0; j<nlines; j++)
    {
         _windows[WindowCount].line[j].chars= new char[ncols];
        _windows[WindowCount].line[j].FG= new char[ncols];
        _windows[WindowCount].line[j].BG= new char[ncols];
        _windows[WindowCount].line[j].touched=true;//Touch them all !?
        for (i=0; i<ncols; i++)
        {
          _windows[WindowCount].line[j].chars[i]=0;
          _windows[WindowCount].line[j].FG[i]=0;
          _windows[WindowCount].line[j].BG[i]=0;
        }
    }
    WindowCount++;
    return newwindow;
};


//Deletes the window and marks it as free. Clears it just in case.
int delwin(WINDOW *win)
{
    int i;
    int j;
    //int w=FindWin(win);
    win->inuse=false;
    win->draw=false;
    for (j=0; j<win->height; j++){
        delete win->line[j].chars;
        delete win->line[j].FG;
        delete win->line[j].BG;
        }
    delete win->line;
    //DrawWindow(0);
    WindowCount--;
    return 1;
};
inline void newline(WINDOW *win){
win->cursory++;
win->cursorx=0;
};

inline int addedchar(WINDOW *win){
win->cursorx++;
win->line[win->cursory].touched=true;
if (win->cursorx > win->width)
newline(win);
};


//Borders the window with fancy lines!
int wborder(WINDOW *win, chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr, chtype bl, chtype br)
{

    int i, j;
//    int w=FindWin(win);
    int oldx=win->cursorx;//methods below move the cursor, save the value!
    int oldy=win->cursory;//methods below move the cursor, save the value!
    if (ls>0)
        for (j=1; j<win->height-1; j++)
            mvwaddch(win, j, 0, 179);
    if (rs>0)
        for (j=1; j<win->height-1; j++)
            mvwaddch(win, j, win->width-1, 179);
    if (ts>0)
        for (i=1; i<win->width-1; i++)
            mvwaddch(win, 0, i, 196);
    if (bs>0)
        for (i=1; i<win->width-1; i++)
            mvwaddch(win, win->height-1, i, 196);
    if (tl>0)
        mvwaddch(win,0, 0, 218);
    if (tr>0)
        mvwaddch(win,0, win->width-1, 191);
    if (bl>0)
        mvwaddch(win,win->height-1, 0, 192);
    if (br>0)
        mvwaddch(win,win->height-1, win->width-1, 217);
    //_windows[w].cursorx=oldx;//methods above move the cursor, put it back
    //_windows[w].cursory=oldy;//methods above move the cursor, put it back
    wmove(win,oldy,oldx);
    return 1;
};

//Refreshes a window, causing it to redraw on top.
int wrefresh(WINDOW *win)
{
    if (win==0) win=&_windows[0];
    if (win->draw)
        DrawWindow(win);
    return 1;
};

//Refreshes window 0 (stdscr), causing it to redraw on top.
int refresh(void)
{
    return wrefresh(&_windows[0]);
};

//Not terribly sure how this function is suppose to work,
//but jday helped to figure most of it out
int getch(void)
{
    refresh();

InvalidateRect(WindowHandle,NULL,true);
    lastchar=ERR;//ERR=-1
    if (inputdelay < 0)
    {
        do
        {
            CheckMessages();
            if (lastchar!=ERR) break;
            MsgWaitForMultipleObjects(0, NULL, FALSE, 50, QS_ALLEVENTS);//low cpu wait!
        }
        while (lastchar==ERR);
    }
    else if (inputdelay > 0)
    {
        unsigned long starttime=GetTickCount();
        unsigned long endtime;
        do
        {
            CheckMessages();        //MsgWaitForMultipleObjects won't work very good here
            endtime=GetTickCount(); //it responds to mouse movement, and WM_PAINT, not good
            if (lastchar!=ERR) break;
            Sleep(2);
        }
        while (endtime<(starttime+inputdelay));
    }
    else
    {
        CheckMessages();
    };
    Sleep(25);
    return lastchar;
};

//The core printing function, prints characters to the array, and sets colors
inline int printstring(WINDOW *win, char *fmt)
{
    char *buf;
    buf = strtok (fmt,"\n");
    while (buf != NULL)
    {
        int size = strlen(buf);

        int j;

        for (j=0; j<size; j++){
        win->line[win->cursory].chars[win->cursorx]=buf[j];
        win->line[win->cursory].FG[win->cursorx]=win->FG;
        win->line[win->cursory].BG[win->cursorx]=win->BG;
        win->line[win->cursory].touched=true;
        addedchar(win);
        }
        buf = strtok (NULL, "\n");
        if (buf != NULL) //{
            newline(win);

       // } else
        //_windows[w].cursorx+=size;
    }
    win->draw=true;
    return 1;
}

//Prints a formatted string to a window at the current cursor, base function
int wprintw(WINDOW *win, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char printbuf[2048];
    vsnprintf(printbuf, 2047, fmt, args);
    va_end(args);
    return printstring(win,printbuf);
};

//Prints a formatted string to a window, moves the cursor
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char printbuf[2048];
    vsnprintf(printbuf, 2047, fmt, args);
    va_end(args);
    if (wmove(win,y,x)==0) return 0;
    return printstring(win,printbuf);
};

//Prints a formatted string to window 0 (stdscr), moves the cursor
int mvprintw(int y, int x, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char printbuf[2048];
    vsnprintf(printbuf, 2047, fmt, args);
    va_end(args);
    if (move(y,x)==0) return 0;
    return printstring(&_windows[0],printbuf);
};

//Prints a formatted string to window 0 (stdscr) at the current cursor
int printw(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char printbuf[2078];
    vsnprintf(printbuf, 2047, fmt, args);
    va_end(args);
    return printstring(&_windows[0],printbuf);
};

//erases a window of all text and attributes
int werase(WINDOW *win)
{
    int j,i;
    for (j=0; j<win->height; j++)
    {
     for (i=0; i<win->width; i++)   {
     win->line[j].chars[i]=0;
     win->line[j].FG[i]=0;
     win->line[j].BG[i]=0;
     }
        win->line[j].touched=true;
    }
    win->draw=true;
    wmove(win,0,0);
    wrefresh(win);
    return 1;
};

//erases window 0 (stdscr) of all text and attributes
int erase(void)
{
    return werase(&_windows[0]);
};

//pairs up a foreground and background color and puts it into the array of pairs
int init_pair(short pair, short f, short b)
{
    colorpairs[pair].FG=f;
    colorpairs[pair].BG=b;
    return 1;
};

//moves the cursor in a window
int wmove(WINDOW *win, int y, int x)
{
    if (x>=win->width)
     {return 0;}//FIXES MAP CRASH -> >= vs > only
    if (y>=win->height)
     {return 0;}// > crashes?
    if (y<0)
     {return 0;}
    if (x<0)
     {return 0;}
    win->cursorx=x;
    win->cursory=y;
    return 1;
};

//Clears windows 0 (stdscr)     I'm not sure if its suppose to do this?
int clear(void)
{
    return wclear(&_windows[0]);
};

//Ends the terminal, destroy everything
int endwin(void)
{
    DeleteObject(font);
    WinDestroy();
    delete _windows;
    BOOL b = RemoveFontResourceExA(
    "\\data\\termfont.fon", // name of font file
    FR_PRIVATE,   		// font characteristics
    NULL         		// Reserved.
    );
    return 1;
};

//adds a character to the window
int mvwaddch(WINDOW *win, int y, int x, const chtype ch)
{
   if (wmove(win,y,x)==0) return 0;
   return waddch(win, ch);
};

//clears a window
int wclear(WINDOW *win)
{
    werase(win);
    wrefresh(win);
    return 1;
};

//gets the max x of a window (the width)
int getmaxx(WINDOW *win)
{
    if (win==0) return _windows[0].width;     //StdScr
    return win->width;
};

//gets the max y of a window (the height)
int getmaxy(WINDOW *win)
{
    if (win==0) return _windows[0].height;     //StdScr
    return win->height;
};

inline RGBQUAD BGR(int b, int g, int r)
{
    RGBQUAD result;
    result.rgbBlue=b;    //Blue
    result.rgbGreen=g;    //Green
    result.rgbRed=r;    //Red
    result.rgbReserved=0;//The Alpha, isnt used, so just set it to 0
    return result;
};

int start_color(void)
{
 colorpairs=new pairs[50];
 windowsPalette=new RGBQUAD[16];     //Colors in the struct are BGR!! not RGB!!
 windowsPalette[0]= BGR(0,0,0);
 windowsPalette[1]= BGR(0, 0, 196);
 windowsPalette[2]= BGR(0,196,0);
 windowsPalette[3]= BGR(30,180,196);
 windowsPalette[4]= BGR(196, 0, 0);
 windowsPalette[5]= BGR(180, 0, 196);
 windowsPalette[6]= BGR(200, 170, 0);
 windowsPalette[7]= BGR(196, 196, 196);
 windowsPalette[8]= BGR(96, 96, 96);
 windowsPalette[9]= BGR(64, 64, 255);
 windowsPalette[10]= BGR(0, 240, 0);
 windowsPalette[11]= BGR(0, 255, 255);
 windowsPalette[12]= BGR(255, 20, 20);
 windowsPalette[13]= BGR(240, 0, 255);
 windowsPalette[14]= BGR(255, 240, 0);
 windowsPalette[15]= BGR(255, 255, 255);
 return SetDIBColorTable(backbuffer, 0, 16, windowsPalette);
};

int keypad(WINDOW *faux, bool bf)
{
return 1;
};

int noecho(void)
{
    return 1;
};
int cbreak(void)
{
    return 1;
};
int keypad(int faux, bool bf)
{
    return 1;
};
int curs_set(int visibility)
{
    return 1;
};

int mvaddch(int y, int x, const chtype ch)
{
    return mvwaddch(&_windows[0],y,x,ch);
};

int wattron(WINDOW *win, int attrs)
{
    bool isBold = !!(attrs & A_BOLD);
    bool isBlink = !!(attrs & A_BLINK);
    int pairNumber = (attrs & A_COLOR) >> 17;
    win->FG=colorpairs[pairNumber].FG;
    win->BG=colorpairs[pairNumber].BG;
    if (isBold) win->FG += 8;
    if (isBlink) win->BG += 8;
    return 1;
};
int wattroff(WINDOW *win, int attrs)
{
     win->FG=8;                                  //reset to white
     win->BG=0;                                  //reset to black
    return 1;
};
int attron(int attrs)
{
    return wattron(&_windows[0],attrs);
};
int attroff(int attrs)
{
    return wattroff(&_windows[0],attrs);
};
int waddch(WINDOW *win, const chtype ch)
{
    char charcode;
    charcode=ch;

    switch (ch){        //LINE_NESW  - X for on, O for off
        case 4194424:   //#define LINE_XOXO 4194424
            charcode=179;
            break;
        case 4194417:   //#define LINE_OXOX 4194417
            charcode=196;
            break;
        case 4194413:   //#define LINE_XXOO 4194413
            charcode=192;
            break;
        case 4194412:   //#define LINE_OXXO 4194412
            charcode=218;
            break;
        case 4194411:   //#define LINE_OOXX 4194411
            charcode=191;
            break;
        case 4194410:   //#define LINE_XOOX 4194410
            charcode=217;
            break;
        case 4194422:   //#define LINE_XXOX 4194422
            charcode=193;
            break;
        case 4194420:   //#define LINE_XXXO 4194420
            charcode=195;
            break;
        case 4194421:   //#define LINE_XOXX 4194421
            charcode=180;
            break;
        case 4194423:   //#define LINE_OXXX 4194423
            charcode=194;
            break;
        case 4194414:   //#define LINE_XXXX 4194414
            charcode=197;
            break;
        default:
            charcode = (char)ch;
            break;
        }


int curx=win->cursorx;
int cury=win->cursory;

//if (win2 > -1){
   win->line[cury].chars[curx]=charcode;
   win->line[cury].FG[curx]=win->FG;
   win->line[cury].BG[curx]=win->BG;


    win->draw=true;
    addedchar(win);
    return 1;
  //  else{
  //  win2=win2+1;

};




//Move the cursor of windows 0 (stdscr)
int move(int y, int x)
{
    return wmove(&_windows[0],y,x);
};

//Set the amount of time getch waits for input
void timeout(int delay)
{
    inputdelay=delay;
};

#endif
