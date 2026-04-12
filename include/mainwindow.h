#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:

public slots:
    void clearErrorLabel();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void onActionLoad();
    void onSaveClicked();
    void onLoadTexAction();
    void onDeleteTexAction();
    void updateTextureDisplay(QImage currentTexture);

private:

    void handleMeshError(int err);
    void handleTextureMessage(QString filename);
    void setTrianglesCount(unsigned int count);
    void setVerticesCount(unsigned int count);

    Ui::MainWindow *ui;
    QTimer *errorTimer;
    QGraphicsScene *scene;
};
#endif // MAINWINDOW_H
