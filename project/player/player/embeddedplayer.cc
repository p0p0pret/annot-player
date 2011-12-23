// embeddedplayer.cc
// 7/16/2011

#include "embeddedplayer.h"
#include "defines.h"
#include "tr.h"
#include "stylesheet.h"
#include "uistyle.h"
#include "signalhub.h"
#ifdef Q_WS_WIN
  #include "win/qtwin/qtwin.h"
#endif // Q_WS_WIN
#include "core/gui/toolbutton.h"
#include "core/gui/withsizehint.h"
#include <QtGui>

#define DEBUG "embeddedplayerui"
#include "module/debug/debug.h"

#define G_INPUTLINE_MINWIDTH    400
#define G_INPUTLINE_MINHEIGHT   25

#define SS_COMBOBOX_OSD \
  SS_BEGIN(QComboBox) \
    SS_TRANSPARENT \
    SS_COLOR(blue) \
  SS_END \
  SS_BEGIN(QComboBox::drop-down) \
  SS_END \
  SS_BEGIN(QComboBox QAbstractItemView) \
    SS_BORDER_IMAGE_URL(RC_IMAGE_BACKGROUND) \
  SS_END
#define SS_LINEEDIT_OSD \
  SS_BEGIN(QLineEdit) \
    SS_TRANSPARENT \
    SS_BORDER(1px groove purple) \
    SS_COLOR(blue) \
  SS_END \
  SS_BEGIN(QLineEdit::hover) \
    SS_BORDER_IMAGE_URL(RC_IMAGE_LINEEDIT) \
    SS_BORDER(1px groove black) \
    SS_COLOR(black) \
  SS_END

// - Constructions -
EmbeddedPlayerUi::EmbeddedPlayerUi(SignalHub *hub, Player *player, ServerAgent *server, QWidget *parent)
  : Base(hub, player, server, parent),
    menuButton_(0), containerWindow_(0), containerWidget_(0),
    fullScreen_(false), top_(false)
{
  setWindowFlags(Qt::FramelessWindowHint);
  setContentsMargins(0, 0, 0, 0);
  createLayout();

  // Auto hide player.
  autoHideTimer_ = new QTimer(this);
  autoHideTimer_->setInterval(G_AUTOHIDE_TIMEOUT);
  connect(autoHideTimer_, SIGNAL(timeout()), SLOT(autoHide()));

  trackingTimer_ = new QTimer(this);
  trackingTimer_->setInterval(G_TRACKING_INTERVAL);
  connect(trackingTimer_, SIGNAL(timeout()), SLOT(invalidateGeometry()));

#define CONNECT_TO_AUTOHIDE(_obj, _signal) \
  connect(_obj, _signal, SLOT(resetAutoHideTimeout())); \
  connect(_obj, _signal, SLOT(showWhenEmbedded()));

  CONNECT_TO_AUTOHIDE(prefixComboBox()->lineEdit(), SIGNAL(cursorPositionChanged(int,int)));
  CONNECT_TO_AUTOHIDE(inputComboBox()->lineEdit(), SIGNAL(cursorPositionChanged(int,int)));
#undef CONNECT_TO_AUTOHIDE

  resize(0, 0); // temporarily
}

