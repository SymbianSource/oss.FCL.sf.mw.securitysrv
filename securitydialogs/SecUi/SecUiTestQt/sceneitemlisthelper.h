//
// Little helper class for populating lists with Hb::SceneItems
//

#ifndef SCENEITEMLISTHELPER_H
#define SCENEITEMLISTHELPER_H

#include <hbnamespace.h>

Q_DECLARE_METATYPE(Hb::SceneItem);

static struct { Hb::SceneItem sceneItem; QString name; } listHelperSceneItems[] =
        {
        {Hb::NoItem, "NoItem"},
        {Hb::ToolBarItem, "ToolBarItem"},
        {Hb::AllItems, "AllItems"},
    };


//
// SceneItemListHelper
//
class SceneItemListHelper
{
public:
        
    static int sceneItemStringsCount(){
        return sizeof(listHelperSceneItems) / sizeof(listHelperSceneItems[0]);
    }

    static void initListWidget(QListWidget *listWidget){
        for(int i = 0; i < SceneItemListHelper::sceneItemStringsCount(); i++ ) {
            // create list widget item and store the sceneItem with the sceneItem name
            QListWidgetItem* item = new QListWidgetItem(listHelperSceneItems[i].name);
            item->setData(Qt::UserRole,QVariant::fromValue(listHelperSceneItems[i].sceneItem));
            item->setSelected( 
                listHelperSceneItems[i].sceneItem == Hb::NoItem ? 
                true : false);
            
        listWidget->addItem (item);
        }        
    }
    
    static Hb::SceneItems sceneItems(const QListWidget *listwidget){
        Hb::SceneItems sceneItems = Hb::NoItem;
    
        for (int i = 0; i < listwidget->count(); i++)   {
            if (listwidget->item(i)->isSelected()) {
                sceneItems |= listwidget->item(i)->data(Qt::UserRole).value<Hb::SceneItem>();
            }
        }
        return sceneItems;
    }
};

#endif //SCENEITEMLISTHELPER_H

