#include "CmdMediator.h"
#include "DlgPreferencesPageCoords.h"
#include "Logger.h"
#include <QComboBox>
#include <QDoubleValidator>
#include <QGridLayout>
#include <QGroupBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "ViewPreview.h"

const QString POLAR_UNITS_DEGREES = QString("Degrees (DDD.DDDDD") + QChar(0260) + QString (")");
const QString POLAR_UNITS_DEGREES_MINUTES = QString("Degrees Minutes (DDD") + QChar(0260) +
                                            QString(" MM.MMM") + QChar(0x2032) +
                                            QString(")");
const QString POLAR_UNITS_DEGREES_MINUTES_SECONDS = QString("Degrees Minutes Seconds (DDD") + QChar(0260) +
                                                    QString(" MM") + QChar(0x2032) +
                                                    QString(" SS.S") + QChar(0x2033) + QString(")");
const QString POLAR_UNITS_GRADIANS = "Gradians";
const QString POLAR_UNITS_RADIANS = "Radians";
const QString POLAR_UNITS_TURNS = "Turns";

DlgPreferencesPageCoords::DlgPreferencesPageCoords(CmdMediator &cmdMediator,
                                                   QWidget *parent) :
  DlgPreferencesPageAbstractBase (cmdMediator,
                                  parent),
  m_coordsType (cmdMediator.coordsType ()),
  m_btnCartesian (0),
  m_btnPolar (0)
{
  QGridLayout *layout = new QGridLayout (this);
  setLayout (layout);

  layout->setColumnStretch(0, 1); // Empty first column
  layout->setColumnStretch(1, 0); // Labels
  layout->setColumnStretch(2, 0); // User controls
  layout->setColumnStretch(3, 1); // Empty last column

  int row = 0;
  createGroupCoordsType(layout, row);
  createGroupScale (layout, row);
  createGroupPolar (layout, row);
  createPreview (layout, row);
  initializeGroupCoordsType();
  updateControls ();
}

void DlgPreferencesPageCoords::createGroupCoordsType (QGridLayout *layout,
                                                      int &row)
{
  m_boxCoordsType = new QGroupBox("Coordinates Types");
  layout->addWidget (m_boxCoordsType, row++, 1, 1, 2);

  QVBoxLayout *layoutGroup = new QVBoxLayout (m_boxCoordsType);

  QString polarButtonText = QString("Polar (R, ") + QChar(0x98, 0x03) + QString(")");

  m_btnCartesian = new QRadioButton ("Cartesian (X, Y)", m_boxCoordsType);
  m_btnCartesian->setWhatsThis (QString(tr("Select cartesian coordinates.\n\n"
                                           "The X and Y coordinates will be used")));
  connect (m_btnCartesian, SIGNAL (pressed()), this, SLOT (slotCartesian ()));
  layoutGroup->addWidget (m_btnCartesian);

  m_btnPolar = new QRadioButton (polarButtonText, m_boxCoordsType);
  m_btnPolar->setWhatsThis (QString(tr("Select polar coordinates.\n\n"
                                       "The Theta and R coordinates will be used.\n\n"
                                       "Polar coordinates are not allowed with log scale for Theta")));
  connect (m_btnPolar, SIGNAL (pressed()), this, SLOT (slotPolar ()));
  layoutGroup->addWidget (m_btnPolar);
}

