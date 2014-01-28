#ifndef _IO_SERVICE_POOL_
#define _IO_SERVICE_POOL_

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/functional/factory.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// define BOOST_ALL_NO_LIB
#include <boost/asio.hpp>
#include <boost/thread.hpp>

/*
实现可以参考
http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp03/http/server2/io_service_pool.hpp
http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/example/cpp03/http/server2/io_service_pool.cpp
*/

class io_service_pool : boost::noncopyable
{
public:
	typedef boost::asio::io_service ios_type;
	typedef boost::asio::io_service::work work_type;
	typedef boost::ptr_vector<ios_type> io_services_type;
	typedef boost::ptr_vector<work_type> works_type;

private:
	io_services_type io_services_;
	works_type works_;

	boost::thread_group threads_;
	std::list<boost::thread*> threads;
	std::size_t next_io_service_;

public:
	//n应该设置为cpu核心数
	explicit io_service_pool(int n=4):
		next_io_service_(0)
	{
		BOOST_ASSERT(n > 0);
		init(n);
	}

private:
	void init(int n)
	{
		for (int i=0; i<n; ++i)
		{
			io_services_.push_back(boost::factory<ios_type*>()());

			works_.push_back( boost::factory<work_type*>()(io_services_.back()) );
		}
	}

public:
	ios_type& get()
	{
		if (next_io_service_ >= io_services_.size())
			next_io_service_ = 0;

		return io_services_[next_io_service_++];
	}

public:
	void start()
	{
		if (threads_.size() > 0)
			return;

		BOOST_FOREACH(ios_type& ios, io_services_)
		{
			boost::thread * thread = threads_.create_thread(boost::bind(&ios_type::run, boost::ref(ios)));
			threads.push_back(thread);
		}
	}

	void run()
	{
		start();
		threads_.join_all();
	}

	void stop()
	{
		works_.clear();

		std::for_each(io_services_.begin(), io_services_.end(), boost::bind(&ios_type::stop, _1));

		/*
		for(std::list<boost::thread*>::iterator it=threads.begin(),end=threads.end();
                it!=end;
                ++it)
		{
			threads_.remove_thread(*it);

			if (*it != NULL)
				delete *it;
		}
		*/
	}
};
#endif
