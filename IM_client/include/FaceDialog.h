#ifndef FACEDIALOG_H
#define FACEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QMouseEvent>
#include "include/MacroDefine.h"
#define ROLE_FACE_NAME Qt::UserRole + 6

namespace Ui {
class FaceDialog;
}

class FaceDialog : public QDialog        
{
    Q_OBJECT

public:
    explicit FaceDialog(QWidget *parent = 0, int type = CHOOSE_FACE);
    ~FaceDialog();

    void initFaces(QString imgDir);
signals:
    void select_face(QString);
    void select_avatar(QString);
private slots:
    void getItem(QTableWidgetItem*);
private:
    Ui::FaceDialog *ui;
    const static int NUMPER_ROW = 8;
    int m_type;
};

#endif // FACEDIALOG_H
