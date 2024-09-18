#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ConfigLoader.h"
#include "ATmegaSerialProg.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define CONFIG_FILENAME           "config.json"
#define MAX_CONSOLETEXT_SIZE      40000

enum GO_STEPS
{
    STEP_IDLE,
    STEP_CHIP_ERASE,
    STEP_FLASH_PROGRAMM,
    STEP_FLASH_READ,
    STEP_FLASH_CHECK,
    STEP_EEPROM_PROGRAMM,
    STEP_EEPROM_READ,
    STEP_EEPROM_CHECK,
    STEP_FUSE_AND_LOCK_PROGRAMM,
    STEP_GO_COMPLETE
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(CConfigLoader &config, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow               *ui;

    CConfigLoader                 m_Config;

    CATmegaSerialProg             m_SerialProg;

    QByteArray                    m_WriteFLASHdata;
    QByteArray                    m_ReadFLASHdata;

    QByteArray                    m_WriteEEPROMdata;
    QByteArray                    m_ReadEEPROMdata;

    int                           m_GOstep;
    QTimer                        m_tGO_Timer;

    QString                       m_ConsoleOut;

    void StopProgramming(void);
    void ConsoleOut(QString baData);

private slots:

    void PushButtonGOclickedSlot(void);
    void CheckBoxDataCompareSlot(int state);
    void CheckBoxLockChipSlot(int state);
    void GO_TimerSlot(void);
    void SpiErrorSlot(void);
    void AllDataProgrammingCompletedSlot(void);
    void PageProgrammingCompletedSlot(int index);
    void ViewHelpSlot(void);
    void ViewAboutSlot(void);

};
#endif // MAINWINDOW_H
