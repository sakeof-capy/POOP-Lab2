#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QFileDialog>
#include <QStandardPaths>

using Tool = Graphics::Tool;

const qreal MainWindow::ZOOM_SCALE_FACTOR = 2.0;
const qint32 MainWindow::MAX_GRAPHICS_LOGSCALE = 3;
const qint32 MainWindow::MIN_GRAPHICS_LOGSCALE = -3;
const QString MainWindow::FILE_FILTERS = "PNG File (*.png);; JPG File (*.jpg)";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      _graphics(new Graphics(centralWidget())),
      _checkedAction(nullptr),
      _graphicsLogScale(0),
      _hasBeenSaved(false),
      _savedPath(nullptr)
{
    ui->setupUi(this);
    _graphics->setFixedSize(_graphics->size());
    QScrollArea* scrollArea = centralWidget()->findChildren<QScrollArea*>("scrollArea").first();
    scrollArea->setWidget(_graphics);
    connect_all_actions_to_slots();
}

MainWindow::~MainWindow()
{
    delete ui;
    ui = nullptr;
    delete _savedPath;
    _savedPath = nullptr;
}

bool MainWindow::increment_graphics_scale()
{
    if(_graphicsLogScale == MAX_GRAPHICS_LOGSCALE)
    {
        return false;
    }

    ++_graphicsLogScale;
    _graphics->scale(ZOOM_SCALE_FACTOR);
    return true;
}

bool MainWindow::decrement_graphics_scale()
{
    if(_graphicsLogScale == MIN_GRAPHICS_LOGSCALE)
    {
        return false;
    }

    --_graphicsLogScale;
    _graphics->scale(1 / ZOOM_SCALE_FACTOR);
    return true;
}

void MainWindow::connect_all_actions_to_slots()
{
    connect_action_to_tool(ui->actionDraw, Tool::PENCIL);
    connect_action_to_tool(ui->actionCircle, Tool::CIRCLE);
    connect_action_to_tool(ui->actionEllipse, Tool::ELLIPSE);
    connect_action_to_tool(ui->actionLine, Tool::LINE);
    connect_action_to_tool(ui->actionPolygon, Tool::POLYGON);
    connect_action_to_tool(ui->actionRectangle, Tool::RECTANGLE);
    connect_action_to_tool(ui->actionRhombus, Tool::RHOMBUS);
    connect_action_to_tool(ui->actionTriangle, Tool::TRIANGLE);

    connect(ui->actionZoom_in, &QAction::triggered,
            this, &MainWindow::increment_graphics_scale);
    connect(ui->actionZoom_out, &QAction::triggered,
            this, &MainWindow::decrement_graphics_scale);

    connect(ui->actionClear, &QAction::triggered,
            _graphics, &Graphics::clear);


    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save_graphics);
    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::save_as_graphics);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::load_graphics);
}

void MainWindow::save_graphics()
{
    if(_hasBeenSaved && QFile(*_savedPath).exists())
        _graphics->save(*_savedPath);
    else
        save_as_graphics();
}

void MainWindow::save_as_graphics()
{
    QString filename = QFileDialog::getSaveFileName
            (
                this,
                "Save a file",
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                FILE_FILTERS
            );
    if(filename.isEmpty()) return;
    _graphics->save(filename);
    _hasBeenSaved = true;
    _savedPath = new QString(std::move(filename));
}

void MainWindow::load_graphics()
{
    QString filename = QFileDialog::getOpenFileName
            (
                this,
                "Open a file",
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                FILE_FILTERS
            );
    if(filename.isEmpty()) return;
    _graphics->load(filename);
    _graphicsLogScale = 0;
    _hasBeenSaved = true;
    _savedPath = new QString(std::move(filename));
}

void MainWindow::connect_action_to_tool(QAction* action, const Tool tool)
{
    auto _slot = [this, tool, action]
    {
        if(action == _checkedAction)
        {
            _graphics->deselect_tool();
            _checkedAction = nullptr;
        }
        else if(_checkedAction)
        {
            _checkedAction->setChecked(false);
            _checkedAction = action;
            _graphics->select_tool(tool);
        }
        else
        {
            _checkedAction = action;
            _graphics->select_tool(tool);
        }
    };

    connect(action, &QAction::triggered, _graphics, _slot);
}
