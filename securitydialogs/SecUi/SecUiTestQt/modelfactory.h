#ifndef MODELFACTORY_H
#define MODELFACTORY_H

class QStandardItemModel;

class ModelFactory
{
public:
    static QStandardItemModel *populateTreeModelDefault();
    static QStandardItemModel *populateTreeModelSimple();
    // for debugging:
    static QStandardItemModel *populateTreeModelSimpleOfSimplest();
    static QStandardItemModel *populateTreeModelDeep();
    static QStandardItemModel *populateTreeModelFlat();
    static QStandardItemModel *populateTreeModelMail();
    static QStandardItemModel *populateTreeModelMixed();
    static QStandardItemModel *populateGreenOddBrownEvenModel();
};

#endif // MODELFACTORY_H
