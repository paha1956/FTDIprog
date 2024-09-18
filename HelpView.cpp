#include <QPushButton>

#include "HelpView.h"
#include "ui_HelpView.h"

CHelpView::CHelpView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CHelpView)
{
    ui->setupUi(this);

    ui->buttonCloseBox->button(QDialogButtonBox::Close)->setText(QString::fromLocal8Bit("Закрыть"));
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    ui->plainHelpText->setReadOnly(true);

    QByteArray HelpText;

    QFile HelpFile(":/documents/help.htm");
    if (!HelpFile.open(QIODevice::ReadOnly))
    {
        HelpText = "<b>Нет данных</b>";
    }
    else
    {
        HelpText = HelpFile.readAll();
    }

    HelpFile.close();

    ui->plainHelpText->appendHtml(QString::fromLocal8Bit(HelpText));
    ui->plainHelpText->moveCursor(QTextCursor::Start);
}

CHelpView::~CHelpView()
{
    delete ui;
}
