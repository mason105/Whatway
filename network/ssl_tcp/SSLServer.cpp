#include "stdafx.h"

#include "network/ssl_tcp/SSLServer.h"



SSLServer::SSLServer(unsigned short port, queue_type& q, int n)
		:
		ios_pool_(*boost::factory<io_service_pool*>()(n))
		,queue_(q)
		,acceptor_(ios_pool_.get(),boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		,context_(boost::asio::ssl::context::sslv23)
{

/*
context_.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::single_dh_use);
    context_.set_password_callback(boost::bind(&server::get_password, this));
    context_.use_certificate_chain_file("server.pem");
    context_.use_private_key_file("server.pem", boost::asio::ssl::context::pem);
    context_.use_tmp_dh_file("dh512.pem");
*/
	if (gConfigManager::instance().m_nAuth)
	{
		context_.set_verify_mode(boost::asio::ssl::context::verify_peer  | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
		context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.crt");
	}
		  
		  

	context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.crt", boost::asio::ssl::context::pem);
	context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\serverkey.pem", boost::asio::ssl::context::pem);

	start_accept();
}

SSLServer::SSLServer(io_service_pool& ios, unsigned short port, queue_type& q)
		:ios_pool_(ios)
		,queue_(q)
		,acceptor_(ios_pool_.get(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		,context_(boost::asio::ssl::context::sslv23)
{
	if (gConfigManager::instance().m_nAuth)
	{
		context_.set_verify_mode(boost::asio::ssl::context::verify_peer  | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
		context_.load_verify_file(gConfigManager::instance().m_sPath  + "\\ca.crt");
	}

	context_.use_certificate_file(gConfigManager::instance().m_sPath + "\\server.crt", boost::asio::ssl::context::pem);
	context_.use_private_key_file(gConfigManager::instance().m_sPath + "\\serverkey.pem", boost::asio::ssl::context::pem);

	start_accept();
}

bool SSLServer::verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)
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

void SSLServer::start()
{
	ios_pool_.start();
}


void SSLServer::stop()
{
	ios_pool_.stop();
}


void SSLServer::run()
{
	ios_pool_.run();
}

void SSLServer::accept_handler(const boost::system::error_code& error, SSLSession* session)
{
	

	if (!error)
	{
		session->start();
		
	}
	else
	{
		delete session;
	}

	start_accept();
}

void SSLServer::start_accept()
{
	SSLSession * session = new SSLSession(ios_pool_.get(), queue_, m_msgType, context_);

	acceptor_.async_accept(session->socket(), 
		boost::bind(&SSLServer::accept_handler, this, boost::asio::placeholders::error, session));
}

std::string SSLServer::get_password()
{
	return "chenhf2011";
}

void SSLServer::SetMsgType(int msgType)
{
	m_msgType = msgType;
}
