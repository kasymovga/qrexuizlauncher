#ifndef DIALOGSELECTINSTALLLOCATION_H
#define DIALOGSELECTINSTALLLOCATION_H

#include <QObject>
#include <QDialog>

namespace Ui {
    class DialogSelectInstallLocation;
}

class DialogSelectInstallLocation : public QDialog
{
    Q_OBJECT
public:
    explicit DialogSelectInstallLocation(QWidget *parent = 0);
    ~DialogSelectInstallLocation();
    Ui::DialogSelectInstallLocation *ui;
public slots:
    void selectDirectory();
};

#endif // DIALOGSELECTINSTALLLOCATION_H
