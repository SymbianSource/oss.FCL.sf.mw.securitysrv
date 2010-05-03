#ifndef KEYCAPTURINGAPPLICATION_H
#define KEYCAPTURINGAPPLICATION_H

#include <HbApplication>

class KeyCapturingApplication : public HbApplication
{

    Q_OBJECT
    
signals:

    void capturedKeyEvent();
 
public:
    
    KeyCapturingApplication(int &argc, char *argv[], int keyCode);
    ~KeyCapturingApplication();
    virtual bool symbianEventFilter(const QSymbianEvent *event);
    
private:

    int mKeyCode;
    int mKeyCaptureHandle;
    
};

#endif // KEYCAPTURINGAPPLICATION_H