void DlgPreferencesPageCoords::createGroupPolar(QGridLayout *layout,
                                                int &row)
{
  m_boxPolarCoords = new QGroupBox ("Polar Coordinates");
  layout->addWidget (m_boxPolarCoords, row++, 1, 1, 2);

  QGridLayout *layoutPolar = new QGridLayout (m_boxPolarCoords);
  m_boxPolarCoords->setLayout (layoutPolar);

  QLabel *labelThetaUnits = new QLabel(QChar (0x98, 0x03) + QString (" Units:"));
  layoutPolar->addWidget (labelThetaUnits, 0, 0);

  m_cmbPolarUnits = new QComboBox;
  m_cmbPolarUnits->addItem (POLAR_UNITS_DEGREES);
  m_cmbPolarUnits->addItem (POLAR_UNITS_DEGREES_MINUTES);
  m_cmbPolarUnits->addItem (POLAR_UNITS_DEGREES_MINUTES_SECONDS);
  m_cmbPolarUnits->addItem (POLAR_UNITS_GRADIANS);
  m_cmbPolarUnits->addItem (POLAR_UNITS_RADIANS);
  m_cmbPolarUnits->addItem (POLAR_UNITS_TURNS);
  m_cmbPolarUnits->setWhatsThis (QString (tr ("Degrees (DDD.DDDDD) format uses a single real number. One complete revolution is 360 degrees.\n\n"
                                              "Degrees Minutes (DDD MM.MMM) format uses one integer number for degrees, and a real number for minutes. There are "
                                              "60 minutes per degree. During input, a space must be inserted between the two numbers.\n\n"
                                              "Degrees Minutes Seconds (DDD MM SS.S) format uses two integer number for degrees and minutes, and a real number for "
                                              "seconds. There are 60 seconds per minute. During input, spaces must be inserted between the three numbers.\n\n"
                                              "Gradians format uses a single real number. One complete revolution is 400 gradians.\n\n"
                                              "Radians format uses a single real number. One complete revolution is 2*pi radians.\n\n"
                                              "Turns format uses a single real number. One complete revolution is one turn.")));
  connect (m_cmbPolarUnits, SIGNAL (currentTextChanged (const QString &)), this, SLOT (slotPolarUnits(const QString &)));
  layoutPolar->addWidget (m_cmbPolarUnits, 0, 1);

  QLabel *labelOriginRadius = new QLabel("Origin radius value:");
  layoutPolar->addWidget (labelOriginRadius, 1, 0);

  m_editOriginRadius = new QLineEdit (m_boxPolarCoords);
  m_editOriginRadius->setValidator (new QDoubleValidator);
  m_editOriginRadius->setWhatsThis (QString(tr("Specify radius value at origin.\n\n"
                                               "Normally the radius at the origin is 0, but a nonzero value may be applied in other cases "
                                               "(like when the radial units are decibels).")));
  connect (m_editOriginRadius, SIGNAL (editingFinished ()), this, SLOT (slotPolarOriginRadius()));
  layoutPolar->addWidget (m_editOriginRadius, 1, 1);
}

void DlgPreferencesPageCoords::createGroupScale (QGridLayout *layout,
                                                 int &row)
{
  m_boxXTheta = new QGroupBox(QString("X/") + QChar (0x98, 0x03) + QString (" Scale"));
  layout->addWidget (m_boxXTheta, row, 1);

  QVBoxLayout *layoutXTheta = new QVBoxLayout (m_boxXTheta);
  m_boxXTheta->setLayout (layoutXTheta);

  m_xThetaLinear = new QRadioButton ("Linear", m_boxXTheta);
  m_xThetaLinear->setWhatsThis (QString(tr("Specifies linear scale for the X or Theta coordinate")));
  connect (m_xThetaLinear, SIGNAL (pressed ()), this, SLOT (slotXThetaLinear()));
  layoutXTheta->addWidget (m_xThetaLinear);

  m_xThetaLog = new QRadioButton ("Log", m_boxXTheta);
  m_xThetaLog->setWhatsThis (QString(tr("Specifies logarithmic scale for the X coordinate.\n\n"
                                        "Log scale is not allowed for the Theta coordinate")));
  connect (m_xThetaLog, SIGNAL (pressed ()), this, SLOT (slotXThetaLog()));
  layoutXTheta->addWidget (m_xThetaLog);

  m_boxYRadius = new QGroupBox ("Y/R Scale");
  layout->addWidget (m_boxYRadius, row++, 2);

  QVBoxLayout *layoutYRadius = new QVBoxLayout (m_boxYRadius);
  m_boxYRadius->setLayout (layoutYRadius);

  m_yRadiusLinear = new QRadioButton ("Linear", m_boxYRadius);
  m_yRadiusLinear->setWhatsThis (QString(tr("Specifies linear scale for the Y or R coordinate")));
  connect (m_yRadiusLinear, SIGNAL(pressed()), this, SLOT (slotYRadiusLinear()));
  layoutYRadius->addWidget (m_yRadiusLinear);

  m_yRadiusLog = new QRadioButton ("Log", m_boxYRadius);
  m_yRadiusLog->setWhatsThis (QString(tr("Specifies logarithmic scale for the Y or R coordinate")));
  connect (m_yRadiusLog, SIGNAL(pressed ()), this, SLOT (slotYRadiusLog ()));
  layoutYRadius->addWidget (m_yRadiusLog);
}

