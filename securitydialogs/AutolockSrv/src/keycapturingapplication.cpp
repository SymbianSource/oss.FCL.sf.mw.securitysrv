#include <QSymbianEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <W32STD.H>
#include <eikenv.h>

#include "keycapturingapplication.h"

/*!
    
 */
void writeDebug(const QString &message)
{
    QFile file("c:/keycapturingapplication.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug() << "DialogLauncherView::writeDebug - file open failed";
        return;
    }
    
    QTextStream out(&file);
    out << message << "\n";
    
    file.close();
}

/*!
    
 */
KeyCapturingApplication::KeyCapturingApplication(int &argc, char *argv[], int keyCode) : 
    HbApplication(argc, argv), mKeyCode(keyCode), mKeyCaptureHandle(-1)
{
    CEikonEnv *env = CEikonEnv::Static();
    mKeyCaptureHandle = env->RootWin().CaptureKey(keyCode, 0, 0);
}

/*!
    
 */
KeyCapturingApplication::~KeyCapturingApplication()
{
    CEikonEnv *env = CEikonEnv::Static();
    env->RootWin().CancelCaptureKey(mKeyCaptureHandle);
}
    
bool KeyCapturingApplication::symbianEventFilter(const QSymbianEvent *event)
{
    if (event->type() == QSymbianEvent::WindowServerEvent) {
        const TWsEvent* wsEvent = event->windowServerEvent();
        if (wsEvent->Type() == EEventKey) {
            writeDebug(QString("Key Event:"));
            writeDebug(QString("\tkey code: %1").arg(wsEvent->Key()->iCode));
            writeDebug(QString("\tscan code: %1").arg(wsEvent->Key()->iScanCode));
            if (wsEvent->Key()->iCode == mKeyCode)
            {
                emit capturedKeyEvent();
            }
        }
    }
    return HbApplication::symbianEventFilter(event);
}
