#include <hbinstance.h>
#include <hbapplication.h>

#include "contentwidget.h"

//#define DEBUG_TO_FILE

#ifdef DEBUG_TO_FILE
#include <stdio.h>
#include <QMutex>

const int maxPathIndex = 1;
const char *paths[/*maxPathIndex*/] = {"f:\\SecUiTestQt-log.txt"};

FILE* file = 0;

void myMessageOutput(QtMsgType type, const char *msg) {
    switch (type) {
        case QtDebugMsg: {
            fprintf(file, "Debug: %s\n", msg);
        }
        break;
        case QtWarningMsg: {
            //fprintf(file, "Warning: %s\n", msg);
        }
        break;
        case QtCriticalMsg: {
            fprintf(file, "Critical: %s\n", msg);
        }
        break;
        case QtFatalMsg: {
            fprintf(file, "Fatal: %s\n", msg);
            abort();
        }
    }
}
#endif


int main(int argc, char *argv[])
{
#ifdef DEBUG_TO_FILE
    int pathIndex = 0;
    while (!file && pathIndex < maxPathIndex) {
        file = fopen(paths[pathIndex], "a");
        if (!file) ++pathIndex;
    }
    if (file) qInstallMsgHandler(myMessageOutput);
    qDebug("============================================================"
        "===========================================================");
    qDebug("============================================================"
        "===========================================================");
#endif
    HbApplication app(argc, argv);    
    app.setApplicationName( "SecUiTestQt" );

    QString appDir = app.applicationDirPath(); 

    HbMainWindow* window = new HbMainWindow();

    ContentWidget *view = new ContentWidget(appDir, window);
    window->addView(view);

    window->show();
    int closeCode = app.exec();
    window->deleteLater();
    
#ifdef DEBUG_TO_FILE
    if (file) {
        fclose(file);
    }
#endif
    return closeCode;
}
