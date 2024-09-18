#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <QDialog>

namespace Ui {
class CHelpView;
}

class CHelpView : public QDialog
{
    Q_OBJECT

public:
    explicit CHelpView(QWidget *parent = nullptr);
    ~CHelpView();

private:
    Ui::CHelpView *ui;
};

#endif // HELPVIEW_H
