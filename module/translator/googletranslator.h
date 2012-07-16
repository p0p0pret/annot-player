#ifndef GOOGLETRANSLATOR_H
#define GOOGLETRANSLATOR_H

// googletranslator.h
// 6/30/2012

#include "module/translator/translator.h"
#include <QtCore/QStack>
#include <QtCore/QUrl>

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)
QT_FORWARD_DECLARE_CLASS(QWebPage)

class GoogleTranslator : public Translator
{
  Q_OBJECT
  Q_DISABLE_COPY(GoogleTranslator)
  typedef GoogleTranslator Self;
  typedef Translator Base;

  QStack<QWebPage *> stack_; // web page pool
  QNetworkAccessManager *nam_; // shared nam

  // - Constructions -
public:
  explicit GoogleTranslator(QObject *parent = 0);

public slots:
  virtual void translate(const QString &text, const QString &to, const QString &from = QString()); ///< \reimp

protected:
  static QUrl translateUrl(const QString &text, const QString &to, const QString &from = QString());
  QNetworkAccessManager *networkAccessManager();
  QWebPage *createWebPage();
  QWebPage *allocateWebPage();
public:
  void releaseWebPage(QWebPage *page); ///< \internal
public slots:
  void processWebPage(QWebPage *page, bool success); ///< \internal
};

#endif // GOOGLETRANSLATOR_H