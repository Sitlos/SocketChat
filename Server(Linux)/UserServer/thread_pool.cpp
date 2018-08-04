#include "thread_pool.h"

namespace gochat {

	template<typename T>
	void gochat::SyncQueue<T>::Put(const T & x){
		Add(x);
	}

	template<typename T>
	void gochat::SyncQueue<T>::Put(T && x){
		//�����ڲ��ӿ�,��������ת��
		Add(std::forward<T>(x));
	}

	template<typename T>
	void gochat::SyncQueue<T>::Take(std::list<T>& list){
		std::unique_lock<std::mutex> locker(mutex_);
		//���������κ�һ����ȴ�,������m_needStopΪtrue����Ϊ����Ҫ��ֹ�����Բ�����
		not_empty_.wait(locker, [this] {return (need_stop || NotEmpty()); });
		if (need_stop)
		{
			return;
		}

		list = std::move(queue_);
		not_full_.notify_one();
	}

	template<typename T>
	void gochat::SyncQueue<T>::Take(T & t){
		std::unique_lock<std::mutex> locker(mutex_);
		not_empty_.wait(locker, [this] {return need_stop || NotEmpty(); });
		if (need_stop)
		{
			return;
		}

		t = queue_.front();
		queue_.pop_front();
		not_full_.notify_one();
	}

	template<typename T>
	void gochat::SyncQueue<T>::Stop(){
		{
			//�������������Դ�������
			std::lock_guard<std::mutex> locker(mutex_);
			//����ֹ��־��Ϊtrue
			need_stop = true;
		}

		//�������н���һһ��ֹ
		not_full_.notify_all();
		not_empty_.notify_all();
	}

	template<typename T>
	bool gochat::SyncQueue<T>::Empty(){
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.empty();
	}

	template<typename T>
	bool gochat::SyncQueue<T>::Full(){
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size() == max_size_;
	}

	template<typename T>
	size_t gochat::SyncQueue<T>::Size(){
		std::lock_guard<std::mutex> locker(mutex_);
		return queue_.size();
	}

	template<typename T>
	int gochat::SyncQueue<T>::Count(){
		return queue_.size();
	}

	template<typename T>
	bool gochat::SyncQueue<T>::NotFull() const{
		bool full = (queue_.size() >= max_size_);
		return !full;
	}

	template<typename T>
	bool gochat::SyncQueue<T>::NotEmpty() const{
		bool empty = queue_.empty();
		return !empty;
	}

	template<typename T>
	void gochat::SyncQueue<T>::Add(T && x){
		std::unique_lock<std::mutex> locker(mutex_);
		//���������κ�һ����ȴ�,������m_needStopΪtrue����Ϊ����Ҫ��ֹ�����Բ�����
		not_full_.wait(locker, [this] {return need_stop || NotFull(); });
		if (need_stop)
		{
			return;
		}

		queue_.push_back(std::forward<T>(x));
		not_empty_.notify_one();
	}

	ThreadPool::~ThreadPool(){
		Stop();
	}

	void ThreadPool::Stop(){
		std::call_once(flag_, [this] { StopThreadGroup(); });
	}

	void ThreadPool::AddTask(Task && task){
		queue_.Put(std::forward<Task>(task));
	}

	void ThreadPool::Start(int numThreads){
		running_ = true;

		for (int i = 0; i < numThreads; i++)
		{
			//����߳����εĴ���
			thread_group.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
		}
	}

	void ThreadPool::RunInThread(){
		while (running_)
		{
			std::list<Task> list;
			queue_.Take(list);

			for (auto & task : list)
			{
				if (!running_)
				{
					return;
				}

				//ִ������
				task();
			}
		}
	}

	void ThreadPool::StopThreadGroup(){
		//��ֹͬ������
		queue_.Stop();
		running_ = false;
		for (auto thread : thread_group)
		{
			if (thread)
			{
				thread->join();
			}
		}
		thread_group.clear();
	}

}