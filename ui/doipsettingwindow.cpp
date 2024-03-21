#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>

#include "doipsettingwindow.h"
#include "ui_doipsettingwindow.h"
#include "doip/doipclientconfig.h"
#include "doip/doipserverconfig.h"

DoIPSettingWindow::DoIPSettingWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoIPSettingWindow)
{
    ui->setupUi(this);

    initWindow();
}

DoIPSettingWindow::~DoIPSettingWindow()
{
    delete ui;
}

void DoIPSettingWindow::initWindow()
{
    using namespace figkey;
    // Fetch the singleton configuration object
    auto& client = DoIPClientConfig::Instance();
    auto& server = DoIPServerConfig::Instance();

    // Define basic table properties
    // Add 1 to rowCount for the new parameter useOEMSpecific
    ui->tableWidget->setRowCount(13);
    ui->tableWidget->setColumnCount(2);

    // Display the label of each configuration parameter.
    QStringList headers;
    headers << "Name" << "Value";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // Add configuration item labels
    QStringList configLabels = {"Version", "Source Address", "Target Address",
                                "Require Routing Activation", "Alive Check Response", "Use OEM Specific",
                                "Additional OEM Specific", "Future Standardization", "Routing Activation Wait Time",
                                "Message Control Time", "Entity ID (EID)", "Group ID (GID)",
                                "Vehicle Identifiy Number (VIN)"};
    for(int row = 0; row < ui->tableWidget->rowCount() && row < configLabels.size(); row++) {
        QTableWidgetItem* labelItem = new QTableWidgetItem(configLabels[row]);
        ui->tableWidget->setItem(row, 0, labelItem);
    }

    // Set version
    QSpinBox* versionBox = new QSpinBox(this);
    versionBox->setRange(0, 254);  //range 0x00~0xFE
    versionBox->setValue(client.getVersion());
    ui->tableWidget->setCellWidget(0, 1, versionBox);
    connect(versionBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [](int newValue){ DoIPClientConfig::Instance().setVersion(newValue); });

    // Set sourceAddress rang  { 0x0e00~0x0eff }
    QLineEdit *sourceAddressEdit = new QLineEdit(this);
    sourceAddressEdit->setValidator(new QRegExpValidator(QRegExp("^[0][Ee][0-9a-fA-F]{2}$"), this));
    sourceAddressEdit->setText(QString("%1").arg(client.getSourceAddress(), 4, 16, QChar('0')));
    ui->tableWidget->setCellWidget(1, 1, sourceAddressEdit);
    connect(sourceAddressEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPClientConfig::Instance().setSourceAddress(newText.toUpper().toUShort(nullptr, 16)); });

    // Set targetAddress not in  { 0x0e00~0x0eff }
    QLineEdit *targetAddressEdit = new QLineEdit(this);
    targetAddressEdit->setValidator(new QRegExpValidator(QRegExp("^(?![0][Ee][0-9a-fA-F]{2}$)[0-9a-fA-F]{4}$"), this));
    targetAddressEdit->setText(QString("%1").arg(client.getTargetAddress(), 4, 16, QChar('0')));
    ui->tableWidget->setCellWidget(2, 1, targetAddressEdit);
    connect(targetAddressEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPClientConfig::Instance().setTargetAddress(newText.toUpper().toUShort(nullptr, 16)); });

    // Add requireRoutingActivation
    QCheckBox *requireRoutingActivationBox = new QCheckBox(this);
    requireRoutingActivationBox->setChecked(client.getRequireRoutingActivation());
    ui->tableWidget->setCellWidget(3, 1, requireRoutingActivationBox);
    connect(requireRoutingActivationBox, &QCheckBox::stateChanged,
            [](int state){ DoIPClientConfig::Instance().setRequireRoutingActivation(state == Qt::Checked); });

    // Add aliveCheckResponse
    QCheckBox *aliveCheckResponseBox = new QCheckBox(this);
    aliveCheckResponseBox->setChecked(client.getAliveCheckResponse());
    ui->tableWidget->setCellWidget(4, 1, aliveCheckResponseBox);
    connect(aliveCheckResponseBox, &QCheckBox::stateChanged,
            [](int state){ DoIPClientConfig::Instance().setAliveCheckResponse(state == Qt::Checked); });

    // Add useOEMSpecific
    QCheckBox *useOEMSpecificBox = new QCheckBox(this);
    useOEMSpecificBox->setChecked(client.getUseOEMSpecific());
    ui->tableWidget->setCellWidget(5, 1, useOEMSpecificBox);
    connect(useOEMSpecificBox, &QCheckBox::stateChanged,
            [](int state){ DoIPClientConfig::Instance().setUseOEMSpecific(state == Qt::Checked); });

    // Set additionalOEMSpecific and futureStandardization
    QRegExp hex8RegExp("^[0-9a-fA-F]{8}$");

    QLineEdit *additionalOEMSpecificEdit = new QLineEdit(this);
    additionalOEMSpecificEdit->setText(client.getAdditionalOEMSpecific().toHex(' '));  // Added spaced between bytes
    ui->tableWidget->setCellWidget(6, 1, additionalOEMSpecificEdit);
    connect(additionalOEMSpecificEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPClientConfig::Instance().setAdditionalOEMSpecific(QByteArray::fromHex(newText.simplified().toUtf8())); });

    QLineEdit *futureStandardizationEdit = new QLineEdit(this);
    futureStandardizationEdit->setText(client.getFutureStandardization().toHex(' '));          // Added spaced between bytes
    ui->tableWidget->setCellWidget(7, 1, futureStandardizationEdit);
    connect(futureStandardizationEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPClientConfig::Instance().setFutureStandardization(QByteArray::fromHex(newText.simplified().toUtf8())); });

    QSpinBox* routingActivationWaitTimeBox = new QSpinBox(this);
    routingActivationWaitTimeBox->setRange(50, 60000);
    routingActivationWaitTimeBox->setValue(client.getRoutingActivationWaitTime());
    ui->tableWidget->setCellWidget(8, 1, routingActivationWaitTimeBox);
    connect(routingActivationWaitTimeBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [](int newValue){ DoIPClientConfig::Instance().setRoutingActivationWaitTime(newValue); });

    QSpinBox* controlTimeBox = new QSpinBox(this);
    controlTimeBox->setRange(50, 60000);
    controlTimeBox->setValue(client.getControlTime());
    ui->tableWidget->setCellWidget(9, 1, controlTimeBox);
    connect(controlTimeBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [](int newValue){ DoIPClientConfig::Instance().setControlTime(newValue); });

    QLineEdit *eidEdit = new QLineEdit(this);
    eidEdit->setText(server.getEid().toHex(' '));
    ui->tableWidget->setCellWidget(10, 1, eidEdit);
    connect(eidEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPServerConfig::Instance().setEid(QByteArray::fromHex(newText.simplified().toUtf8())); });

    QLineEdit *gidEdit = new QLineEdit(this);
    gidEdit->setText(server.getGid().toHex(' '));
    ui->tableWidget->setCellWidget(11, 1, gidEdit);
    connect(gidEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPServerConfig::Instance().setGid(QByteArray::fromHex(newText.simplified().toUtf8())); });

    QLineEdit *vinEdit = new QLineEdit(this);
    vinEdit->setText(server.getVin().toHex(' '));
    ui->tableWidget->setCellWidget(12, 1, vinEdit);
    connect(vinEdit, &QLineEdit::textChanged,
            [](const QString &newText){ DoIPServerConfig::Instance().setVin(QByteArray::fromHex(newText.simplified().toUtf8())); });

    ui->tableWidget->setColumnWidth(0, 195);
    ui->tableWidget->setColumnWidth(1, 400);
}

void DoIPSettingWindow::closeEvent(QCloseEvent *event) {
     figkey::DoIPClientConfig::Instance().save();
     figkey::DoIPServerConfig::Instance().save();
     event->accept();
}
