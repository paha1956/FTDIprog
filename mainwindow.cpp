#include <QThread>
#include <QTextCursor>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "HelpView.h"

#include "IntelHEXparcer.h"

MainWindow::MainWindow(CConfigLoader &config, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_Config = config;

    setWindowTitle(QString::fromLocal8Bit("Программатор цифрового бесконтактного амперметра"));
    setWindowFlags(Qt::Widget | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    setFixedSize(size());

    ui->checkBoxDataCompare->setTristate(false);
    if (m_Config.getMemoryTestFlag()) { ui->checkBoxDataCompare->setCheckState(Qt::Checked);
                                      }
    else                              { ui->checkBoxDataCompare->setCheckState(Qt::Unchecked);
                                      }

    ui->checkBoxLockChip->setTristate(false);
    if (m_Config.getMemoryLockFlag()) { ui->checkBoxLockChip->setCheckState(Qt::Checked);
                                      }
    else                              { ui->checkBoxLockChip->setCheckState(Qt::Unchecked);
                                      }

    ui->labelConsole->clear();

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    m_ConsoleOut.clear();
    ui->plainTextEditConsole->clear();
    ui->plainTextEditConsole->setReadOnly(true);

    m_GOstep = STEP_IDLE;

    connect(ui->pushButtonGO,        &QPushButton::clicked,    this, &MainWindow::PushButtonGOclickedSlot);
    connect(ui->checkBoxDataCompare, &QCheckBox::stateChanged, this, &MainWindow::CheckBoxDataCompareSlot);
    connect(ui->checkBoxLockChip,    &QCheckBox::stateChanged, this, &MainWindow::CheckBoxLockChipSlot);

    connect(&m_SerialProg, &CATmegaSerialProg::spi_alldata_programming_completed_signal, this, &MainWindow::AllDataProgrammingCompletedSlot);
    connect(&m_SerialProg, &CATmegaSerialProg::spi_page_programming_completed_signal,    this, &MainWindow::PageProgrammingCompletedSlot);
    connect(&m_SerialProg, &CATmegaSerialProg::spi_error_signal,                         this, &MainWindow::SpiErrorSlot);

    connect(ui->actionHelp,  &QAction::triggered, this, &MainWindow::ViewHelpSlot);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::ViewAboutSlot);

    ConsoleOut(QString::fromLocal8Bit("Программатор цифрового бесконтактного амперметра.\n"
                                      "г. Санкт-Петербург, 2024\n\n\n"));

    QList<QString> errors_list = m_Config.getErrorsList();
    if (errors_list.size() != 0)
    {
        foreach (QString error_key, errors_list)
        {
            ConsoleOut(error_key + "\n");
        }
    }

    m_tGO_Timer.setSingleShot(true);
    connect(&m_tGO_Timer, &QTimer::timeout, this, &MainWindow::GO_TimerSlot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::PushButtonGOclickedSlot(void)
{
    if (m_GOstep != STEP_IDLE)
    {
        StopProgramming();

        ConsoleOut(QString::fromLocal8Bit("\nПроцесс программирования остановлен\n\n\n"));
        ui->labelConsole->setText(QString::fromLocal8Bit("Процесс программирования остановлен"));
        ui->progressBar->setValue(0);
        return;
    }

    ui->pushButtonGO->setText("Stop");

    enum SPI_EVENTS spi_status = m_SerialProg.Init();
    switch (spi_status)
    {
    case SPI_ERROR:
    {   ConsoleOut(QString::fromLocal8Bit("Ошибка инициализации ATmega!\n"));
        StopProgramming();
        return;
    }

    default:
    case SPI_NOFTDI:
    {   ConsoleOut(QString::fromLocal8Bit("Нет доступного интерфейса FTDI!\n"));
        StopProgramming();
        return;
    }

    case SPI_OK: break;
    }

    ConsoleOut(QString::fromLocal8Bit("Инициализации ATmega... Ok\n"));

    struct DEVICE_INFO DeviceInfo = m_SerialProg.ReadDeviceInfo();
    if (DeviceInfo.status != SPI_OK)
    {
        ConsoleOut(QString::fromLocal8Bit("Ошибка чтения сигнатуры\n"));
        StopProgramming();
        return;
    }
    ConsoleOut(QString::fromLocal8Bit("Чтение сигнатуры... Ok [%0: %1]\n").arg(QString(DeviceInfo.signature.toHex(' '))).arg(DeviceInfo.name));

    ConsoleOut(QString::fromLocal8Bit("Файл данных FLASH: %0\n").arg(m_Config.getFLASHfileName()));
    QFile File(m_Config.getFLASHfileName());
    if (!File.open(QIODevice::ReadOnly))
    {
        ConsoleOut(QString::fromLocal8Bit("Ошибка открытия файла FLASH %0\n").arg(File.fileName()));
        StopProgramming();
        return;
    }

    QByteArray FileData = File.readAll();
    File.close();

    CIntelHEXparcer HEXparcer(FileData, DeviceInfo.flash_size);
    QMap<int, QString> errors_list = HEXparcer.getErrorsList();
    if (errors_list.size() != 0)
    {
        foreach (int error_key, errors_list.keys())
        {
            ConsoleOut(QString::fromLocal8Bit("Ошибка HEX данных FLASH, строка %0: %1").arg(error_key).arg(errors_list.value(error_key)));
        }

        StopProgramming();
        return;
    }
    else
    {
        ConsoleOut(QString::fromLocal8Bit("Чтение файла FLASH... Ok\n"));
    }

    m_WriteFLASHdata = HEXparcer.getRAWdata();
    m_GOstep = STEP_CHIP_ERASE;
    m_tGO_Timer.start(1);
}

void MainWindow::CheckBoxDataCompareSlot(int state)
{
    if (m_GOstep == STEP_EEPROM_PROGRAMM)
    {
        if (state == Qt::Checked)
        {   m_GOstep = STEP_FLASH_READ;
        }
    }
}

void MainWindow::CheckBoxLockChipSlot(int state)
{
    if (state == Qt::Checked)
    {
        QMessageBox *pMsg = new QMessageBox(this);

        pMsg->setWindowTitle(QString::fromLocal8Bit("Подтверждение защиты чипа"));
        pMsg->setIcon(QMessageBox::Question);
        pMsg->setText(QString::fromLocal8Bit("Защитить FLASH память чипа от копирования?"));
        pMsg->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        pMsg->setDefaultButton(QMessageBox::Cancel);
        pMsg->button(QMessageBox::Yes)->setText(QString::fromLocal8Bit("Да"));
        pMsg->button(QMessageBox::Cancel)->setText(QString::fromLocal8Bit("Отмена"));

        int res = pMsg->exec();

        if (res != QMessageBox::Yes)
        {
            ui->checkBoxLockChip->setCheckState(Qt::Unchecked);
        }
    }
}

void MainWindow::GO_TimerSlot(void)
{
    switch (m_GOstep)
    {
    case STEP_CHIP_ERASE:
    {
        m_SerialProg.ChipErase();
        m_tGO_Timer.start(100);
        m_GOstep = STEP_FLASH_PROGRAMM;
        break;
    }

    case STEP_FLASH_PROGRAMM:
    {
        ui->labelConsole->setText(QString::fromLocal8Bit("Запись FLASH..."));
        ConsoleOut(QString::fromLocal8Bit("Запись FLASH... "));
        ui->progressBar->setValue(0);
        m_SerialProg.WriteFLASH(0, &m_WriteFLASHdata);

        if (ui->checkBoxDataCompare->checkState() == Qt::Checked)
        {   m_GOstep = STEP_FLASH_READ;
        }
        else
        {   m_GOstep = STEP_EEPROM_PROGRAMM;
        }

        break;
    }

    case STEP_FLASH_READ:
    {
        ui->checkBoxDataCompare->setEnabled(false);

        ui->labelConsole->setText(QString::fromLocal8Bit("Чтение FLASH..."));
        ConsoleOut(QString::fromLocal8Bit("Чтение FLASH... "));
        ui->progressBar->setValue(0);
        m_ReadFLASHdata.clear();
        m_SerialProg.ReadFLASH(0, &m_ReadFLASHdata, m_WriteFLASHdata.size());
        m_GOstep = STEP_FLASH_CHECK;
        break;
    }

    case STEP_FLASH_CHECK:
    {
        ConsoleOut(QString::fromLocal8Bit("Проверка FLASH... "));

        bool compare_flag = true;
        for (unsigned short data_index = 0; data_index < m_WriteFLASHdata.size(); data_index++)
        {
            if (m_WriteFLASHdata.at(data_index) != m_ReadFLASHdata.at(data_index))
            {
                compare_flag = false;
                QByteArray index;
                index.append((data_index >> 24) & 0xff);
                index.append((data_index >> 16) & 0xff);
                index.append((data_index >> 8)  & 0xff);
                index.append( data_index        & 0xff);
                ConsoleOut(QString::fromLocal8Bit("\n%0: wr=0x%1 rd=0x%2").arg(QString::fromLocal8Bit(index.toHex())).arg(QString::fromLocal8Bit(m_WriteFLASHdata.mid(data_index, 1).toHex())).arg(QString::fromLocal8Bit(m_ReadFLASHdata.mid(data_index, 1).toHex())));
            }
        }

        if (compare_flag)
        {   ConsoleOut(QString::fromLocal8Bit("Ok\n"));
        }
        else
        {   ConsoleOut(QString::fromLocal8Bit("\nПрочитанные из FLASH данные не соответствуют записанным!\n"));
        }

        m_GOstep = STEP_EEPROM_PROGRAMM;
        m_tGO_Timer.start(10);
        break;
    }

    case STEP_EEPROM_PROGRAMM:
    {
        if (m_Config.getEEPROMfileName().isEmpty())
        {
            ConsoleOut(QString::fromLocal8Bit("Запись EEPROM... файл данных EEPROM не определён. Запись EEPROM не осуществляется\n"));

            m_GOstep = STEP_FUSE_AND_LOCK_PROGRAMM;
            m_tGO_Timer.start(10);
        }
        else
        {
            ConsoleOut(QString::fromLocal8Bit("Файл данных EEPROM: %0\n").arg(m_Config.getEEPROMfileName()));
            QFile File(m_Config.getEEPROMfileName());
            if (!File.open(QIODevice::ReadOnly))
            {
                ConsoleOut(QString::fromLocal8Bit("Запись EEPROM... ошибка открытия файла EEPROM %0\n").arg(File.fileName()));

                m_GOstep = STEP_FUSE_AND_LOCK_PROGRAMM;
                m_tGO_Timer.start(10);
                return;
            }

            QByteArray FileData = File.readAll();
            File.close();

            CIntelHEXparcer HEXparcer(FileData, m_SerialProg.getDeviceInfo().eeprom_size, 0xff, false);
            QMap<int, QString> errors_list = HEXparcer.getErrorsList();
            if (errors_list.size() != 0)
            {
                foreach (int error_key, errors_list.keys())
                {
                    ConsoleOut(QString::fromLocal8Bit("Ошибка HEX данных EEPROM, строка %0: %1").arg(error_key).arg(errors_list.value(error_key)));
                }

                m_GOstep = STEP_FUSE_AND_LOCK_PROGRAMM;
                m_tGO_Timer.start(10);
                break;
            }
            else
            {
                ConsoleOut(QString::fromLocal8Bit("Чтение файла EEPROM... Ok\n"));
            }

            m_WriteEEPROMdata = HEXparcer.getRAWdata();

            ui->labelConsole->setText(QString::fromLocal8Bit("Запись EEPROM..."));
            ConsoleOut(QString::fromLocal8Bit("Запись EEPROM... "));
            ui->progressBar->setValue(0);
            m_SerialProg.WriteEEPROM(0, &m_WriteEEPROMdata);

            if (ui->checkBoxDataCompare->checkState() == Qt::Checked)
            {   m_GOstep = STEP_EEPROM_READ;
            }
            else
            {   m_GOstep = STEP_FUSE_AND_LOCK_PROGRAMM;
            }
        }

        break;
    }

    case STEP_EEPROM_READ:
    {
        ui->labelConsole->setText(QString::fromLocal8Bit("Чтение EEPROM..."));
        ConsoleOut(QString::fromLocal8Bit("Чтение EEPROM... "));
        ui->progressBar->setValue(0);
        m_ReadEEPROMdata.clear();
        m_SerialProg.ReadEEPROM(0, &m_ReadEEPROMdata, m_WriteEEPROMdata.size());
        m_GOstep = STEP_EEPROM_CHECK;
        break;
    }

    case STEP_EEPROM_CHECK:
    {
        ConsoleOut(QString::fromLocal8Bit("Проверка EEPROM... "));

        bool compare_flag = true;
        for (unsigned short data_index = 0; data_index < m_WriteEEPROMdata.size(); data_index++)
        {
            if (m_WriteEEPROMdata.at(data_index) != m_ReadEEPROMdata.at(data_index))
            {
                compare_flag = false;
                QByteArray index;
                index.append((data_index >> 8)  & 0xff);
                index.append( data_index        & 0xff);
                ConsoleOut(QString::fromLocal8Bit("\n%0: wr=0x%1 rd=0x%2").arg(QString::fromLocal8Bit(index.toHex())).arg(QString::fromLocal8Bit(m_WriteEEPROMdata.mid(data_index, 1).toHex())).arg(QString::fromLocal8Bit(m_ReadEEPROMdata.mid(data_index, 1).toHex())));
            }
        }

        if (compare_flag)
        {   ConsoleOut(QString::fromLocal8Bit("Ok\n"));
        }
        else
        {   ConsoleOut(QString::fromLocal8Bit("\nПрочитанные из EEPROM данные не соответствуют записанным!\n"));
        }

        m_GOstep = STEP_FUSE_AND_LOCK_PROGRAMM;
        m_tGO_Timer.start(10);
        break;
    }

    case STEP_FUSE_AND_LOCK_PROGRAMM:
    {
        QByteArray WrFuses(FUSE_BYTES_NUMBER, 0);
        WrFuses[LOW_FUSE_BYTE_INDEX] = m_Config.getLowFuseByte();
        WrFuses[HI_FUSE_BYTE_INDEX]  = m_Config.getHiFuseByte();
        WrFuses[EXT_FUSE_BYTE_INDEX] = m_Config.getExtFuseByte();
        ConsoleOut(QString::fromLocal8Bit("Запись FUSE байт [%0]... ").arg(QString::fromLocal8Bit(WrFuses.toHex(' '))));

        if (m_SerialProg.WriteFuseBytes(WrFuses) == SPI_OK)
        {
            QByteArray RdFuses;
            if (m_SerialProg.ReadFuseBytes(RdFuses) == SPI_OK)
            {
                if (WrFuses.compare(RdFuses) == 0)
                {   ConsoleOut(QString::fromLocal8Bit("Ok\n"));
                }
                else
                {   ConsoleOut(QString::fromLocal8Bit("Ошибка проверки записи FUSE байт [%0] -> [%1]\n").arg(QString::fromLocal8Bit(WrFuses.toHex(' '))).arg(QString::fromLocal8Bit(RdFuses.toHex(' '))));
                }
            }
            else
            {   ConsoleOut(QString::fromLocal8Bit("Ошибка при чтении FUSE байт\n"));
            }
        }
        else
        {   ConsoleOut(QString::fromLocal8Bit("Ошибка при записи FUSE байт\n"));
        }

        if (ui->checkBoxLockChip->isChecked())
        {
            ConsoleOut(QString::fromLocal8Bit("Защита FLASH памяти и EEPROM от копирования..."));

            if (m_SerialProg.WriteLockBits(DEFAULT_CHIP_LOCK_BITS) == SPI_OK)
            {   ConsoleOut(QString::fromLocal8Bit("Ok\n"));
            }
            else
            {   ConsoleOut(QString::fromLocal8Bit("Ошибка записи lock-бит\n"));
            }
        }

        m_GOstep = STEP_GO_COMPLETE;
        m_tGO_Timer.start(10);
        break;
    }

    case STEP_GO_COMPLETE:
    {
        ui->checkBoxDataCompare->setEnabled(true);

        ui->labelConsole->setText(QString::fromLocal8Bit("Программирование устройства завершено!"));
        ConsoleOut(QString::fromLocal8Bit("Программирование устройства завершено!\n\n\n"));

        StopProgramming();
        break;
    }
    }
}

void MainWindow::SpiErrorSlot(void)
{
    ui->labelConsole->setText(QString::fromLocal8Bit("Ошибка связи с устройством!"));
    ConsoleOut(QString::fromLocal8Bit("Ошибка связи с устройством!\n\n"));
    StopProgramming();
}

void MainWindow::AllDataProgrammingCompletedSlot(void)
{
    ConsoleOut(QString::fromLocal8Bit("Ok\n"));

    ui->progressBar->setValue(100);

    m_tGO_Timer.start(1);
}

void MainWindow::PageProgrammingCompletedSlot(int index)
{
    ui->progressBar->setValue(index);
}

void MainWindow::ViewHelpSlot(void)
{
    CHelpView *pHelpWindow = new CHelpView(this);

    pHelpWindow->setWindowTitle(QString::fromLocal8Bit("Справочная информация"));
    pHelpWindow->show();
}

void MainWindow::ViewAboutSlot(void)
{
    QMessageBox *pMsg = new QMessageBox(this);

    pMsg->setWindowTitle(QString::fromLocal8Bit("О программе"));
    pMsg->setStyleSheet("background-color: whitesmoke");
    pMsg->setIcon(QMessageBox::Information);
    pMsg->setText(QString::fromLocal8Bit("<p align='center'>FTDI USB-RS232 программатор<br>"
                                         "цифрового бесконтактного амперметра<br><br>"
                                         "Санкт-Петербург<br>"
                                         "2024<br>"));
    pMsg->setStandardButtons(QMessageBox::Close);
    pMsg->setDefaultButton(QMessageBox::Close);
    pMsg->button(QMessageBox::Close)->setText(QString::fromLocal8Bit("Закрыть"));

    pMsg->exec();
}

void MainWindow::StopProgramming(void)
{
    ui->pushButtonGO->setText("GO!");
    ui->checkBoxDataCompare->setEnabled(true);

    m_SerialProg.OperationCancel();
    m_SerialProg.Deinit();

    m_GOstep = STEP_IDLE;
    m_tGO_Timer.stop();
}

void MainWindow::ConsoleOut(QString Data)
{
    m_ConsoleOut.append(Data);
    if (m_ConsoleOut.size() > MAX_CONSOLETEXT_SIZE)
        m_ConsoleOut.remove(0, m_ConsoleOut.size() - MAX_CONSOLETEXT_SIZE);

    ui->plainTextEditConsole->setPlainText(m_ConsoleOut);
    ui->plainTextEditConsole->moveCursor (QTextCursor::End);
}

