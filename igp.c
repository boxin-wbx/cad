#include "graphics.h"#include "extgraph.h"#include "genlib.h"#include "simpio.h"#include "conio.h"#include <stdio.h>#include <stdlib.h>#include <stddef.h>#include <windows.h>#include <olectl.h>#include <mmsystem.h>#include <wingdi.h>#include <ole2.h>#include <ocidl.h>#include <winuser.h>#define deltax 0.03#define deltay 0.03#define TIMER_BLINK500  1     /*500ms定时器事件标志号*/#define TIMER_BLINK1000 2     /*1000ms定时器时间标志号*/const int mseconds500 = 500;const int mseconds1000 = 1000;static double ccx = 1.0, ccy = 1.0, bx, by;/*圆心坐标*/static double radius = 1.0;/*圆半径*/static char text[100];/*输入的字符串缓冲区*/static int textlen = 0;/*输入的字符串长度*/static double textx, texty;/*字符串的起始位置*/static bool isBlink = FALSE;/*是否闪烁标志*/static bool isDisplayText = FALSE;/*字符串显示标志*/static bool isDisplayCircle = TRUE;static bool isMove = FALSE;/*移动标志*/static bool isChangeSize = FALSE;/*改变大小标志*//*圆显示标志*/void DrawCenteredCircle(double x, double y, double r);/*画中心圆*//*判断点(x0,y0)是否在矩形包围盒(x1, y1) --> (x2, y2)范围内*/bool inBox(double x0, double y0, double x1, double x2, double y1, double y2);void KeyboardEventProcess(int key, int event);/*键盘消息回调函数*/void CharEventProcess(char c);/*字符消息回调函数*/void MouseEventProcess(int x, int y, int button, int event);/*鼠标消息回调函数*/void TimerEventProcess(int timerID);/*定时器消息回调函数*/double boxHeight, boxWidth;double lineAreax = 0, rectanAreax, ellipsAreax, txtAreax;void drawBaseLine();void drawBaseRectan();void drawBaseEllips();void drawBaseText();void drawBase();bool inBase(double x, double y) {    return inBox(x, y, 0, boxWidth * 4, 0, boxHeight);}bool newLine = FALSE, newRectan = FALSE, newEllips = FALSE, newTxt = FALSE;bool isLine = FALSE, isRectan = FALSE, isEllips = FALSE, isTxt = FALSE;typedef struct __Ellips {    double x, y;    double rx, ry;    int num;} Ellips;Ellips *ellips[100];int EllipsSize = 0;typedef struct __Rectan {    double x, y;    double width, height;    int num;} Rectan;Rectan *rectan[100];int RectanSize = 0;typedef struct __Line {    double x, y;    double dx, dy;    int num;} Line;Line *line[100];int LineSize = 0;typedef struct __Txt {    double x, y, pointSize;    string text;    int num;} Txt;Txt *txt[100];int TxtSize;void *pElement = 0;void drawLine(Line *tl) {    MovePen(tl->x, tl->y);    DrawLine(tl->dx, tl->dy);}void moveLine(Line *tl, double mx, double my) {    SetEraseMode(TRUE);/*擦除前一个*/    MovePen(tl->x, tl->y);    DrawLine(tl->dx, tl->dy);    tl->x = mx;    tl->y = my;    SetEraseMode(FALSE);/*画新的*/    MovePen(tl->x, tl->y);    DrawLine(tl->dx, tl->dy);}void drawRectan(Rectan *r) {    DrawBox(r->x, r->y, r->width, r->height);}void moveRectan(Rectan *r, double mx, double my) {    SetEraseMode(TRUE);/*擦除前一个*/    DrawBox(r->x, r->y, r->width, r->height);    r->x = mx;    r->y = my;    SetEraseMode(FALSE);/*画新的*/    DrawBox(r->x, r->y, r->width, r->height);}void drawEllips(Ellips *e) {    MovePen(e->x + e->rx, e->y);    DrawEllipticalArc(e->rx, e->ry, 0, 360.0);}void moveEllips(Ellips *e, double mx, double my) {    SetEraseMode(TRUE);/*擦除前一个*/    drawEllips(e);    e->x = mx;    e->y = my;    SetEraseMode(FALSE);/*画新的*/    drawEllips(e);}void drawTxt(Txt *t) {    SetPointSize(t->pointSize);    MovePen(t->x, t->y);    DrawTextString(t->text);}void moveTxt(Txt *t, double mx, double my) {    SetEraseMode(TRUE);/*擦除前一个*/    SetPointSize(t->pointSize);    MovePen(t->x, t->y);    DrawTextString(t->text);    t->x = mx;    t->y = my;    SetEraseMode(FALSE);/*画新的*/    MovePen(t->x, t->y);    DrawTextString(t->text);}bool inLine(Line *l, double mx, double my) {    if (l == 0) return FALSE;    // point is on the line    if (my == l->y && l->dy == 0) return TRUE;    if ((mx - l->x) / (my - l->y) == l->dx / l->dy) return TRUE;    return FALSE;}bool inRectan(Rectan *r, double mx, double my) {    if (r == 0) return FALSE;    //point is in the triangle;    if (my <= (r->height + r->y) && my >= r->y && mx <= (r->width + r->x) && mx >= r->x) return TRUE;    return FALSE;}bool inEllips(Ellips *e, double mx, double my) {    if (e == 0) return FALSE;    // point in the ellips    double tx = mx - e->x, ty = my - e->y;    if (tx * tx / (e->rx * e->rx) + (ty * ty) / (e->ry * e->ry) <= 1) return TRUE;    return FALSE;}bool inTxt(Txt *t, double mx, double my) {    if (t == 0) return FALSE;    // point in the text    double height = GetFontHeight();    double width = TextStringWidth(t->text);    if (inBox(mx, my, t->x, t->x + width, t->y, t->y + height)) return TRUE;    return FALSE;}void getElement(double mx, double my) {    int i;    if (pElement != 0) {        if (isEllips) {            isEllips = FALSE;            SetEraseMode(TRUE);            drawEllips(pElement);            SetEraseMode(FALSE);            SetPenColor("black");            drawEllips(pElement);            pElement = 0;        }        else if (isRectan) {            isRectan = FALSE;            SetEraseMode(TRUE);            drawRectan(pElement);            SetEraseMode(FALSE);            SetPenColor("black");            drawRectan(pElement);            pElement = 0;        }        else if (isLine) {            isLine = FALSE;            SetEraseMode(TRUE);            drawLine(pElement);            SetEraseMode(FALSE);            SetPenColor("black");            drawLine(pElement);            pElement = 0;        }        else if (isTxt) {            isTxt = FALSE;            SetEraseMode(TRUE);            drawTxt(pElement);            SetEraseMode(FALSE);            SetPenColor("black");            drawTxt(pElement);            pElement = 0;        }    }    for (i = 0; i < LineSize; i++) {        if (inLine(line[i], mx, my)) {            pElement = line[i];            isLine = TRUE;            SetEraseMode(TRUE);            drawLine(pElement);            SetEraseMode(FALSE);            SetPenColor("Red");            drawLine(pElement);            isMove = TRUE;            return;        }    }    for (i = 0; i < RectanSize; i++) {        if (inRectan(rectan[i], mx, my)) {            pElement = rectan[i];            isRectan = TRUE;            SetEraseMode(TRUE);            drawRectan(pElement);            SetEraseMode(FALSE);            SetPenColor("Red");            drawRectan(pElement);            isMove = TRUE;            return;        }    }    for (i = 0; i < EllipsSize; i++) {        if (inEllips(ellips[i], mx, my)) {            pElement = ellips[i];            isEllips = TRUE;            SetEraseMode(TRUE);            drawEllips(pElement);            SetEraseMode(FALSE);            SetPenColor("Red");            drawEllips(pElement);            isMove = TRUE;            return;        }    }    for (i = 0; i < TxtSize; i++) {        if (inTxt(txt[i], mx, my)) {            pElement = txt[i];            isTxt = TRUE;            SetEraseMode(TRUE);            drawTxt(pElement);            SetEraseMode(FALSE);            SetPenColor("red");            drawTxt(pElement);            isMove = TRUE;            return;        }    }}void changeElementSize(double dx) {    if (pElement == 0) return;    dx += 1;    if (isEllips) {        Ellips *e = pElement;        SetEraseMode(TRUE);        drawEllips(pElement);        SetEraseMode(FALSE);        e->rx *= dx;        e->ry *= dx;        SetPenColor("red");        drawEllips(pElement);    }    else if (isRectan) {        Rectan *r = pElement;        SetEraseMode(TRUE);        drawRectan(pElement);        SetEraseMode(FALSE);        r->width *= dx;        r->height *= dx;        SetPenColor("red");        drawRectan(pElement);    }    else if (isLine) {        Line *l = pElement;        SetEraseMode(TRUE);        drawLine(pElement);        SetEraseMode(FALSE);        l->dx *= dx;        l->dy *= dx;        SetPenColor("red");        drawLine(pElement);    }    else if (isTxt) {        dx -= 1;        Txt *t = pElement;        SetEraseMode(TRUE);        drawTxt(pElement);        SetEraseMode(FALSE);        dx /= 0.1;        if (t->pointSize + 10 * dx >= 0)            t->pointSize += 10 * dx;        SetPointSize(t->pointSize);        SetPenColor("red");        drawTxt(pElement);    }}void changeElementSite(double dx, double dy) {    if (pElement == 0) return;    if (isEllips) {        Ellips *e = pElement;        SetEraseMode(TRUE);        drawEllips(pElement);        SetEraseMode(FALSE);        e->x += dx;        e->y += dy;        SetPenColor("red");        drawEllips(pElement);    }    else if (isRectan) {        Rectan *r = pElement;        SetEraseMode(TRUE);        drawRectan(pElement);        SetEraseMode(FALSE);        r->x += dx;        r->y += dy;        SetPenColor("red");        drawRectan(pElement);    }    else if (isLine) {        Line *l = pElement;        SetEraseMode(TRUE);        drawLine(pElement);        SetEraseMode(FALSE);        l->x += dx;        l->y += dy;        SetPenColor("red");        drawLine(pElement);    }    else if (isTxt) {        Txt *t = pElement;        SetEraseMode(TRUE);        drawTxt(pElement);        SetEraseMode(FALSE);        t->x += dx;        t->y += dy;        SetPenColor("red");        drawTxt(pElement);    }}void deleteElement() {    if (pElement == 0) return;    if (isEllips) {        Ellips *e = pElement;        SetEraseMode(TRUE);        drawEllips(pElement);        SetEraseMode(FALSE);        ellips[e->num] = 0;        FreeBlock(pElement);        pElement = 0;        isEllips = FALSE;    }    else if (isRectan) {        Rectan *r = pElement;        SetEraseMode(TRUE);        drawRectan(pElement);        SetEraseMode(FALSE);        rectan[r->num] = 0;        FreeBlock(pElement);        pElement = 0;        isRectan = FALSE;    }    else if (isLine) {        Line *l = pElement;        SetEraseMode(TRUE);        drawLine(pElement);        SetEraseMode(FALSE);        line[l->num] = 0;        FreeBlock(pElement);        pElement = 0;        isLine = FALSE;    }    else if (isTxt) {        Txt *t = pElement;        SetEraseMode(TRUE);        drawTxt(pElement);        SetEraseMode(FALSE);        txt[t->num] = 0;        FreeBlock(pElement);        pElement = 0;        isTxt = FALSE;    }}void Main() /*仅初始化执行一次*/{    InitGraphics();    boxWidth = GetWindowWidth() * 0.1;    boxHeight = GetWindowHeight() * 0.1;    lineAreax = 0;    rectanAreax = boxWidth;    ellipsAreax = boxWidth * 2;    txtAreax = boxWidth * 3;    registerKeyboardEvent(KeyboardEventProcess);/*注册键盘消息回调函数*/    registerCharEvent(CharEventProcess);/*注册字符消息回调函数*/    registerMouseEvent(MouseEventProcess);/*注册鼠标消息回调函数*/    registerTimerEvent(TimerEventProcess);/*注册定时器消息回调函数*/    SetPenColor("Black");    SetPenSize(1);    drawBase();}void drawBaseLine() {    bx = 0;    by = 0;    SetPenSize(3);    DrawBox(bx, by, boxWidth, boxHeight);    SetPenSize(2);    MovePen(bx + boxWidth * 0.2, by + boxHeight / 2);    DrawLine(boxWidth * 0.6, 0);}void drawBaseRectan() {    bx += boxWidth;    SetPenSize(3);    DrawBox(bx, by, boxWidth, boxHeight);    SetPenSize(2);    double cx = bx + boxWidth * 0.2;    double cy = by + boxHeight * 0.2;    DrawBox(cx, cy, boxWidth * 0.6, boxHeight * 0.6);}void drawBaseEllips() {    bx += boxWidth;    SetPenSize(3);    DrawBox(bx, by, boxWidth, boxHeight);    SetPenSize(2);    MovePen(bx + boxWidth * 0.8, by + boxHeight / 2);    DrawEllipticalArc(boxWidth * 0.3, boxHeight * 0.3, 0, 360.0);}void drawBaseText() {    bx += boxWidth;    SetPenSize(3);    DrawBox(bx, by, boxWidth, boxHeight);    SetPointSize(40);    MovePen(bx + boxWidth * 0.4, by + boxHeight * 0.3);    DrawTextString("A");}void drawBase() {    SetPenColor("Black");    double tx = ccx, ty = ccy;    drawBaseLine();    drawBaseRectan();    drawBaseEllips();    drawBaseText();    ccx = tx, ccy = ty;    SetPenSize(1);}void DrawBox(double x, double y, double width, double height) {    MovePen(x, y);    DrawLine(0, height);    DrawLine(width, 0);    DrawLine(0, -height);    DrawLine(-width, 0);}void DrawCenteredCircle(double x, double y, double r) {/*    StartFilledRegion(1);*/    MovePen(x + r, y);    DrawArc(r, 0.0, 360.0);/*    EndFilledRegion();*/}void KeyboardEventProcess(int key, int event)/*每当产生键盘消息，都要执行*/{    drawBase();    double oldradius;    switch (event) {        case KEY_DOWN:            switch (key) {                case VK_UP:/*UP*/                    changeElementSite(0, deltay);                    break;                case VK_DOWN:                    changeElementSite(0, -deltay);                    break;                case VK_LEFT:                    changeElementSite(-deltax, 0);                    break;                case VK_RIGHT:                    changeElementSite(deltax, 0);                    break;                case VK_F3:                case VK_PRIOR: //pageup                    changeElementSize(0.1);                    break;                case VK_F4:                case VK_NEXT:                    changeElementSize(-0.1);                    break;                case VK_F5:                    break;                case VK_F6:                    break;                case VK_F9:                    break;                case VK_BACK:                    deleteElement();                    break;            }            break;        case KEY_UP:            break;    }}void CharEventProcess(char c) {    static char str[2] = {0, 0};    drawBase();    switch (c) {        case '\r':  /* 注意：回车在这里返回的字符是'\r'，不是'\n'*/            isDisplayText = TRUE;/*设置字符串显示标志*/            textx = GetCurrentX() - TextStringWidth(text);/*设置字符串的起始坐标*/            texty = GetCurrentY();            break;        case 27: /*ESC*/            break;        case 8: //backspace            break;        default:            str[0] = c;/*形成当前字符的字符串*/            text[textlen++] = c;/*将当前字符加入到整个字符缓冲区中*/            text[textlen] = '\0';            DrawTextString(str);/*输出当前字符，且输出位置相应后移*/            break;    }}bool inBox(double x0, double y0, double x1, double x2, double y1, double y2) {    return (x0 >= x1 && x0 <= x2 && y0 >= y1 && y0 <= y2);}void MouseEventProcess(int x, int y, int button, int event) {    static double r = 0.2;    static double omx = 0.0, omy = 0.0;    double mx, my;    drawBase();    mx = ScaleXInches(x);/*pixels --> inches*/    my = ScaleYInches(y);/*pixels --> inches*/    switch (event) {        case BUTTON_DOWN:            if (button == LEFT_BUTTON) {                getElement(mx, my);                if (pElement == 0) {                    if (inBox(mx, my, lineAreax, lineAreax + boxWidth, 0, boxHeight)) {                        line[LineSize] = New(Line *);                        line[LineSize]->x = mx;                        line[LineSize]->y = my;                        line[LineSize]->dx = boxWidth * 0.6;                        line[LineSize]->dy = 0;                        line[LineSize]->num = LineSize;                        pElement = line[LineSize++];                        newLine = TRUE;                    }                    else if (inBox(mx, my, rectanAreax, rectanAreax + boxWidth, 0, boxHeight)) {                        rectan[RectanSize] = New(Rectan *);                        rectan[RectanSize]->x = mx;                        rectan[RectanSize]->y = my;                        rectan[RectanSize]->height = boxHeight * 0.6;                        rectan[RectanSize]->width = boxWidth * 0.6;                        rectan[RectanSize]->num = RectanSize;                        pElement = rectan[RectanSize++];                        newRectan = TRUE;                    }                    else if (inBox(mx, my, ellipsAreax, ellipsAreax + boxWidth, 0, boxHeight)) {                        ellips[EllipsSize] = New(Ellips *);                        ellips[EllipsSize]->x = mx;                        ellips[EllipsSize]->y = my;                        ellips[EllipsSize]->rx = boxWidth * 0.3;                        ellips[EllipsSize]->ry = boxHeight * 0.3;                        ellips[EllipsSize]->num = EllipsSize;                        pElement = ellips[EllipsSize++];                        newEllips = TRUE;                    }                    else if (inBox(mx, my, txtAreax, txtAreax + boxWidth, 0, boxHeight)) {                        txt[TxtSize] = New(Txt *);                        txt[TxtSize]->x = mx;                        txt[TxtSize]->y = my;                        txt[TxtSize]->pointSize = 40;                        txt[TxtSize]->text = New(string);                        txt[TxtSize]->text = "A";                        txt[TxtSize]->num = TxtSize;                        pElement = txt[TxtSize++];                        newTxt = TRUE;                    }                }                else if (pElement != 0 && newLine) {                    Line *l = pElement;                    if (!inBase(l->x, l->y))                        newLine = FALSE;                }                else if (pElement != 0 && newRectan) {                    Rectan *r = pElement;                    if (!inBase(r->x, r->y))                        newRectan = FALSE;                }                else if (pElement != 0 && newEllips) {                    Ellips *e = pElement;                    if (!inBase(e->x, e->y))                        newEllips = FALSE;                }                else if (pElement != 0 && newTxt) {                    Txt *t = pElement;                    if (!inBase(t->x, t->y))                        newTxt = FALSE;                }                // 元素已经选定            } else if (button == RIGHT_BUTTON) {                isChangeSize = TRUE;            }            omx = mx;            omy = my;            break;        case BUTTON_DOUBLECLICK:            break;        case BUTTON_UP:            if (button == LEFT_BUTTON) {                isMove = FALSE;            } else if (button == RIGHT_BUTTON) {                isChangeSize = FALSE;            }            break;        case MOUSEMOVE:            if (pElement != 0 && newLine) {                moveLine((Line *) pElement, mx, my);            }            else if (pElement != 0 && newRectan) {                moveRectan((Rectan *) pElement, mx, my);            }            else if (pElement != 0 && newEllips) {                moveEllips((Ellips *) pElement, mx, my);            }            else if (pElement != 0 && newTxt) {                moveTxt((Txt *) pElement, mx, my);            }            else if (pElement != 0 && isMove) {                if (isEllips) {                    SetPenColor("red");                    moveEllips(pElement, mx, my);                    SetPenColor("black");                }                else if (isRectan) {                    SetPenColor("red");                    moveRectan(pElement, mx, my);                    SetPenColor("black");                }                else if (isTxt) {                    SetPenColor("red");                    moveTxt(pElement, mx, my);                    SetPenColor("black");                }                else if (isLine) {                    SetPenColor("red");                    moveLine(pElement, mx, my);                    SetPenColor("black");                }            }            else if (pElement != 0 && isChangeSize) {                double dx = mx - omx;                omx = mx;                omy = my;                changeElementSize(dx);            }            break;    }}void TimerEventProcess(int timerID) {    bool erasemode;    switch (timerID) {        case TIMER_BLINK500: /*500ms文本闪烁定时器*/            if (!isBlink) return;            erasemode = GetEraseMode();            MovePen(textx, texty);/*起始位置*/            SetEraseMode(isDisplayText);/*根据当前显示标志来决定是显示还是隐藏字符串*/            DrawTextString(text);/*当前位置会随字符串后移*/            SetEraseMode(erasemode);            isDisplayText = !isDisplayText;/*交替显示/隐藏字符串符号*/            break;        case TIMER_BLINK1000: /*1000ms圆闪烁定时器*/            if (!isBlink) return;            erasemode = GetEraseMode();            SetEraseMode(isDisplayCircle);            DrawCenteredCircle(ccx, ccy, radius);            SetEraseMode(erasemode);            isDisplayCircle = !isDisplayCircle;            break;        default:            break;    }}