void DlgPreferencesPageCoords::createPreview (QGridLayout *layout,
                                              int &row)
{
  QLabel *labelPreview = new QLabel ("Preview");
  layout->addWidget (labelPreview, row++, 0, 1, 4);

  m_scenePreview = new QGraphicsScene (this);
  m_viewPreview = new ViewPreview (m_scenePreview, this);
  m_viewPreview->setWhatsThis (tr ("Preview window that shows how current settings affect the coordinate system."));
  m_viewPreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_viewPreview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  layout->addWidget (m_viewPreview, row++, 0, 1, 4);
}

void DlgPreferencesPageCoords::initializeGroupCoordsType()
{
  switch (m_coordsType) {
    case COORDS_TYPE_CARTESIAN:
      m_btnCartesian->setChecked (true);
      loadPixmap (":/engauge/img/plot_cartesian.png");
      break;

    case COORDS_TYPE_POLAR:
      m_btnPolar->setChecked (true);
      loadPixmap (":/engauge/img/plot_polar.png");
      break;
  }
}

void DlgPreferencesPageCoords::load ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::load";
}

void DlgPreferencesPageCoords::loadPixmap (const QString &image)
{
  if (m_scenePreview->items().count () > 0) {
    m_scenePreview->removeItem (m_scenePreview->items().first());
  }

  QPixmap pixmap (image);
  m_scenePreview->addPixmap (pixmap);
}

void DlgPreferencesPageCoords::slotCartesian ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotCartesian";

  m_coordsType = COORDS_TYPE_CARTESIAN;
  loadPixmap (":/engauge/img/plot_cartesian.png");
  updateControls();
}

void DlgPreferencesPageCoords::slotPolar ()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotPolar";

  m_coordsType = COORDS_TYPE_POLAR;
  loadPixmap (":/engauge/img/plot_polar.png");
  updateControls();
}

void DlgPreferencesPageCoords::slotPolarOriginRadius()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotPolarOriginRadius";

  updateControls();
}

void DlgPreferencesPageCoords::slotPolarUnits(const QString &)
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotPolarUnits";

  updateControls ();
}

void DlgPreferencesPageCoords::slotXThetaLinear()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotXThetaLinear";

  updateControls ();
}

void DlgPreferencesPageCoords::slotXThetaLog()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotXThetaLog";

  updateControls ();
}

void DlgPreferencesPageCoords::slotYRadiusLinear()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotYRadiusLinear";

  updateControls ();
}

void DlgPreferencesPageCoords::slotYRadiusLog()
{
  LOG4CPP_INFO_S ((*mainCat)) << "DlgPreferencesPageCoords::slotYRadiusLog";

  updateControls ();
}

void DlgPreferencesPageCoords::updateControls ()
{
  m_btnPolar->setEnabled (!m_xThetaLog->isChecked ());

  m_xThetaLog->setEnabled (!m_btnPolar->isChecked ());

  m_cmbPolarUnits->setEnabled (m_btnPolar->isChecked ());

  m_editOriginRadius->setEnabled (m_btnPolar->isChecked ());
}