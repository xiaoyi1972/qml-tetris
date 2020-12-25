#ifndef KEYSTATE_H
#define KEYSTATE_H
#include<functional>
#include<QVector>
class Tetris;
class Task;
class KeyState
{
public:
    KeyState(Tetris *_tetris, const std::function<void()> &_func, std::function<void()>_endFunc = nullptr,
         bool _isDown = false, bool _noDas = false);
    bool keyDown();//按下
    void keyUp();//弹起
    void dasCall();//移动延迟
    void moveCall(int delay);//加速延迟
    void stop();//取消
    void switchStop();//连续方向取消
    std::function<void()> func;
    std::function<void()> endFunc;
    bool isDown, noDas, press = false, das;
    int dasHandle, arrHandle;
    int switchStopFlag = false;
    QString name;
    Tetris *tetris = nullptr;
};


#endif // KEYSTATE_H
