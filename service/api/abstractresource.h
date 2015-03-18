/* 
 * File:   AbstractResource.h
 * Author: Christian Surlykke <christian@surlykke.dk>
 *
 * Created on 15. marts 2015, 09:42
 */

#ifndef ABSTRACTRESOURCE_H
#define	ABSTRACTRESOURCE_H

#include <pthread.h>
#include <map>

#include "httpprotocol.h"

class AbstractResource
{
public:
	AbstractResource();
	virtual ~AbstractResource();

	virtual void doGET(int socket, const char* path, const char* queryString) = 0;
	virtual void doPATCH(int socket, const char* path, const char* patchJS) { throw Status::Http411; }

};

struct ResourceMappings;

class ResourceMap
{
public:
	ResourceMap();
	virtual ~ResourceMap();

	void map(const char* path, AbstractResource* resource);
	void unMap(AbstractResource* resource);
	AbstractResource* resource(const char* path);

private:
	pthread_rwlock_t mLock;
	ResourceMappings* mResourceMappings;
};


#endif	/* ABSTRACTRESOURCE_H */
