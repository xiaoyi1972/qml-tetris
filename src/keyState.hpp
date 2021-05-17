#ifndef KEYSTATE_HPP
#define KEYSTATE_HPP
#include<functional>
template <class Tetris, class Task>
class KeyState {
public:
    KeyState(Task *_task, const std::function<void()> &_func, std::function<void()>_endFunc = nullptr,
             bool _isDown = false) {
        func = _func;
        endFunc = _endFunc;
        isDown = _isDown;
        task = _task;
    }

    bool keyDown() {
        if (!press) {
            press = true;
            func();
            if (isDown)
                moveCall(Tetris::keyconfig["softdropDelay"].template value<int>());
            else
                dasHandle = task->setTimeOut(std::bind(&KeyState::dasCall, this), Tetris::keyconfig["dasDelay"].template value<int>());
            return true;
        } else  return false;
    }

    void keyUp() {
        switchStopFlag = false;
        press = false;
        das = false;
        stop();
    }

    void dasCall() {
        das = true;
        dasHandle = -1;
        moveCall(Tetris::keyconfig["arrDelay"].template value<int>());
    }

    void moveCall(int delay) {
        /*  if (delay ==  0)
            endFunc();
          else*/
        {
            // func();
            arrHandle = task->setInterval(std::bind(&KeyState::funcChecked, this), delay);
        }
    }

    void stop() {
        if (!isDown) {
            task->clearTimeout(dasHandle);
            dasHandle = -1;
        }
        task->clearInterval(arrHandle);
        arrHandle = -1;
    }

    void switchStop() {
        switchStopFlag = true;
        das = false;
        stop();
    }

    int funcChecked() {
        if (das || isDown) {
            func();
            return 0;
        } else {
            //qDebug() << "bad";
            return -1;
        }
    }

    std::function<void()> func;
    std::function<void()> endFunc;
    bool isDown, press = false, das = false;
    int dasHandle = -1, arrHandle = -1;
    int switchStopFlag = false;
    Task *task = nullptr;
};


#endif // KEYSTATE_HPP
