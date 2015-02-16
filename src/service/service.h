/* 
 * File:   service_listener.h
 * Author: Christian Surlykke <christian@surlykke.dk>
 *
 * Created on 14. februar 2015, 19:10
 */

#ifndef SERVICELISTENER_H
#define	SERVICELISTENER_H

#include <QLocalServer>
#include <QThread>

class QIODevice;

class ServiceListener : public QThread {
public:
    ServiceListener();
    virtual ~ServiceListener();

protected:
	void Q_DECL_OVERRIDE run();

private:
	QLocalServer mServer;
};



#endif	/* SERVICELISTENER_H */
