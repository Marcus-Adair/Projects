 #ifndef SCRIPTDEBUGGER_H
 #define SCRIPTDEBUGGER_H

 #include <QtCore/qlist.h>
 #include <QtCore/qstring.h>
 #include <QObject>

 class QScriptEngine;

 class ScriptDebuggerPrivate;
 class ScriptDebugger : public QObject
 {
     Q_OBJECT

 public:
     ScriptDebugger(QScriptEngine *engine);
     virtual ~ScriptDebugger();

     void breakAtNextStatement();
     void setBreakpoint(int lineNumber);
     void moveNext();

 protected:
     ScriptDebuggerPrivate *d_ptr;

 private:
     Q_DECLARE_PRIVATE(ScriptDebugger)
     Q_DISABLE_COPY(ScriptDebugger)

 signals:
     void signalLineChange(int currentLine);

 };



 #endif // SCRIPTDEBUGGER_H