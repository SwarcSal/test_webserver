/*
 * @Descripttion: 
 * @version: v0.1
 * @Author: youngchany
 * @Date: 2020-06-03 20:35:41
 * @LastEditors: youngchany
 * @LastEditTime: 2020-06-03 20:56:35
 */ 
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/lock.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
	threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_request = 10000);
	~threadpool();
	bool append(T *request, int state);
	bool append_p(T *request);

private:
	static void *worker(void *arg);
	void run();

private:
	int m_thread_number;
	int m_max_requests;

	pthread_t *m_threads;
	std::list<T *> m_workqueue;
	locker m_queuelocker;
	sem m_queuestat;
	connection_pool *m_connPool;
	int m_actor_model;
};
template <typename T>
thread_pool<T>::threadpool(int actor_model, connection_pool *connPool, int thread_number, int max_requests) : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL), m_connPool(connPool)
{
	if (thread_number <= 0 || max_requests <= 0)
		throw std::exception();

	m_threads = new pthread_t[m_thread_number];
	if (!m_threads)
		throw std::exception();
	for (int i = 0; i < thread_number; ++i)
	{
		if (pthread_create(m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		if (pthread_detach(m_threads[i]))
		{
			delete[] m_threads;
			throw std::exception();
		}
	}
}

