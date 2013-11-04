#include "logspechandler.hh"

#include <QStringList>
#include <QTextStream>

namespace LOG
{

  class LogSpecHandlerPrivate
  {
  public:
    LogSpecHandlerPrivate()
      : error(LogSpecHandler::None)
    { }

    void createErrorString(const QString &log_spec, int pos);

    QString errorString;
    LogSpecHandler::Error error;
  };

  struct Component
  {
    QString name;
    Level level;
  };

} // LOG

using namespace LOG;

LogSpecHandler::LogSpecHandler()
  : d(new LogSpecHandlerPrivate())
{ }

LogSpecHandler::~LogSpecHandler()
{
  delete d;
}

bool
LogSpecHandler::update_logsystem(const QString &log_spec)
{
  d->error = None;
  d->errorString.clear();

  QStringList setting_entries = log_spec.split(";", QString::SkipEmptyParts);

  typedef QList< Component > Components;
  Components components;

  int pos = 0;
  foreach (const QString &component_setup, setting_entries)
    {
      QStringList pair = component_setup.split(":");

      Component component;

      if (pair.size() != 2)
        {
          d->error = InvalidPair;
          d->createErrorString(log_spec, pos);
          return false;
        }

      component.name = pair.at(0);

      pos += component.name.size();
      pos += 1; // < :

      bool ok = false;
      int log_level = pair.at(1).toInt(&ok);

      if (!ok)
        {
          d->error = InvalidLogLevel;
          d->createErrorString(log_spec, pos);
          return false;
        }

      component.level = get_log_level_from_int(log_level);

      pos += pair.at(1).size();
      pos += 1; // < ;

      components << component;
    }

  Manager::Locker locker;
  foreach (const Component &component, components)
    {
      locker.mutable_logger(component.name)->set_level(component.level);
    }

  return true;
}

LogSpecHandler::Error
LogSpecHandler::error() const
{
  return d->error;
}

const QString &
LogSpecHandler::errorString() const
{
  return d->errorString;
}

// LogSpecHandlerPrivate

void
LogSpecHandlerPrivate::createErrorString(const QString &log_spec, int pos)
{
  QTextStream stream(&errorString);
  stream.setCodec("utf-8");

  stream << "Invalid logspec syntax:" << endl << endl;
  stream << log_spec << endl;
  stream << QString("^").rightJustified(pos + 1);
}
