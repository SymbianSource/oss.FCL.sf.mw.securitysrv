#include <e32debug.h>

#include "modelfactory.h"

#include <hbnamespace.h>
#include <hbicon.h>

#include <QStandardItemModel>

void insertMixedItems(QStandardItem *parent)
{
    QString longSecondaryText;
    for (int i = 0; i < 20; ++i) {
        longSecondaryText.append("Second text ");
    }

    HbIcon icon(QString(":/demo/generic"));

    QVariantList strings;
    QVariantList icons;

    // text
    QStandardItem *child = new QStandardItem();
    strings << "text-1";
    child->setData(strings, Qt::DisplayRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // text + icon
    child = new QStandardItem();
    strings << "text-1+icon-2";
    icons << QVariant() << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + text
    child = new QStandardItem();
    strings << "icon-1+text-1";
    icons << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + text + icon
    child = new QStandardItem();
    strings << "icon-1+text-1+icon-2";
    icons << icon << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // text + text
    child = new QStandardItem();
    strings << "text-1+text-3" << QVariant() << "third text";
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // text + text + icon
    child = new QStandardItem();
    strings << "text-1+text-3+icon-2" << QVariant() << "third text";
    icons << QVariant() << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + text + text
    child = new QStandardItem();
    strings << "icon-1+text-1+text-3" << QVariant() << "third text";
    icons << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + text + text + icon
    child = new QStandardItem();
    strings << "icon-1+text-1+text-3+icon-2" << QVariant() << "third text";
    icons << icon << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // 2 texts
    child = new QStandardItem();
    strings << "text-1+text-2" << longSecondaryText;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // 2 texts + icon
    child = new QStandardItem();
    strings << "text-1+text-2+icon-2" << longSecondaryText;
    icons << QVariant() << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + 2 texts
    child = new QStandardItem();
    strings << "icon-1+text-1+text-2" << longSecondaryText;
    icons << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + 2 texts + icon
    child = new QStandardItem();
    strings << "icon-1+text-1+text-2+icon-2" << longSecondaryText;
    icons << icon << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // 3 texts
    child = new QStandardItem();
    strings << "text-1+text-2+text-3" << longSecondaryText << "third text";
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // 3 texts + icon
    child = new QStandardItem();
    strings << "text-1+text-2+text-3+icon-2" << longSecondaryText << "third text";
    icons << QVariant() << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + 3 texts
    child = new QStandardItem();
    strings << "icon-1+text-1+text-2+text-3" << longSecondaryText << "third text";
    icons << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // icon + 3 texts + icon
    child = new QStandardItem();
    strings << "icon-1+text-1+text-2+text-3+icon-2" << longSecondaryText << "third text";
    icons << icon << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // separator
    child = new QStandardItem();
    strings << "Separator";
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    child->setData(Hb::SeparatorItem, Hb::ItemTypeRole);
    child->setEnabled(false);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // disabled
    child = new QStandardItem();
    strings << "Disabled item" << longSecondaryText << "third text";
    icons << icon << icon;
    child->setData(strings, Qt::DisplayRole);
    child->setData(icons, Qt::DecorationRole);
    child->setEnabled(false);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // disabled parent item
    child = new QStandardItem();
    strings << "Disabled parent item";
    child->setData(strings, Qt::DisplayRole);
    child->setData(Hb::ParentItem, Hb::ItemTypeRole);
    child->setEnabled(false);
    parent->appendRow(child);

    strings.clear();
    icons.clear();

    // disabled parent item's child
    QStandardItem *child2 = new QStandardItem();
    strings << "Disabled parent item's child";
    child2->setData(strings, Qt::DisplayRole);
    child2->setEnabled(false);
    child->appendRow(child2);

}

QStandardItemModel *ModelFactory::populateTreeModelMail()
{
    QStandardItemModel* model = new QStandardItemModel();

    //________________________________________
    int messageRole = Qt::UserRole+1;
    int dateRole = Qt::UserRole+2;

    QStandardItem *date = new QStandardItem;
    date->setData(QString("27. lokakuuta 2009"), dateRole);
    model->appendRow(date);

    QStandardItem *message = new QStandardItem;
    QStringList data;
    data << "Fotonetti.com" << "19:51:14" << "Fotonetti Pro";
    message->setData(data, messageRole);
    date->appendRow(message);

    date = new QStandardItem;
    date->setData(QString("25. lokakuuta 2009"), dateRole);
    model->appendRow(date);

    message = new QStandardItem;
    data.clear();
    data << "Amazon.co.uk" << "5:51:52" << "Your Amazon Order # 123-3456789-0123 ";
    message->setData(data, messageRole);
    date->appendRow(message);

    date = new QStandardItem;
    date->setData(QString("23. lokakuuta 2009"), dateRole);
    model->appendRow(date);

    message = new QStandardItem;
    data.clear();
    data << "MBnet" << "12:12:12" << "MBnetin pullopostia 43/2009: Aurinkoa odotellessa... ";
    message->setData(data, messageRole);
    date->appendRow(message);

    date = new QStandardItem;
    date->setData(QString("21. lokakuuta 2009"), dateRole);
    model->appendRow(date);

    message = new QStandardItem;
    data.clear();
    data << "Skype" << "21:59:20" << "Skype 4.1 - Too late";
    message->setData(data, messageRole);
    date->appendRow(message);
    
    message = new QStandardItem;
    data.clear();
    data << "Skype" << "20:58:19" << "Skype 4.1 - Act now";
    message->setData(data, messageRole);
    date->appendRow(message);
    
    message = new QStandardItem;
    data.clear();
    data << "Skype" << "19:57:18" << "Skype 4.1 - Time to act";
    message->setData(data, messageRole);
    date->appendRow(message);

    message = new QStandardItem;
    data.clear();
    data << "Skype" << "18:56:17" << "Skype 4.1 - New release";
    message->setData(data, messageRole);
    date->appendRow(message);

    date = new QStandardItem;
    date->setData(QString("5. lokakuuta 2009"), dateRole);
    model->appendRow(date);

    message = new QStandardItem;
    data.clear();
    data << "Hit Booster" << "21:12:00" << "Have your own website traffic generator ";
    message->setData(data, messageRole);
    date->appendRow(message);

    message = new QStandardItem;
    data.clear();
    data << "noreply@helsinkiexpert.fi" << "11:08:01" << "Terveisiä Helsinki Expertiltä";
    message->setData(data, messageRole);
    date->appendRow(message);


    return model;
}


QStandardItemModel *ModelFactory::populateTreeModelDefault()
{
    QStandardItemModel* model = new QStandardItemModel();

    //________________________________________

    QStandardItem *paintDevice = new QStandardItem;
    paintDevice->setText(QString("QPaintDevice"));
    model->appendRow(paintDevice);

    QStandardItem *pixmap = new QStandardItem;
    pixmap->setText(QString("QPixmap"));
    paintDevice->appendRow(pixmap);

    QStandardItem *bitmap = new QStandardItem;
    bitmap->setText(QString("QBitmap"));
    pixmap->appendRow(bitmap);

    QStandardItem *customRasterPaintDevice = new QStandardItem;
    customRasterPaintDevice->setText(QString("QCustomRasterPaintDevice"));
    paintDevice->appendRow(customRasterPaintDevice);

    QStandardItem *glPixelBuffer = new QStandardItem;
    glPixelBuffer->setText(QString("QGLPixelBuffer"));
    paintDevice->appendRow(glPixelBuffer);

    QStandardItem *image = new QStandardItem;
    image->setText(QString("QImage"));
    paintDevice->appendRow(image);

    QStandardItem *picture = new QStandardItem;
    picture->setText(QString("QPicture"));
    paintDevice->appendRow(picture);

    QStandardItem *printer = new QStandardItem;
    printer->setText(QString("QPrinter"));
    paintDevice->appendRow(printer);

    QStandardItem *svgGenerator = new QStandardItem;
    svgGenerator->setText(QString("QSvgGenerator"));
    paintDevice->appendRow(svgGenerator);

    //________________________________________

    QStandardItem *object = new QStandardItem;
    object->setText(QString("QObject"));
    model->appendRow(object);

    QStandardItem *widget = new QStandardItem;
    widget->setText(QString("QWidget"));
    object->appendRow(widget);

    QStandardItem *absItemModel = new QStandardItem;
    absItemModel->setText(QString("QAbstractItemModel"));
    object->appendRow(absItemModel);

    QStandardItem *absListModel = new QStandardItem;
    absListModel->setText(QString("QAbstractListModel"));
    absItemModel->appendRow(absListModel);

    QStandardItem *strListModel = new QStandardItem;
    strListModel->setText(QString("QStringListModel"));
    absListModel->appendRow(strListModel);

    QStandardItem *absProxyModel = new QStandardItem;
    absProxyModel->setText(QString("QAbstractProxyModel"));
    absItemModel->appendRow(absProxyModel);

    QStandardItem *sFilterProxyModel = new QStandardItem;
    sFilterProxyModel->setText(QString("QSortFilterProxyModel"));
    absProxyModel->appendRow(sFilterProxyModel);

    QStandardItem *absTableModel = new QStandardItem;
    absTableModel->setText(QString("QAbstractTableModel"));
    absItemModel->appendRow(absTableModel);

    QStandardItem *sqlQueryModel = new QStandardItem;
    sqlQueryModel->setText(QString("QSqlQueryModel"));
    absTableModel->appendRow(sqlQueryModel);

    QStandardItem *sqlTableModel = new QStandardItem;
    sqlTableModel->setText(QString("QSqlTableModel"));
    sqlQueryModel->appendRow(sqlTableModel);

    QStandardItem *sqlRTableModel = new QStandardItem;
    sqlRTableModel->setText(QString("QSqlRelationalTableModel"));
    sqlTableModel->appendRow(sqlRTableModel);

    QStandardItem *dirModel = new QStandardItem;
    dirModel->setText(QString("QDirModel"));
    absItemModel->appendRow(dirModel);

    QStandardItem *proxyModel = new QStandardItem;
    proxyModel->setText(QString("QProxyModel"));
    absItemModel->appendRow(proxyModel);

    QStandardItem *stanItemModel = new QStandardItem;
    stanItemModel->setText(QString("QStandardItemModel"));
    absItemModel->appendRow(stanItemModel);

    QStandardItem *ioDevice = new QStandardItem;
    ioDevice->setText(QString("QIODevice"));
    object->appendRow(ioDevice);

    QStandardItem *absSocket = new QStandardItem;
    absSocket->setText(QString("QAbstractSocket"));
    ioDevice->appendRow(absSocket);

    QStandardItem *topSocket = new QStandardItem;
    topSocket->setText(QString("QTopSocket"));
    absSocket->appendRow(topSocket);

    QStandardItem *sslSocket = new QStandardItem;
    sslSocket->setText(QString("QSslSocket"));
    topSocket->appendRow(sslSocket);

    QStandardItem *udpSocket = new QStandardItem;
    udpSocket->setText(QString("QUdpSocket"));
    absSocket->appendRow(udpSocket);

    QStandardItem *file = new QStandardItem;
    file->setText(QString("QFile"));
    ioDevice->appendRow(file);

    QStandardItem *tmpFile = new QStandardItem;
    tmpFile->setText(QString("QTemporaryFile"));
    file->appendRow(tmpFile);

    QStandardItem *buffer = new QStandardItem;
    buffer->setText(QString("QBuffer"));
    ioDevice->appendRow(buffer);

    QStandardItem *process = new QStandardItem;
    process->setText(QString("QProcess"));
    ioDevice->appendRow(process);

    QStandardItem *validator = new QStandardItem;
    validator->setText(QString("QValidator"));
    object->appendRow(validator);

    QStandardItem *dValidator = new QStandardItem;
    dValidator->setText(QString("QDoubleValidator"));
    validator->appendRow(dValidator);

    QStandardItem *intValidator = new QStandardItem;
    intValidator->setText(QString("QIntValidator"));
    validator->appendRow(intValidator);

    QStandardItem *rgValidator = new QStandardItem;
    rgValidator->setText(QString("QRegExpValidator"));
    validator->appendRow(rgValidator);

    QStandardItem *action = new QStandardItem;
    action->setText(QString("QAction"));
    object->appendRow(action);

    QStandardItem *menuItem = new QStandardItem;
    menuItem->setText(QString("QMenuItem"));
    action->appendRow(menuItem);

    QStandardItem *widgetAction = new QStandardItem;
    widgetAction->setText(QString("QWidgetAction"));
    action->appendRow(widgetAction);

    QStandardItem *dBusAbsInterface = new QStandardItem;
    dBusAbsInterface->setText(QString("QDBusAbstractInterface"));
    object->appendRow(dBusAbsInterface);

    QStandardItem *dBusConInterface = new QStandardItem;
    dBusConInterface->setText(QString("QDBusConnectionInterface"));
    dBusAbsInterface->appendRow(dBusConInterface);

    QStandardItem *dBusInterface = new QStandardItem;
    dBusInterface->setText(QString("QDBusInterface"));
    dBusAbsInterface->appendRow(dBusInterface);

    QStandardItem *textObject = new QStandardItem;
    textObject->setText(QString("QTextObject"));
    object->appendRow(textObject);

    QStandardItem *textBlockGroup = new QStandardItem;
    textBlockGroup->setText(QString("QTextBlockGroup"));
    textObject->appendRow(textBlockGroup);

    QStandardItem *textList = new QStandardItem;
    textList->setText(QString("QTextList"));
    textBlockGroup->appendRow(textList);

    QStandardItem *textFrame = new QStandardItem;
    textFrame->setText(QString("QTextFrame"));
    textObject->appendRow(textFrame);

    QStandardItem *textTable = new QStandardItem;
    textTable->setText(QString("QTextTable"));
    textFrame->appendRow(textTable);

    QStandardItem *absItemDelegate = new QStandardItem;
    absItemDelegate->setText(QString("QAbstractItemDelegate"));
    object->appendRow(absItemDelegate);

    QStandardItem *itemDelegate = new QStandardItem;
    itemDelegate->setText(QString("QItemDelegate"));
    absItemDelegate->appendRow(itemDelegate);

    QStandardItem *sqlRelationalDelegate = new QStandardItem;
    sqlRelationalDelegate->setText(QString("QSqlRelationalDelegate"));
    itemDelegate->appendRow(sqlRelationalDelegate);

    //________________________________________

    QStandardItem *layoutItem = new QStandardItem;
    layoutItem->setText(QString("QLayoutItem"));
    model->appendRow(layoutItem);

    QStandardItem *layout = new QStandardItem;
    layout->setText(QString("QLayout"));
    layoutItem->appendRow(layout);

    QStandardItem *boxLayout = new QStandardItem;
    boxLayout->setText(QString("QBoxLayout"));
    layout->appendRow(boxLayout);

    QStandardItem *hBoxLayout = new QStandardItem;
    hBoxLayout->setText(QString("QHBoxLayout"));
    boxLayout->appendRow(hBoxLayout);

    QStandardItem *vBoxLayout = new QStandardItem;
    vBoxLayout->setText(QString("QVBoxLayout"));
    boxLayout->appendRow(vBoxLayout);

    QStandardItem *gridLayout = new QStandardItem;
    gridLayout->setText(QString("QGridLayout"));
    layout->appendRow(gridLayout);

    QStandardItem *stackedLayout = new QStandardItem;
    stackedLayout->setText(QString("QStackedLayout"));
    layout->appendRow(stackedLayout);

    QStandardItem *spacerItem = new QStandardItem;
    spacerItem->setText(QString("QSpacerItem"));
    layoutItem->appendRow(spacerItem);

    QStandardItem *widgetItem = new QStandardItem;
    widgetItem->setText(QString("QWidgetItem"));
    layoutItem->appendRow(widgetItem);

    //________________________________________

    QStandardItem *axBase = new QStandardItem;
    axBase->setText(QString("QAxBase"));
    model->appendRow(axBase);

    QStandardItem *axWidget = new QStandardItem;
    axWidget->setText(QString("QAxWidget"));
    axBase->appendRow(axWidget);

    QStandardItem *axObject = new QStandardItem;
    axObject->setText(QString("QAxObject"));
    axBase->appendRow(axObject);

    QStandardItem *axScriptEngine = new QStandardItem;
    axScriptEngine->setText(QString("QAxScriptEngine"));
    axObject->appendRow(axScriptEngine);

    //________________________________________

    QStandardItem *absFormBuilder = new QStandardItem;
    absFormBuilder->setText(QString("QAbstractFormBuilder"));
    model->appendRow(absFormBuilder);

    QStandardItem *formBuilder = new QStandardItem;
    formBuilder->setText(QString("QFormBuilder"));
    absFormBuilder->appendRow(formBuilder);

    //________________________________________

    QStandardItem *domNote = new QStandardItem;
    domNote->setText(QString("QDomNote"));
    model->appendRow(domNote);

    QStandardItem *domCharData = new QStandardItem;
    domCharData->setText(QString("QDomCharacterData"));
    domNote->appendRow(domCharData);

    QStandardItem *domText = new QStandardItem;
    domText->setText(QString("QDomText"));
    domCharData->appendRow(domText);

    QStandardItem *domCDATASection = new QStandardItem;
    domCDATASection->setText(QString("QDomCDATASection"));
    domText->appendRow(domCDATASection);

    QStandardItem *domComment = new QStandardItem;
    domComment->setText(QString("QDomComment"));
    domCharData->appendRow(domComment);

    QStandardItem *domAttr = new QStandardItem;
    domAttr->setText(QString("QDomAttr"));
    domNote->appendRow(domAttr);

    QStandardItem *domDoc = new QStandardItem;
    domDoc->setText(QString("QDomDocument"));
    domNote->appendRow(domDoc);

    QStandardItem *domDocFrag = new QStandardItem;
    domDocFrag->setText(QString("QDomDocumentFragment"));
    domNote->appendRow(domDocFrag);

    QStandardItem *domDocType = new QStandardItem;
    domDocType->setText(QString("QDomDocumentType"));
    domNote->appendRow(domDocType);

    QStandardItem *domElement = new QStandardItem;
    domElement->setText(QString("QDomElement"));
    domNote->appendRow(domElement);

    QStandardItem *domEntity = new QStandardItem;
    domEntity->setText(QString("QDomEntity"));
    domNote->appendRow(domEntity);

    QStandardItem *domEntityRef = new QStandardItem;
    domEntityRef->setText(QString("QDomEntityReference"));
    domNote->appendRow(domEntityRef);

    QStandardItem *domNotation = new QStandardItem;
    domNotation->setText(QString("QDomNotation"));
    domNote->appendRow(domNotation);

    QStandardItem *domProcInst = new QStandardItem;
    domProcInst->setText(QString("QDomProcessingInstruction"));
    domNote->appendRow(domProcInst);

    //________________________________________

    QStandardItem *xmlContentHandler = new QStandardItem;
    xmlContentHandler->setText(QString("QXmlContentHandler"));
    model->appendRow(xmlContentHandler);

    QStandardItem *xmlDTDHandler = new QStandardItem;
    xmlDTDHandler->setText(QString("QXmlDTDHandler"));
    model->appendRow(xmlDTDHandler);

    QStandardItem *xmlDecHandler = new QStandardItem;
    xmlDecHandler->setText(QString("QXmlDecHandler"));
    model->appendRow(xmlDecHandler);

    QStandardItem *xmlEntityHandler = new QStandardItem;
    xmlEntityHandler->setText(QString("QXmlEntityHandler"));
    model->appendRow(xmlEntityHandler);

    QStandardItem *xmlErrorHandler = new QStandardItem;
    xmlErrorHandler->setText(QString("QXmlErrorHandler"));
    model->appendRow(xmlErrorHandler);

    QStandardItem *xmlLexicalHandler = new QStandardItem;
    xmlLexicalHandler->setText(QString("QXmlLexicalHandler"));
    model->appendRow(xmlLexicalHandler);

    QStandardItem *xmlDefaultHandler = new QStandardItem;
    xmlDefaultHandler->setText(QString("QXmlDefaultHandler"));
    xmlContentHandler->appendRow(xmlDefaultHandler);

    //________________________________________

    QStandardItem *xmlReader = new QStandardItem;
    xmlReader->setText(QString("QXmlReader"));
    model->appendRow(xmlReader);

    QStandardItem *xmlSimpleReader = new QStandardItem;
    xmlSimpleReader->setText(QString("QXmlSimpleReader"));
    xmlReader->appendRow(xmlSimpleReader);

    //________________________________________

    QStandardItem *absFileEngine = new QStandardItem;
    absFileEngine->setText(QString("QAbstractFileEngine"));
    model->appendRow(absFileEngine);

    QStandardItem *fsFileEngine = new QStandardItem;
    fsFileEngine->setText(QString("QFSFileEngine"));
    absFileEngine->appendRow(fsFileEngine);

    //________________________________________

    QStandardItem *genArg = new QStandardItem;
    genArg->setText(QString("QGenericArgument"));
    model->appendRow(genArg);

    QStandardItem *genRetArg = new QStandardItem;
    genRetArg->setText(QString("QGenericReturnArgument"));
    genArg->appendRow(genRetArg);

    //________________________________________

    QStandardItem *textStream = new QStandardItem;
    textStream->setText(QString("QTextStream"));
    model->appendRow(textStream);

    QStandardItem *textIStream = new QStandardItem;
    textIStream->setText(QString("QTextIStream"));
    textStream->appendRow(textIStream);

    QStandardItem *textOStream = new QStandardItem;
    textOStream->setText(QString("QTextOStream"));
    textStream->appendRow(textOStream);

    //________________________________________

    QStandardItem *screen = new QStandardItem;
    screen->setText(QString("QScreen"));
    model->appendRow(screen);

    QStandardItem *vncScreen = new QStandardItem;
    vncScreen->setText(QString("QVNCScreen"));
    screen->appendRow(vncScreen);

    //________________________________________

    QStandardItem *wsMouseHandler = new QStandardItem;
    wsMouseHandler->setText(QString("QWSMouseHandler"));
    model->appendRow(wsMouseHandler);

    QStandardItem *calWsMouseHandler = new QStandardItem;
    calWsMouseHandler->setText(QString("QWSCalibratedMouseHandler"));
    wsMouseHandler->appendRow(calWsMouseHandler);

    //________________________________________

    QStandardItem *painter = new QStandardItem;
    painter->setText(QString("QPainter"));
    model->appendRow(painter);

    QStandardItem *sPainter = new QStandardItem;
    sPainter->setText(QString("QStylePainter"));
    painter->appendRow(sPainter);

    //________________________________________

    QStandardItem *paintEngine = new QStandardItem;
    paintEngine->setText(QString("QPaintEngine"));
    model->appendRow(paintEngine);


    QStandardItem *paintREngine = new QStandardItem;
    paintREngine->setText(QString("QRasterPaintEngine"));
    paintEngine->appendRow(paintREngine);

    //________________________________________

    QStandardItem *palette = new QStandardItem;
    palette->setText(QString("QPalette"));
    model->appendRow(palette);

    QStandardItem *colorGroup = new QStandardItem;
    colorGroup->setText(QString("QColorGroup"));
    palette->appendRow(colorGroup);

    //________________________________________

    QStandardItem *qevent = new QStandardItem;
    qevent->setText(QString("QEvent"));
    model->appendRow(qevent);

    return model;
}

QStandardItemModel *ModelFactory::populateTreeModelSimple()
{
    QStandardItemModel* model = new QStandardItemModel();

    // =====================================================================
    // Depth 1
    // =====================================================================
    QStandardItem *depth1_parent0 = new QStandardItem(QString("Lock"));
    QStandardItem *depth1_parent1 = new QStandardItem(QString("Settings"));
    QStandardItem *depth1_parent2 = new QStandardItem(QString("Handler"));
    QStandardItem *depth1_parent3 = new QStandardItem(QString("Notifier"));
    QStandardItem *depth1_parent4 = new QStandardItem(QString("Properties"));
    QStandardItem *depth1_parent5 = new QStandardItem(QString("Repository"));
    QStandardItem *depth1_parent6 = new QStandardItem(QString("Other"));
    QStandardItem *depth1_parent7 = new QStandardItem(QString("KeyLockPolicy"));
    QStandardItem *depth1_parent8 = new QStandardItem(QString("Schedule"));
    QStandardItem *depth1_parent9 = new QStandardItem(QString("Settings2"));

    depth1_parent0->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent1->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent2->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent3->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent4->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent5->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent6->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent7->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent8->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent9->setData(Hb::ParentItem, Hb::ItemTypeRole);

    model->setItem(0,0,depth1_parent0);
    model->setItem(1,0,depth1_parent1);
    model->setItem(2,0,depth1_parent2);
    model->setItem(3,0,depth1_parent3);
    model->setItem(4,0,depth1_parent4);
    model->setItem(5,0,depth1_parent5);
    model->setItem(6,0,depth1_parent6);
    model->setItem(7,0,depth1_parent7);
    model->setItem(8,0,depth1_parent8);
    model->setItem(9,0,depth1_parent9);


		RDebug::Printf( "%s %s (%u) Lock depth2-0=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item00 = new QStandardItem(QString("00:xxx"));
    QStandardItem *depth2_item01 = new QStandardItem(QString("01:DeviceLockOff"));
    QStandardItem *depth2_item02 = new QStandardItem(QString("02:KeyguardOn+Note"));
    QStandardItem *depth2_item03 = new QStandardItem(QString("03:KeyguardOff"));
    QStandardItem *depth2_item04 = new QStandardItem(QString("04:OfferDevicelock"));
    QStandardItem *depth2_item05 = new QStandardItem(QString("05:KeyguardOn-Note"));
    QStandardItem *depth2_item06 = new QStandardItem(QString("06:Wait20-DeviceLockOff"));
    QStandardItem *depth2_item07 = new QStandardItem(QString("07:Wait20-KeyguardOff"));
    QStandardItem *depth2_item08 = new QStandardItem(QString("08:Wait20-ShowKeysLockedNote"));
    QStandardItem *depth2_item09 = new QStandardItem(QString("09:DeviceLockOn"));

    depth1_parent0->setChild(0, 0, depth2_item00);
    depth1_parent0->setChild(1, 0, depth2_item01);
    depth1_parent0->setChild(2, 0, depth2_item02);
    depth1_parent0->setChild(3, 0, depth2_item03);
    depth1_parent0->setChild(4, 0, depth2_item04);
    depth1_parent0->setChild(5, 0, depth2_item05);
    depth1_parent0->setChild(6, 0, depth2_item06);
    depth1_parent0->setChild(7, 0, depth2_item07);
    depth1_parent0->setChild(8, 0, depth2_item08);
    depth1_parent0->setChild(9, 0, depth2_item09);
    
		RDebug::Printf( "%s %s (%u) Settings depth2-1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item10 = new QStandardItem(QString("10:Call1"));
    QStandardItem *depth2_item11 = new QStandardItem(QString("11:ChangePinL"));
    QStandardItem *depth2_item12 = new QStandardItem(QString("12:IsLockEnabledL"));
    QStandardItem *depth2_item13 = new QStandardItem(QString("13:AskSecCodeL"));
    QStandardItem *depth2_item14 = new QStandardItem(QString("14:AskPin2L"));
    QStandardItem *depth2_item15 = new QStandardItem(QString("15:GetFdnMode"));
    QStandardItem *depth2_item16 = new QStandardItem(QString("16:IsUpinBlocked"));
    QStandardItem *depth2_item17 = new QStandardItem(QString("17:ChangeSecCodeL"));
    QStandardItem *depth2_item18 = new QStandardItem(QString("18:ChangeAutoLockPeriodL=30"));
    QStandardItem *depth2_item19 = new QStandardItem(QString("19:ChangeAutoLockPeriodL=0"));

    depth1_parent1->setChild(0, 0, depth2_item10);
    depth1_parent1->setChild(1, 0, depth2_item11);
    depth1_parent1->setChild(2, 0, depth2_item12);
    depth1_parent1->setChild(3, 0, depth2_item13);
    depth1_parent1->setChild(4, 0, depth2_item14);
    depth1_parent1->setChild(5, 0, depth2_item15);
    depth1_parent1->setChild(6, 0, depth2_item16);
    depth1_parent1->setChild(7, 0, depth2_item17);
    depth1_parent1->setChild(8, 0, depth2_item18);
    depth1_parent1->setChild(9, 0, depth2_item19);
    
		RDebug::Printf( "%s %s (%u) Handler depth2-2=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item20 = new QStandardItem(QString("20:Notif.EPin1Required"));
    QStandardItem *depth2_item21 = new QStandardItem(QString("21:EPin1Required"));
    QStandardItem *depth2_item22 = new QStandardItem(QString("22:EPin2Required"));
    QStandardItem *depth2_item23 = new QStandardItem(QString("23:EPhonePasswordRequired"));
    QStandardItem *depth2_item24 = new QStandardItem(QString("24:EPuk1Required"));
    QStandardItem *depth2_item25 = new QStandardItem(QString("25:EPuk2Required"));
    QStandardItem *depth2_item26 = new QStandardItem(QString("26:EUniversalPinRequired"));
    QStandardItem *depth2_item27 = new QStandardItem(QString("27:EUniversalPukRequired"));
    QStandardItem *depth2_item28 = new QStandardItem(QString("28:xxx"));
    QStandardItem *depth2_item29 = new QStandardItem(QString("29:xxx"));

    depth1_parent2->setChild(0, 0, depth2_item20);
    depth1_parent2->setChild(1, 0, depth2_item21);
    depth1_parent2->setChild(2, 0, depth2_item22);
    depth1_parent2->setChild(3, 0, depth2_item23);
    depth1_parent2->setChild(4, 0, depth2_item24);
    depth1_parent2->setChild(5, 0, depth2_item25);
    depth1_parent2->setChild(6, 0, depth2_item26);
    depth1_parent2->setChild(7, 0, depth2_item27);
    depth1_parent2->setChild(8, 0, depth2_item28);
    depth1_parent2->setChild(9, 0, depth2_item29);
    
		RDebug::Printf( "%s %s (%u) Notifier depth2-3=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item30 = new QStandardItem(QString("30:Op.0"));
    QStandardItem *depth2_item31 = new QStandardItem(QString("31:EPin1Required"));
    QStandardItem *depth2_item32 = new QStandardItem(QString("32:EPin2Required"));
    QStandardItem *depth2_item33 = new QStandardItem(QString("33:EPhonePasswordRequired"));
    QStandardItem *depth2_item34 = new QStandardItem(QString("34:EPuk1Required"));
    QStandardItem *depth2_item35 = new QStandardItem(QString("35:EPuk2Required"));
    QStandardItem *depth2_item36 = new QStandardItem(QString("36:EUniversalPinRequired"));
    QStandardItem *depth2_item37 = new QStandardItem(QString("37:EUniversalPukRequired"));
    QStandardItem *depth2_item38 = new QStandardItem(QString("38:Op.0x222"));
    QStandardItem *depth2_item39 = new QStandardItem(QString("39:Op.0"));

    depth1_parent3->setChild(0, 0, depth2_item30);
    depth1_parent3->setChild(1, 0, depth2_item31);
    depth1_parent3->setChild(2, 0, depth2_item32);
    depth1_parent3->setChild(3, 0, depth2_item33);
    depth1_parent3->setChild(4, 0, depth2_item34);
    depth1_parent3->setChild(5, 0, depth2_item35);
    depth1_parent3->setChild(6, 0, depth2_item36);
    depth1_parent3->setChild(7, 0, depth2_item37);
    depth1_parent3->setChild(8, 0, depth2_item38);
    depth1_parent3->setChild(9, 0, depth2_item39);
    
		RDebug::Printf( "%s %s (%u) Properties depth2-4=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item40 = new QStandardItem(QString("40:KAknKeyguardStatus=8"));
    QStandardItem *depth2_item41 = new QStandardItem(QString("41:Pass=1234"));
    QStandardItem *depth2_item42 = new QStandardItem(QString("42:Pass=12345"));
    QStandardItem *depth2_item43 = new QStandardItem(QString("43:Pass=20499"));
    QStandardItem *depth2_item44 = new QStandardItem(QString("44:Read-Prop"));
    QStandardItem *depth2_item45 = new QStandardItem(QString("45:Read-Prop8"));
    QStandardItem *depth2_item46 = new QStandardItem(QString("46:Stop-Prop8"));
    QStandardItem *depth2_item47 = new QStandardItem(QString("47:EAutolockOff"));
    QStandardItem *depth2_item48 = new QStandardItem(QString("48:EManualLocked"));
    QStandardItem *depth2_item49 = new QStandardItem(QString("49:Uninitialized"));

    depth1_parent4->setChild(0, 0, depth2_item40);
    depth1_parent4->setChild(1, 0, depth2_item41);
    depth1_parent4->setChild(2, 0, depth2_item42);
    depth1_parent4->setChild(3, 0, depth2_item43);
    depth1_parent4->setChild(4, 0, depth2_item44);
    depth1_parent4->setChild(5, 0, depth2_item45);
    depth1_parent4->setChild(6, 0, depth2_item46);
    depth1_parent4->setChild(7, 0, depth2_item47);
    depth1_parent4->setChild(8, 0, depth2_item48);
    depth1_parent4->setChild(9, 0, depth2_item49);
    
		RDebug::Printf( "%s %s (%u) Repository depth2-5=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item50 = new QStandardItem(QString("50:KeyguardTime=0s"));
    QStandardItem *depth2_item51 = new QStandardItem(QString("51:KeyguardTime=10s"));
    QStandardItem *depth2_item52 = new QStandardItem(QString("52:KeyguardTime=30s"));
    QStandardItem *depth2_item53 = new QStandardItem(QString("53:KeyguardT=10+60s"));
    QStandardItem *depth2_item54 = new QStandardItem(QString("54:AutoLockTime=0m"));
    QStandardItem *depth2_item55 = new QStandardItem(QString("55:AutoLockTime=1m"));
    QStandardItem *depth2_item56 = new QStandardItem(QString("56:AutoLockTime=2m"));
    QStandardItem *depth2_item57 = new QStandardItem(QString("57:AutoLockT=65535m"));
    QStandardItem *depth2_item58 = new QStandardItem(QString("58:read"));
    QStandardItem *depth2_item59 = new QStandardItem(QString("59:xxx"));

    depth1_parent5->setChild(0, 0, depth2_item50);
    depth1_parent5->setChild(1, 0, depth2_item51);
    depth1_parent5->setChild(2, 0, depth2_item52);
    depth1_parent5->setChild(3, 0, depth2_item53);
    depth1_parent5->setChild(4, 0, depth2_item54);
    depth1_parent5->setChild(5, 0, depth2_item55);
    depth1_parent5->setChild(6, 0, depth2_item56);
    depth1_parent5->setChild(7, 0, depth2_item57);
    depth1_parent5->setChild(8, 0, depth2_item58);
    depth1_parent5->setChild(9, 0, depth2_item59);
    
		RDebug::Printf( "%s %s (%u) Other depth2-6=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item60 = new QStandardItem(QString("60:Wait30+Cancel_P&S"));
    QStandardItem *depth2_item61 = new QStandardItem(QString("61:Cancel_P&S"));
    QStandardItem *depth2_item62 = new QStandardItem(QString("62:TSecUi::InitializeLibL"));
    QStandardItem *depth2_item63 = new QStandardItem(QString("63:TSecUi::UnInitializeLib"));
    QStandardItem *depth2_item64 = new QStandardItem(QString("64:Wait30+CancelSecCodeQuery"));
    QStandardItem *depth2_item65 = new QStandardItem(QString("65:CancelSecCodeQuery"));
    QStandardItem *depth2_item66 = new QStandardItem(QString("66:EStdKeyDeviceF"));
    QStandardItem *depth2_item67 = new QStandardItem(QString("67:EKeyDeviceF"));
    QStandardItem *depth2_item68 = new QStandardItem(QString("68:EKeyBell"));
    QStandardItem *depth2_item69 = new QStandardItem(QString("69:Stop-iPeriodicExt"));

    depth1_parent6->setChild(0, 0, depth2_item60);
    depth1_parent6->setChild(1, 0, depth2_item61);
    depth1_parent6->setChild(2, 0, depth2_item62);
    depth1_parent6->setChild(3, 0, depth2_item63);
    depth1_parent6->setChild(4, 0, depth2_item64);
    depth1_parent6->setChild(5, 0, depth2_item65);
    depth1_parent6->setChild(6, 0, depth2_item66);
    depth1_parent6->setChild(7, 0, depth2_item67);
    depth1_parent6->setChild(8, 0, depth2_item68);
    depth1_parent6->setChild(9, 0, depth2_item69);
    
		RDebug::Printf( "%s %s (%u) KeyLockPolicy depth2-7=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item70 = new QStandardItem(QString("70:KeyguardAllowed?"));
    QStandardItem *depth2_item71 = new QStandardItem(QString("71:EnableKeyguardFeature"));
    QStandardItem *depth2_item72 = new QStandardItem(QString("72:DisableKeyguardFeature"));
    QStandardItem *depth2_item73 = new QStandardItem(QString("73:ShowErrorCodes"));
    QStandardItem *depth2_item74 = new QStandardItem(QString("74:Expiration+Consecutive"));
    QStandardItem *depth2_item75 = new QStandardItem(QString("75:Minlength+SpecificStrings"));
    QStandardItem *depth2_item76 = new QStandardItem(QString("76:Chars_Numbers"));
    QStandardItem *depth2_item77 = new QStandardItem(QString("77:xxx"));
    QStandardItem *depth2_item78 = new QStandardItem(QString("78:xxx"));
    QStandardItem *depth2_item79 = new QStandardItem(QString("79:xxx"));

    depth1_parent7->setChild(0, 0, depth2_item70);
    depth1_parent7->setChild(1, 0, depth2_item71);
    depth1_parent7->setChild(2, 0, depth2_item72);
    depth1_parent7->setChild(3, 0, depth2_item73);
    depth1_parent7->setChild(4, 0, depth2_item74);
    depth1_parent7->setChild(5, 0, depth2_item75);
    depth1_parent7->setChild(6, 0, depth2_item76);
    depth1_parent7->setChild(7, 0, depth2_item77);
    depth1_parent7->setChild(8, 0, depth2_item78);
    depth1_parent7->setChild(9, 0, depth2_item79);
    
		RDebug::Printf( "%s %s (%u) Schedule depth2-8=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item80 = new QStandardItem(QString("80:xxxx"));
    QStandardItem *depth2_item81 = new QStandardItem(QString("81:xxx"));
    QStandardItem *depth2_item82 = new QStandardItem(QString("82:xxx"));
    QStandardItem *depth2_item83 = new QStandardItem(QString("83:xxx"));
    QStandardItem *depth2_item84 = new QStandardItem(QString("84:xxx"));
    QStandardItem *depth2_item85 = new QStandardItem(QString("85:xxx"));
    QStandardItem *depth2_item86 = new QStandardItem(QString("86:xxx"));
    QStandardItem *depth2_item87 = new QStandardItem(QString("87:xxx"));
    QStandardItem *depth2_item88 = new QStandardItem(QString("88:xxx"));
    QStandardItem *depth2_item89 = new QStandardItem(QString("89:xxx"));

    depth1_parent8->setChild(0, 0, depth2_item80);
    depth1_parent8->setChild(1, 0, depth2_item81);
    depth1_parent8->setChild(2, 0, depth2_item82);
    depth1_parent8->setChild(3, 0, depth2_item83);
    depth1_parent8->setChild(4, 0, depth2_item84);
    depth1_parent8->setChild(5, 0, depth2_item85);
    depth1_parent8->setChild(6, 0, depth2_item86);
    depth1_parent8->setChild(7, 0, depth2_item87);
    depth1_parent8->setChild(8, 0, depth2_item88);
    depth1_parent8->setChild(9, 0, depth2_item89);
    
		RDebug::Printf( "%s %s (%u) Settings2 depth2-9=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    QStandardItem *depth2_item90 = new QStandardItem(QString("90:ChangeSimSecurityL"));
    QStandardItem *depth2_item91 = new QStandardItem(QString("91:ChangePinRequestL"));
    QStandardItem *depth2_item92 = new QStandardItem(QString("92:ChangeUPinRequestL"));
    QStandardItem *depth2_item93 = new QStandardItem(QString("93:SwitchPinCodesL"));
    QStandardItem *depth2_item94 = new QStandardItem(QString("94:ChangePin2L"));
    QStandardItem *depth2_item95 = new QStandardItem(QString("95:SwitchPinCodesL"));
    QStandardItem *depth2_item96 = new QStandardItem(QString("96:ChangeRemoteLockStatusL"));
    QStandardItem *depth2_item97 = new QStandardItem(QString("97:ChangeRLStat-0"));
    QStandardItem *depth2_item98 = new QStandardItem(QString("98:ChangeRLStat-Off"));
    QStandardItem *depth2_item99 = new QStandardItem(QString("99:xxx"));

    depth1_parent9->setChild(0, 0, depth2_item90);
    depth1_parent9->setChild(1, 0, depth2_item91);
    depth1_parent9->setChild(2, 0, depth2_item92);
    depth1_parent9->setChild(3, 0, depth2_item93);
    depth1_parent9->setChild(4, 0, depth2_item94);
    depth1_parent9->setChild(5, 0, depth2_item95);
    depth1_parent9->setChild(6, 0, depth2_item96);
    depth1_parent9->setChild(7, 0, depth2_item97);
    depth1_parent9->setChild(8, 0, depth2_item98);
    depth1_parent9->setChild(9, 0, depth2_item99);
    
    return model;
}

QStandardItemModel *ModelFactory::populateTreeModelSimpleOfSimplest()
{
    QStandardItemModel* model = new QStandardItemModel();

    // =====================================================================
    // Depth 1
    // =====================================================================
    QStandardItem *depth1_item0 = new QStandardItem(QString("Leaf 0"));
    //QStandardItem *depth1_item1 = new QStandardItem(QString("Leaf A-1"));
    QStandardItem *depth1_parent0 = new QStandardItem(QString("Node A"));
    QStandardItem *depth1_parent1 = new QStandardItem(QString("Node B"));

    depth1_parent0->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent1->setData(Hb::ParentItem, Hb::ItemTypeRole);

    model->setItem(0,0,depth1_item0);
    model->setItem(1,0,depth1_parent0);
    model->setItem(2,0,depth1_parent1);

    // =====================================================================
    // Depth 2
    // =====================================================================
    QStandardItem *depth2_item0 = new QStandardItem(QString("Leaf A-1"));
    QStandardItem *depth2_item1 = new QStandardItem(QString("Leaf A-2"));
    QStandardItem *depth2_item2 = new QStandardItem(QString("Leaf B-1"));
    /*QStandardItem *depth2_parent0 = new QStandardItem(QString("Node B-0"));
    QStandardItem *depth2_parent1 = new QStandardItem(QString("Node B-1"));
    QStandardItem *depth2_parent2 = new QStandardItem(QString("Node B-2"));

    depth2_parent0->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth2_parent1->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth2_parent2->setData(Hb::ParentItem, Hb::ItemTypeRole);
*/
    depth1_parent0->setChild(0,0,depth2_item0);
    depth1_parent0->setChild(1, 0, depth2_item1);
    depth1_parent1->setChild(0, 0, depth2_item2);
/*    depth1_parent1->setChild(0, 0, depth2_parent1);
    depth1_parent1->setChild(1, 0, depth2_parent2);

    // =====================================================================
    // Depth 3
    // =====================================================================
    QStandardItem *depth3_item0 = new QStandardItem(QString("Leaf C-0"));
    QStandardItem *depth3_item1 = new QStandardItem(QString("Leaf C-1"));
    QStandardItem *depth3_item2 = new QStandardItem(QString("Leaf C-2"));
    QStandardItem *depth3_item3 = new QStandardItem(QString("Leaf C-3"));
    QStandardItem *depth3_item4 = new QStandardItem(QString("Leaf C-4"));
    QStandardItem *depth3_item5 = new QStandardItem(QString("Leaf C-5"));

    depth2_parent0->setChild(0, 0, depth3_item0);
    depth2_parent0->setChild(1, 0, depth3_item1);
    depth2_parent1->setChild(0, 0, depth3_item2);
    depth2_parent2->setChild(0, 0, depth3_item3);
    depth2_parent2->setChild(1, 0, depth3_item4);
    depth2_parent2->setChild(2, 0, depth3_item5);
*/
    return model;
}

QStandardItemModel *ModelFactory::populateGreenOddBrownEvenModel()
{
    QStandardItemModel* model = new QStandardItemModel();

    // =====================================================================
    // Depth 1
    // =====================================================================

    QStandardItem *depth1_item0 = new QStandardItem();
    QStringList data;
    data << "Odd numbered items are green" << "Even numbered items are brown";
    depth1_item0->setData(QVariant(data), Qt::DisplayRole);

    QStandardItem *depth1_item1 = new QStandardItem(QString("1"));
    QStandardItem *depth1_item2 = new QStandardItem(QString("Item 2"));
    QStandardItem *depth1_item3 = new QStandardItem(QString("Item 3"));
    QStandardItem *depth1_item4 = new QStandardItem(QString("4"));
    QStandardItem *depth1_item5 = new QStandardItem(QString("5"));
    QStandardItem *depth1_parent0 = new QStandardItem(QString("Parent 1"));
    QStandardItem *depth1_parent1 = new QStandardItem(QString("Parent 2"));

    depth1_parent0->setData(Hb::ParentItem, Hb::ItemTypeRole);
    depth1_parent1->setData(Hb::ParentItem, Hb::ItemTypeRole);

    model->setItem(0,0,depth1_item0);
    model->setItem(1,0,depth1_item1);
    model->setItem(2,0,depth1_item2);
    model->setItem(3,0,depth1_item3);
    model->setItem(4,0,depth1_parent0);
    model->setItem(5,0,depth1_parent1);
    model->setItem(6,0,depth1_item4);
    model->setItem(7,0,depth1_item5);

    // =====================================================================
    // Depth 2
    // =====================================================================
    QStandardItem *depth2_item0 = new QStandardItem(QString("11"));
    QStandardItem *depth2_item1 = new QStandardItem(QString("12"));
    QStandardItem *depth2_item2 = new QStandardItem(QString("Item 13"));
    QStandardItem *depth2_item4 = new QStandardItem(QString("Item 14"));
    QStandardItem *depth2_item5 = new QStandardItem(QString("15"));
    QStandardItem *depth2_item6 = new QStandardItem(QString("16"));
    QStandardItem *depth2_item7 = new QStandardItem(QString("17"));
    QStandardItem *depth2_parent0 = new QStandardItem(QString("Parent 11"));

    depth2_parent0->setData(Hb::ParentItem, Hb::ItemTypeRole);

    depth1_parent0->setChild(0, 0, depth2_item0);
    depth1_parent0->setChild(1, 0, depth2_item1);
    depth1_parent0->setChild(2, 0, depth2_item2);
    depth1_parent0->setChild(3, 0, depth2_parent0);

    depth1_parent1->setChild(0, 0, depth2_item4);
    depth1_parent1->setChild(1, 0, depth2_item5);
    depth1_parent1->setChild(2, 0, depth2_item6);
    depth1_parent1->setChild(3, 0, depth2_item7);

    // =====================================================================
    // Depth 3
    // =====================================================================
    QStandardItem *depth3_item0 = new QStandardItem(QString("21"));
    QStandardItem *depth3_item1 = new QStandardItem(QString("22"));
    QStandardItem *depth3_item2 = new QStandardItem(QString("Item 23"));
    QStandardItem *depth3_item3 = new QStandardItem(QString("Item 24"));
    QStandardItem *depth3_item4 = new QStandardItem(QString("25"));
    QStandardItem *depth3_item5 = new QStandardItem(QString("26"));

    depth2_parent0->setChild(0, 0, depth3_item0);
    depth2_parent0->setChild(1, 0, depth3_item1);
    depth2_parent0->setChild(2, 0, depth3_item2);
    depth2_parent0->setChild(3, 0, depth3_item3);
    depth2_parent0->setChild(4, 0, depth3_item4);
    depth2_parent0->setChild(5, 0, depth3_item5);

    return model;
}


QStandardItemModel *ModelFactory::populateTreeModelDeep()
{
    QStandardItemModel* model = new QStandardItemModel();

    QStandardItem *root = new QStandardItem;
    model->setItem(0,0,root);

    QStandardItem *parent = root;
    const int maxDeep = 50;
    for (int current = 1; current <= maxDeep; ++current) {
        parent->setData(Hb::ParentItem, Hb::ItemTypeRole);
        parent->setText(QString("Parent %0").arg(current));

        for (int current2 = 0; current2 < 5; ++current2) {
                QStandardItem *item = new QStandardItem;
                item->setText(QString("Item %0").arg(current2));
                parent->setChild(current2, 0, item);
        }
        for (int current2 = 5; current2 < 10; ++current2) {
            QStandardItem *item = new QStandardItem;
            item->setText(QString("Item %0").arg(current2));
            parent->setChild(current2, 0, item);

            QStandardItem *itemPrevious = item;
            for (int current3 = current+1; current3 < maxDeep - (current2-5)*maxDeep/5; ++current3) {
                itemPrevious->setData(Hb::ParentItem, Hb::ItemTypeRole);
                QStandardItem *item2 = new QStandardItem;
                item2->setText(QString("Folder %0").arg(current3));
                itemPrevious->setChild(0, 0, item2);

                QStandardItem *itemTemp = new QStandardItem;
                itemTemp->setText(QString("Folder %0 item").arg(current3));
                itemPrevious->setChild(1, 0, itemTemp);

                itemPrevious = item2;
            }
            itemPrevious->setData(Hb::ParentItem, Hb::ItemTypeRole);
            QStandardItem *itemTemp = new QStandardItem;
            itemTemp->setText(QString("Folder 9 item"));
            itemPrevious->setChild(0, 0, itemTemp);
        }

        if(current < maxDeep) {
            QStandardItem *item = new QStandardItem;
            parent->setChild(10, 0, item);
            parent = item;
        }
    }
    return model;
}

QStandardItemModel *ModelFactory::populateTreeModelFlat()
{
    QStandardItemModel* model = new QStandardItemModel();
    for (int i = 0; i < 1000; ++i) {
        QStandardItem* item = new QStandardItem(QString("Item number %1").arg(i));
        model->appendRow(item);
    }
    return model;
}

QStandardItemModel *ModelFactory::populateTreeModelMixed()
{
    QStandardItemModel* model = new QStandardItemModel();

    QStandardItem *parent = model->invisibleRootItem();
    insertMixedItems(parent);

    for (int current = 1; current <= 10; ++current) {
        QStandardItem *newParent = new QStandardItem;
        parent->appendRow(newParent);
        parent = newParent;

        parent->setData(Hb::ParentItem, Hb::ItemTypeRole);
        parent->setText(QString("Parent %0").arg(current));

        insertMixedItems(parent);
    }

    return model;
}



