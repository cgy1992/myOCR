#include "MainWindow.h"

#include <QtGui>
#include "ImageWidget.h"
#include "ImageAdapter.h"


MainWindow::MainWindow()
{
	cvImage = NULL;
	boxes = NULL;
	tessBaseAPI = NULL;

	resize(960, 600);

	imageWidget = new ImageWidget();
	textEdit = new QTextEdit();
	textEdit->setMinimumWidth(200);
	textEdit->setMaximumWidth(600);
	QFont fontTextEdit = textEdit->font();
	fontTextEdit.setPixelSize(18);
	textEdit->setFont(fontTextEdit);

	QHBoxLayout* hbox = new QHBoxLayout();
	hbox->addWidget(imageWidget,2);
	hbox->addWidget(textEdit,1);
	
	QWidget* centerWidget = new QWidget();
	centerWidget->setLayout(hbox);

	this->setCentralWidget(centerWidget);

	menuFile = menuBar()->addMenu(tr("&File"));
	menuImage = menuBar()->addMenu(tr("&Image"));
	menuHelp = menuBar()->addMenu(tr("&Help"));

	toolBarFile = addToolBar(tr("&File"));
	toolBarFile->setMovable(false);
	toolBarImage = addToolBar(tr("&Image"));
	toolBarImage->setMovable(false);

	actionFileOpen = new QAction(tr("&Open"), this);
	actionImageOCR = new QAction(tr("&OCR"), this);
	actionImageCV = new QAction(tr("&CV"), this);
	actionHelpAbout = new QAction(tr("&About"), this);

	actionFileOpen->setIcon(QIcon(":/open.png"));
	actionImageOCR->setIcon(QIcon(":/ocr.png"));
	actionImageCV->setIcon(QIcon(":/paint.png"));

	connect(actionFileOpen, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(actionImageOCR, SIGNAL(triggered()), this, SLOT(startOCR()));
	connect(actionImageCV, SIGNAL(triggered()), this, SLOT(startCV()));

	menuFile->addAction(actionFileOpen);
	menuImage->addAction(actionImageCV);
	menuImage->addAction(actionImageOCR);
	menuHelp->addAction(actionHelpAbout);

	toolBarFile->addAction(actionFileOpen);
	toolBarImage->addAction(actionImageCV);
	toolBarImage->addAction(actionImageOCR);
}

MainWindow::~MainWindow()
{
	if(cvImage)
	{
		cvReleaseImage(&cvImage);
	}
	if(tessBaseAPI)
	{
		tessBaseAPI->End();
	}
}

void MainWindow::openFile()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Image"), ".",
		tr("Image files (*.png *.jpg *.bmp *.tiff)"));
    if (!fileName.isEmpty())
	{
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("gbk"));//解决中文路径的问题
		if(cvImage)
		{
			cvReleaseImage(&cvImage);
		}
		cvImage = cvLoadImage(fileName.toStdString().c_str());
		if(cvImage)
		{
			QImage* image = ImageAdapter::IplImage2QImage(cvImage);
			imageWidget->setImage(image);
		}
	}
}

void MainWindow::startCV()
{
	IplImage* grayImage = cvCreateImage(cvGetSize(cvImage), 8, 1);
	if(cvImage->nChannels == 3)
	{
		cvCvtColor(cvImage, grayImage, CV_RGB2GRAY);
		cvReleaseImage(&cvImage);
		cvImage = grayImage;
	}
	
	cvAdaptiveThreshold(cvImage, cvImage, 255);

	QImage* image = ImageAdapter::IplImage2QImage(cvImage);
	imageWidget->setImage(image);
}

void MainWindow::startOCR()
{
	if(!cvImage)
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText(tr("No image loaded.\nPlease load an image file before OCR."));
		msgBox.exec();
		return;
	}
	else
	{
		//设置环境变量TESSDATA_PREFIX
		if(tessBaseAPI == NULL)
		{
			tessBaseAPI = new tesseract::TessBaseAPI(); 
		}
		//或者在Init函数中设置datapath
		if (tessBaseAPI->Init(NULL, "chi_sim")) {
			QMessageBox msgBox;
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.setText(tr("Could not initialize tesseract."));
			msgBox.exec();
			return;
		}

		tessBaseAPI->SetImage((uchar*)cvImage->imageData, cvImage->width, cvImage->height, cvImage->nChannels, cvImage->widthStep);

		boxes = tessBaseAPI->GetComponentImages(tesseract::RIL_SYMBOL, true, NULL, NULL);
		
		QRect* rects = new QRect[boxes->n];
		for(int i = 0; i< boxes->n; i++)
		{
			rects[i].setX(boxes->box[i]->x);
			rects[i].setY(boxes->box[i]->y);
			rects[i].setWidth(boxes->box[i]->w);
			rects[i].setHeight(boxes->box[i]->h);
		}
		imageWidget->setBoxes(rects, boxes->n);

		char *outText = tessBaseAPI->GetUTF8Text();

		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
		textEdit->setText(outText);
	}
}