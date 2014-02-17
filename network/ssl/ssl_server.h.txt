#ifndef _SSL_SERVER_
#define _SSL_SERVER_


#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/functional/factory.hpp>
#include <boost/asio/ssl.hpp>

#include "./network/ssl/ssl_session.h"
#include "./network/io_service_pool.h"

#include "./config/ConfigManager.h"

class ssl_server
{
public:
	typedef ssl_session::ios_type ios_type;
	typedef boost::asio::ip::tcp::acceptor acceptor_type;
	//typedef boost::asio::ip::tcp tcp_type;
	typedef ssl_session::queue_type queue_type;

private:
	io_service_pool& ios_pool_;
	acceptor_type acceptor_;
	queue_type& queue_;
	
	boost::asio::ssl::context context_;

	
public:

	ssl_server( unsigned short port, queue_type& q, int n=4)
		:
		ios_pool_(*boost::factory<io_service_pool*>()(n))
		,queue_(q)
		,acceptor_(ios_pool_.get(),boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		,context_(boost::asio::ssl::context::sslv23)
	  {

		  if (gConfigManager::instance().m_nAuth)
		  {
			context_.set_verify_mode(boost::asio::ssl::context::verify_peer  | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
			//context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.crt");
			context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.cer");
		  }
		  
		  
		  
//		context_.set_options(
//				boost::asio::ssl::context::default_workarounds
//				| boost::asio::ssl::context::no_sslv2
//				| boost::asio::ssl::context::verify_peer);

		//context_.set_verify_callback(boost::bind(&ssl_server::verify_certificate, this, _1, _2));

			//context_.set_password_callback(boost::bind(&ssl_server::get_password, this));

			//context_.use_certificate_chain_file("d:\\product\\ssl\\server.crt");
			
			//context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.crt", boost::asio::ssl::context::pem);
			//context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\serverkey.pem", boost::asio::ssl::context::pem);
		  context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.cer", boost::asio::ssl::context::pem);
		context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\server.key.pem", boost::asio::ssl::context::pem);

		  start_accept();
	  }

	ssl_server(io_service_pool& ios, unsigned short port, queue_type& q)
		:ios_pool_(ios)
		,queue_(q)
		,acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		,context_(boost::asio::ssl::context::sslv23)
		{
		  if (gConfigManager::instance().m_nAuth)
		  {
			context_.set_verify_mode(boost::asio::ssl::context::verify_peer  | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
			//context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.crt");
			context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.cer");
		  }

			//context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.crt", boost::asio::ssl::context::pem);
			//context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\serverkey.pem", boost::asio::ssl::context::pem);
		  context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.cer", boost::asio::ssl::context::pem);
			context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\server.key.pem", boost::asio::ssl::context::pem);

		  start_accept();
	}

	bool verify_certificate(bool preverified,      boost::asio::ssl::verify_context& ctx)
	{
		// The verify callback can be used to check whether the certificate that is
		// being presented is valid for the peer. For example, RFC 2818 describes
		// the steps involved in doing this for HTTPS. Consult the OpenSSL
		// documentation for more details. Note that the callback is called once
		// for each certificate in the certificate chain, starting from the root
		// certificate authority.

		// In this example we will simply print the certificate's subject name.
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		OutputDebugString("Verifying " );
		OutputDebugString(subject_name);
		OutputDebugString("\n");

		return preverified;
	}



	std::string get_password() const
	{
		return "cssweb";
	}

private:
	void start_accept()
	{
		
		ssl_session_ptr session = boost::factory<ssl_session_ptr>()(ios_pool_.get(), queue_, context_);
		 //ssl_session_ptr session (new ssl_session(ios_pool_.get(), queue_, context_));
		 

		acceptor_.async_accept(session->socket(), 
			boost::bind(&ssl_server::accept_handler, this, boost::asio::placeholders::error, session));
	}

	void accept_handler(const boost::system::error_code& error, ssl_session_ptr session)
	{
		start_accept();

		if (error)
		{
			session->close();
			return;
		}

		session->start();
	}
public:
	void start()
	{
		ios_pool_.start();
	}

	void run()
	{
		ios_pool_.run();
	}

	void stop()
	{
		ios_pool_.stop();
	}
};

#endif