void
EmbeddedPlayerUi::createLayout()
{
  // Reset Ui style
  inputComboBox()->setStyleSheet(SS_COMBOBOX_OSD);
  inputComboBox()->lineEdit()->setStyleSheet(SS_LINEEDIT_OSD);
  prefixComboBox()->setStyleSheet(SS_COMBOBOX_OSD);
  prefixComboBox()->lineEdit()->setStyleSheet(SS_LINEEDIT_OSD);

  Core::Gui::WithSizeHint*
  w = dynamic_cast<Core::Gui::WithSizeHint*>(inputComboBox());
  if (w)
    w->setSizeHint(QSize(G_INPUTLINE_MINWIDTH, G_INPUTLINE_MINHEIGHT));
  w = dynamic_cast<Core::Gui::WithSizeHint*>(prefixComboBox());
  if (w)
    w->setSizeHint(QSize(G_PREFIXLINE_MAXWIDTH, G_INPUTLINE_MINHEIGHT));

  // Set layout
  QVBoxLayout *rows = new QVBoxLayout; {
    QHBoxLayout *row = new QHBoxLayout;
    rows->addWidget(positionSlider());
    rows->addLayout(row);

    row->addWidget(menuButton());
    row->addWidget(playButton());
    row->addWidget(toggleAnnotationButton());
    row->addWidget(nextFrameButton());
    row->addWidget(stopButton());
    row->addWidget(openButton());
    row->addWidget(previousButton());
    row->addWidget(nextButton());
    row->addWidget(toggleEmbedModeButton());
    row->addWidget(toggleMiniModeButton());
    row->addWidget(toggleFullScreenModeButton());
    row->addStretch();
    row->addWidget(userButton());
    row->addWidget(prefixComboBox());
    row->addWidget(inputComboBox());
    //row->addStretch();
    row->addWidget(volumeSlider());
    row->addWidget(positionButton());
  }
  setLayout(rows);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

// - Properties -

bool
EmbeddedPlayerUi::isFullScreenMode() const
{ return fullScreen_; }

void
EmbeddedPlayerUi::setFullScreenMode(bool t)
{
  if (fullScreen_ != t) {
    fullScreen_ = t;
    invalidateTrackingTimer();
    invalidateGeometry();
    emit fullScreenModeChanged(fullScreen_);
  }
}

bool
EmbeddedPlayerUi::isOnTop() const
{ return top_; }

void
EmbeddedPlayerUi::setOnTop(bool t)
{
  if (top_ != t) {
    top_ = t;
    invalidateGeometry();
  }
}

// - Auto hide -

void
EmbeddedPlayerUi::autoHide()
{
  if (underMouse() ||
      inputComboBox()->hasFocus() || prefixComboBox()->hasFocus())
    resetAutoHideTimeout();
  else if (isVisible())
    hide();
}

bool
EmbeddedPlayerUi::autoHideEnabled() const
{
  Q_ASSERT(autoHideTimer_);
  return autoHideTimer_->isActive();
}

void
EmbeddedPlayerUi::setAutoHideEnabled(bool enabled)
{
  Q_ASSERT(autoHideTimer_);
  if (enabled) {
    if (!autoHideTimer_->isActive())
      autoHideTimer_->start();
  } else  {
    if (autoHideTimer_->isActive())
      autoHideTimer_->stop();
  }
}

void
EmbeddedPlayerUi::resetAutoHideTimeout()
{
  Q_ASSERT(autoHideTimer_);
  //if (autoHideTimer_->isActive())
  //  autoHideTimer_->stop();
  //autoHideTimer_->start();
  QTimer::singleShot(0, autoHideTimer_, SLOT(start()));
}

// - Geometry -

void
EmbeddedPlayerUi::invalidateGeometry()
{
  if (fullScreen_ && QApplication::desktop()) {
    QRect r = QApplication::desktop()->screenGeometry(this);
    Q_ASSERT(!r.isNull);

    // Invalidate size
    int w_max = r.width();
    int h_hint = sizeHint().height();
    QSize newSize(w_max, h_hint);
    if (newSize != size())
      resize(newSize);

    int x_left = r.x();
    int y = top_ ? r.y() :
                   r.y() + r.height() - height();
    moveToGlobalPos(QPoint(x_left, y));

  } else if (containerWindow_) {
#ifdef Q_WS_WIN
    QRect r = QtWin::getWindowRect(containerWindow_);
    if (r.isNull()) {
      if (!QtWin::isValidWindow(containerWindow_))
        setContainerWindow(0);

    } else {
      int w_max = r.width();
      int h_hint = sizeHint().height();
      QSize newSize(w_max, h_hint);
      if (newSize != size())
        resize(newSize);

      //int window_header_height = hub()->isSignalTokenMode() ? 40 : 0;
      //enum { window_header_height = 40 };
      enum { window_header_height = 0 };

      int x_left = r.x();
      int y = top_ ? r.top() + window_header_height : r.bottom() - height();

      moveToGlobalPos(QPoint(x_left, y));
    }
#endif // Q_WS_WIN
  } else {
    QWidget *w = containerWidget_ ? containerWidget_ : parentWidget();
    if(w) {
      // Invalidate size
      int w_max = w->width();
      int h_hint = sizeHint().height();
      QSize newSize(w_max, h_hint);
      if (newSize != size())
        resize(newSize);

      // Invalidate position
      //int x_center = widget->width() / 2 - 2 * width();
      QPoint g = w->mapToGlobal(QPoint());
      int x_left = g.x();
      int y = top_ ? g.y() :
                     g.y() + w->height() - height();
      moveToGlobalPos(QPoint(x_left, y));
    }
  }
}

void
EmbeddedPlayerUi::moveToGlobalPos(const QPoint &globalPos)
{
  // Currently only work on Windows
  QPoint newPos = frameGeometry().topLeft() + pos() // relative position
                  + globalPos - mapToGlobal(pos()); // absolute distance
  if (newPos != pos())
    move(newPos);
}

WId
EmbeddedPlayerUi::containerWindow() const
{ return containerWindow_; }

QWidget*
EmbeddedPlayerUi::containerWidget() const
{ return containerWidget_; }

void
EmbeddedPlayerUi::setContainerWindow(WId winId)
{
  if (containerWindow_ != winId) {
    containerWindow_ = winId;
    invalidateTrackingTimer();
  }
}

void
EmbeddedPlayerUi::setContainerWidget(QWidget *w)
{
  if (containerWidget_ != w) {
    containerWidget_ = w;
    invalidateTrackingTimer();
  }
}

void
EmbeddedPlayerUi::invalidateTrackingTimer()
{
  if (isVisible() && !fullScreen_ &&
      (containerWindow_ || containerWidget_))
    startTracking();
  else
    stopTracking();
}


void
EmbeddedPlayerUi::startTracking()
{
  if (!trackingTimer_->isActive())
    trackingTimer_->start();
}

void
EmbeddedPlayerUi::stopTracking()
{
  if (trackingTimer_->isActive())
    trackingTimer_->stop();
}

void
EmbeddedPlayerUi::setVisible(bool visible)
{
  if (visible == isVisible())
    return;

  if (visible)
    invalidateGeometry();

  Base::setVisible(visible);

  invalidateTrackingTimer();
}

void
EmbeddedPlayerUi::showWhenEmbedded()
{
  if (!isVisible() && hub()->isEmbeddedPlayerMode())
    show();
}

// - Menu button -

void
EmbeddedPlayerUi::setMenu(QMenu *menu)
{ menuButton()->setMenu(menu); }

QToolButton*
EmbeddedPlayerUi::menuButton()
{
  if (!menuButton_) {
    menuButton_ = new Core::Gui::ToolButton(this);
    menuButton_->setStyleSheet(SS_TOOLBUTTON_MENU);
    menuButton_->setToolTip(TR(T_TOOLTIP_MENU));
    UiStyle::globalInstance()->setToolButtonStyle(menuButton_);

    connect(menuButton_, SIGNAL(clicked()), SLOT(popupMenu()));
  }
  return menuButton_;
}

void
EmbeddedPlayerUi::popupMenu()
{
  emit invalidateMenuRequested();
  menuButton()->showMenu();
}

// EOF