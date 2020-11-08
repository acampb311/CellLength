#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QImage>
#include <QGraphicsView>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QGraphicsSceneMouseEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

	QImage Threshold(const int& thresh, const QImage& input);
	void createActions();
	void createMenus();
	void initMainWindow();

private slots:
	void openFile();
    bool eventFilter(QObject *target, QEvent *event);

private:
	QMenu* fileMenu;
	QAction* openAct;
	QGraphicsScene* scene;
	QGraphicsView* view;
    QImage img;
};

// https://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html
static void initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode)
{
	static bool firstDialog = true;

	if (firstDialog) {
		firstDialog = false;
		const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}

	QStringList mimeTypeFilters;
	const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
		? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
	for (const QByteArray& mimeTypeName : supportedMimeTypes)
		mimeTypeFilters.append(mimeTypeName);
	mimeTypeFilters.sort();
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.selectMimeTypeFilter("image/jpeg");
	if (acceptMode == QFileDialog::AcceptSave)
		dialog.setDefaultSuffix("jpg");
}

#endif // MAINWINDOW_H
