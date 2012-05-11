#ifndef ACAPPLICATION_H
#define ACAPPLICATION_H

// acapplication.h
// 11/18/2011

#include <QtGui/QApplication>
#include <QtGui/QWidgetList>

class AcApplication : public QApplication
{
  Q_OBJECT
  Q_DISABLE_COPY(AcApplication)
  typedef AcApplication Self;
  typedef QApplication Base;

  QList<QtMsgHandler> messageHandlers_;
  QString logFileName_,
          lockFileName_;
  QWidgetList windows_;

  // - Constructions -
public:
  static Self *globalInstance() { return dynamic_cast<Self *>(qApp); }

  AcApplication(int &argc, char **argv);
  AcApplication(int &argc, char **argv, bool gui);
  ~AcApplication();

public slots:
  void abortAll(); // kill all instances
  void abort();    // kill this instance

  // - States -
public:
  bool isSingleInstance() const;

  // - Logs -
public:
  void setLogFileName(const QString &path) { logFileName_ = path; }
  void setLockFileName(const QString &path) { lockFileName_ = path; }

  void installMessageHandlers(); ///< invoked only once
  void addMessageHandler(QtMsgHandler callback)
  { messageHandlers_.append(callback); }

  // - Windows -
public:
  QWidget *mainWindow() const;
  void setMainWindow(QWidget *w) { addWindow(w); }
  void addWindow(QWidget *w);
  void removeWindow(QWidget *w);

  // - Events -
public:
  virtual bool event(QEvent *e); ///< \override

  // - Implementation -
private:
  static void messageHandler(QtMsgType type, const char *msg);
  static void loggedMessageHandler(QtMsgType type, const char *msg);
};

#endif // ACAPPLICATION_H