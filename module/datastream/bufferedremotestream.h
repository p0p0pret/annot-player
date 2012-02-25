#ifndef BUFFEREDREMOTESTREAM_H
#define BUFFEREDREMOTESTREAM_H

// bufferedremotestream.h
// 2/15/2012

#include "remotestream.h"
#include <QNetworkReply>
#include <QMutex>

class BufferedRemoteStream :  public RemoteStream
{
  Q_OBJECT
  typedef BufferedRemoteStream Self;
  typedef RemoteStream Base;

  QMutex m_;
  QByteArray data_;
  qint64 pos_;
  QNetworkReply *reply_;
  QUrl redirectUrl_;
  qint64 bufferSize_;

  bool stopped_;

public:
  explicit BufferedRemoteStream(QObject *parent = 0)
    : Base(parent), pos_(0), reply_(0), bufferSize_(0), stopped_(false) { }

  ~BufferedRemoteStream();

signals:
  void stopped();

public:
  virtual qint64 size() const { return Base::size() ? Base::size() : data_.size(); } ///< \override

  virtual qint64 pos() const { return pos_; } ///< \override

  virtual qint64 read(char *data, qint64 maxSize); ///< \override

  virtual QByteArray readAll(); ///< \override

  virtual bool reset() { m_.lock(); pos_ = 0; m_.unlock(); return true; } ///< \override
  virtual bool seek(qint64 pos) ///< \override
  {
    if (Base::size() && pos >= Base::size()) return false;
    m_.lock(); pos_ = pos; m_.unlock(); return true;
  }

  virtual qint64 skip(qint64 count) ///< \override
  {
    m_.lock();
    qint64 specifiedSize = Base::size();
    qint64 ret;
    if (specifiedSize && pos_ + count >= specifiedSize) {
       ret = specifiedSize - pos_;
       pos_ = specifiedSize;
    } else {
      pos_ += count;
      ret = count;
    }
    m_.unlock();
    return ret;
  }

  virtual QString contentType() const ///< \override
  {
    return reply_ ? reply_->header(QNetworkRequest::ContentTypeHeader).toString() :
                    QString();
  }

  bool isRunning() const { return reply_ && reply_->isRunning(); }
  bool isFinished() const { return reply_ && reply_->isFinished(); }

public slots:
  void waitForFinished();
  void waitForReadyRead();

  virtual void run(); ///< \override
  virtual void stop(); ///< \override

  virtual void setBufferSize(qint64 size) { bufferSize_ = size; } ///< \override

protected:
  bool isRedirected() const;

protected slots:
  void leadOut();
  void redirect();
  void invalidateSize();
  void networkError();
};

#endif // BUFFEREDREMOTESTREAM_H