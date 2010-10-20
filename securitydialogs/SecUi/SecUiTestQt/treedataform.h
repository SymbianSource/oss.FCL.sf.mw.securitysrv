/*
* ====================================================
*  Name        : treedataform.h
*  Part of     : fute/SecUiTestQt
*  Description : Provides DataForm for Tree
*  Version     : %version: 1 %
*
*  Copyright (c) 2009 Nokia.  All rights reserved.
*  This material, including documentation and any related computer
*  programs, is protected by copyright controlled by Nokia.  All
*  rights are reserved.  Copying, including reproducing, storing,
*  adapting or translating, any or all of this material requires the
*  prior written consent of Nokia.  This material also contains
*  confidential information which may not be disclosed to others
*  without the prior written consent of Nokia.
* ====================================================
*/

#ifndef TREEDATAFORM_H
#define TREEDATAFORM_H

#include "viewfutedataform.h"
#include <QStringList>

class TreeDataForm : public ViewFuteDataForm
{
    Q_OBJECT

public:

    enum CustomIndex {
        ViewItemType = ViewFuteDataForm::SettingsIndexLast+1,
        Depth,
        Indentation,
        ItemUserExpandable,
        TreeCustomLast
    };

    explicit TreeDataForm(HbAbstractItemView &view,
                            QGraphicsItem *parent = 0);
    virtual ~TreeDataForm();

    virtual int populateCustomSettingsItem(int previousItem);
    virtual void initialise();

    virtual void resolveSettingsResults();
    void setDirViewItemEnabled(bool enable);

    void setDepth(int depth);
    int getDepth() const;

    void setIndentation(int indentation);

    void setItemUserExpandable(bool value);

private:
    int customTreeSettingsIndexes[TreeCustomLast];
    int depth;
    int indentation;
    bool dirViewItemEnabled;
};

#endif // TREEDATAFORM_H
