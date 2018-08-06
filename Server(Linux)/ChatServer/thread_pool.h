#pragma once

#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <atomic>
#include <functional>

#include"util.h"

namespace gochat {

	template <typename T>
	class SyncQueue
	{
	public:
		SyncQueue(int maxSize) : max_size_(maxSize), need_stop(false) { }
		//����¼�
		void Put(const T& x);
		//����¼�
		void Put(T && x);
		//�Ӷ�����ȡ�¼�,ȡ�����¼�
		void Take(std::list<T> &list);
		//ȡһ���¼�
		void Take(T &t);
		//��ֹͬ������
		void Stop();
		//����Ϊ��
		bool Empty();
		//����Ϊ��
		bool Full();
		//���д�С
		size_t Size();
		//���д�С
		int Count();
	private:
		//���в�Ϊ��
		bool NotFull() const;
		//���в�Ϊ��
		bool NotEmpty() const;

		void Add(T && x);
		//������
		std::list<T> queue_;
		//������
		std::mutex mutex_;
		//���в�Ϊ�յ���������
		std::condition_variable not_empty_;
		//���в�Ϊ������������
		std::condition_variable not_full_;
		//���������󳤶�
		int max_size_;
		//��ֹ�ı�ʶ,��Ϊtrueʱ����ͬ������Ҫ��ֹ
		bool need_stop;

	};


	class ThreadPool
	{
	public:
		using Task = std::function<void()>;

		ThreadPool(int numThreads = std::thread::hardware_concurrency()) :queue_(kMaxTaskCount) {
			Start(numThreads);
		}

		~ThreadPool();

		//��֤���̻߳�����ֻ����һ��StopThreadGroup����
		void Stop();

		//�������,��ֵ����ת��
		void AddTask(Task && task);

	private:
		//����numThreads�������߳���
		void Start(int numThreads);
		//ȡ����������е�ȫ��,����ִ��
		void RunInThread();
		//��ֹ���������ִ��
		void StopThreadGroup();
		//����������߳���
		std::list<std::shared_ptr<std::thread>> thread_group;
		//ͬ������
		SyncQueue<Task> queue_;
		//���еı�־,flase������ֹ
		std::atomic_bool running_;
		//��֤�ں����ڶ��̻߳�����ֻ������һ��
		std::once_flag flag_;
	};

}

