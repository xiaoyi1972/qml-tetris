#ifndef KEYSTATE_H
#define KEYSTATE_H
#include<functional>
#include<QMutex>
#include<QWaitCondition>
class Tetris;
class Task;
class KeyState
{
public:
    KeyState(Task *, const std::function<void()> &_func, std::function<void()>_endFunc = nullptr,
             bool _isDown = false);
    bool keyDown();//按下
    void keyUp();//弹起
    void dasCall();//移动延迟
    void moveCall(int delay);//加速延迟
    void stop();//取消
    void switchStop();//连续方向取消
    int funcChecked();//防抖arr
  /*  int test()
    {
        using std::chrono::high_resolution_clock;
        using std::chrono::milliseconds;
        auto now = high_resolution_clock::now();
        milliseconds timeDelta = std::chrono::duration_cast<milliseconds>(now - startTime);
        startTime = now;
        return timeDelta.count();
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;*/
    std::function<void()> func;
    std::function<void()> endFunc;
    bool isDown, press = false, das = false;
    int dasHandle = -1, arrHandle = -1;
    int switchStopFlag = false;
    Task *task = nullptr;


};


#endif // KEYSTATE_H
