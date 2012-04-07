#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

// clipboardmonitor.h
// 3/10/2012

#include <QObject>

class ClipboardMonitor : public QObject
{
  Q_OBJECT
  typedef ClipboardMonitor Self;
  typedef QObject Base;

  bool enabled_;

  // - Constructions -
public:
  explicit ClipboardMonitor(QObject *parent = 0);

  bool isEnabled() const { return enabled_; }

signals:
  void mediaUrlEntered(const QString &url);
  void annotationUrlEntered(const QString &url);

public slots:
  void setEnabled(bool t);
  void invalidateClipboard();

protected slots:
  void parseUrl(const QString &text);

protected:
  static bool isSupportedMediaUrl(const QString &url);
  static bool isSupportedAnnotationUrl(const QString &url);
};

#endif // CLIPBOARDMONITOR_H