#include "codemanager.h"

#include <QDebug>

CodeManager::CodeManager()
{
    engine = new QScriptEngine();
    debugger = new QScriptEngineDebugger();
    commandImpl = new CommandImpl();

    QScriptValue command = engine->newQObject(commandImpl);
    engine->globalObject().setProperty("command", command);
}

void CodeManager::run(QString test)
{
    qDebug() << "Run : " << test;

    // Set variable for user

    // Interrpt immidiatly
    debugger->setAutoShowStandardWindow(true);
    debugger->attachTo(engine);
    debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
    //actionNextLine = debugger->action(QScriptEngineDebugger::StepOverAction);
    QScriptValue result = engine->evaluate(test);

    if (engine->hasUncaughtException()) {
        int line = engine->uncaughtExceptionLineNumber();
        qDebug() << "uncaught exception at line" << line << ":" << result.toString();
    }
}