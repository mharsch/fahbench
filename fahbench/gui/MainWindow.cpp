
#include "MainWindow.h"
#include "../FAHBenchVersion.h"

#include <QMessageBox>
#include <QDebug>
//TODO: remove debug

using namespace std;


MainWindow::MainWindow() : QMainWindow() {
    qRegisterMetaType<Simulation>();
    qRegisterMetaType<SimulationResult>();

    // Set up SimulationWorker on another thread and connect signals and slots
    worker = new SimulationWorker();
    worker->moveToThread(&thread);
    connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MainWindow::start_new_simulation, worker, &SimulationWorker::run_simulation);
    connect(worker, &SimulationWorker::simulation_finished, this, &MainWindow::simulation_finished);
    thread.start();

    // Set up layout and add components
    central_widget = new CentralWidget();
    setCentralWidget(central_widget);
    connect(central_widget->start_button, &QAbstractButton::clicked, this, &MainWindow::start_button_clicked);
    connect(worker, &SimulationWorker::progress_update, central_widget, &CentralWidget::progress_update);
    connect(worker, &SimulationWorker::message_update, central_widget, &CentralWidget::message_update);
    setWindowTitle("FAHBench");

    make_actions();
    make_menu_bar();
}

MainWindow::~MainWindow() {
    thread.quit();
    thread.wait();
}

void MainWindow::make_actions() {
    about_action = new QAction("About", this);
    connect(about_action, SIGNAL(triggered(bool)), this, SLOT(about()));

    exit_action = new QAction("Exit", this);
    connect(exit_action, SIGNAL(triggered(bool)), this, SLOT(close()));
}


void MainWindow::make_menu_bar() {
    auto file_menu = menuBar()->addMenu("&File");
    file_menu->addAction(exit_action);
    auto help_menu = menuBar()->addMenu("Help");
    help_menu->addAction(about_action);
}


void MainWindow::start_button_clicked() {
    Simulation sim;

    sim.verifyAccuracy = false;
    sim.run_length = std::chrono::seconds(10);
    //TODO: Configure simulation


    auto pbar = central_widget->progress_bar;
    auto sbut = central_widget->start_button;
    pbar->reset();
    // Show "busy" bar
    pbar->setMinimum(0);
    pbar->setMaximum(0);
    sbut->setEnabled(false);
    emit start_new_simulation(sim);
}

void MainWindow::simulation_finished(const SimulationResult & score) {
    auto pbar = central_widget->progress_bar;
    auto sbut = central_widget->start_button;
    pbar->setMinimum(0);
    pbar->setMaximum(1);
    pbar->setValue(pbar->maximum());
    sbut->setEnabled(true);

    // TODO: Update result
    qDebug() << score.score();
    qDebug() << score.scaled_score();
}

void MainWindow::about() {
    QString about_text(
        "<h1>FAHBench</h1>"
        "<h3>GPU Benchmarking for Folding@home</h3>"
    );
    about_text += "<p>";
    about_text += "Version " + QString::fromStdString(getVersion());
    about_text += "<br />";
    about_text += "OpenMM version " + QString::fromStdString(getOpenMMVersion());
    about_text += "</p>";
    QMessageBox::about(this, "About FAHBench", about_text);
}


#include "MainWindow.moc"
